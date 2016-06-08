#include <include/mftransportheader.hh>
#include "mfflowcontroller.h"
#include "mfreliabtransport.h"

// base class
MF_FlowController::MF_FlowController() {
}

MF_FlowController::~MF_FlowController() {
}

// MF_HighestRateController
void MF_HighestRateController::sendData(MF_ReliabTransport *rtransp, MF_EventQueue *eq, u_int dstGUID) {
  // basically need the sendImmediately() function, plus timer insertion
  list<MF_ChunkInfo*> *chk_list = rtransp->queryDstToChks(dstGUID);
  if (!chk_list) { 
    MF_Log::mf_log(MF_ERROR, "HighestRateController::sendData(): dstGUID not found in _dst_to_chks\n");
    return;
  }
  while (!chk_list->empty()) {
    MF_ChunkInfo *ci = chk_list->front();
    MF_EvDownTransp *erc = new MF_EvDownTransp(ci);
    eq->add(erc);
    chk_list->pop_front();
  }
  delete chk_list;
  rtransp->eraseDstToChks(dstGUID); 
}
void MF_HighestRateController::handleInDataFlag(MF_ReliabTransport *rtransp, TransHeader *thdr,
        MF_EventQueue *eq, u_int dstGUID) { }
void MF_HighestRateController::handleFlowCtrlNotif(MF_ReliabTransport *rtransp, TransHeader *thdr,
        MF_EventQueue *eq, u_int dstGUID) { }


// MF_WindowController
// assuming MF_Buffer has a function to return the remaining number of MB available in buffer
void MF_WindowController::sendData(MF_ReliabTransport *rtransp, MF_EventQueue *eq, u_int dstGUID) {
  MF_Log::mf_log(MF_DEBUG, "WindowController::sendData()");
  // find out what the recv buff is first
  dst_info_s *dst = rtransp->queryGUIDToDst(dstGUID);
  list<MF_ChunkInfo*> *chk_list = rtransp->queryDstToChks(dstGUID);
  if (!dst || !chk_list) { 
    MF_Log::mf_log(MF_ERROR, "WindowController::sendData(): dstGUID not found in _dst_to_chks\n");
    return;
  }
  
  u_int recv_wd = dst->recv_wd;
  u_int i = 0;
  while (i<recv_wd && !chk_list->empty()) {
    MF_ChunkInfo *ci = chk_list->front();
    chk_list->pop_front();
    if (i == recv_wd - 1) {
      // need to mark the last chunk to request window update
      TransHeader *thdr = TransHeaderFactory::newTransHeader(
          MF_PacketSupport::getTransportHeaderPtr(ci->getPacketList()->front(), true));
      thdr->setTransFlag(REQUEST_UPDATE); // ask for recv window update
      delete thdr;
    }
    MF_EvDownTransp *erc = new MF_EvDownTransp(ci);
    eq->add(erc);
    ++i;
    MF_Log::mf_log(MF_DEBUG, "WindowController::sendData(): pushed down one chunk");
  }
  if (chk_list->empty()) {
    delete chk_list;
    rtransp->eraseDstToChks(dstGUID);
  }
}

void MF_WindowController::handleInDataFlag(MF_ReliabTransport *rtransp, TransHeader *thdr,
        MF_EventQueue *eq, u_int dstGUID) {
  MF_Log::mf_log(MF_DEBUG, "WindowController::handleInDataFlag()");
  if (thdr->getTransFlag() != REQUEST_UPDATE) return; 

  vector<u_char*> *pList = new vector<u_char*>(); // who's de-allocating this piece of memory
  if(rtransp->getSendBuffer()->getVectorBySize(pList, MF_PacketSupport::HIGH_HEADER_LEN) < 0) {
    MF_Log::mf_log(MF_WARN, "WindowController::handleInDataFlag() no space available in buffer");
    return;
  }
  TransHeader *mythdr = TransHeaderFactory::newTransHeader(
      MF_PacketSupport::getTransportHeaderPtr(pList->front(), true), DATA_T);
  //mythdr->fillBaseTransHdr(seq, chk_size, chk_pkt_cnt, start_offset, end_offset,
  //    type, flag, pref, hdr_size); 
  u_short start_offset = 0;
  u_short end_offset = 0;
  u_int chk_size = 0;
  u_int chk_pkt_cnt = 1;
  u_int seq = rtransp->getNextSeq(dstGUID);
  mythdr->setSeq(seq).
          setChkSize(chk_size).
          setPktCnt(chk_pkt_cnt).
          setStartOffset(start_offset).
          setEndOffset(end_offset).
          setTransType(DATA_T).
          setTransFlag(NO_REQUEST).
          setReliabPref(PREF_ACK).
          setTransOffset(TRANS_BASE_HEADER_SIZE).
          setCongNotif(R_NOCHANGE). //// decide based on current buffer status, or just assume it's empty?
          setRecvWd(E2E_RECVWD);
  
  delete mythdr;
  MF_ChunkInfo *ci = new MF_ChunkInfo();
  rtransp->fillChunkInfo(ci, dstGUID, start_offset, end_offset, chk_size, 
      chk_pkt_cnt, seq, seq, false); // is message id used any where?
  ci->getPacketList()->push_back(pList->at(0));

  MF_EvDownTransp *erc = new MF_EvDownTransp(ci);
  eq->add(erc);

}

void MF_WindowController::handleFlowCtrlNotif(MF_ReliabTransport *rtransp, TransHeader *thdr,
        MF_EventQueue *eq, u_int dstGUID) {
  MF_Log::mf_log(MF_DEBUG, "WindowController::handleFlowCtrlNotif() ");
  if (thdr->getCongNotif() == R_NOFEEDBACK) return;
  // update rcwd for this dst
  dst_info_s *dst = rtransp->queryGUIDToDst(dstGUID); 
  if (!dst) return;

  dst->recv_wd = thdr->getRecvWd();
  sendData(rtransp, eq, dstGUID); 
}
