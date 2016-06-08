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
#ifndef MF_TRANSPORTHEADER_HH
#define MF_TRANSPORTHEADER_HH

#include <string>
#include <iostream>
#include "mfguid.hh"
#include "mfheader.hh"


enum trsp_pkt_type {
  DATA_T = 0,
  E2E_ACK_T,
  E2E_NACK_T,
  PUSH_SYN_T,
  STORE_T,
};

enum reliab_pref {
  PREF_DONT_CARE = 0,
  PREF_NACK,
  PREF_ACK,
};

enum rcwd_update_request {
  NO_REQUEST,
  REQUEST_UPDATE,
};

enum cong_notif {
  R_NOCHANGE = 0,
  R_DEC,
  R_STP,
  R_INC,
  R_NOFEEDBACK,
};




const int TRANS_BASE_HEADER_SIZE = 24;

// for transport base header
const int TRANS_HEADER_SEQ_OFFSET = 0;
const int TRANS_HEADER_SIZE_OFFSET = 4;
const int TRANS_HEADER_PKTCNT_OFFSET = 8;
const int TRANS_HEADER_STARTOFF_OFFSET = 12;
const int TRANS_HEADER_ENDOFF_OFFSET = 14;
const int TRANS_HEADER_TYPEFLAG_OFFSET = 16;
const int TRANS_HEADER_RELIABPREF_OFFSET = 17;
const int TRANS_HEADER_OFF_OFFSET = 18;
const int TRANS_HEADER_CONGNOTIF_OFFSET = 20;
const int TRANS_HEADER_RECVWD_OFFSET = 22;

// for ack
const int TRANS_HEADER_ACKSEQ_OFFSET = TRANS_BASE_HEADER_SIZE;
const int TRANS_HEADER_NUMACKCHK_OFFSET = TRANS_BASE_HEADER_SIZE + 4;
const int TRANS_HEADER_BITMAP_OFFSET = TRANS_BASE_HEADER_SIZE + 6;

// for store/push-syn message
const int TRANS_HEADER_STORESRC_OFFSET = TRANS_BASE_HEADER_SIZE;
const int TRANS_HEADER_STOREDST_OFFSET = TRANS_BASE_HEADER_SIZE + 20;
const int TRANS_HEADER_NUMSTOREDCHK_OFFSET = TRANS_BASE_HEADER_SIZE + 40;
const int TRANS_HEADER_STOREDSEQS_OFFSET = TRANS_BASE_HEADER_SIZE + 44;


#define TRANS_HEADER_SIZE sizeof(transport_header)

// Old transport header used in mfchunksink.cc, muchunksource.cc, mftofile.cc, mffromfile.cc
//size 28
typedef struct transport_header {
	u_int chunkID;
	u_int chunkSize;
	u_int chunkPktCnt;
	u_int srcTID;
	u_int dstTID;
	u_int msgID;
	u_short msgNum;
	u_short offset;
} transport_header_t;

class TransHeader : public PacketHeader {
public: 
  TransHeader(const uint8_t *hdr) : PacketHeader(hdr) {
  }
  virtual ~TransHeader() {}

  virtual void init() {
    _chk_seq = (uint32_t *) (header + TRANS_HEADER_SEQ_OFFSET);
    _chk_size = (uint32_t *) (header + TRANS_HEADER_SIZE_OFFSET);
    _pkt_cnt = (uint32_t *) (header + TRANS_HEADER_PKTCNT_OFFSET);
    _start_offset = (uint16_t *) (header + TRANS_HEADER_STARTOFF_OFFSET);
    _end_offset = (uint16_t *) (header + TRANS_HEADER_ENDOFF_OFFSET);
    _type_and_flag = (uint8_t *) (header + TRANS_HEADER_TYPEFLAG_OFFSET);
    _reliab_pref = (uint8_t *) (header + TRANS_HEADER_RELIABPREF_OFFSET);
    _trans_offset = (uint16_t *) (header + TRANS_HEADER_OFF_OFFSET);
    _cong_notif = (uint16_t *) (header + TRANS_HEADER_CONGNOTIF_OFFSET);
    _recv_wd = (uint16_t *) (header + TRANS_HEADER_RECVWD_OFFSET); // in MB
  }
 
