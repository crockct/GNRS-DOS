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
/*************************************************************************
 * MF_Segmentor.cc
 * Segmentor is the last element of MobilityFirst click router(except Queue
 * and ToDevice. It receives routing layer datagram from previous element 
 * within this router, sends out CSYN and data packets, and also receives ACK.
 * Created on: Jun 2, 2011
 *      Author: Kai Su
 *************************************************************************/
#include <fcntl.h>
#include <limits>

#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/args.hh>
#include <click/handlercall.hh>
#include <click/vector.hh>
#include <clicknet/ether.h>

#include "mfsegmentor.hh"


CLICK_DECLS

MF_Segmentor::MF_Segmentor() : _task(this), _cur_sending_win(0), _outstanding_chk_buf_lock(),
                               _cur_csyn_win(0), _sending_win_size(1500), _mono_hop_ID(0),
                               logger(MF_Logger::init()), _sent_chk_count(0) {
  srand(time(NULL));
  u_int rnum = rand();
  double ratio = double(rnum)/RAND_MAX;
  // comment out the following line to disable random starting hop ID
  _mono_hop_ID = std::numeric_limits<uint32_t>::max() * ratio;

  _outstanding_chk_buf = new OutstandingChunkBuffer();
  _packet_outQ = new PacketOutQ(); 
  _ready_chunkQ = new ReadyChunkQ();
  _waiting_chunk_cache = new WaitingChunkTable();
  _internal_msg_tbl = new DgramTable();

  pthread_mutex_init(&_hop_id_lock, NULL);
 
  pthread_mutex_init(&_sending_win_lock, NULL); 
  pthread_mutex_init(&_csyn_win_lock, NULL);

  //pthread_mutex_init(&_outstanding_chk_buf_lock, NULL);  
  pthread_mutex_init(&_ready_chunkQ_lock, NULL);
  pthread_mutex_init(&_waiting_chunk_cache_lock, NULL);
  pthread_mutex_init(&_packet_outQ_lock, NULL); 
  pthread_mutex_init(&_internal_msg_tbl_lock, NULL); 

  pthread_mutex_init(&_task_lock, NULL);

  int pfd[2];
  if (pipe(pfd) == -1) exit(1);
  if (fcntl(pfd[0], F_SETFL, O_NONBLOCK) == -1) exit(1);
  _rfd = pfd[0];
  _wfd = pfd[1];
}

MF_Segmentor::~MF_Segmentor() {
}

int MF_Segmentor::configure(Vector<String> &conf, ErrorHandler *errh){
  if(cp_va_kparse(conf, this, errh,
                  "ROUTER_STATS", cpkP+cpkM, cpElement, &_routerStats,
                  "CHUNK_MANAGER", cpkP+cpkM, cpElement, &_chunkManager,
                  "WINDOW_SIZE", cpkP, cpUnsigned, &_sending_win_size, 
                  cpEnd) < 0) {
    return -1;
  }
  return 0;
}

int MF_Segmentor::initialize(ErrorHandler *errh) {
  int tid = home_thread()->thread_id(); 
  logger.log(MF_INFO, "seg: thread_id: %u", tid);
  logger.log(MF_INFO, "seg: sending window size in pkt: %u", _sending_win_size); 
  ScheduleInfo::initialize_task(this, &_task, true, errh);

  //add_select(_rfd, SELECT_READ);
  return 0; 
}

inline void MF_Segmentor::push(int port, Packet *p) {
  if (port==0) {
    handleChkPkt(p);
  } else if (port==1) {
    handleACKPkt(p);
  } else {
    logger.log(MF_FATAL, "seg: Packet recvd on unsupported port");
    exit(-1);
  }
}

void MF_Segmentor::selected(int fd, int mask) {
  assert(fd == _rfd);
  char buf[20];
  int num = read(_rfd, buf, 20);
  assert(num > 0); // because 'selected()' is only called when data is ready.
  
  logger.log(MF_DEBUG, "seg: selected() called");

  // call run_task
}

bool MF_Segmentor::run_task(Task *) { 
  logger.log(MF_INFO, "seg: task with thread id: %u", home_thread()->thread_id());

  // move chunks to readyChunkQ
  pthread_mutex_lock(&_waiting_chunk_cache_lock); 
  checkWaitingChunkCache(); 
  pthread_mutex_unlock(&_waiting_chunk_cache_lock);

  pthread_mutex_lock(&_csyn_win_lock); 
  while (_cur_csyn_win < _sending_win_size) {
    //check if there are ready chunks
    pthread_mutex_lock(&_ready_chunkQ_lock);
    Chunk *ready_chunk = popReadyChunkQ();
    pthread_mutex_unlock(&_ready_chunkQ_lock);
    if (!ready_chunk) {
      //pthread_mutex_unlock(&_csyn_win_lock);
      logger.log(MF_DEBUG, "seg: no ready chunk in ready_chunkQ");
      break;
    } else {
      uint32_t num_pkts = ready_chunk->getChkPktCnt();
      _cur_csyn_win += num_pkts;
      if (num_pkts <= SMALL_CHUNK_LIMIT) {
        pthread_mutex_lock(&_sending_win_lock);
        _cur_sending_win += num_pkts;
        pthread_mutex_unlock(&_sending_win_lock);
        sendSmallChunk(ready_chunk);
      } else {
        sendChunk(ready_chunk); 
      }
    } 
  }
  pthread_mutex_unlock(&_csyn_win_lock); 
  
  //_task.reschedule();
  return true;
}


