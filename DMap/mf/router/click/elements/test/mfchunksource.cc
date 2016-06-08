/*
 *  Copyright (c) 2010-2013, Rutgers, The State University of New Jersey
 *  All rights reserved.
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of the organization(s) stated above nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*****************************************************************
 * MF_ChunkSource.cc
 * 
 * Creates and outputs chunks with specified properties. Properties 
 * include:
 * chunk size 
 * SID (service ID)
 * source GUID
 * dest GUID
 * inter-chunk interval
 * number of chunks, -1 for infinite
 * L2 pkt size for parameter eval
 * delay - in starting transmission
 *
 * The data block is constructed in the format expected by other elements
 * in the router pipeline, viz. a chain of MTU sized packets, with the 
 * L3, L4... headers present in the first packet. 
 *
 * Checks if downstream has capacity before pushing out a chunk, and
 * listens to downstream signals of available capacity to schedule the
 * next chunk out
 *
 *****************************************************************/

#include <click/config.h>
#include <click/args.hh>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/ether.h>
#include <click/vector.hh>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include "mfchunksource.hh"


CLICK_DECLS

MF_ChunkSource::MF_ChunkSource() 
    : _task(this), _timer(&_task), _active(true), _start_msg_sent(false), 
     _curr_chk_ID(1), _curr_chk_cnt(0), _service_id(MF_ServiceID::SID_UNICAST),
     _pkt_size(DEFAULT_PKT_SIZE), _chk_intval_msec(0), _delay_sec(10), 
     _chk_size_variation(0), _chk_size_list_str(), 
     _dst_NAs_str(), logger(MF_Logger::init()) {
}

MF_ChunkSource::~MF_ChunkSource() {

}

int MF_ChunkSource::configure(Vector<String> &conf, ErrorHandler *errh) {

  if (cp_va_kparse(conf, this, errh,
                   "SRC_GUID", cpkP+cpkM, cpUnsigned, &_src_GUID,
                   "DST_GUID", cpkP+cpkM, cpUnsigned, &_dst_GUID,
                   "LIMIT", cpkP+cpkM, cpInteger, &_chk_lmt,
                   "ROUTER_STATS", cpkP+cpkM, cpElement, &_routerStats,
                   "CHUNK_MANAGER", cpkP+cpkM, cpElement, &_chunkManager,
                   "SIZE", cpkP, cpString, &_chk_size_list_str,
                   "VARIATION", cpkP, cpUnsigned, &_chk_size_variation,
                   "SERVICE_ID", cpkP, cpUnsigned, &_service_id,
                   "PKT_SIZE", cpkP, cpUnsigned, &_pkt_size,
                   "INTERVAL", cpkP, cpUnsigned, &_chk_intval_msec,
                   "DST_NA", cpkP, cpString, &_dst_NAs_str,
                   "DELAY", cpkP, cpUnsigned, &_delay_sec,
                   cpEnd) < 0) {
    return -1;
  }
  return 0; 
}