  TransHeader& setSeq(uint32_t s) { 
    *_chk_seq = htonl(s); 
    return *this;
  }
  uint32_t getSeq() { return ntohl(*_chk_seq); }
  static uint32_t getSeqStatic(const unsigned char *trans_layer_ptr) {
    uint32_t *seq_ptr = (uint32_t *)(trans_layer_ptr + TRANS_HEADER_SEQ_OFFSET);
    return ntohl(*seq_ptr);
  }

  TransHeader& setChkSize(uint32_t s) { 
    *_chk_size = htonl(s); 
    return *this;
  }
  uint32_t getChkSize() { return ntohl(*_chk_size); }

  TransHeader& setPktCnt(uint32_t s) { 
    *_pkt_cnt = htonl(s); 
    return *this;
  }
  uint32_t getPktCnt() { return ntohl(*_pkt_cnt); }

  TransHeader& setStartOffset(uint16_t s) { 
    *_start_offset = htons(s); 
    return *this;
  }
  uint16_t getStartOffset() { return ntohs(*_start_offset); }

  TransHeader& setEndOffset(uint16_t s) { 
    *_end_offset = htons(s); 
    return *this;
  }
  uint16_t getEndOffset() { return ntohs(*_end_offset); }

  static uint8_t getTransTypeStatic(const unsigned char *trans_layer) {
    uint8_t type_flag = *(trans_layer + TRANS_HEADER_TYPEFLAG_OFFSET);
    return (type_flag & 0xf0) >> 4;
  }

  // set 4 most significant bits of _type_flag
  TransHeader& setTransType(uint8_t s) { 
    s = (s & 0x0f) << 4;
    *_type_and_flag = (*_type_and_flag & 0x0f) | s;
    return *this;
  }
  uint8_t getTransType() { return (*_type_and_flag & 0xf0) >> 4; }
  
  TransHeader& setTransFlag(uint8_t s) { 
    s = (s & 0x0f);
    *_type_and_flag = (*_type_and_flag & 0xf0) | s;
    return *this;
  }
  uint8_t getTransFlag() { return (*_type_and_flag & 0x0f); }

  TransHeader& setReliabPref(uint8_t s) { 
    *_reliab_pref = s; 
    return *this;
  }
  uint8_t getReliabPref() { return *_reliab_pref; }

  TransHeader& setTransOffset(uint16_t s) { 
    *_trans_offset = htons(s); 
    return *this;
  }
  uint16_t getTransOffset() { return ntohs(*_trans_offset); }

  TransHeader& setCongNotif(uint16_t s) {  
    *_cong_notif = htons(s); 
    return *this;
  }
  uint16_t getCongNotif() { return ntohs(*_cong_notif); }

  TransHeader& setRecvWd(uint16_t s) { 
    *_recv_wd = htons(s); 
    return *this;
  }
  uint16_t getRecvWd() { return ntohs(*_recv_wd); }

  void fillBaseTransHdr(uint32_t seq, uint32_t size, uint32_t pkt_cnt, 
        uint16_t start_off, uint16_t end_off, uint8_t type, uint8_t flag, 
        uint8_t pref, uint16_t offset) {
    setSeq(seq);
    setChkSize(size);
    setPktCnt(pkt_cnt);
    setStartOffset(start_off);
    setEndOffset(end_off);
    setTransType(type);
    setTransFlag(flag);
    setReliabPref(pref);
    setTransOffset(offset);
  }

