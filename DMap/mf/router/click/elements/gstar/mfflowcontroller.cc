#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include "mfflowcontroller.hh"
#include "mftransportheader.hh"

CLICK_DECLS

MF_FlowController::MF_FlowController() : _logger(MF_Logger::init()) {  
}

MF_FlowController::~MF_FlowController() {
}

int MF_FlowController::configure(Vector<String> &conf, ErrorHandler *errh)
{
  if (cp_va_kparse(conf, this, errh,
      "STORAGE_MANAGER", cpkP+cpkM, cpElement, &_stg_mgr,
    cpEnd) < 0)
    return -1;
  return 0;
}


uint8_t MF_FlowController::getCongFlag(Chunk &chk, 
    Pair<uint32_t, uint32_t> &sd_pair, Packet &p) {

  // need to check if cong control is needed for the src who sends the chunk for storage    
  
  // return value (score) from queryOccupancy() is in the range [0, 100] 
  uint8_t ocpc = _stg_mgr->queryFlowBufOccupancy(sd_pair);
  uint8_t cong_flag = R_STP;
  if (ocpc < E2E_RDEC_THRESH) {
    cong_flag = R_NOCHANGE;
  } else if (ocpc < E2E_RSTP_THRESH) {
    cong_flag = R_DEC;
  } 
  return cong_flag;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(MF_FlowController)
ELEMENT_REQUIRES(userlevel)
