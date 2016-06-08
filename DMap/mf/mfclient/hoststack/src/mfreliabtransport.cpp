#include <limits>
#include <include/mftransportheader.hh>
#include <cassert>

#include "mftransport.h"
#include "mfreliabtransport.h"
#include "mfflowcontroller.h"

// sender timers: 1. a timer for a message(session), NACK/ACK related. 
// NACK case: if no NACK recvd before timer expire, purge chunks; else, 
// resend and reschedule timer; ACK: if no ACK recvd before timer expire, 
// retransmit all chunks; else, purge chunks.
// 2. in flow control, a timer for next ready-for-sending chk in the same
// message; upon expiration, send this chk, and schedule timer for next chk.

// recver timers: 1. NACK/ACK timer

MF_ReliabTransport::MF_ReliabTransport() {
  //test();
}

void MF_ReliabTransport::test() {
  MF_Log::mf_log(MF_DEBUG, "************* Running driver function to test:"); 
  u_int dst = 1000;
  vector<u_int> seq;
  for (u_int i=5; i<=9; ++i) seq.push_back(i);
  //sendPushSyn(dst, seq);
}

MF_ReliabTransport::MF_ReliabTransport(u_int tid, MF_Buffer *recvBuffer,  
      MF_Buffer *sendBuffer,MFSystem *_system) :
      MF_BasicTransport(tid, recvBuffer, sendBuffer, _system) {
  _fctrl = new MF_WindowController();
}

MF_ReliabTransport::~MF_ReliabTransport() {
  delete _fctrl;
}

void MF_ReliabTransport::sendData(vector <u_char *> *data, u_int size, 
      int srcGUID, int dstGUID, mfflag_t opts) {
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::sendData() send data received" 
      "from API, srcGUID: %u, dstGUID: %u", srcGUID, dstGUID);
  u_int count = computeMsgChkCnt(size);

  MF_Log::mf_log(MF_DEBUG, "ReliabTrans:sendData new message of size %u" 
      ", with %u buffers"
      "will be composed of %u messages", size, data->size(), count);

  if (_guid_to_dst.find(dstGUID) == _guid_to_dst.end()) {
    dst_info_s *di = new dst_info_s(dstGUID);
    di->recv_wd = E2E_RECVWD; // should be passed by SockMan
    _guid_to_dst[dstGUID] = di;
  }

  
  // for now, considering beyond a certain threshold, will do NACK.
  //const u_char reliab_pref = (count > CHK_COUNT_NACK_THRESH) ? PREF_NACK : PREF_ACK;
  const u_char reliab_pref = (count > _chk_count_NACK_thresh) ? PREF_NACK : PREF_DONT_CARE;

  u_int start_seq = prepareChks(data, size, srcGUID, dstGUID, opts, count, 
      DATA_T, reliab_pref);

  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::sendData() dstGUID: %d", dstGUID);

  //sendImmediately(dstGUID);
  _fctrl->sendData(this, _system->getEventQueue(), dstGUID);
  

  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::sendData() after sendImme");
  pair<u_int, u_int> dstseq(dstGUID, start_seq);
  // initiate timers at sender, so that later if acked, or no nack, we can 
  // release memory, instead of holding data indefinitely

  if (reliab_pref == PREF_NACK) { // one timer for entire message (session)
    // 
    MF_Log::mf_log(MF_DEBUG, "ReliabTrans::sendData() if reliab==PREF_NACK");
    timerdata_s *ts = new timerdata_s(SEND_NACK_TIMEOUT_T, dstseq);
    u_int timerid = _system->getTimeoutManager()->AddTimeout(this, ts, _send_NACK_timeout*count);
    MF_Log::mf_log(MF_DEBUG, "ReliabTrans::sendData() after AddTimeout");
    
    ts->timer_id = timerid;

    insertTimerdata(ts, dstseq);
    MF_Log::mf_log(MF_DEBUG, "ReliabTrans::sendData() after insert");

  } else if (reliab_pref == PREF_DONT_CARE) { 
    // no timer, after it is pushed out, release memory immediately
    MF_Log::mf_log(MF_DEBUG, "ReliabTrans::sendData() if reliab==PREF_DONT_CARE");

  } else if (reliab_pref == PREF_ACK) {

  }

}


/*
void MF_ReliabTransport::sendImmediately(u_int dstGUID) {
  if (_dst_to_chks.find(dstGUID) == _dst_to_chks.end()) {
    MF_Log::mf_log(MF_ERROR, "ReliabTrans::sendImme(): dstGUID not found in _dst_to_chks\n");
    return;
  }
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::sendImme() to dstGUID: ", dstGUID);

  list<MF_ChunkInfo*> *chk_list = _dst_to_chks[dstGUID];
  while (!chk_list->empty()) {
    MF_Log::mf_log(MF_DEBUG, "ReliabTrans::sendImme(), list size: %u", chk_list->size());
    MF_ChunkInfo *ci = chk_list->front();
    MF_EvDownTransp *erc = new MF_EvDownTransp(ci);
    eq->add(erc);
    chk_list->pop_front();
  }
  delete chk_list;
  _dst_to_chks.erase(dstGUID);
} */