  inline char* to_log(char *buf) {
    u_char type = getTransType(), reliab_pref = getReliabPref();
    u_char update_req = getTransFlag();
    u_short cong_notif = getCongNotif();

    std::string type_s; 
    std::string req_s;
    std::string pref_s;
    std::string cong_s;

    switch (type) {
      case DATA_T:
        type_s = "DATA_T";
        break;
      case E2E_ACK_T:
        type_s = "E2E_ACK_T";
        break;
      case E2E_NACK_T:
        type_s = "E2E_NACK_T";
        break;
      case PUSH_SYN_T:
        type_s = "PUSH_SYN_T";
        break;
      case STORE_T:
        type_s = "STORE_T";
        break;
      default:
        std::cout << "TransHdr::to_log: Type ERROR. Exiting." << std::endl;
        exit(-1);
    }

    // cong flag print is yet to be implemented.
    switch (update_req) {
      case NO_REQUEST: 
        req_s = "NO_REQUEST";
        break;
      case REQUEST_UPDATE:
        req_s = "REQUEST_UPDATE";
        break;
      default:
        std::cout << "TransHdr::to_log: Request ERROR. Exiting." << std::endl;
        exit(-1);
    }
        
    switch (cong_notif) {
      case R_NOCHANGE:
        cong_s = "R_NOCHANGE";
        break;
      case R_DEC:
        cong_s = "R_DEC";
        break;
      case R_STP:
        cong_s = "R_STP";
        break;
      case R_INC:
        cong_s = "R_INC";
        break;
      case R_NOFEEDBACK:
        cong_s = "R_NOFEEDBACK";
        break;
      default:
        std::cout << "TransHdr::to_log: CongNotif ERROR. Exiting." << std::endl;
        exit(-1);
      
    }

    switch (reliab_pref) {
      case PREF_DONT_CARE:
        pref_s = "PREF_DONT_CARE";
        break;
      case PREF_NACK:
        pref_s = "PREF_NACK";
        break;
      case PREF_ACK:
        pref_s = "PREF_ACK";
        break;
      default:
        std::cout << "TransHdr::to_log: ReliabPref ERROR. Exiting." << std::endl;
        exit(-1);
    }

    sprintf(buf, "\n----TransHdr::to_log: seq: %u, chk_size: %u, pkt_cnt: %u," 
            "start_off: %u, end_off: %u,\n----type: %s, buf_update_req: %s, reliab_pref: %s,"
            "trans_offset: %u,\n----cong_notif: %s, recv_wd: %d\n", getSeq(), getChkSize(), getPktCnt(), getStartOffset(),
            getEndOffset(), type_s.c_str(), req_s.c_str(), pref_s.c_str(), getTransOffset(), cong_s.c_str(), getRecvWd());

    return buf;
  }

  // 
  //virtual void setTransAckSeq(uint32_t) { }
  //virtual uint32_t getTransAckSeq()  { return 0; }

  //virtual void setNumAckChk(uint32_t) { }
  //virtual uint32_t getNumAckChk() { return 0; }

  //This should not be here if there is a subclass; generates lot of warnings
  /*virtual void setStoreSrcGUID(const GUID &) { }
  virtual GUID& getStoreSrcGUID() { }

  virtual void setStoreDstGUID(const GUID &) { }
  virtual GUID& getStoreDstGUID() { }*/

  //virtual void setNumStoredChk(const uint32_t) { }
  //virtual uint32_t getNumStoredChk() { return 0; }

  //virtual void setStoredSeqs(const uint32_t *, const uint32_t) { }
  //virtual void getStoredSeqs(uint32_t s[], const uint32_t) { } 

private: 
  virtual void parseHeader() {}
  uint32_t *_chk_seq;
  uint32_t *_chk_size;
  uint32_t *_pkt_cnt;
  uint16_t *_start_offset;
  uint16_t *_end_offset;
  uint8_t *_type_and_flag; // MSB side 4 bits are for type, LSB side 4 bits for flag
  uint8_t *_reliab_pref;
  uint16_t *_trans_offset;
  uint16_t *_cong_notif;
  uint16_t *_recv_wd; // in MB
};

class TransAckHeader : public TransHeader {
public:
  TransAckHeader(const uint8_t *hdr) : TransHeader(hdr) {
  }

  virtual void init() {
    TransHeader::init(); 
    _ack_seq = (uint32_t *) (header + TRANS_HEADER_ACKSEQ_OFFSET); 
    _num_ack_chk = (uint16_t *) (header + TRANS_HEADER_NUMACKCHK_OFFSET); 
    _ack_bitmap = (uint8_t *) (header + TRANS_HEADER_BITMAP_OFFSET); 
  }

  TransAckHeader& setTransAckSeq(uint32_t s) { 
    *_ack_seq = htonl(s); 
    return *this;
  }
  uint32_t getTransAckSeq() { return ntohl(*_ack_seq); }

  TransAckHeader& setNumAckChk(uint16_t s) { 
    *_num_ack_chk = htons(s); 
    return *this;
  }
  uint16_t getNumAckChk() { return ntohs(*_num_ack_chk); }

  uint8_t* getBitmapPtr() { return _ack_bitmap; }