int MF_ChunkSource::initialize(ErrorHandler * errh) {
  //check dst na config
  int ret = parseDstNAs();
  if (ret < 0) {
    logger.log(MF_WARN, "chnk_src: Dst NAs are partially parsed! %u of \"%s\"",
               _dst_NAs.size(), _dst_NAs_str.c_str());
  } else if (ret == 0) {
    logger.log(MF_DEBUG, "chnk_src: NO Dst NAs are set"); 
  } else {
    logger.log(MF_INFO, "chnk_src: All NAs of \"%s\" are parsed", 
               _dst_NAs_str.c_str());
  }
  
  for (uint32_t i = 0; i != _dst_NAs.size(); ++i) {
    logger.log(MF_INFO, "chnk_src: NA#%u: %u", i, _dst_NAs.at(i).to_int() );
  }
  
  //check check size config
  ret = parseChunkSizeList(); 
  if (ret < 0) {
    logger.log(MF_WARN, 
               "chnk_src: chunk size list are partially parsed! %u of \"%s\"", 
               _chk_size_list.size(), _chk_size_list_str.c_str()); 
  }
  if (_chk_size_list.size() > 1) {
    if (_chk_size_variation != 0) {
      logger.log(MF_WARN, "chnk_src: ignore chunk size variation: %u", 
               _chk_size_variation); 
      _chk_size_variation = 0;
    }
  } else {     //_chk_size_list.size() is 0 or 1;
    int _chk_size_base = getChunkSize();
    if (_chk_size_variation > _chk_size_base) {
      logger.log(MF_WARN, "chnk_src: variation of chunk size: %u is too large, "
                 "set it to chunk size: %u", 
                 _chk_size_variation, _chk_size_base);
      _chk_size_variation = _chk_size_base;
    }
    logger.log(MF_INFO, "chnk_src: Sending chunks of size: %u, variation: %u", 
                        _chk_size_base, _chk_size_variation);
  }

  logger.log(MF_INFO, "chnk_src: MTU: %u, count %u, interval: %u msecs", 
             _pkt_size, _chk_lmt, _chk_intval_msec);
  
  logger.log(MF_INFO, "chnk_src: service ID: %x", _service_id); 
        
  //init task, but use timer to schedule after delay
  ScheduleInfo::initialize_task(this, &_task, false, errh);
  _timer.initialize(this);
  /* listen for downstream capacity available notifications */
  _signal = Notifier::downstream_full_signal(this, 0, &_task);

  _timer.schedule_after_msec(_delay_sec * 1000);
  logger.log(MF_INFO, 
             "chnk_src: Transmission begins in: %u sec", _delay_sec);
  srand(time(NULL));   //seed;
  return 0;
}