void MF_Segmentor::handleChkPkt(Packet *p){
  chk_internal_trans_t *chk_trans = (chk_internal_trans_t*)(p->data()); 
  uint32_t chunk_id = chk_trans->chunk_id;
  //got initialized chunk from chunk manager
  Chunk *chunk = _chunkManager->get(chunk_id, ChunkStatus::ST_INITIALIZED);
  if (!chunk) {
    logger.log(MF_ERROR,
               "seg: handleChkPkt: Cannot find initialized chunk id: %u in "
               "chunk manager", chunk_id);
    p->kill(); 
    return; 
  }
  //get next_hop_guid of current chunk
  uint32_t next_hop_GUID = p->anno_u32(NXTHOP_ANNO_OFFSET);
  chunk->setNextHopGUID(next_hop_GUID);
  
  logger.log(MF_DEBUG, "seg: Got chunk_id: %u, next_hop_guid: %u",
             chunk_id, next_hop_GUID);
  
  //store interanl msg for rebinding; delete after complete transmission 
  pthread_mutex_lock(&_internal_msg_tbl_lock); 
  _internal_msg_tbl->set(chunk_id, p);
  pthread_mutex_unlock(&_internal_msg_tbl_lock); 
  
  //if chunk is incomplete, push to waitingCache
  if (!chunk->getStatus().isComplete()) {
    //uint32_t num_pkts = chunk->getChkPktCnt();
    //insert chunk to waiting chunk table
    pthread_mutex_lock(&_waiting_chunk_cache_lock);
    int ret = _waiting_chunk_cache->set(chunk_id, chunk);
    pthread_mutex_unlock(&_waiting_chunk_cache_lock);
    if (ret) {
      logger.log(MF_DEBUG, 
                 "seg: Incomplete Chunk! chunk is pushed into waiting chunk "
                 "cache, chunk_id: %u.", chunk_id); 
    } else {
      logger.log(MF_ERROR, 
                 "seg: Incomplete Chunk! chunk exists in waiting chunk cache, "
                 "chunk id: %u", chunk_id); 
    }
    //update router status
    _routerStats->setNumWaitingChunk(_waiting_chunk_cache->size());
  } else {                             //chunk is not complete
    uint32_t num_pkts = chunk->getChkPktCnt();
    logger.log(MF_DEBUG, "seg: Complete Chunk! chunk_id: %u, num_pkts: %u ,"
               "next hop GUID: %u", chunk_id, num_pkts, next_hop_GUID);
    //pop all packets in Q
    popPacketOutQ();
    
    // this part is handled by run_task now. Should schedule run_task here.
    
    /*
    if (num_pkts <= SMALL_CHUNK_LIMIT) {
      pthread_mutex_lock(&_sending_win_lock);
      if (_cur_sending_win < _sending_win_size) {
        //update csyn window size
        pthread_mutex_lock(&_csyn_win_lock);
        _cur_csyn_win += num_pkts;
        pthread_mutex_unlock(&_csyn_win_lock); 
        //update sending window size
        _cur_sending_win += num_pkts;
        pthread_mutex_unlock(&_sending_win_lock);
        sendSmallChunk(chunk); 
        return; 
      } else {
        pthread_mutex_unlock(&_sending_win_lock);
      }
    }
    */
    
    //push to readyQ
    uint32_t ready_q_size = 0; 
    pthread_mutex_lock(&_ready_chunkQ_lock);
    pushReadyChunkQ(chunk);
    ready_q_size = _ready_chunkQ->size(); 
    pthread_mutex_unlock(&_ready_chunkQ_lock);
    logger.log(MF_DEBUG, "seg: Push chunk to ready_chunk_Q, chunk id: %u, "
               "queue size: %u", chunk_id, ready_q_size);

    pthread_mutex_lock(&_task_lock);
    if (!_task.scheduled())
      _task.reschedule();
    pthread_mutex_unlock(&_task_lock);
  }
}

