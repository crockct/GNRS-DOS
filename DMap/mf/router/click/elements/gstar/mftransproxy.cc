#include <map>
#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/deque.hh>
#include <clicknet/ether.h>
#include "mf.hh"
#include "mftransproxy.hh"
#include "mfchunk.hh"
#include "mfchunkmanager.hh"

CLICK_DECLS

MF_TransProxy::MF_TransProxy() : _id_to_chks(), _logger(MF_Logger::init()) { 
  _logger.log(MF_DEBUG, "prx initialized!\n");
}

MF_TransProxy::~MF_TransProxy() {
}

int MF_TransProxy::configure(Vector<String> &conf, ErrorHandler *errh)
{
  if (cp_va_kparse(conf, this, errh, 
      "MY_GUID", cpkP+cpkM, cpUnsigned, &_my_GUID,
      "STORAGE_MANAGER", cpkP+cpkM, cpElement, &_stg_mgr,
      "FLOW_CTRL", cpkP+cpkM, cpElement, &_fctrl,
      "CHUNK_MANAGER", cpkP+cpkM, cpElement, &_chk_mgr,
      cpEnd) < 0)
    return -1;
  return 0;
}

void MF_TransProxy::push(int port, Packet *p) {
  if (port > 1 || port < 0) {  
    _logger.log(MF_FATAL,
               "TransProxy: Packet received on unsupported port");
    exit(-1);
  }

  _logger.log(MF_DEBUG, "TransProxy::push() pkt recvd on port %d", port);

  chk_internal_trans_t *chk_trans = (chk_internal_trans_t*)(p->data());
  //uint32_t sid = ntohl(chk_trans->sid);
  uint32_t chunk_id = chk_trans->chunk_id;
  _logger.log(MF_DEBUG, "TransProxy::push() chk with chk_id: %u", chunk_id);
 
  // port == 0, regular up-stack msg (just to hand chunk to transport for further processing)
  Chunk *chunk = _chk_mgr->get(chunk_id, ChunkStatus::ST_INITIALIZED);
  if (!chunk) {
    _logger.log( MF_ERROR, "intra_lkup: cannot find chunk");
    p->kill();
    return;
  }

  _logger.log(MF_DEBUG, "TransProxy::push()");
  if (chunk->transHeader() == NULL)
    chunk->initTransHeader();

  if (port == 1) { // stored chunk pushed-out success notification from IntraLkup;
    handleIntraSuccessPkt(chunk, chunk_id);
    p->kill();
    return;
  }
  
  // is valid, then
  uint8_t type = chunk->transHeader()->getTransType();

  _logger.log(MF_DEBUG, "TransProxy::push()");
  switch (type) {
    case DATA_T:
      handleTransData(chunk, p);
      break;
    case E2E_ACK_T:
      //handleTransAck(chunk);
      p->kill();
      break;
    case PUSH_SYN_T:
      handleTransPush(chunk);
      p->kill();
      break;
    default:
      p->kill();
      _logger.log(MF_ERROR, "error: unsupported data type, or received in error \n");
      break;
  }
  //chunk->deleteTransHeader();
}

void MF_TransProxy::handleTransPush(Chunk *chk) {
  _logger.log(MF_DEBUG, "TransProxy::handleTransPush()");
  RoutingHeader &rhdr = *(chk->routingHeader());
  //TransHeader &thdr = *(chk->transHeader());

  TransStoreHeader *pthdr = dynamic_cast<TransStoreHeader *>(chk->transHeader());
  
  // get the set of sequence numbers requested by src
  uint32_t num_chks = pthdr->getNumStoredChk();

  if (num_chks == 0) {
    _logger.log(MF_ERROR, "TransProxy::handleTransPush() num_chk to push is 0, returning");
    return;
  }
  uint32_t pushed_seqs[num_chks];
  memset(pushed_seqs, 0, num_chks*sizeof(int));
  pthdr->getStoredSeqs(pushed_seqs, num_chks);

  // update related states in storage_manager
  uint32_t src = rhdr.getSrcGUID().to_int();
  uint32_t orig_dst = pthdr->getStoreDstGUID().to_int();
  //uint32_t pld_size = pthdr->getChkSize();
  Pair<uint32_t, uint32_t> sd(src, orig_dst);
  Vector<uint32_t> chk_ids;
  // in proxy, chks are referenced by chk_id; so needs (src, dst, seq--->chk_id mapping
  _stg_mgr->unscheduleTimers(sd, pushed_seqs, num_chks, chk_ids);

  // push these chunks down the stack
  uint32_t num_pushed_chks = chk_ids.size();
  for (uint32_t i=0; i<num_pushed_chks; ++i) {
    pushChk(chk_ids[i]);
  }
}