u_int MF_ReliabTransport::prepareChks(vector <u_char *> *data, u_int size,
      int srcGUID, int dstGUID, mfflag_t opts, u_int num_chks, u_char trans_type, 
      u_char reliab_pref) {
  if (_dst_to_chks.find(dstGUID) == _dst_to_chks.end()) {
    _dst_to_chks[dstGUID] = new list<MF_ChunkInfo*>();
  }
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::prepareChks()");

  list<MF_ChunkInfo*> &chk_list = *(_dst_to_chks[dstGUID]);

  u_int rem_buf_cnt = data->size(); // *data is a vec of bufs, this is a count of unused ones
  u_int rem_size_bytes = size;

  u_int &seq = (_guid_to_dst[dstGUID])->last_seq; 
  u_int start_seq = seq + 1;
  
  MF_Log::mf_log(MF_DEBUG, "MF_ReliabTransport::prepareChks(): buf_cnt: %u, size: %u\n", rem_buf_cnt, rem_size_bytes);
  MF_Log::mf_log(MF_DEBUG, "MF_ReliabTransport::prepareChks(): num_chks: %u\n", num_chks);

  char debug_buf[400];    
  for(u_int i = 0; i<num_chks; i++) {
    MF_ChunkInfo *ciTemp = new MF_ChunkInfo();
    ciTemp->putSrcTID(mTID);
    ciTemp->putSrcGUID(srcGUID);
    ciTemp->putDstGUID(dstGUID);
    ciTemp->putStartOffset((u_short)i);
    ciTemp->putEndOffset((u_short)(num_chks-1-i));
    vector<u_char*> *pList = ciTemp->getPacketList();

    //copy pointers in chunk
    u_int chk_pkt_cnt = (rem_buf_cnt >= MAX_CHK_PKT_CNT) ? MAX_CHK_PKT_CNT : rem_buf_cnt;

    //If fits, I can use the whole data remaining, otherwise I can use only max minus HIGH_HEADER_LEN
    u_int chk_size_bytes = (rem_buf_cnt > MAX_CHK_PKT_CNT) ? chk_pkt_cnt*MF_PacketSupport::MAX_PAYLOAD_SIZE - 
              MF_PacketSupport::HIGH_HEADER_LEN : rem_size_bytes;

    rem_size_bytes -= chk_size_bytes;

    rem_buf_cnt -= chk_pkt_cnt;

    for(u_int j = 0; j<chk_pkt_cnt;  j++){
      pList->push_back(data->at(i*MAX_CHK_PKT_CNT + j));
    }

    TransHeader *thdr = TransHeaderFactory::newTransHeader(
        MF_PacketSupport::getTransportHeaderPtr(pList->front(), true), trans_type);

    // needs to change start/end offset
    seq++;
    //thdr->fillBaseTransHdr(seq, chk_size_bytes, chk_pkt_cnt, i, num_chks-1-i, trans_type,
    //    R_NOFEEDBACK, reliab_pref, TRANS_BASE_HEADER_SIZE); 
    thdr->setSeq(seq).
          setChkSize(chk_size_bytes).
          setPktCnt(chk_pkt_cnt).
          setStartOffset(i).
          setEndOffset(num_chks-1-i).
          setTransType(trans_type).
          setTransFlag(NO_REQUEST).
          setReliabPref(reliab_pref).
          setTransOffset(TRANS_BASE_HEADER_SIZE).
          setCongNotif(R_NOFEEDBACK). 
          setRecvWd(E2E_RECVWD);




    // fill my recv buf size
    if (i == 0) {
      thdr->setCongNotif(R_NOCHANGE);
      thdr->setRecvWd(E2E_RECVWD);
    }
    memset(debug_buf, 0, 400);
    MF_Log::mf_log(MF_DEBUG, "%s", thdr->to_log(debug_buf));
    delete thdr;    
    thdr = NULL;

    ciTemp->putChunkSize(chk_size_bytes);
    ciTemp->putChunkPktCnt(chk_pkt_cnt);
    ciTemp->putChunkID(seq);
    //TODO read options for service ID
    ciTemp->putServiceID(0);
    ciTemp->setOwner(mTID); 
    ciTemp->setOpts(opts);
    
    // use start_seq
    ciTemp->putMsgID(seq - i);

    chk_list.push_back(ciTemp);
    MF_Log::mf_log(MF_DEBUG, "MF_ReliabTransport::prepareChks() end: num_chks: %u, i: %u\n", num_chks, i);
  }
  return start_seq;
}

// handle different types of trans msg
void MF_ReliabTransport::recvData(MF_ChunkInfo *ci) {
  vector<u_char *> *pList = ci->getPacketList();
  
  // VERITY WHAT THIS POINTER POINTS TO!!!
  u_char *trans_ptr = MF_PacketSupport::getTransportHeaderPtr(pList->front(), true);
  u_char trans_type = TransHeader::getTransTypeStatic(trans_ptr);
 

  switch (trans_type) {
    case DATA_T:
      handleTransData(ci);
      MF_Log::mf_log(MF_DEBUG, "ReliabTrans::recvData(), recvd Transport layer data"); 
      break;
    case E2E_ACK_T:
      handleTransACK(ci);
      MF_Log::mf_log(MF_DEBUG, "ReliabTrans::recvData(), recvd Transport ACK"); 
      break;
    case E2E_NACK_T:
      handleTransNACK(ci);
      MF_Log::mf_log(MF_DEBUG, "ReliabTrans::recvData(), recvd Transport NACK"); 
      break;
    case STORE_T:
      handleTransStore(ci);
      MF_Log::mf_log(MF_DEBUG, "ReliabTrans::recvData(), recvd Transport Store msg"); 
      break;
    default:
      MF_Log::mf_log(MF_ERROR, "ReliabTrans::recvData() error: unsupported data type, or received in error \n");
      break;
  }
  //TransHeader *thdr = TransHeaderFactory::newTransHeader(trans_ptr);
}