void MF_Segmentor::handleACKPkt(Packet *p){
  //parse csyn ack
  csyn_ack_t *ack = (csyn_ack_t*)p->data();
  uint32_t hopID = ntohl(ack->hop_ID);
  uint32_t chk_pkt_count = ntohl(ack->chk_pkt_count);
  Vector<int> *lost = getLostSeq(ack->bitmap, chk_pkt_count);
  uint32_t lost_size = lost->size();
  uint32_t rcvd_size = chk_pkt_count - lost_size;
  
  _outstanding_chk_buf_lock.acquire(); 
  OutstandingChunkBuffer::iterator it = _outstanding_chk_buf->find(hopID);
  if (it == _outstanding_chk_buf->end()) {
    _outstanding_chk_buf_lock.release(); 
    logger.log(MF_ERROR, 
               "seg: Received CSYN_ACK for hop_id : %u, but it cannot be found "
               "in outstanding buffer. Discard!", hopID); 
    p->kill();
    return;
  } 

  Chunk *chunk = it.value()->chunk;
  if (!chunk) {
    //pthread_mutex_unlock(&_outstanding_chk_buf_lock);
    _outstanding_chk_buf_lock.release();
    logger.log(MF_ERROR, "seg: Received CSYN_ACK for hop_id : %u, but Got an "
              "invalid chunk from outstanding buffer! Discard", hopID);
    p->kill();
    return;
  }

  uint32_t chunk_id = chunk->getChunkID();
  
  //update outstanding chunk table info
  it.value()->csyn_acked = true;  
  uint32_t win_shrink_size = 0;
  
  //bool is_resend_chunk = false;
  int priority = it.value()->priority; 
  if (it.value()->buffer_granted == false) {
    it.value()->buffer_granted = true;
    if (it.value()->small) {
      //pthread_mutex_unlock(&_outstanding_chk_buf_lock);
      _outstanding_chk_buf_lock.release();
      logger.log(MF_DEBUG,
                 "seg: Received a ack for a small chunk, hop_id: %u, chunk_id: %u",
                 hopID, chunk_id);
      p->kill();
      return;
    } 
  } else {     //if buffer is already granted
    win_shrink_size = chk_pkt_count - it.value()->rcvd_num_pkts;
    if (lost_size != 0) {
      //is_resend_chunk = true;
      priority = 1;
      it.value()->priority = priority; 
    }
  }

  //if timer is not scheduled
  if (!it.value()->csyn_timer->scheduled()) {
    _outstanding_chk_buf_lock.release();
    //pthread_mutex_unlock(&_outstanding_chk_buf_lock);
    logger.log(MF_WARN,
               "seg: CSYN timer expired, received a timeout CSYN-ACK, hop id: %u, "
               "chunk_id: %u", hopID, chunk_id);
    p->kill();
    return;
  }
  
  //unschedule the timer if scheduled
  it.value()->csyn_timer->unschedule();
  logger.log(MF_DEBUG,
             "seg: CSYN timer is alive, unschedule it. hop id: %u, chunk_id %u",
             hopID, chunk_id);
  
  uint32_t new_acked_num_pkts = rcvd_size - it.value()->rcvd_num_pkts;
  it.value()->rcvd_num_pkts = rcvd_size;
  
  _outstanding_chk_buf_lock.release();
  //pthread_mutex_unlock(&_outstanding_chk_buf_lock);
  
  //uint32_t next_hop_GUID = chunk->getNextHopGUID().to_int();
  //TEMP for transport experiment
  if (rcvd_size != 0 && lost_size != 0) {
    uint32_t theader_offset = chunk->routingHeader()->getPayloadOffset();
    uint32_t theader_seq = TransHeader::getSeqStatic(chunk->routingHeader()->getHeader() + theader_offset);
    logger.log(MF_TIME, "seg: EVENT LOSS, hop_id: %u, transport_seq: %u",
               hopID, theader_seq);
  }
  
  if (lost_size) {
    logger.log(MF_INFO,
               "seg: Got CSYN-ACK for hopID: %u chunk_id: %u "
               "rcvd/**lost**/total: %u/**%u**/%u",
                hopID, chunk_id, rcvd_size, lost_size, chk_pkt_count);

  } else {       //lost size is 0, chunk transfer complete
    logger.log(MF_DEBUG,
               "seg: Got CSYN-ACK for hopID: %u chunk_id: %u "
               "rcvd/**lost**/total: %u/**%u**/%u",
               hopID, chunk_id, rcvd_size, lost_size, chk_pkt_count);
  }

  //shink window size and csyn window size
  pthread_mutex_lock(&_sending_win_lock);
  //_cur_sending_win -= new_acked_num_pkts;
  _cur_sending_win -= win_shrink_size; 
  int cur_sending_win = _cur_sending_win; 
  pthread_mutex_unlock(&_sending_win_lock);
  
  pthread_mutex_lock(&_csyn_win_lock);
  _cur_csyn_win -= new_acked_num_pkts;
  int cur_csyn_win = _cur_csyn_win; 
  pthread_mutex_unlock(&_csyn_win_lock); 
  
  logger.log(MF_DEBUG, 
             "seg: # of new acked packet: %u, current window size: %u, "
             "current csyn window size: %u",
             new_acked_num_pkts, cur_sending_win, cur_csyn_win);
  
  if (lost_size == 0) {    //chunk transmission completed
    _sent_chk_count++;
    if (!releaseOutstandingChunk(hopID)) {
      logger.log(MF_ERROR, "seg: fail to release chunk with hop_id: %u from "
                 "outstanding chunk buffer", hopID); 
    }
    //free memory
    _chunkManager->removeData(chunk);

    //delete internal chunk transfer msg
    pthread_mutex_lock(&_internal_msg_tbl_lock); 
    DgramTable::iterator internal_msg_it = _internal_msg_tbl->find(chunk_id);
    if (internal_msg_it != _internal_msg_tbl->end()) {
      internal_msg_it.value()->kill(); 
      _internal_msg_tbl->erase(internal_msg_it);
    }
    pthread_mutex_unlock(&_internal_msg_tbl_lock); 
    logger.log(MF_TIME, "seg: SENT chunk_id: %u, hop_id: %u", chunk_id, hopID); 
  }
  
  //pop as many packets from outQ as prossbile
  uint32_t data_pkt_cnt = popPacketOutQ();
  logger.log(MF_DEBUG, "seg: pop %u data packets from packet sendingQ",
             data_pkt_cnt); 
  sendPackets(hopID, chunk, lost, priority);  
  
  delete lost;
  p->kill();
  
  // this part is handled by run_task now.
  /*
  pthread_mutex_lock(&_sending_win_lock);
  if (_cur_sending_win < _sending_win_size) {  
    Chunk *chunk_to_send_csyn = NULL; 
    pthread_mutex_lock(&_csyn_win_lock); 
    //while of if (cur_csyn_win < _sending_win_size * 2 && lost_size != 0) {
    if ((_cur_csyn_win < _cur_sending_win * 2 && lost_size != 0) || 
           _cur_csyn_win < _sending_win_size) {
      //a pop method can be specified here, default is FIFO
      pthread_mutex_lock(&_ready_chunkQ_lock); 
      chunk_to_send_csyn = popReadyChunkQ();
      pthread_mutex_unlock(&_ready_chunkQ_lock);
      if (chunk_to_send_csyn != NULL) {
        //unlock cysn win after updating cur size
        int num_pkts = chunk_to_send_csyn->getChkPktCnt();
        _cur_csyn_win += num_pkts;
        pthread_mutex_unlock(&_csyn_win_lock);
        pthread_mutex_unlock(&_sending_win_lock);
        sendChunk(chunk_to_send_csyn); 
      } else {
        pthread_mutex_unlock(&_csyn_win_lock);
        pthread_mutex_unlock(&_sending_win_lock);
        logger.log(MF_DEBUG, "seg: cannot get valid chunk from ready chunk Q"); 
      }
    } else {
      pthread_mutex_unlock(&_csyn_win_lock);
      pthread_mutex_unlock(&_sending_win_lock);
      logger.log(MF_DEBUG, 
                 "seg: not going to send csyn because of csyn window is big enough"); 
    }
  } else {
    pthread_mutex_unlock(&_sending_win_lock);
    logger.log(MF_DEBUG, 
               "seg: not going to send csyn because packet out Q is overflow"); 
  }
  */

  pthread_mutex_lock(&_task_lock);
  if (!_task.scheduled())
    _task.reschedule();
  pthread_mutex_unlock(&_task_lock);

}