//void MF_TransProxy::handleTransAck(Chunk *chk) {
//}

// could be: a new chunk to be stored; an old chunk, aka a stored, then emitted and then unpushed chunk.
// if first time store, do tryStore(). Then when timer expires, timer will be un-scheduled, but data entry for that chunk will be there
// otherwise, do reStore(). 
void MF_TransProxy::handleTransData(Chunk *chk, Packet *p) {
  _logger.log(MF_DEBUG, "TransProxy::handleTransData()");
  chk_internal_trans_t *chk_trans = (chk_internal_trans_t*)(p->data());
  //uint32_t sid = ntohl(chk_trans->sid);
  uint32_t chunk_id = chk_trans->chunk_id;

  RoutingHeader &rhdr = *(chk->routingHeader());
  // because this NA is no longer useful
  rhdr.setDstNA(NULL);

  TransHeader &thdr = *(chk->transHeader());

  uint32_t src = rhdr.getSrcGUID().to_int(), dst = rhdr.getDstGUID().to_int();
  uint32_t seq = thdr.getSeq(), pld_size = thdr.getChkSize(); 

  char debug_buf[400];
  memset(debug_buf, 0, 400);
  _logger.log(MF_DEBUG, "%s", thdr.to_log(debug_buf));

  Deque<flow_s*> popped;

  // see if a flow control msg is needed for src
  Pair<uint32_t, uint32_t> sd(src, dst);

  if (_id_to_chks.find(chunk_id) != _id_to_chks.end()) { // a previously stored chunk
    _logger.log(MF_DEBUG, "TransProxy::handleTransData() chk id: %u is a previously stored chk", chunk_id);
    _stg_mgr->reStore(*chk, sd, seq);
    p->kill();
    return;
  }

  _logger.log(MF_DEBUG, "TransProxy::handleTransData() chk id: %u is a new chk to be stored", chunk_id);

  bool stored = _stg_mgr->tryStore(*chk, chunk_id, sd, seq, pld_size, popped, *this);
  uint8_t cong_flag = _fctrl->getCongFlag(*chk, sd, *p); 

  // send "Store" to src
  if (stored) {
    // this is disabled because it's not fully functional
    sendStoreMsg(*chk, cong_flag);

    _id_to_chks.set(chunk_id, Pair<Chunk*, Packet*>(chk, p));

  } else { // drop this chunk because unable to store, i.e. flow_cpct reached
  
    // destroy cur chk, notify src of dropping, and send RSTP
    //sendDropMsg(*chk, cong_flag);
    destroyChunk(chk);
    p->kill();
  }

  // send "Dropped" of oldest flows to src
  while (!popped.empty()) {
      // release memory, and other resources related to this flow; remove entries pertaining to this flow 
      // from data structures; notify the corresponding src of dropping
      flow_s *fs = popped.front();
      popped.pop_front();

      //Deque<Chunk*> &chks = fs->chks; 
      std::map<uint32_t, Chunk*> &seq_to_chks = fs->seq_to_chks;
      std::map<uint32_t, Timer*> &seq_to_timers = fs->seq_to_timers;
      std::map<uint32_t, Chunk*>::iterator ckit = seq_to_chks.begin();
      while (ckit != seq_to_chks.end()) {
        std::map<uint32_t, Chunk*>::iterator temp_it = ckit; 
        ckit++;
        if (temp_it->second) {
          Chunk *chk = temp_it->second;
          //sendDropMsg(*chk, R_NOCHANGE);
          destroyChunk(chk);
        }
        seq_to_chks.erase(temp_it);
      }

      std::map<uint32_t, Timer*>::iterator tmit = seq_to_timers.begin();
      while (tmit != seq_to_timers.end()) {
        std::map<uint32_t, Timer*>::iterator temp_it = tmit; 
        tmit++;
        if (temp_it->second) {
          delete temp_it->second;
        }
        seq_to_timers.erase(temp_it);
      }

      delete fs;
  }
}

// what to do with the internal packet, free it?
void MF_TransProxy::handleIntraSuccessPkt(Chunk *chk, uint32_t chk_ID) {
  _logger.log(MF_DEBUG, "TransProxy::handleIntraSuccessPkt()");
  // send a pushed notification (this can be supported, but not now)

  // clean up entries in data struction related to this chk_ID
  _id_to_chks.erase(chk_ID);

  // stg manager should also clean up corresponding entries
  TransHeader &thdr = *(chk->transHeader());
  RoutingHeader &rhdr = *(chk->routingHeader());

  uint32_t src = rhdr.getSrcGUID().to_int(), dst = rhdr.getDstGUID().to_int();
  uint32_t seq = thdr.getSeq(), pld_size = thdr.getChkSize(); 
  Pair<uint32_t, uint32_t> sd(src, dst);

  _stg_mgr->unStore(*chk, sd, seq, pld_size);

}