// recv data; acknowledge; process recv buffer update.
void MF_ReliabTransport::handleTransData(MF_ChunkInfo *ci) {
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::handleTransData()");
  vector<u_char *> *pList = ci->getPacketList();

  TransHeader *thdr = TransHeaderFactory::newTransHeader(
      MF_PacketSupport::getTransportHeaderPtr((*pList)[0], true));
  RoutingHeader rhdr(MF_PacketSupport::getNetworkHeaderPtr((*pList)[0]));
  
  // to myself, remote side is the dst.
  u_int dstGUID = rhdr.getSrcGUID().to_int();

   // flow control feedback & ack
  //_fctrl->handleFlowCtrlNotif(this, thdr, eq, dstGUID);
  // check if recv buf update is requested
  //_fctrl->handleInDataFlag(this, thdr, eq, dstGUID);

  char debug_buf[400];
  memset(debug_buf, 0, 400);

  // need to deal with 0-length chunk: discard it
  if (thdr->getChkSize() == 0) {
    MF_Log::mf_log(MF_DEBUG, "MF_ReliabTransport::handleTransData() recvd chunk of 0 size:\n%s", thdr->to_log(debug_buf));
    delete thdr;
    return;
  }

  u_int seq = thdr->getSeq(), msgID = 0;
  u_short start_offset = thdr->getStartOffset();
  u_int start_seq = seq - start_offset;

  pair<u_int, u_int> dstseq(dstGUID, start_seq);
  map<pair<u_int, u_int>, MF_Message*>::iterator it = 
      _dstseq_to_msgs.find(dstseq);



  MF_Log::mf_log(MF_DEBUG, "MF_ReliabTransport::handleTransData() recvd chunk:\n%s", thdr->to_log(debug_buf));
  
  u_char reliab_pref = thdr->getReliabPref();
  MF_Message *msg = NULL;
  if (it == _dstseq_to_msgs.end()) {
    MF_Log::mf_log(MF_DEBUG, "MF_ReliabTransport::handleTransData received chunk of new message %hu", msgID);
    u_short num_chks = thdr->getStartOffset() + thdr->getEndOffset() + 1;
    if (num_chks <= 0) {
      MF_Log::mf_log(MF_ERROR, "MF_ReliabTransport::handleTransData 0-chunk message received!! exiting");
      exit(-1);
    }
        
    msg = new MF_Message(num_chks);
    msg->setTotal(num_chks);
    msg->setStartSeq(start_seq);
    MF_Log::mf_log(MF_DEBUG, "MF_ReliabTransport::handleTransData new message will have %hu chunks", msg->getTotal());
    _dstseq_to_msgs[dstseq] = msg;
    
    // for reliability, if sdr requsted NACK, needs to set a NACK timer now
    // if ACK, needs to send ack
    if (reliab_pref == PREF_NACK) {

      // create a timer
      timerdata_s *ts = new timerdata_s(RECV_NACK_TIMEOUT_T, dstseq);
      u_int msg_timeout = (_recv_NACK_timeout <= std::numeric_limits<u_int>::max()/num_chks ? 
                          _recv_NACK_timeout*num_chks : std::numeric_limits<u_int>::max());
      u_int timerid = _system->getTimeoutManager()->AddTimeout(this, ts, msg_timeout);
      MF_Log::mf_log(MF_DEBUG, "MF_ReliabTransport::handleTransData created a timer with timeout: %u", msg_timeout);
      ts->timer_id = timerid;
      insertTimerdata(ts, dstseq);
      //_dstseq_to_timerdata[dstseq] = ts;
    }
  } else {
    msg = it->second;
  }

 
  // if this is last chunk of the msg, push it up first.
  vector<u_char *> *rec = new vector<u_char *>();
  ci->putChunkSize(thdr->getChkSize());
  ci->putStartOffset(thdr->getStartOffset());
  ci->putEndOffset(thdr->getEndOffset());
  MF_Log::mf_log_time("MF_ReliabTransport::handleTransData id=%u source=%u destination=%u size=%u packets=%u",
  			ci->getChunkID(), ci->getSrcGUID(), ci->getDstGUID(), ci->getChunkSize(), ci->getChunkPktCnt());

  // return value of -1 should incur a "R_STP", but not vice versa
  //if(mBuffer->getVectorBySize(rec, ci->getChunkSize() + MF_PacketSupport::HIGH_HEADER_LEN)<0){
  if(recvBuffer->getVectorBySize(rec, ci->getChunkSize() + MF_PacketSupport::HIGH_HEADER_LEN)<0){
    MF_Log::mf_log(MF_WARN, "MF_ReliabTransport::handleTransData no space available in buffer");
    delete rec;
    rec = NULL;
    //return;
  }

  //MF_ChunkInfo *ci = doAck();
  if (!rec) {
    //ci = doAck(); // note that this chunk is not accepted. 
    //if (!ci) sendCongFeedback();
  } else {
    //ci = doAck(); // note that this chunk is about to be accepted.
    if (ci) {
      // incorporate the cong flag
    } else { 
      // no need for ack/nack message at the moment, check if cong feedback needs to be sent
      //if (cong_flag != R_NOFEEDBACK) {
        // create message just for cong feedback
      //}
    }
  }
 
  // somewhere in this function, needs to check validity of the received chunk 

  for(u_int i = 0; i<ci->getChunkPktCnt(); i++){
    //MF_Log::mf_log(MF_DEBUG, "MF_ReliabTransport::recvData copying packet %u of %u", i+1, ci->getPacketCount());
    memcpy((*rec)[i], (*pList)[i], MF_PacketSupport::MAX_PAYLOAD_SIZE + MF_PacketSupport::LOW_HEADER_LEN);
  }

  MF_ChunkInfo *newCi = new MF_ChunkInfo();
  newCi->putChunkID(ci->getChunkID());
  newCi->putChunkSize(ci->getChunkSize());
  newCi->putChunkPktCnt(ci->getChunkPktCnt());
  newCi->putStartOffset(ci->getStartOffset());
  newCi->putEndOffset(ci->getEndOffset());
  newCi->putPktList(rec);

  msg->addChunk(newCi);
  msg->setBitmapBit(start_offset);


  if(msg->isComplete()){
    MF_Log::mf_log(MF_DEBUG, "MF_ReliabTransport::handleTransData received whole message, alerting stack manager");

    if (reliab_pref == PREF_NACK) {
      // should cancel NACK timer.(what if at this moment timer has already fired?)
      cancelNACKTimer(dstseq, RECV_NACK_TIMEOUT_T);
    }

    // does application read from this "complete" structure?
    complete.push_back(msg);

    //Make the parameters to be correct
    MF_EvUpTransp *event = new MF_EvUpTransp(mTID, msg->getRemaining(), ci->getDstGUID());
    _system->getEventQueue()->add(event);
  }
  delete thdr; 
}