//
//
//
//
//
bool MF_Segmentor::startOutstandingCsynTimer(uint32_t hop_id, uint32_t msec) {
  bool ret = false;
  //pthread_mutex_lock(&_outstanding_chk_buf_lock); 
  _outstanding_chk_buf_lock.acquire();
  OutstandingChunkBuffer::iterator it = _outstanding_chk_buf->find(hop_id); 
  if (it != _outstanding_chk_buf->end()) {
    Chunk *chunk = it.value()->chunk;
    if (!chunk) {
      //pthread_mutex_unlock(&_outstanding_chk_buf_lock);
      _outstanding_chk_buf_lock.release();
      return ret; 
    }
    //timer is initialized when inserted to outstanding table; 
    if (it.value()->csyn_timer->initialized()) {
      it.value()->csyn_timer->schedule_after_msec(msec); 
      ret = true; 
    }
  }
  //pthread_mutex_unlock(&_outstanding_chk_buf_lock);
  _outstanding_chk_buf_lock.release();
  return ret; 
}

bool MF_Segmentor::stopOutstandingCsynTimer(uint32_t hop_id) {
  bool ret = false;
  //pthread_mutex_lock(&_outstanding_chk_buf_lock);
  _outstanding_chk_buf_lock.acquire();
  OutstandingChunkBuffer::iterator it = _outstanding_chk_buf->find(hop_id);
  if (it != _outstanding_chk_buf->end()) {
    it.value()->csyn_timer->unschedule();
    ret = true; 
  }
  //pthread_mutex_unlock(&_outstanding_chk_buf_lock);
  _outstanding_chk_buf_lock.release();
  return ret;
}

//release chunk from outstanding chunk buffer
//return 0 or 1; 
int MF_Segmentor::releaseOutstandingChunk(uint32_t hop_id) {
  int ret = 0; 
  //pthread_mutex_lock(&_outstanding_chk_buf_lock);
  _outstanding_chk_buf_lock.acquire();
  OutstandingChunkBuffer::iterator it = _outstanding_chk_buf->find(hop_id);
  if (it != _outstanding_chk_buf->end() && it.value()) {
    deleteOutstandingChunkStruct(it.value()); 
    _outstanding_chk_buf->erase(it);
    ret = 1;
  } 
  _outstanding_chk_buf_lock.release();
  //pthread_mutex_unlock(&_outstanding_chk_buf_lock);
  return ret; 
}

/** 
 * getLostSeq() takes as input the bitmap from ACK message 
 * and return the indices of
 * lost and received packets.
 **/
Vector<int>* MF_Segmentor::getLostSeq (char *bitmap, uint32_t chunksize) {
  Vector<int> *lost = new Vector<int>(); 
  for(unsigned i=0; i<chunksize; i++) {
    if ((bitmap[(i)/8] & (0x01 << ((i)%8)))==0) {
      lost->push_back(i);
    }
  }
  return lost;
}