bool MF_ChunkSource::run_task(Task *) {
  
  if (!_active) {
    return false;    //don't reschedule 
  }
  
  //check if downstream is full
  if (!_signal) {
    return false; //don't reschedule wait for notification
  }
  
  //if sender is busy, delay reschedule
  if (_routerStats->isBusy()) {
    uint32_t new_intval = 0; 
    if (_chk_intval_msec == 0) {
      new_intval = 10; 
    } else {
      new_intval = 2 * _chk_intval_msec; 
    }
    logger.log(MF_DEBUG,
               "chnk_src: Busy! inter-chunk delay, reschedule after %u msecs",
                new_intval);
    _timer.reschedule_after_msec(new_intval);
    return false;
  }

  //send start timer msg to receiver for measuring bitrate
  if (!_start_msg_sent) {
    sendStartMeasurementMsg();
    _start_msg_sent = true;
    _task.reschedule(); 
    return true;
  }
    
  uint32_t pkt_cnt = 0;
  uint32_t curr_chk_size = 0;
  char *payload;
  routing_header *rheader = NULL;
  transport_header *theader = NULL;
  Chunk *chunk = NULL;
  Vector<Packet*> *chk_pkts = NULL; 

  // prepare a convenience Chunk pkt for Click pipeline
  WritablePacket *chk_pkt = Packet::make(0, 0, sizeof(chk_internal_trans_t), 0); 
  if (!chk_pkt) {
    logger.log(MF_FATAL, "chnk_src: can't make chunk packet!");
    exit(EXIT_FAILURE);
  }
  memset(chk_pkt->data(), 0, chk_pkt->length());
  
  //calculate chunk size from configuration
  int _chk_size = getChunkSize(); 
  
  WritablePacket *pkt = NULL;                                                    //pkts in chunk
  while (curr_chk_size < _chk_size) {
    
    if (pkt_cnt == 0) {                                                          //if first pkt
      pkt = Packet::make(sizeof(click_ether), 0, _pkt_size, 0);                  //allocate memory for first pkt
      if (!pkt) {
        logger.log(MF_FATAL, "chnk_src: can't make packet!");
        exit(EXIT_FAILURE);
      }
      memset(pkt->data(), 0, pkt->length());
    
      // fill HOP header
      hop_data_t * pheader = (hop_data_t *)(pkt->data());
      pheader->type = htonl(DATA_PKT);
      pheader->seq_num = htonl(pkt_cnt);                                        //starts at 0
      pheader->hop_ID = htonl(_curr_chk_ID);

      //first pkt of chunk -- add L3, L4 headers, followed by data
      //L3  header
      rheader = new RoutingHeader(pkt->data() + HOP_DATA_PKT_SIZE);             //routing header
      rheader->setVersion(1);                                                   //set version
      switch (_service_id) {                                                    //set service id
       case MF_ServiceID::SID_UNICAST:
        rheader->setServiceID(MF_ServiceID::SID_UNICAST);                                         
        break;
       case MF_ServiceID::SID_ANYCAST:                                          //not implemented
        break;
       case MF_ServiceID::SID_MULTICAST:                                        //not implemented
        break; 
       case MF_ServiceID::SID_MULTIHOMING:                                      //multihoming needs extension header
        rheader->setServiceID(MF_ServiceID::SID_MULTIHOMING | 
                              MF_ServiceID::SID_EXTHEADER);
        logger.log(MF_DEBUG, 
                   "chnk_src: set SID_MULTIHOMING and SID_EXTHEADER"); 
        break;
       default:
        logger.log(MF_ERROR, 
                   "chnk_src: unsupported Service ID, set to Unicast");
        rheader->setServiceID(MF_ServiceID::SID_UNICAST); 
        break; 
      }
    
      //use transport protocal
      rheader->setUpperProtocol(TRANSPORT); 
      rheader->setDstGUID(_dst_GUID);
      //if dst NAs are defined in config file
      if (_dst_NAs.size() != 0) {
        //use the first one in routing header
        rheader->setDstNA(_dst_NAs.at(0));
        logger.log(MF_DEBUG, "chnk_src: set routing header dst NA: %u", 
                   _dst_NAs.at(0).to_int()); 
      }
      rheader->setSrcGUID(_src_GUID);
      rheader->setSrcNA(0);
      
      //extension header
      uint32_t extensionHeaderSize = 0;
      //TODO: support multiple extension headers
      if (rheader->hasExtensionHeader()) {                                      //if routing header needs extension header
        if (rheader->getServiceID().isMultiHoming()) {                          //if multihoming
          MultiHomingExtHdr *extHeader;
          //if dst NAs are defined in the config file
          if (_dst_NAs.size() != 0) {
            //use defined NAs 
            extHeader = new MultiHomingExtHdr(pkt->data() + 
                HOP_DATA_PKT_SIZE + ROUTING_HEADER_SIZE, 0, _dst_NAs.size());
            for (uint32_t i = 0; i != _dst_NAs.size(); ++i) {
              extHeader->setDstNA(_dst_NAs.at(i), i); 
            }
            extensionHeaderSize = extHeader->size();
            char buf[512];
            logger.log(MF_DEBUG, "chnk_src: multihoming extension header: %s",
                       extHeader->to_log(buf));
          } else {  
            //if no dst NAs are defined in config file
            //reserve EXTHDR_MULTIHOME_MAX_SIZE
            logger.log(MF_DEBUG, "chnk_src: missing NAs, %u bytes are reserved " 
                       "for multihoming extension header",
                      EXTHDR_MULTIHOME_MAX_SIZE);
            extensionHeaderSize = EXTHDR_MULTIHOME_MAX_SIZE;
          } 
        } else {
          logger.log(MF_WARN, "chnk_src: not implemented"); 
        }
      }

      //calculate where the transport header starts
      uint32_t payloadOffset = ROUTING_HEADER_SIZE + extensionHeaderSize; 
      rheader->setPayloadOffset(payloadOffset);
      //paylaod for routing header
      rheader->setChunkPayloadSize(_chk_size + TRANS_HEADER_SIZE);
      char buf[512];
      logger.log(MF_DEBUG,
                 "routing header info: %s", rheader->to_log(buf));
      chk_pkts = new Vector<Packet*> ();              
      
      //fill L4  header, starts from payloadOffset
      theader = (transport_header*) (pkt->data() + HOP_DATA_PKT_SIZE + 
                    payloadOffset);
      /*
      theader->chk_ID = htonl(_curr_chk_ID);
      theader->chk_size = htonl(_chk_size);	
      theader->src_app_ID = 0;
      theader->dst_app_ID = 0;
      theader->reliability = 0;
      */
      theader->chunkID = htonl(_curr_chk_ID);
      theader->chunkSize = htonl(_chk_size);
      theader->srcTID = 0;
      theader->dstTID = 0;
      theader->msgID = 0;
      theader->msgNum = 0;
      theader->offset = 0; 
      
      curr_chk_size += _pkt_size - (sizeof(click_ether) + payloadOffset);
      pheader->pld_size = htonl(_pkt_size - (sizeof(click_ether) 
                + HOP_DATA_PKT_SIZE));
    } else {
      uint32_t remaining_bytes = _chk_size - curr_chk_size;
      uint32_t bytes_to_send = _pkt_size;
      
      // if remaining data size is less than data pkt payload, calculate byte_to_send
      // to avoid empty bytes in last pkt 
      if (remaining_bytes < (_pkt_size - sizeof(click_ether) - 
              HOP_DATA_PKT_SIZE)) {
         bytes_to_send = remaining_bytes + sizeof(click_ether) + 
              HOP_DATA_PKT_SIZE;
      }

      uint32_t pkt_payload = bytes_to_send - sizeof(click_ether) -
                             HOP_DATA_PKT_SIZE;
      
      pkt = Packet::make(sizeof(click_ether), 0, bytes_to_send, 0);
      if (!pkt) {
          logger.log(MF_FATAL, "chnk_src: can't make packet!");
          exit(EXIT_FAILURE);
      }
      memset(pkt->data(), 0, pkt->length());
      
      // fill HOP header
      hop_data_t * pheader = (hop_data_t *)(pkt->data());
      pheader->type = htonl(DATA_PKT);
      pheader->seq_num = htonl(pkt_cnt);                                         //at least 1;
      pheader->hop_ID = htonl(_curr_chk_ID);
      pheader->pld_size = htonl(pkt_payload);
      curr_chk_size += pkt_payload;                                             //curr_chk_size == chk_size, loop ends
    }
    chk_pkts->push_back(pkt); 
    ++pkt_cnt;
    logger.log(MF_TRACE, 
               "chnk_src: Created pkt #: %u for chunk #: %u, curr size: %u", 
               pkt_cnt, _curr_chk_ID, curr_chk_size);
    pkt = NULL; 
  }
  //allocate chunk in chunk manager
  chunk = _chunkManager->alloc(chk_pkts, 
                     ChunkStatus::ST_INITIALIZED||ChunkStatus::ST_COMPLETE); 
  
  //fill internal transfer msg
  chk_internal_trans_t *chk_trans = ( chk_internal_trans_t*) chk_pkt->data(); 
  chk_trans->sid = htonl(chunk->getServiceID().to_int());
  chk_trans->chunk_id = chunk->getChunkID(); 
    
  output(0).push(chk_pkt);

  _curr_chk_cnt++;
  logger.log(MF_INFO, 
             "chnk_src: PUSHED ready chunk #: %u, chunk_id: %u size: %u, "
             "num_pkts: %u", 
              _curr_chk_ID, chunk->getChunkID(), _chk_size, pkt_cnt);
  _curr_chk_ID++;

  if (_chk_lmt > 0 && _curr_chk_cnt >= _chk_lmt) {
    _active = false; //done, no more
    return false;
  }

  // if valid interval was specified, schedule after duration
  if (_chk_intval_msec > 0) {
    logger.log(MF_DEBUG, 
               "chnk_src: inter-chunk delay, cheduling after %u msecs", 
               _chk_intval_msec);
    _timer.reschedule_after_msec(_chk_intval_msec);
    return false;
  }

  _task.fast_reschedule();
  return true;
}

