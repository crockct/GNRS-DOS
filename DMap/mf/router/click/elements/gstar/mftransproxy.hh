#ifndef MF_TRANSPROXY_HH_
#define MF_TRANSPROXY_HH_

#include <click/element.hh>
#include <click/hashtable.hh>
#include "mftransportheader.hh"
#include "mfchunk.hh"
#include "mftransstoragemgr.hh"
#include "mfflowcontroller.hh"


class MF_ChunkManager;
class Chunk;
struct stg_timer_data_s;

CLICK_DECLS

const uint32_t TRANS_CONTROL_MSG_OUTPORT = 0;
const uint32_t TRANS_DATA_MSG_OUTPORT = 1;

class MF_TransProxy : public Element {
public:
  MF_TransProxy();
  ~MF_TransProxy();
  const char *class_name() const    { return "MF_TransProxy"; }

  // Incoming ports: 0 - regular up-stack msg (just to hand chunk to transport for further processing)
  //  from IntraLookup, 1 - stored chunk pushed-out success notification from IntraLkup;
  // Outgoing port: 0 - transport layer generated msg ("store"/"e2e ack") to IntraLkup for sending;
  // 1 - stored chunk internal msg after timeout for binding
  // prx[0] --> [1]intra_lkup; prx[1] --> [2]intra_lkup
  // intra_lkup[1] --> prx[0]; intra_lkup[2] --> prx[1].
  const char *port_count() const    { return "2/2"; }
  const char *processing() const    { return "h/h"; }
  int configure(Vector<String>&, ErrorHandler *);

  void push(int port, Packet *p);

  static void handleStgExpiry(Timer *, void *data);
  void pushChk(uint32_t chk_id);

  void handleTransData(Chunk *chk, Packet *p);
  void handleTransPush(Chunk *chk);
  //void handleTransAck(Chunk *chk);

  void handleIntraSuccessPkt(Chunk *chk, uint32_t chk_ID);

  void sendStoreMsg(Chunk &chk, uint8_t cong_flag = R_NOCHANGE);
  //void sendDropMsg(Chunk &chk, uint8_t cong_flag = R_NOCHANGE);

  void destroyChunk(Chunk *chk);


  uint32_t getSeqForGUID(uint32_t guid);

private:
  typedef HashTable<uint32_t, Pair<Chunk*, Packet*> > IDToChks;
  IDToChks _id_to_chks;

  // manages outgoing seq numbers to dst with certain GUID
  typedef HashTable<uint32_t, uint32_t> GUIDToSeq;
  GUIDToSeq _guid_to_seq; 

  MF_Logger _logger;  
  MF_StorageManager *_stg_mgr;
  MF_FlowController *_fctrl;
  MF_ChunkManager *_chk_mgr;

  uint32_t _my_GUID;

  WritablePacket* createPkt(int pkt_size);
  WritablePacket* createInternalMsgPkt(Chunk &chk);
  void fillHopHdr(WritablePacket &p, uint32_t pld_size);
  void fillRoutingHdr(Chunk &chk, WritablePacket &p);
  TransHeader* fillTransHdr(Chunk &chk, WritablePacket &p, uint8_t type,
    uint32_t trans_hdr_size);
};

CLICK_ENDDECLS
#endif /* MF_TRANSPROXY_HH_ */