//
void MF_Segmentor::resendCsyn(uint32_t hop_id, uint32_t chk_size) {
  //find outstanding chunk
  _outstanding_chk_buf_lock.acquire(); 
  OutstandingChunkBuffer::iterator it = _outstanding_chk_buf->find(hop_id);
  if (it == _outstanding_chk_buf->end() || !it.value()) {
    _outstanding_chk_buf_lock.release();
    logger.log(MF_ERROR,
               "seg: Cannot find valid outstanding chunk in buffer. hop_id: %u",
               hop_id);
    return;
  }
  Chunk *chunk = it.value()->chunk;
  //check whether chunk is valid, return if invalid;
  if (!chunk) {
    _outstanding_chk_buf_lock.release();
    logger.log(MF_ERROR,
               "seg: Got an invalid chunk in sending pool");
    return;
  }
  //Temp for transport experiment
  uint32_t theader_offset = chunk->routingHeader()->getPayloadOffset();
  uint32_t theader_seq = TransHeader::getSeqStatic(chunk->routingHeader()->getHeader() + theader_offset); 
  logger.log(MF_TIME, "seg: EVENT CSYN RESEND, hop_id: %u, transport_seq: %u", 
             hop_id, theader_seq); 
  //check
  if (chunk->isNATimeout() || chunk->isNextHopTimeout()) {
    logger.log(MF_DEBUG, "seg: next hop timeout");
    //Temp solution for push to net_binder
    int rcvd_num_pkts = it.value()->rcvd_num_pkts;
    int shrink_size = chunk->getChkPktCnt() - rcvd_num_pkts;
    
    pthread_mutex_lock(&_csyn_win_lock);
    _cur_csyn_win -= shrink_size;
    pthread_mutex_unlock(&_csyn_win_lock);
    
    if (it.value()->buffer_granted) {
      pthread_mutex_lock(&_sending_win_lock);
      _cur_sending_win -= shrink_size;
      pthread_mutex_unlock(&_sending_win_lock);
    }

    deleteOutstandingChunkStruct(it.value());
    //it.value()->csyn_timer->unschedule();

    _outstanding_chk_buf->erase(it);
    _outstanding_chk_buf_lock.release();
    pushToNetBinder(chunk->getChunkID());

    // should reschedule run_task
    pthread_mutex_lock(&_task_lock);
    if (!_task.scheduled())
      _task.reschedule();
    pthread_mutex_unlock(&_task_lock);
    
    return;
  }

  uint32_t next_hop_GUID = chunk->getNextHopGUID().to_int();
  uint32_t chunk_id = chunk->getChunkID(); 
  it.value()->priority = 1; 
  it.value()->csyn_timer->reschedule_after_msec(CSYN_TIMEOUT); 
  _outstanding_chk_buf_lock.release();

  Packet* csyn_pkt = makeCsynPacket(hop_id, chk_size, next_hop_GUID);
  if (!csyn_pkt) {
    //pthread_mutex_unlock(&_outstanding_chk_buf_lock);
    _outstanding_chk_buf_lock.release(); 
    logger.log(MF_CRITICAL, "seg: cannot make csyn packet");
    return;
  }

  output(SEG_PORT_OUT_PRIORITY_DATA).push(csyn_pkt);
  logger.log(MF_DEBUG,
             "seg: resent CSYN hop id: %u, chunk_id: %u, nexthop: %u, "
             "size: %u pkts with priority",
             hop_id, chunk_id, next_hop_GUID, chk_size);
}


void MF_Segmentor::pushToNetBinder(uint32_t chunk_id) {
  logger.log(MF_DEBUG, "seg: pushToNetBinder");
  pthread_mutex_lock(&_internal_msg_tbl_lock);
  DgramTable::iterator itt = _internal_msg_tbl->find(chunk_id);
  assert(itt != _internal_msg_tbl->end());
  Packet *internal_msg_pkt = itt.value();
  _internal_msg_tbl->erase(itt);
  pthread_mutex_unlock(&_internal_msg_tbl_lock);
  output(SEG_PORT_TO_NET_BINDER).push(internal_msg_pkt);
}

//get the next chunk to be sent.
Chunk* MF_Segmentor::popReadyChunkQ(uint32_t method) {
  Chunk *chunk_to_pop = NULL;
  if (_ready_chunkQ->size() == 0) {
    return chunk_to_pop;
  }
  if (method == READY_CHUNK_Q_SCHEDULE_FIFO) {
    chunk_to_pop = _ready_chunkQ->front();
    _ready_chunkQ->pop_front();
  }
  return chunk_to_pop; 
}

//check if there are complete chunks in waiting cache, 
//if yes, push to ReadyChunkQ
//if no, do nothing
void MF_Segmentor::checkWaitingChunkCache() {
  WaitingChunkTable::iterator it = _waiting_chunk_cache->begin();
  while (it != _waiting_chunk_cache->end()) {
    Chunk *waiting_chunk = it.value();
    if (waiting_chunk->getStatus().isComplete()) {
      pthread_mutex_lock(&_ready_chunkQ_lock);
      pushReadyChunkQ(waiting_chunk);
      pthread_mutex_unlock(&_ready_chunkQ_lock);
      logger.log(MF_TIME,
                 "seg: push chunk_id: %u in waitingCache to readyQ",
                 waiting_chunk->getChunkID());
      //remov entry from waitingCache
      it = _waiting_chunk_cache->erase(it);
    } else {
      ++it;
    }
  }
}