  void fillNACKHdr(uint32_t ack_seq, uint32_t num_chk, char *bitmap) {
    setTransAckSeq(ack_seq);
    setNumAckChk(num_chk);  
    int num_char = num_chk / 8 + 1;
    memset(_ack_bitmap, 0, num_char);
    memcpy(_ack_bitmap, bitmap, num_char);
  }

private: 
  uint32_t *_ack_seq;
  uint16_t *_num_ack_chk;
  uint8_t *_ack_bitmap; 
};

// same header structore for both "push-syn" and "store" msg.
class TransStoreHeader : public TransHeader {
public:
  TransStoreHeader(const uint8_t *hdr) : TransHeader(hdr) {
  }

  virtual void init() {
    TransHeader::init();

    _store_src.init(header + TRANS_HEADER_STORESRC_OFFSET);
    _store_dst.init(header + TRANS_HEADER_STOREDST_OFFSET);

    _num_stored_chk = (uint32_t *) (header + TRANS_HEADER_NUMSTOREDCHK_OFFSET);
    _stored_seqs = (uint32_t *) (header + TRANS_HEADER_STOREDSEQS_OFFSET);
  }

  TransStoreHeader& setStoreSrcGUID(const GUID &g) { 
    _store_src = g; 
    return *this;
  }
  TransStoreHeader& setStoreSrcGUID(uint32_t intg) {
    GUID guid;
    guid.init(intg);
    _store_src = guid;
    return *this;
  }
  GUID& getStoreSrcGUID() { return _store_src; }
  uint32_t getUintStoreSrcGUID() { return _store_src.to_int(); }

  TransStoreHeader& setStoreDstGUID(const GUID &g) { 
    _store_dst = g; 
    return *this;
  }
  TransStoreHeader& setStoreDstGUID(uint32_t intg) {
    GUID guid;
    guid.init(intg);
    
    _store_dst = guid;
    return *this;
  }
  GUID& getStoreDstGUID() { return _store_dst; }
  uint32_t getUintStoreDstGUID() { return _store_dst.to_int(); }

  TransStoreHeader& setNumStoredChk(const uint32_t n) { 
    *_num_stored_chk = htonl(n); 
    return *this;
  }
  uint32_t getNumStoredChk() { return ntohl(*_num_stored_chk); }

  TransStoreHeader& setStoredSeqs(const uint32_t *s, const uint32_t num_chk) { 
    if (num_chk != ntohl(*_num_stored_chk)) {
      //logger.log(MF_ERROR, "TransStoreHeader: num_stored_chk don't agree\n");
      exit(-1);
    }
    for (uint32_t i=0; i<num_chk; ++i)
      *(_stored_seqs + i) = htonl(*(s + i));
    return *this;
  }
  void getStoredSeqs(uint32_t s[], const uint32_t num_chk) { 
    if (num_chk != ntohl(*_num_stored_chk)) {
      //logger.log(MF_ERROR, "TransStoreHeader: num_stored_chk don't agree\n");
      exit(-1);
    }
    for (uint32_t i=0; i<num_chk; ++i)
      *(s + i) = ntohl(*(_stored_seqs + i));
  } 

private:
  GUID _store_src;
  GUID _store_dst;
  uint32_t *_num_stored_chk;
  uint32_t *_stored_seqs;  
};

class TransHeaderFactory {
public:
  static TransHeader* newTransHeader(const uint8_t *trans_data) {
    uint8_t type = TransHeader::getTransTypeStatic(trans_data);
    return newTransHeader(trans_data, type);
  } 

  static TransHeader* newTransHeader(const uint8_t *trans_data, uint8_t type) {
    TransHeader *th = NULL;

    switch (type) {
      case (DATA_T):
        th = new TransHeader(trans_data);
        break;
      case (E2E_NACK_T):
        th = new TransAckHeader(trans_data);
        break;
      case (E2E_ACK_T):
        th = new TransAckHeader(trans_data);
        break;
      case (PUSH_SYN_T):
        th = new TransStoreHeader(trans_data);
        break;
      case (STORE_T):
        th = new TransStoreHeader(trans_data);
        break;
      default: 
        break; 
    }
    if (th) {
      th->init();
      th->setTransType(type);
    }
    
    return th;
  }
};

#endif