//
bool MF_ChunkSource::sendStartMeasurementMsg() {
  uint32_t pkt_size = HOP_DATA_PKT_SIZE + ROUTING_HEADER_SIZE + 
                      sizeof(start_measurement_msg_t); 
  WritablePacket *start_pkt = Packet::make(sizeof(click_ether), 0, pkt_size, 0);
  if (!start_pkt) {
    logger.log(MF_FATAL, "chnk_src: can't make packet!");
    exit(EXIT_FAILURE);
  }
  memset(start_pkt->data(), 0, start_pkt->length());
  
  uint32_t curr_chk_size = 0;

  //HOP header
  hop_data_t * pheader = (hop_data_t *)(start_pkt->data());
  pheader->type = htonl(DATA_PKT);
  pheader->seq_num = htonl(0);                //starts at 0
  pheader->hop_ID = htonl(_curr_chk_ID);      

  //routing header
  routing_header *rheader = 
      new RoutingHeader(start_pkt->data() + HOP_DATA_PKT_SIZE);
  rheader->setVersion(1);
  rheader->setServiceID(MF_ServiceID::SID_UNICAST);         //0-unicast
  rheader->setUpperProtocol(MEASUREMENT);                   //measurement pkt
  rheader->setDstGUID(_dst_GUID);
  if (_dst_NAs.size() > 0) {
    rheader->setDstNA(_dst_NAs.at(0));
  } else {
    rheader->setDstNA(0);
  }
  rheader->setSrcGUID(_src_GUID);
  rheader->setSrcNA(0);
  rheader->setPayloadOffset(ROUTING_HEADER_SIZE);
  rheader->setChunkPayloadSize(sizeof(start_measurement_msg_t)); 

  start_measurement_msg_t *msg = 
      (start_measurement_msg_t*)(rheader->getHeader() + 
      rheader->getPayloadOffset());
  msg->num_to_measure = htonl(_chk_lmt); 
  
  pheader->pld_size = htonl(pkt_size);

  Chunk *chunk = _chunkManager->alloc(start_pkt, 1, ChunkStatus::ST_COMPLETE); 
  
  WritablePacket *chk_pkt = Packet::make(sizeof(click_ether), 0, 
                                         sizeof(chk_internal_trans_t), 0);
  memset(chk_pkt->data(), 0, chk_pkt->length()); 
  chk_internal_trans_t *chk_trans = (chk_internal_trans_t*)chk_pkt->data();  
  chk_trans->sid = htonl(rheader->getServiceID().to_int());
  chk_trans->chunk_id = chunk->getChunkID();
  output(0).push(chk_pkt); 
  logger.log(MF_DEBUG,
             "chnk_src: PUSHED start measurement msg, chunk_id %u",
             chunk->getChunkID());
  _curr_chk_ID++; 
  return true; 
}