//return NULL => fail to make csyn packet
Packet* MF_Segmentor::makeCsynPacket(uint32_t hop_id, uint32_t num_of_packets, 
                                     uint32_t next_hop_GUID) {
  //create csyn pkt
  WritablePacket * csyn_pkt = Packet::make(sizeof(click_ether) + sizeof(click_ip),
                                           0, CSYN_SIZE, 0);
  if (csyn_pkt) {
    //fill csyn pkt field
    memset(csyn_pkt->data(), 0, csyn_pkt->length());
    csyn_t * csyn = (csyn_t *)csyn_pkt->data();
    csyn->type = htonl(CSYN_PKT);
    csyn->hop_ID = htonl(hop_id);
    csyn->chk_pkt_count = htonl(num_of_packets);
    csyn_pkt->set_anno_u32(NXTHOP_ANNO_OFFSET, next_hop_GUID);
  }
  return csyn_pkt; 
}

outstanding_chunk_t * MF_Segmentor::createOutstandingChunkStruct(uint32_t hop_id, 
                                      Chunk *chunk, bool direct) {
  uint32_t num_pkts = chunk->getChkPktCnt();
  //construct outstanding_chk
  outstanding_chunk_t *outstanding_chk = new outstanding_chunk_t(); 
  outstanding_chk->chunk = chunk;
  outstanding_chk->csyn_acked = false;
  outstanding_chk->buffer_granted = false;
  outstanding_chk->priority = 0;
  //if (num_pkts <= 5) {
  //  outstanding_chk->small = true;
  //} else {
  //  outstanding_chk->small = false;
  //}
  if (direct) {
    outstanding_chk->small = true; 
  } else {
    outstanding_chk->small = false;
  }
  outstanding_chk->rcvd_num_pkts = 0;

  //construct timer data
  outstanding_chk->timer_data = new csyn_timer_data_t();
  outstanding_chk->timer_data->elem = this;
  outstanding_chk->timer_data->hop_id = hop_id;
  outstanding_chk->timer_data->chk_pkt_count = num_pkts;
  
  //outstanding_chk->csyn_task = new Task(&MF_Segmentor::handleCsynExpiry,
  //                                      outstanding_chk->timer_data);
  //outstanding_chk->csyn_task->initialize(this, false); 
  //outstanding_chk->csyn_timer = new Timer(outstanding_chk->csyn_task); 
  outstanding_chk->csyn_timer = new Timer(&MF_Segmentor::handleCsynExpiry, 
                                         outstanding_chk->timer_data);
  outstanding_chk->csyn_timer->initialize(this);
  return outstanding_chk; 
}

void MF_Segmentor::deleteOutstandingChunkStruct(outstanding_chunk_t *outstanding_chk) {
  outstanding_chk->csyn_timer->clear();
  delete outstanding_chk->timer_data;
  delete outstanding_chk->csyn_timer; 
  //delete outstanding_chk->csyn_task; 
}

uint32_t MF_Segmentor::popPacketOutQ() {
  pthread_mutex_lock(&_sending_win_lock); 
  uint32_t avail_win_size = _sending_win_size - _cur_sending_win;
  pthread_mutex_unlock(&_sending_win_lock);
  
  //pop 0 pkt is availabe win size is less than 0
  if (avail_win_size <= 0) {
    logger.log(MF_DEBUG, 
               "seg: packets sending buffer overflow, available window: %u", 
               avail_win_size);
    return 0;  
  }

  //if more pkts can be sent
  int data_pkt_cnt= 0;
  pthread_mutex_lock(&_packet_outQ_lock);
  //pull pkts as many as possible, if not empty
  while (!_packet_outQ->empty()) {
    Packet *pkt_in_outQ = _packet_outQ->front();
    _packet_outQ->pop_front();
    assert(pkt_in_outQ);
    //check packet type 
    uint32_t *pkt_type = (uint32_t*)pkt_in_outQ->data();
    if (ntohl(*pkt_type) == DATA_PKT) { 
      data_pkt_cnt++;
    } else if (ntohl(*pkt_type) == CSYN_PKT) {
      csyn_t *csyn_pkt = (csyn_t*)pkt_in_outQ->data();
      uint32_t hop_id = ntohl(csyn_pkt->hop_ID);
      if (!startOutstandingCsynTimer(hop_id)) {
        logger.log(MF_ERROR,
                   "seg: Cannot start csyn timer, hop_id: %u, chunk_id: %u",
                   hop_id);
      } else {
        logger.log(MF_DEBUG,
                   "seg: pop OutPacketQ, send CSYN and enable CSYN timer, hop_id: %u",
                   hop_id);
      }
    } else {
      logger.log(MF_ERROR, "seg: error packet in out packet Q"); 
    }
    output(0).push(pkt_in_outQ);
  }
  pthread_mutex_unlock(&_packet_outQ_lock);
  return data_pkt_cnt;
} 