void MF_TransProxy::destroyChunk(Chunk *chunk) {
  _logger.log(MF_DEBUG, "TransProxy::destroyChunk()");
  uint32_t cid = chunk->getChunkID();
  //uint32_t dst_GUID = chunk->getDstGUID().to_int();
  chunk->setStatus(ChunkStatus::ST_READY_TO_DEL);

  //free momery
  Vector<Packet*> *vec_to_free = chunk->allPkts();
  _logger.log(MF_DEBUG,
             "trans proxy: deleting chunk %u , # of pkts %u",
             cid, vec_to_free->size());
  Vector<Packet*>::iterator vec_end = vec_to_free->end();
  for (Vector<Packet*>::iterator vec_it = vec_to_free->begin();
           vec_it != vec_end; ++vec_it) {
    Packet *pkt_to_free = *vec_it;
    assert(pkt_to_free);
    pkt_to_free->kill();
  }

  delete chunk;
}

void MF_TransProxy::sendStoreMsg(Chunk &chk, uint8_t cong_flag) {
  _logger.log(MF_DEBUG, "TransProxy::sendStoreMsg()");
  uint32_t trans_hdr_size = TRANS_BASE_HEADER_SIZE + (20*2 + 4*2);

  // src/dst GUIDs, 1 offset, 1 seq num
  uint32_t pkt_size = HOP_DATA_PKT_SIZE + ROUTING_HEADER_SIZE + EXTHDR_MULTIHOME_MAX_SIZE + trans_hdr_size;
  WritablePacket *p = createPkt(pkt_size);

  // fill headers
  fillHopHdr(*p, pkt_size - HOP_DATA_PKT_SIZE);
  fillRoutingHdr(chk, *p);
  TransHeader *thdr = fillTransHdr(chk, *p, STORE_T, trans_hdr_size);
  thdr->setTransFlag(cong_flag);
  delete thdr;

  // encapsulate the packet with a 'chunk', and create states in ChkMgr so that later it can be scheduled
  Chunk *courier_chk = _chk_mgr->alloc(1); // chk pkt count is 1
  int ret = courier_chk->insert(0, p);
  if (ret < 0) {
    _logger.log(MF_ERROR, "TransProxy: pkt cannot insert to the chunk");
  }
  _chk_mgr->addToDstGUIDTable(courier_chk->getChunkID());

  // create an internal message pkt for this chunk
  WritablePacket *internal_p = createInternalMsgPkt(*courier_chk);

  output(TRANS_CONTROL_MSG_OUTPORT).push(internal_p);
}

//void MF_TransProxy::sendDropMsg(Chunk &chk, uint8_t cong_flag) {
//}

WritablePacket* MF_TransProxy::createPkt(int pkt_size) {
  WritablePacket *p = Packet::make(sizeof(click_ether), 0, pkt_size, 0);;
  if (!p) {
    _logger.log(MF_FATAL, "TransProxy: can't make packet!");
    exit(EXIT_FAILURE);
  }
  memset(p->data(), 0, p->length()); 
  return p;
}

WritablePacket* MF_TransProxy::createInternalMsgPkt(Chunk &chk) {
  WritablePacket *p = createPkt(sizeof(chk_internal_trans_t));
  chk_internal_trans_t* chk_trans = (chk_internal_trans_t*)(p->data());
  chk_trans->sid = htonl(chk.getServiceID().to_int());
  chk_trans->chunk_id = chk.getChunkID();
  chk_trans->upper_protocol = htonl(chk.getUpperProtocol());
  return p;
}

void MF_TransProxy::fillHopHdr(WritablePacket &p, uint32_t pld_size) {
  hop_data_t *hop_hdr = reinterpret_cast<hop_data_t *>(p.data()); 
  hop_hdr->type = htonl(DATA_PKT);
  hop_hdr->pld_size = htonl(pld_size);
  hop_hdr->seq_num = htonl(0);
}

void MF_TransProxy::fillRoutingHdr(Chunk &chk, WritablePacket &p) {
  _logger.log(MF_DEBUG, "TransProxy::fillRoutingHdr()");
  RoutingHeader &prev_rhdr = *(chk.routingHeader());

  RoutingHeader rhdr(p.data() + HOP_DATA_PKT_SIZE);  
  rhdr.setVersion(1);

  // service id: unicast, and maybe content-based?
  rhdr.setServiceID(MF_ServiceID::SID_UNICAST);
  rhdr.setUpperProtocol(TRANSPORT);
  rhdr.setDstGUID(prev_rhdr.getSrcGUID());
  rhdr.setDstNA(prev_rhdr.getSrcNA());

  // how to pass my GUID here?
  rhdr.setSrcGUID(_my_GUID);
  rhdr.setSrcNA(_my_GUID);
  
  // ext header is considered by default
  rhdr.setPayloadOffset(ROUTING_HEADER_SIZE + EXTHDR_MULTIHOME_MAX_SIZE);
}