int MF_ChunkSource::getChunkSize() {
  if (_chk_size_list.size() == 0) {
    return DEFAULT_CHUNK_SIZE;
  } else if (_chk_size_list.size() == 1) {
    if (_chk_size_variation == 0) {
      return _chk_size_list.at(0);
    } else {
      return rand() % (_chk_size_variation * 2) + _chk_size_list.at(0) - 
             _chk_size_variation;
    }
  } else {
    return _chk_size_list.at(_curr_chk_cnt%_chk_size_list.size());
  }
}

int MF_ChunkSource::parseDstNAs() {
  //TODO+ tmp size should equal to MAX_NA_SIZE
  char tmp[256];
  uint32_t cnt = 0; 
  for (String::iterator it = _dst_NAs_str.begin(); it != _dst_NAs_str.end(); ++it) {
    if (*it >= '0' && *it <= '9') {
      tmp[cnt] = *it;
      ++cnt; 
    } else if ( *it == ':') {
      tmp[cnt] = '\0';
      NA na;
      na.init(atoi(tmp));
      _dst_NAs.push_back(na); 
      cnt = 0; 
    } else {
      return -1; 
    }
  }
  return _dst_NAs.size(); 
}

int MF_ChunkSource::parseChunkSizeList() {
  char tmp[256];
  uint32_t cnt = 0;
  String::iterator it = _chk_size_list_str.begin(); 
  while (it <= _chk_size_list_str.end()) {
    if ( it == _chk_size_list_str.end() || *it == '=') {
      tmp[cnt] = '\0';
      int size = atoi(tmp);
      if (size != 0) {
        _chk_size_list.push_back(size);
      }
      cnt = 0;
      if (it == _chk_size_list_str.end() || ++it  == _chk_size_list_str.end()) {
        break;
      } else {
        if (*it != '>') {
          return -1;
        }
      }
    } else if (*it >= '0' && *it <= '9') {
      tmp[cnt] = *it;
      ++cnt;
    } else {
      return -1;
    }
    ++it;
  }
  return _chk_size_list.size(); 
}

void MF_ChunkSource::add_handlers() {

    add_data_handlers("count", Handler::OP_READ, &_curr_chk_cnt);
    add_task_handlers(&_task, &_signal);

}

/*
#if EXPLICIT_TEMPLATE_INSTANCES
template class Vector<pkt*>;
template class Vector<int>;
#endif
*/
CLICK_ENDDECLS
EXPORT_ELEMENT(MF_ChunkSource)
ELEMENT_REQUIRES(userlevel)