//send data without receiving ack(only for small chunk)
//return ture if success, otherwise, return false
bool MF_Segmentor::sendSmallChunk(Chunk *chunk) {
  if (!chunk) {
    return false; 
  }

  //get new hop id for sending
  uint32_t new_local_hop_id = getNewLocalHopID();
  uint32_t chunk_id = chunk->getChunkID();
  logger.log(MF_DEBUG,
             "seg: about to send a small chunk! hop_id: %u, chunk_id: %u",
             new_local_hop_id, chunk_id);

  outstanding_chunk_t *outstanding_chk =
                         createOutstandingChunkStruct(new_local_hop_id, chunk, true);
  
  //insert into outstanding table
  //pthread_mutex_lock(&_outstanding_chk_buf_lock);
  _outstanding_chk_buf_lock.acquire();
  bool ret = _outstanding_chk_buf->set(new_local_hop_id, outstanding_chk);
  _outstanding_chk_buf_lock.release();
  //pthread_mutex_unlock(&_outstanding_chk_buf_lock);

  //create csyn pkt
  uint32_t num_pkts = chunk->getChkPktCnt();
  uint32_t next_hop_GUID = chunk->getNextHopGUID().to_int();
  Packet *csyn_pkt = makeCsynPacket(new_local_hop_id, num_pkts, next_hop_GUID);
  if (!csyn_pkt) {
    logger.log(MF_CRITICAL, "seg: Cannot make csyn packet");
    return false;
  }

  //make second csyn pkt;   
  Packet *csyn_end_pkt = csyn_pkt->clone();
  if (!csyn_end_pkt) {
    logger.log(MF_CRITICAL, "seg: Cannot make csyn packet!"); 
  }

  logger.log(MF_DEBUG, "seg: sent CSYN, hop_id: %u, chunk_id: %u ",
             new_local_hop_id, chunk_id);
  //send the first csyn       
  output(SEG_PORT_OUT_DATA).push(csyn_pkt);
  
  //send data packets
  for (int ii = 0; ii < num_pkts; ++ii) {
    Packet *pkt = chunk->allPkts()->at(ii); 
    pkt->set_anno_u32(NXTHOP_ANNO_OFFSET, next_hop_GUID);
    assert(pkt);
    //clone pkt for sending out
    Packet *cloned_pkt = pkt->clone();
    //update hop header of cloned packet
    hop_data_t * cur_pkt = (hop_data_t *)(cloned_pkt->data());
    cur_pkt->hop_ID = htonl(new_local_hop_id);
    output(SEG_PORT_OUT_DATA).push(cloned_pkt);     //push to next element for sending out
  }
   
  //start csyn timer
  if (!startOutstandingCsynTimer(new_local_hop_id)) {
    logger.log(MF_ERROR,
               "seg: Cannot start csyn timer, hop_id: %u, chunk_id: %u",
               new_local_hop_id, chunk_id);
    //TODO: release table, kill pkts. 
    return false;
  } else {
    logger.log(MF_DEBUG,
               "seg: send CSYN and enable CSYN timer, hop_id: %u, chunk_id: %u",
               new_local_hop_id, chunk_id);
    output(SEG_PORT_OUT_DATA).push(csyn_end_pkt);
    return true; 
  }
}

bool MF_Segmentor::sendChunk(Chunk *chunk) {
  //get new hop id
  uint32_t new_local_hop_id = getNewLocalHopID();
  //construct outstanding chunk
  outstanding_chunk_t *outstanding_chk = 
                         createOutstandingChunkStruct(new_local_hop_id, chunk); 
  //schedule timer
  outstanding_chk->csyn_timer->schedule_after_msec(CSYN_TIMEOUT); 
  //insert into outstanding table
  _outstanding_chk_buf_lock.acquire();
  //pthread_mutex_lock(&_outstanding_chk_buf_lock);
  bool ret = _outstanding_chk_buf->set(new_local_hop_id, outstanding_chk);
  _outstanding_chk_buf_lock.release();
  //pthread_mutex_unlock(&_outstanding_chk_buf_lock);

  uint32_t chunk_id = chunk->getChunkID();
  uint32_t num_pkts = chunk->getChkPktCnt(); 
  uint32_t next_hop_GUID = chunk->getNextHopGUID().to_int();
  
  //create csyn pkt
  Packet *csyn_pkt = makeCsynPacket(new_local_hop_id, num_pkts, next_hop_GUID);
  if (!csyn_pkt) {
    logger.log(MF_CRITICAL, "seg: Cannot make csyn packet");
    return false;
  }
  
  output(SEG_PORT_OUT_DATA).push(csyn_pkt);
  logger.log(MF_DEBUG, 
             "seg: sent CSYN and enabled CSYN Timer, hop_id: %u, chunk_id: %u ",
             new_local_hop_id, chunk_id);
}