// NACK message from receiver
void MF_ReliabTransport::handleTransNACK(MF_ChunkInfo *ci) {
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::handleTransNACK()");
  vector<u_char*> *pList = ci->getPacketList();
  TransHeader *thdr = TransHeaderFactory::newTransHeader(
      MF_PacketSupport::getTransportHeaderPtr(pList->front(), true), E2E_NACK_T);

  TransAckHeader *athdr = (TransAckHeader *)(thdr);
  RoutingHeader rhdr(MF_PacketSupport::getNetworkHeaderPtr((*pList)[0]));

  // need to get the dst guid somewhere
  reTxBasedonBitmap(athdr, rhdr.getSrcGUID().to_int());

  delete thdr;
}

// handles "store" message from storage proxy (router)
void MF_ReliabTransport::handleTransStore(MF_ChunkInfo *ci) {
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::handleTransStore()");
  vector<u_char*> *pList = ci->getPacketList();
  TransHeader *thdr = TransHeaderFactory::newTransHeader(
      MF_PacketSupport::getTransportHeaderPtr(pList->front(), true), STORE_T);

  TransStoreHeader *sthdr = (TransStoreHeader *)(thdr);
  RoutingHeader rhdr(MF_PacketSupport::getNetworkHeaderPtr((*pList)[0]));

  // for adding stored seqs to _stored_chks
  u_int num_stored_chks = sthdr->getNumStoredChk(); 
  u_int stored_seqs[num_stored_chks];
  //u_int stored_seqs[num_stored_chks] = {0}; // initialize the array to be all-0, generates a compile error
  memset(stored_seqs, 0, sizeof(u_int)*num_stored_chks);
  sthdr->getStoredSeqs(stored_seqs, num_stored_chks); 

  // potential threat: rhdr did not change byte order when calling getSrcGUID()
  u_int storer_guid = rhdr.getSrcGUID().to_int(), dst_guid = sthdr->getUintStoreDstGUID(); 
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::handleTransStore(): storer: %u, dst: %u", 
                            storer_guid, dst_guid);

  pair<u_int, u_int> dstseq(dst_guid, 0);

  for (u_int i=0; i<num_stored_chks; ++i) {
    // check first to see if this chunk is already stored
    dstseq.second = stored_seqs[i];

    if (_stored_chks.find(dstseq) != _stored_chks.end()) {
      MF_Log::mf_log(MF_DEBUG, "ReliabTrans::handleTransStore(): chk with dst: %u, seq: %u"
                                " already stored, ignore the Store request", 
                                dstseq.first, dstseq.second);
      continue;
    }
    stored_chk_s *stchk = new stored_chk_s(storer_guid, dst_guid, stored_seqs[i]);
    _stored_chks[dstseq] = stchk;
  }

  delete sthdr;

}


