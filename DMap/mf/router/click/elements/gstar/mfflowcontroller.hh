#ifndef MF_FLOWCONTROLLER_HH_
#define MF_FLOWCONTROLLER_HH_

#include <click/element.hh>
#include <click/vector.hh>
#include "mf.hh"

#include "mftransstoragemgr.hh"

CLICK_DECLS

class Chunk;
class MF_Logger;

class MF_FlowController : public Element {
public:
  MF_FlowController();
  ~MF_FlowController();
  const char *class_name() const    { return "MF_FlowController"; }
  const char *port_count() const    { return "0/0"; }
  const char *processing() const    { return AGNOSTIC; }
  int configure(Vector<String>&, ErrorHandler *);

  uint8_t getCongFlag(Chunk &chk, Pair<uint32_t, uint32_t> &sd_pair, Packet &p);

private:
  MF_Logger _logger;  
  MF_StorageManager *_stg_mgr;
  MF_ChunkManager *_chk_mgr;

  typedef HashTable<uint32_t, dst_ctrl_s*> DstToCtrl;
  DstToCtrl _dst_to_ctrl;

};

CLICK_ENDDECLS
#endif /* MF_TRANSPROXY_HH_ */