//
//
//
//
void MF_Segmentor::sendPackets(uint32_t hop_id, Chunk *chunk, Vector<int> *lost, 
                               bool priority) {
  int lost_size = lost->size();
  uint32_t chunk_id = chunk->getChunkID(); 
  if (lost_size == 0) {
    return; 
  }

  int chk_pkt_count = chunk->getChkPktCnt(); 
  pthread_mutex_lock(&_sending_win_lock);
  int avail_win_size = _sending_win_size - _cur_sending_win;
  
  _cur_sending_win += lost_size;
  logger.log(MF_DEBUG, "seg: cur_win_size/window_size: %u/%u",
             _cur_sending_win, _sending_win_size);
  pthread_mutex_unlock(&_sending_win_lock);
  
  if (avail_win_size < 0) {
    avail_win_size = 0;
  }
  int num_pkts_to_interface = avail_win_size > lost_size ? lost_size : avail_win_size;
  logger.log(MF_DEBUG, 
             "seg: hop_id: %u, chunk_id: %u, pkt_to_send/lost_size: %u/%u", 
             hop_id, chunk_id, num_pkts_to_interface, lost_size);
  uint32_t next_hop_GUID = chunk->getNextHopGUID().to_int(); 
  int ii = 0;
  while (ii < lost_size) {
    int lost_index = lost->at(ii);
    Packet *lost_pkt = chunk->allPkts()->at(lost_index);
    assert(lost_pkt);
    lost_pkt->set_anno_u32(NXTHOP_ANNO_OFFSET, next_hop_GUID); //next_hop_GUID);
    Packet *cloned_lost_pkt = lost_pkt->clone();
    if (!cloned_lost_pkt) {
      logger.log(MF_ERROR, "seg: Cannot make cloned packet");
      ++ii; 
      break;
    }
    hop_data_t * cur_pkt = (hop_data_t *)(cloned_lost_pkt->data());
    //update hop ID
    cur_pkt->hop_ID = htonl(hop_id);
    if (priority) {
      output(SEG_PORT_OUT_PRIORITY_DATA).push(cloned_lost_pkt); 
    } else if (ii < num_pkts_to_interface) {
      output(SEG_PORT_OUT_DATA).push(cloned_lost_pkt);    //push to next element for sending out
    } else {
      pthread_mutex_lock(&_packet_outQ_lock); 
      _packet_outQ->push_back(cloned_lost_pkt);
      pthread_mutex_unlock(&_packet_outQ_lock); 
    }
    ++ii;
  }
  
  Packet *csyn_pkt = makeCsynPacket(hop_id, chk_pkt_count, next_hop_GUID);
  if (!csyn_pkt) {
    logger.log(MF_CRITICAL, "seg: Cannot make csyn packet");
    return; 
  } 
  
  pthread_mutex_lock(&_packet_outQ_lock);
  if (!_packet_outQ->empty() && !priority) {     //if packet outQ is not empty();
    _packet_outQ->push_back(csyn_pkt);  
    pthread_mutex_unlock(&_packet_outQ_lock);
  } else {
    pthread_mutex_unlock(&_packet_outQ_lock);
    //start timer; 
    if (!startOutstandingCsynTimer(hop_id)) {
      logger.log(MF_ERROR,
                 "seg: Cannot start csyn timer, hop_id: %u, chunk_id: %u",
                 hop_id, chunk_id);
      //TODO: release chunk; 
    } else if (priority) {
      output(SEG_PORT_OUT_PRIORITY_DATA).push(csyn_pkt);
      logger.log(MF_DEBUG,
                 "seg: sent CSYN hop id: %u, chunk_id: %u, nexthop: %u, "
                 "size: %u pkts with priority",
                 hop_id, chunk_id, next_hop_GUID, chk_pkt_count);
    } else {
      output(SEG_PORT_OUT_DATA).push(csyn_pkt);
      logger.log(MF_DEBUG,
                 "seg: sent CSYN hop id: %u, chunk_id: %u, nexthop: %u, "
                 "size: %u pkts",
                 hop_id, chunk_id, next_hop_GUID, chk_pkt_count);
    }
  }
}

void MF_Segmentor::handleCsynExpiry(Timer *, void *data) {
//bool MF_Segmentor::handleCsynExpiry(Task *task, void *data) {
  CsynTimerData *td = (CsynTimerData*)data;
  assert(td);
  td->elem->CsynExpire(td->hop_id, td->chk_pkt_count);
  //return true; 
}

void MF_Segmentor::CsynExpire(uint32_t hop_ID, uint32_t chk_size) {
  logger.log(MF_INFO, 
             "seg: Timer expired for unACKed hop id: %u ; resending CSYN", 
             hop_ID);
  resendCsyn(hop_ID, chk_size); 
}

void MF_Segmentor::rescheduleTask() {
  pthread_mutex_lock(&_task_lock);
  if (!_task.scheduled())
    _task.reschedule();
  pthread_mutex_unlock(&_task_lock);
}

String MF_Segmentor::read_handler(Element *e, void *thunk) {
  MF_Segmentor *seg = (MF_Segmentor *)e;
  switch ((intptr_t)thunk) {
  case 1:
    return String(seg->_sent_chk_count);
  default:
    return "<error>";
  }
}

void MF_Segmentor::add_handlers() {
  add_read_handler("sent_chk_count", read_handler, (void *)1);
}

/*
#if EXPLICIT_TEMPLATE_INSTANCES
template class Vector<hop_data_t *>;
template class HashMap<int, Vector<hop_data_t *> >;
#endif
*/

CLICK_ENDDECLS
EXPORT_ELEMENT(MF_Segmentor)
ELEMENT_REQUIRES(userlevel)