void MF_ReliabTransport::cancelNACKTimer(pair<u_int, u_int> dstseq, 
      u_char timeout_type) {
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::cancelNACKTimer(), timer with dst: %u, seq: %u, type: %u to be cancelled",
                  dstseq.first, dstseq.second, timeout_type);
  map<pair<u_int, u_int>, timerdata_s*>::iterator it = _dstseq_to_timerdata.find(dstseq);
  if (it == _dstseq_to_timerdata.end()) {
    MF_Log::mf_log(MF_ERROR, "ReliabTrans::cancelNACKTimer(): timer id not found. ");
    exit(-1);
  } 
  timerdata_s *td = it->second;
  if (!td) {
    MF_Log::mf_log(MF_ERROR, "ReliabTrans::cancelNACKTimer() timerdata not found");
    exit(-1);
  }
  
  // if it's at the head
  if (td->timeout_type == timeout_type) {
    MF_Log::mf_log(MF_DEBUG, "ReliabTrans::cancelNACKTimer(), timer record found");
    if (!td->next) {
      _dstseq_to_timerdata.erase(dstseq);
    } else {
      _dstseq_to_timerdata[dstseq] = td->next;
    }
  } else {
    timerdata_s *prev = td;
    for (; td->next != NULL; td = td->next) {
      if (td->timeout_type == timeout_type) {
        MF_Log::mf_log(MF_DEBUG, "ReliabTrans::cancelNACKTimer(), timer record found");
        break;
      }
    }
    prev->next = td->next; 
  }
  _system->getTimeoutManager()->ClearTimeout(td->timer_id);
  //delete td;
}

// delete chunks
void MF_ReliabTransport::handleSendNACKTOut(pair<u_int, u_int> dstseq) {
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::handleSendNACKTOut(), for msg with dst: %u, start_seq: %u", 
                  dstseq.first, dstseq.second);
  map<pair<u_int, u_int>, MF_ChunkInfo*>::iterator it = _unacked_chks.find(dstseq);
  if (it == _unacked_chks.end()) {
    MF_Log::mf_log(MF_ERROR, "ReliabTrans::handleSendNACKTOut()");
    exit(-1);
  }
  MF_ChunkInfo *first_ci = it->second;
  
  // get a range of seq numbers for this message
  // TODO: also clear corresponding states in stored_chks
  u_int end_offset = first_ci->getEndOffset();
  bool newSpace = false;
  for (u_int i=0; i<=end_offset; ++i) {
    map<pair<u_int, u_int>, MF_ChunkInfo*>::iterator itt = _unacked_chks.find(dstseq); 
    if (itt == _unacked_chks.end()) {
      MF_Log::mf_log(MF_ERROR, "ReliabTrans::handleSendNACKTOut(), chunk not found in unacked_chks");
    } else {
      MF_ChunkInfo *ci = itt->second;
      vector<u_char*> *v = ci->getPacketList(); 
      if (v) {
        //mBuffer->putVector(v);
        sendBuffer->putVector(v);
        newSpace = true;
      } else {
        MF_Log::mf_log(MF_WARN, "ReliabTrans::handleSendNACKTOut(), trying to put empty buffer");
      }
      _unacked_chks.erase(itt);
      MF_Log::mf_log(MF_DEBUG, "ReliabTrans::handleSendNACKTOut(), deleted chk with dst: %u, seq: %u",
                      dstseq.first, dstseq.second);
      delete ci;
    }
    dstseq.second++; 
  }
  if (newSpace) {
	  MF_EvSpaceAvail *event = new MF_EvSpaceAvail(mTID);
	  _system->getEventQueue()->add(event);
  }
}

void MF_ReliabTransport::insertTimerdata(timerdata_s *ts, pair<u_int, u_int> &dstseq) {
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::insertTimerData, timer type: %u, timer_id: %u, dst: %u, seq: %u", 
                ts->timeout_type, ts->timer_id, dstseq.first, dstseq.second);
  map<pair<u_int, u_int>, timerdata_s*>::iterator it = _dstseq_to_timerdata.find(dstseq); 
  if (it == _dstseq_to_timerdata.end()) {
    //it->second = ts;
    _dstseq_to_timerdata[dstseq] = ts;
    MF_Log::mf_log(MF_DEBUG, "ReliabTrans::insertTimerData, dstseq not present, so insert new entry in map"); 
  } else {
    if (!it->second) {
      //it->second = ts;
      MF_Log::mf_log(MF_ERROR, "ReliabTrans::insertTimerData, present state (NULL entry) is wrong. exiting");
      exit(-1);
    } else {
      timerdata_s *cur_ts = it->second;
      while (cur_ts->next) cur_ts = cur_ts->next;
      cur_ts->next = ts;
      MF_Log::mf_log(MF_DEBUG, "ReliabTrans::insertTimerData, dstseq present, so append new entry to linked list"); 
    }
  }
}

timerdata_s* MF_ReliabTransport::getTimerdata(pair<u_int, u_int> &dstseq, u_char timeout_type) {
  map<pair<u_int, u_int>, timerdata_s*>::iterator it = _dstseq_to_timerdata.find(dstseq);

  if (it == _dstseq_to_timerdata.end()) {
    MF_Log::mf_log(MF_ERROR, "ReliabTrans::getTimerdata(), timerdata not found!!!");
    return NULL;
  }

  timerdata_s *td = _dstseq_to_timerdata[dstseq];
  while (td) {
    if (td->timeout_type == timeout_type) return td;
    td = td->next;
  }
  return NULL;
}

