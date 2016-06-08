#ifndef MF_RELIABTRANSPORT_H
#define MF_RELIABTRANSPORT_H

#include <time.h>
#include <limits>
#include "mfsystem.h"
#include "mftimeoutmanager.h"
#include "mftimeoutlistener.h"
#include "mftransport.h"
#include "mfsettings.h"
//#include "mfflowcontroller.h"

class MF_BasicTransport;
class MF_TimeoutManager;
class MF_FlowController;
//class MFTimeoutListenerInterface;

//const u_int CHK_COUNT_NACK_THRESH = 1;
//const long int SEND_NACK_TIMEOUT_MSEC_PER_CHK = 2500;
//const long int RECV_NACK_TIMEOUT_MSEC_PER_CHK = 2000;

// types of timeouts
const u_char RECV_NACK_TIMEOUT_T = 1;
const u_char SEND_NACK_TIMEOUT_T = 2;
const u_char FLOW_CTRL_TIMEOUT_T = 3;

// flow ctrl related constants
const u_int E2E_RSTP_THRESH = 80;
const u_int E2E_RDEC_THRESH = 50;
const u_int E2E_RECVWD = 10; // 10MB to begin with

const u_int MAX_CHK_PKT_CNT = MAX_CHUNK_SIZE;


struct dst_info_s {
  u_int guid;
  u_int last_seq;
  u_int last_acked_seq;

  // for flow control
  time_t last_tx_ts; // last Tx timestamp
  time_t last_chk_size;
  u_int cong_flag;
  u_int recv_wd; // remote side recv window

  dst_info_s(u_int g) : guid(g) {
    last_seq = std::numeric_limits<unsigned int>::max();
    last_acked_seq = last_seq;
  }
};


struct timerdata_s {
  u_int timer_id;
  u_char timeout_type;
  pair<u_int, u_int> dstseq;
  timerdata_s *next;
  timerdata_s(u_char type, pair<u_int, u_int> ds) : timeout_type(type),
      dstseq(ds), next(NULL) { }
};

struct stored_chk_s {
  u_int storer_guid;
  u_int dst_guid;
  u_int seq;
  stored_chk_s(u_int sg, u_int dg, u_int s) : storer_guid(sg), dst_guid(dg),
      seq(s) { }
};

class MF_ReliabTransport : public MF_BasicTransport, public MFTimeoutListenerInterface {
public:
  MF_ReliabTransport();
  //MF_ReliabTransport(u_int, MF_Buffer *, MF_EventQueue *);
  MF_ReliabTransport(u_int, MF_Buffer *recvBuffer, MF_Buffer *sendBuffer, MFSystem *_system);
  ~MF_ReliabTransport();

  // for sending
  void sendData(vector <u_char *> *data, u_int size, int srcGUID, int dstGUID, mfflag_t opts);
  u_int prepareChks(vector <u_char *> *data, u_int size, int srcGUID, 
        int dstGUID, mfflag_t opts, u_int num_chks, u_char trans_type, 
        u_char reliab_pref);
  void sendImmediately(u_int dstGUID);

  // for receiving
  void recvData(MF_ChunkInfo *);
  void handleTransData(MF_ChunkInfo *);
  void handleTransNACK(MF_ChunkInfo *);
  void handleTransACK(MF_ChunkInfo *) {}
  void handleTransStore(MF_ChunkInfo *);
  
  // timeout related
  virtual void OnTimeout(void *timerdata, unsigned int id);
  void handleSendNACKTOut(pair<u_int, u_int> dstseq);
  void handleRecvNACKTOut(pair<u_int, u_int> dstseq);
  void handleFCtrlTOut(pair<u_int, u_int> dstseq);

  inline void setTransSetting(TransportSetting ts) {
    _chk_count_NACK_thresh = ts.chkCountNACKThresh;
    _send_NACK_timeout = ts.sendNACKTOutPerChk; 
    _recv_NACK_timeout = ts.recvNACKTOutPerChk;
  }

  u_char computeCongFlag(u_int dstGUID, u_int chk_size);