TransHeader* MF_TransProxy::fillTransHdr(Chunk &chk, WritablePacket &p, uint8_t type,
    uint32_t trans_hdr_size) {
  _logger.log(MF_DEBUG, "TransProxy::fillTransHdr()");
  RoutingHeader &prev_rhdr = *(chk.routingHeader());
  TransHeader &prev_thdr = *(chk.transHeader());

  _logger.log(MF_ERROR, "1"); 
  TransHeader *thdr = TransHeaderFactory::newTransHeader(p.data() + 
                      HOP_DATA_PKT_SIZE + ROUTING_HEADER_SIZE + EXTHDR_MULTIHOME_MAX_SIZE, type);

  _logger.log(MF_ERROR, "2"); 
  uint32_t dst_GUID = prev_rhdr.getSrcGUID().to_int();
  _logger.log(MF_ERROR, "3, dst: %u", dst_GUID);
  thdr->setSeq(getSeqForGUID(dst_GUID)).
        setChkSize(1). // dummy number
        setPktCnt(1).
        setStartOffset(0).
        setEndOffset(0).  // because there's only one pkt in this chunk
        setTransFlag(NO_REQUEST).
        setReliabPref(PREF_DONT_CARE).
        setTransOffset(trans_hdr_size).
        setCongNotif(R_NOFEEDBACK).
        setRecvWd(20); // doesn't matter

  char debug_buf[400];
  memset(debug_buf, 0, 400);
  _logger.log(MF_DEBUG, "%s", thdr->to_log(debug_buf));

  switch (type) {
    case DATA_T:
      break;
    case E2E_ACK_T:
      break;
    case PUSH_SYN_T:
      break;
    case STORE_T: {
      uint32_t num_stored_chk = 1;
      TransStoreHeader *store_thdr = dynamic_cast<TransStoreHeader *>(thdr);
      _logger.log(MF_ERROR, "4"); 
      store_thdr->setStoreSrcGUID(prev_rhdr.getSrcGUID()).
            setStoreDstGUID(prev_rhdr.getDstGUID()).
            setNumStoredChk(num_stored_chk);
      _logger.log(MF_ERROR, "5"); 
      uint32_t stored_seq = prev_thdr.getSeq();
      store_thdr->setStoredSeqs(const_cast<uint32_t *>(&stored_seq), num_stored_chk);
      break;
    }
    default: 
      break;
  }

  _logger.log(MF_DEBUG, "TransProxy::fillTransHdr() finished");

  return thdr;
}

void MF_TransProxy::handleStgExpiry(Timer *tm, void *data) {
  stg_timer_data_s *td = reinterpret_cast<stg_timer_data_s*>(data);
  fprintf(stderr, "TransProxy::handleStgExpiry() stg timer expires for chk_id: %u\n",
              td->chk_ID);
  fprintf(stderr, "TIME,STG_PUSH,%u\n", td->chk_ID);
  td->trans_proxy->pushChk(td->chk_ID);
}

//void MF_TransProxy::chkStgExpire(stg_timer_data_s *td) {
void MF_TransProxy::pushChk(uint32_t chk_id) {
  _logger.log(MF_DEBUG, "TransProxy::pushChk() called when stg timer expires for chk_id: %u",
              chk_id);
  // should prepare to do a dst NA lookup
  //uint32_t chk_id = td->chk_ID;
  if (_id_to_chks.find(chk_id) == _id_to_chks.end()) {
    _logger.log(MF_ERROR, "TransProxy::pushChk() chk_id is not found");
    exit(-1);
  }

  Pair<Chunk*, Packet*> &pr = _id_to_chks[chk_id];
  if (pr.second) { 
    // Note this is to be updated, need to to flow control, i.e. here set a inter-sending timer
    Packet *p = pr.second->clone();
    output(1).push(p); 
  } else {
    _logger.log(MF_ERROR, "trans_proxy: expiry error\n");
  }
  //pr.second = NULL;
}

uint32_t MF_TransProxy::getSeqForGUID(uint32_t guid) {
  _logger.log(MF_DEBUG, "TransProxy::getSeqForGUID()");
  uint32_t seq_to_use = 0;
  if (_guid_to_seq.find(guid) == _guid_to_seq.end()) {
    _guid_to_seq[guid] = 0;
    seq_to_use = 0;
  } else {
    seq_to_use = ++_guid_to_seq[guid];
  } 
  return seq_to_use;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(MF_TransProxy)
ELEMENT_REQUIRES(userlevel)