u_char MF_ReliabTransport::computeCongFlag(u_int dstGUID, u_int chk_size) {
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::computeCongFlag()");
  //u_int cur_num_pkts = mBuffer->getSize(), capacity = mBuffer->getTotalSizePackets();
  u_int cur_num_pkts = recvBuffer->getSize(), capacity = recvBuffer->getTotalSizePackets();
  double occu = static_cast<double>(cur_num_pkts/capacity) * 100;

  u_char cong_flag = R_STP;
  if (occu < E2E_RDEC_THRESH) {
    cong_flag = R_INC;
  } else if (occu < E2E_RSTP_THRESH) {
    cong_flag = R_DEC;
  }
  return cong_flag;
}

// for now timerdata
void MF_ReliabTransport::OnTimeout(void *timerdata, unsigned int id) {
  timerdata_s *td = static_cast<timerdata_s*>(timerdata);

  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::OnTimeout(), timer with dst: %u, seq: %u, type: %u, id: %u",
                  td->dstseq.first, td->dstseq.second, td->timeout_type, td->timer_id);

  MF_EvTransTOut *e = new MF_EvTransTOut(this, td->timeout_type, td->dstseq);
  _system->getEventQueue()->add(e);
}

void MF_ReliabTransport::handleRecvNACKTOut(pair<u_int, u_int> dstseq) {
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::handleRecvNACKTOut()");
  map<pair<u_int, u_int>, MF_Message*>::iterator it = _dstseq_to_msgs.find(dstseq);
  if (it == _dstseq_to_msgs.end()) {
    MF_Log::mf_log(MF_ERROR, "ReliabTrans::handleRecvNACKTOut(), msg not found, exiting");  
    exit(-1);
  }
  MF_Message *msg = _dstseq_to_msgs[dstseq];
  // check if this message has completed or not. If it has, just clean up and return  
  if(msg->isComplete()) {
    cancelNACKTimer(dstseq, RECV_NACK_TIMEOUT_T);
    return;
  }

  // else, send NACK packet to src, and reset nack timer
  sendNACK(dstseq); 
}

void MF_ReliabTransport::handleFCtrlTOut(pair<u_int, u_int> dstseq) {

}

void MF_ReliabTransport::sendNACK(pair<u_int, u_int> dstseq) {
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::sendNACK()");
  MF_Message *msg = _dstseq_to_msgs[dstseq];
  vector<u_char *> *pList = new vector<u_char*>();

  // calculate NACK message size
  u_short num_ack_chk = msg->getTotal();
  u_int hdr_size = decideNACKHdrSize(num_ack_chk);

  
  MF_Log::mf_log(MF_INFO, ",SEND_NACK,%u,%u", dstseq.first, dstseq.second);
  //if(mBuffer->getVectorBySize(pList, MF_PacketSupport::HIGH_HEADER_LEN + 6 + 
  if(sendBuffer->getVectorBySize(pList, MF_PacketSupport::HIGH_HEADER_LEN + 6 + 
      ((num_ack_chk-1)/8 + 1)) < 0) {
    MF_Log::mf_log(MF_WARN, "MF_ReliabTransport::sendNACK() no space available in buffer");
    return;
  }

  TransHeader *thdr = TransHeaderFactory::newTransHeader(
      MF_PacketSupport::getTransportHeaderPtr(pList->front(), true), E2E_NACK_T);

  // needs to change start/end offset
  u_short start_offset = 0, end_offset = 0;

  u_int chk_size = 6 + ((num_ack_chk-1)/8 + 1); // currently this is set to be TotalTransHdrSize - TransBaseHdrSize
  u_int chk_pkt_cnt = 1; 
  u_int seq = getNextSeq(dstseq.first);
  //thdr->fillBaseTransHdr(seq, chk_size, chk_pkt_cnt, start_offset, end_offset, 
  //    E2E_NACK_T, R_NOFEEDBACK, PREF_DONT_CARE, hdr_size);
  thdr->setSeq(seq).
        setChkSize(chk_size).
        setPktCnt(chk_pkt_cnt).
        setStartOffset(start_offset).
        setEndOffset(end_offset).
        setTransType(E2E_NACK_T).
        setTransFlag(NO_REQUEST).
        setReliabPref(PREF_DONT_CARE).
        setTransOffset(hdr_size).
        setCongNotif(R_NOFEEDBACK).
        setRecvWd(E2E_RECVWD);

  // fill NACK part of Trans header
  TransAckHeader *athdr = (TransAckHeader *)(thdr);
  athdr->setTransAckSeq(msg->getStartSeq());
  athdr->setNumAckChk(num_ack_chk);
  msg->copyBitmap(athdr->getBitmapPtr()); 

  // Debug use only: print bitmap
  char debug_buf[400];
  memset(debug_buf, 0, 400);
  for (int i=0; i<(num_ack_chk-1)/8 + 1; ++i) {
    sprintf(debug_buf+i, "%X", *(athdr->getBitmapPtr()+i));
  }
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::sendNACK() NACK message with start seq: %u, num_ack_chk: %u,"
                  " bitmap in sent NACK packet: %s", msg->getStartSeq(), num_ack_chk, debug_buf);
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::sendNACK() NACK message with start seq: %u, num_ack_chk: %u,"
                  " bitmap in sent NACK packet: %s", athdr->getTransAckSeq(), athdr->getNumAckChk(), debug_buf);
  memset(debug_buf, 0, 200);
  MF_Log::mf_log(MF_DEBUG, "%s", thdr->to_log(debug_buf));
    
  delete thdr;

  // push down
  MF_ChunkInfo *ci = new MF_ChunkInfo();

  // exactly what does msg id mean?
  fillChunkInfo(ci, dstseq.first, start_offset, end_offset, chk_size, 1, seq, seq-0, false); 
  ci->getPacketList()->push_back(pList->at(0));

  MF_EvDownTransp *erc = new MF_EvDownTransp(ci);
  _system->getEventQueue()->add(erc);
}