  inline u_int getNextSeq(u_int dstGUID) {
    if (_guid_to_dst.find(dstGUID) == _guid_to_dst.end()) {
      dst_info_s *di = new dst_info_s(dstGUID);
      _guid_to_dst[dstGUID] = di;
      di->last_seq = 0;
      return 0;
    }

    u_int &seq = (_guid_to_dst[dstGUID])->last_seq;
    if (seq == std::numeric_limits<unsigned int>::max()) 
      seq = 0;
    else seq++;
    return seq;
  }

  inline pair<u_int, u_int> getDstSeqFromChk(MF_ChunkInfo *ci) {
    return pair<u_int, u_int>(ci->getDstGUID(), ci->getChunkID());
  }

  void cancelNACKTimer(pair<u_int, u_int>, u_char);

  void insertTimerdata(timerdata_s *ts, pair<u_int, u_int> &dstseq);
  timerdata_s* getTimerdata(pair<u_int, u_int> &dstseq, u_char timeout_type);


  // simple functions for deciding transport header len: 
  inline u_int decideNACKHdrSize(u_int num_ack_chk) {
    return TRANS_BASE_HEADER_SIZE + 6 + (num_ack_chk/8 + 1);
  }

  inline u_int decidePushSynHdrSize(u_int num_chks_to_syn) {
    return TRANS_BASE_HEADER_SIZE + 20*2 + 4 + num_chks_to_syn*4;
  }

  void fillChunkInfo(MF_ChunkInfo *ci, u_int dstGUID, u_short start_offset,
      u_short end_offset, u_int chk_size, u_int chk_pkt_cnt, u_int seq, 
      u_int msg_id, bool toRouter); 

  int releaseChunk(MF_ChunkInfo *);
  void reTxBasedonBitmap(TransAckHeader *, u_int);

  // for other classes to access this' data structures
  inline list<MF_ChunkInfo*>* queryDstToChks(u_int dstGUID) {
    if (_dst_to_chks.find(dstGUID) == _dst_to_chks.end()) return NULL;
    else return _dst_to_chks[dstGUID];
  }
  inline void eraseDstToChks(u_int dstGUID){
    _dst_to_chks.erase(dstGUID);
  }
  dst_info_s* queryGUIDToDst (u_int dstGUID) {
    if (_guid_to_dst.find(dstGUID) == _guid_to_dst.end()) return NULL;
    else return _guid_to_dst[dstGUID];
  }
  //inline MF_Buffer* getBuffer() { return mBuffer; }
  inline MF_Buffer* getSendBuffer() { return sendBuffer; }
  inline MF_Buffer* getRecvBuffer() { return recvBuffer; }

private:
  void sendNACK(pair<u_int, u_int> dstseq);
  void sendPushSyn(u_int storer, u_int orig_dst, vector<u_int> &seqs_stored);

  MF_FlowController *_fctrl;
  map<u_int, dst_info_s*> _guid_to_dst;
  map<u_int, list<MF_ChunkInfo*>*> _dst_to_chks;

  // sender side data structures
  // <dstGUID, seq> --> ChunkInfo, chunks to be pushed down but held temporarily 
  // because of flow control
  //map<pair<u_int, u_int>, MF_ChunkInfo*> _pending_chks;
  
  // thse chunks have been pushed out by lower layer, but need to hold for reTx
  map<pair<u_int, u_int>, MF_ChunkInfo*> _unacked_chks; 
  // these chunks have been stored in intermediate routers
  map<pair<u_int, u_int>, stored_chk_s*> _stored_chks; 
   

  // internal data structure to manage different 'messages'
  // pair<dstGUID, msg_start_seq> --> msg
  map<pair<u_int, u_int>, MF_Message*> _dstseq_to_msgs;
  map<pair<u_int, u_int>, timerdata_s*> _dstseq_to_timerdata;
  u_int _send_NACK_timeout;
  u_int _recv_NACK_timeout;
  u_int _chk_count_NACK_thresh;
  void test();
};

#endif
