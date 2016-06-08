#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/args.hh>
#include <clicknet/ether.h>

#include "mfpacketprint.hh"

MF_PacketPrint::MF_PacketPrint() 
    : offset(0), csyn(true), csyn_ack(true), data(false), link_probe(false), 
    internal_chk_msg(false), logger(MF_Logger::init()) {
}

MF_PacketPrint::~MF_PacketPrint(){

}; 

int MF_PacketPrint::configure(Vector<String> &conf, ErrorHandler *errh) {
  if (cp_va_kparse(conf, this, errh,
        "OFFSET", cpkP, cpUnsigned, &offset, 
        cpEnd) < 0) {
    return -1; 
  }
  return 0; 
}

void MF_PacketPrint::push(int port, Packet *p) {
  if (port != 0) {
    logger.log(MF_FATAL, "pktprint: Packet recvd on unsupported port");
    exit(-1);
  }
  //parse pkt type
  uint32_t type = ntohl(*(uint32_t*)(p->data() + offset));
  
  if (type == CSYN_PKT && csyn) {
    printCsyn(p);
  } else if (type == CSYN_ACK_PKT && csyn_ack) {
    printCsynACK(p);
  } else if (type == DATA_PKT && data) {
    printData(p); 
  } else {
    //logger.log(MF_DEBUG, "pktprint: type %u", type);
  } 
  /*if (type == LP_PKT) {
    link_probe_t *link_probe_pkt = 
                    (link_probe_t*)(p->data() + sizeof(click_ether));
    uint32_t sourceLP = link_probe_pkt->sourceLP;
    uint32_t destinationLP = link_probe_pkt->destinationLP;
    uint32_t seq = link_probe_pkt->seq; 
    logger.log(MF_DEBUG,
      "pktprint: link probe pkt is lost, src: %u, dst: %u, seq: %u",
      sourceLP, destinationLP, seq); 
  } else if (type == LP_ACK_PKT) {
    link_probe_ack_t *link_probe_ack_pkt = 
                        (link_probe_ack_t*)(p->data() + sizeof(click_ether));
r
    uint32_t sourceLP = ntohl(link_probe_ack_pkt->sourceLPACK);
    uint32_t destinationLP = ntohl(link_probe_ack_pkt->destinationLPACK);
    uint32_t seq = ntohl(link_probe_ack_pkt->seq_no_cp);
    logger.log(MF_DEBUG, 
      "pktprint: link probe ack is lost, src: %u, dst: %u, seq: %u", 
      sourceLP, destinationLP, seq); 
  } else if (type == DATA_PKT) {
    hop_data* data_pkt = (hop_data*)(p->data() + sizeof(click_ether));
    uint32_t hopID = ntohl(data_pkt->hop_ID);
    uint32_t seq = ntohl(data_pkt->seq_num);
    logger.log(MF_DEBUG,
      "pktprint: data pkt is lost, hopID: %u, seq_num:%u",
      hopID, seq);     
  }*/
  output(0).push(p);
}

void MF_PacketPrint::printCsyn(Packet *p) {
  csyn_t *csyn_pkt = (csyn_t*)(p->data() + offset);
  uint32_t hopID = ntohl(csyn_pkt->hop_ID);
  uint32_t pkt_cnt = ntohl(csyn_pkt->chk_pkt_count);
  logger.log(MF_TIME,
             "pkt_print: CSYN hopID: %u, pkt_cnt: %u", hopID, pkt_cnt); 
}

void MF_PacketPrint::printCsynACK(Packet *p) {
  csyn_ack_t *csyn_pkt = (csyn_ack_t*)(p->data() + offset);
  uint32_t hopID = ntohl(csyn_pkt->hop_ID);
  uint32_t pkt_cnt = ntohl(csyn_pkt->chk_pkt_count);
  logger.log(MF_TIME,
             "pkt_print: CSYN_ACK hopID: %u, pkt_cnt: %u, bit_map: ...", 
             hopID, pkt_cnt);
}

void MF_PacketPrint::printData(Packet *p) {
  hop_data_t *data_pkt = (hop_data_t*)(p->data() + offset);
  uint32_t seq = ntohl(data_pkt->seq_num); 
  uint32_t hopID = ntohl(data_pkt->hop_ID);
  logger.log(MF_TIME, 
             "pkt_print: DATA hopID: %u, seq#: %u", hopID, seq); 
}

void MF_PacketPrint::printLinkProbe(Packet *p) {
  
}

CLICK_DECLS
EXPORT_ELEMENT(MF_PacketPrint)
ELEMENT_REQUIRES(userlevel)