//u_int MF_ReliabTransport::decideNACKHdrSize(u_int num_ack_chk) {
//  return TRANS_BASE_HEADER_SIZE + 6 + (num_ack_chk/8 + 1);
//}
void MF_ReliabTransport::sendPushSyn(u_int storer, u_int orig_dst, vector<u_int> &seqs_stored) {
  u_int num_chks_to_syn = seqs_stored.size();
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::sendPushSyn() to storer: %u, num_to_syn: %u", storer, num_chks_to_syn);

  vector<u_char *> *pList = new vector<u_char*>();
  u_int trans_hdr_size = decidePushSynHdrSize(num_chks_to_syn);
  //if(mBuffer->getVectorBySize(pList, MF_PacketSupport::HIGH_HEADER_LEN + 
  if(sendBuffer->getVectorBySize(pList, MF_PacketSupport::HIGH_HEADER_LEN + 
      (trans_hdr_size - TRANS_BASE_HEADER_SIZE)) < 0) { // as high_hdr_len includes base_hdr_size
    MF_Log::mf_log(MF_WARN, "MF_ReliabTransport::sendPushSyn() no space available in buffer");
    return;
  }

  TransHeader *thdr = TransHeaderFactory::newTransHeader(
      MF_PacketSupport::getTransportHeaderPtr(pList->front(), true), PUSH_SYN_T);

  u_short start_offset = 0, end_offset = 0;
  u_int seq = getNextSeq(storer); 
  u_int chk_size = trans_hdr_size - TRANS_BASE_HEADER_SIZE;
  u_int chk_pkt_cnt = 1;

  thdr->setSeq(seq).
        setChkSize(chk_size).
        setPktCnt(chk_pkt_cnt).
        setStartOffset(start_offset).
        setEndOffset(end_offset).
        setTransType(PUSH_SYN_T).
        setTransFlag(NO_REQUEST).
        setReliabPref(PREF_DONT_CARE).
        setTransOffset(trans_hdr_size).
        setCongNotif(R_NOFEEDBACK).
        setRecvWd(E2E_RECVWD);

  pair<u_int, u_int> orig_dstseq(orig_dst, seqs_stored[0]);
  stored_chk_s *stored_chk = _stored_chks[orig_dstseq];
  assert(stored_chk->dst_guid == orig_dst);
  TransStoreHeader *push_thdr = (TransStoreHeader *)(thdr);

  // TODO: THIS IS A COMPLETE HACK, BUT SOCKMAN IS NOT RETURNING THE CORRECT GUID, THEREFORE...
  //u_int my_guid = 1;

  push_thdr->setStoreSrcGUID(getMyGUID()).
            setStoreDstGUID(stored_chk->dst_guid).
            setNumStoredChk(num_chks_to_syn);
  u_int seqs[num_chks_to_syn];
  memset(seqs, 0, sizeof(u_int)*num_chks_to_syn);
  std::copy(seqs_stored.begin(), seqs_stored.end(), seqs);
  push_thdr->setStoredSeqs(seqs, num_chks_to_syn);

  // debug use
  char debug_buf[400];
  memset(debug_buf, 0, 400);
  
  for (u_int i=0; i<num_chks_to_syn; ++i) {
    sprintf(debug_buf+i, "%u ", seqs[i]);
  } 
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::sendPushSyn() PushSyn message with src: %u, dst: %u, storer: %u,"
                  " seqs: %s", push_thdr->getUintStoreSrcGUID(), push_thdr->getUintStoreDstGUID(), 
                  stored_chk->storer_guid, debug_buf);
  memset(debug_buf, 0, 400);
  MF_Log::mf_log(MF_DEBUG, "%s", thdr->to_log(debug_buf));
  // end debug prints

  // push down
  MF_ChunkInfo *ci = new MF_ChunkInfo();
  // exactly what does msg id mean?
  fillChunkInfo(ci, storer, start_offset, end_offset, chk_size, chk_pkt_cnt, seq, seq-0, true);
  ci->getPacketList()->push_back(pList->at(0));

  MF_EvDownTransp *erc = new MF_EvDownTransp(ci);
  _system->getEventQueue()->add(erc);
}

// this is called when lower layer pushed out a chunk.
// not really releasing the chunk, it holds it 
int MF_ReliabTransport::releaseChunk(MF_ChunkInfo *ci) {
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::releaseChunk()");
 
  // figure out end-to-end requirement for this chunk: NACK, ACK or DONT_CARE?
  // TODO: this info could be cached
  vector<u_char*> *pList = ci->getPacketList();
  TransHeader *thdr = TransHeaderFactory::newTransHeader(
      MF_PacketSupport::getTransportHeaderPtr((*pList)[0], true));
  u_char reliab_pref = thdr->getReliabPref();
  delete thdr;

  if (reliab_pref == PREF_NACK) {
    pair<u_int, u_int> dstseq = getDstSeqFromChk(ci);
    map<pair<u_int, u_int>, MF_ChunkInfo*>::iterator it = _unacked_chks.find(dstseq);
    if (it != _unacked_chks.end()) {
      MF_Log::mf_log(MF_ERROR, "error");
      return -1;
    }
    _unacked_chks[dstseq] = ci;
    MF_Log::mf_log(MF_DEBUG, "ReliabTrans::releaseChunk(), inserted chk with dst: %u, seq: %u, into unacked_chks",
                    dstseq.first, dstseq.second);
  } else if (reliab_pref == PREF_DONT_CARE) {
    // release memory immediately
    //mBuffer->putVector(pList);
    sendBuffer->putVector(pList);
    delete ci;
    ci = NULL;

    MF_EvSpaceAvail *event = new MF_EvSpaceAvail(mTID);
    _system->getEventQueue()->add(event);
    
  }

  // assuming if nothing wrong we should return 0?
  return 0;
}

void MF_ReliabTransport::reTxBasedonBitmap(TransAckHeader *nack, u_int dst_guid) {
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::reTxBasedonBitmap()");
  u_int start_seq = nack->getTransAckSeq();
  u_short num_chks = nack->getNumAckChk();
  const u_char *bitmap = nack->getBitmapPtr(); 

  pair<u_int, u_int> dstseq(dst_guid, start_seq);

  // Debug use only: print bitmap
  char debug_buf[400];
  memset(debug_buf, 0, 400);
  MF_Log::mf_log(MF_DEBUG, "%s", nack->to_log(debug_buf));
  memset(debug_buf, 0, 400);
  for (u_short i=0; i<(num_chks-1)/8 + 1; ++i) {
    sprintf(debug_buf+i, "%X", *(bitmap+i));
  }
  MF_Log::mf_log(MF_DEBUG, "ReliabTrans::reTxBasedonBitmap() NACK message with start seq: %u, num_ack_chk: %u,"
                  " bitmap in sent NACK packet: %s", start_seq, num_chks, debug_buf);

  int num_local_retx = 0;
  vector<u_int> seqs_stored;
  for (u_int i=0; i<num_chks; ++i) {
    if (*(bitmap+i/8) & (0x01 << i%8)) {
      MF_Log::mf_log(MF_DEBUG, "ReliabTrans::reTxBasedonBitmap(), chk with seq: %u, dst: %u, doesn't need reTx",
                      start_seq+i, dst_guid); 
    } else { // need to reTx
      dstseq.second = start_seq + i; 
      
      // first see if there's a copy stored in the network 
      if (_stored_chks.find(dstseq) != _stored_chks.end()) {
        seqs_stored.push_back(dstseq.second); 
      
      } else {

        // if _unacked doesn't have this chunk, it means sender timeout happened and the NACK is recvd too late.
        // thus won't reTx
        map<pair<u_int, u_int>, MF_ChunkInfo*>::iterator it = _unacked_chks.find(dstseq);
        if (it == _unacked_chks.end()) {
          MF_Log::mf_log(MF_ERROR, "ReliabTrans::reTxBasedonBitmap(), chk to reTx not found!!!");
        } else {
          MF_ChunkInfo *ci = it->second;
          MF_EvDownTransp *erc = new MF_EvDownTransp(ci);
          _system->getEventQueue()->add(erc);
          num_local_retx++;
        }
      } //  if (_stored_chks.find(dstseq) != _stored_chks.end()) 
    } // if need reTx
  } // for every chk 
  
  if (!seqs_stored.empty()) {
    // assuming all the seqs are stored in a single location
    pair<u_int, u_int> dst_storedseq(dstseq.first, seqs_stored[0]);
    stored_chk_s *stored_chk = _stored_chks[dst_storedseq]; 

    // create a 'push' message, and send it to corresponding router
    sendPushSyn(stored_chk->storer_guid, dstseq.first, seqs_stored);
  }

  // Need to reset the sender_NACK_timer
  dstseq.second = start_seq;
  timerdata_s *td = getTimerdata(dstseq, SEND_NACK_TIMEOUT_T);
  if (!td) {
    MF_Log::mf_log(MF_ERROR, "ReliabTrans::reTxBasedonBitmap(), timerdata not found!!!");
    exit(-1);
  }
  _system->getTimeoutManager()->ClearTimeout(td->timer_id);
  u_int new_tid = _system->getTimeoutManager()->AddTimeout(this, td, _send_NACK_timeout*num_chks);
  td->timer_id = new_tid;

}

void MF_ReliabTransport::fillChunkInfo(MF_ChunkInfo *ci, u_int dstGUID, u_short start_offset,
      u_short end_offset, u_int chk_size, u_int chk_pkt_cnt, u_int seq, u_int msg_id,
      bool toRouter) {
  ci->putSrcTID(mTID);
  ci->putSrcGUID(getMyGUID());
  ci->putDstGUID(dstGUID);
  ci->putStartOffset(start_offset);
  ci->putEndOffset(end_offset);

  ci->putChunkSize(chk_size);
  ci->putChunkPktCnt(chk_pkt_cnt);
  ci->putChunkID(seq);
  ci->putServiceID(0);
  ci->setOwner(mTID);
  ci->setOpts(0);
  ci->putMsgID(seq);
  if (toRouter) ci->putDstNA(dstGUID);
}

