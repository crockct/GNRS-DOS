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
#ifndef MF_MULTIHOMINGEXTHEADER_HH_
#define MF_MULTIHOMINGEXTHEADER_HH_

#include "mfextheader.hh"
#include "mfmhomeopt.hh"

#define ERROR_MAX_NA_NUM -1
#define ERROR_NO_SPACE -2

#define MAX_NA_NUM 10

#define EXTHDR_MULTIHOME_MAX_SIZE 88
#define EXTHDR_MULTIHOME_OPT_OFFSET 4
#define EXTHDR_MULTIHOME_SRC_NA_NUM_OFFSET 6
#define EXTHDR_MULTIHOME_DST_NA_NUM_OFFSET 7
#define EXTHDR_MULTIHOME_SRC_NA_OFFSET 8
/*
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 Bit
0             1               2               3                 Octet 
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Service identifier        |          Next Header          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Options                  | Src. NA Count | Dest. NA Count|
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
~                                                             ~
|                         Source NA(s)                        |
~                                                             ~
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
~                                                             ~
|                         Destination NA(s)                   |
~                                                             ~
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
*/

class MultiHomingExtHdr : public ExtensionHeader {
public:
  MultiHomingExtHdr(const uint8_t *extHdr, uint32_t srcNaNum,
     uint32_t dstNaNum);              //must specify number of NAs
  MultiHomingExtHdr(const uint8_t *extHdr);

  ~MultiHomingExtHdr(){
    delete [] src_NAs;
    delete [] dst_NAs; 
  }

  //getter and setter
  uint8_t getNumOfSrcNA() {
    return *src_NA_num;
  }
  
  int32_t setNumOfSrcNA(uint32_t num) {
    if (num > MAX_NA_NUM) {
      *this->src_NA_num = MAX_NA_NUM;
      return ERROR_MAX_NA_NUM; 
    } else {
      *this->src_NA_num = num;
      return 0; 
    }
  }
  
  NA getSrcNA (uint32_t pos) {
    if (pos >= *src_NA_num) {
      return *(new NA());                      //first NA 
    }
    return  src_NAs[pos];            //if has no valid na, return 0; 
  }

  int32_t setSrcNA(NA& na, uint32_t pos) {
    if (pos < *src_NA_num) {
      this->src_NAs[pos] = na;
      return 0; 
    } else {
      return ERROR_NO_SPACE; 
    }
  }
  int32_t setSrcNA(uint32_t na, uint32_t pos) {
    NA *addr = new NA();
    addr->init(na); 
    return setSrcNA(*addr, pos); 
  }
  
  int32_t setNumOfDstNA( uint32_t num ) {
    if (num >= MAX_NA_NUM) {
      *this->src_NA_num = MAX_NA_NUM;
      return ERROR_MAX_NA_NUM; 
    } else {
      *this->dst_NA_num = num; 
      return 0; 
    }
  }

  uint32_t getNumOfDstNA() {
    return *this->dst_NA_num;
  }

  NA& getDstNA (uint32_t pos) {
    if (pos >= *dst_NA_num) {
      return *(new NA());
    }
    return  dst_NAs[pos];
  }
  int32_t setDstNA(NA& na, uint32_t pos) {
    if (pos < *dst_NA_num) {
      *(this->dst_NAs+pos) = na;
      return 0; 
    } else {
      return ERROR_NO_SPACE; 
    }
  }
  
  int32_t setDstNA( uint32_t na, uint32_t pos) {
    NA *addr = new NA();
    addr->init(na); 
    return setDstNA(*addr, pos); 
  }
  
  //for logging
  char * to_log(char *buf);

private:
  virtual void parseHeader();
  MF_MultiHomeOptions *options; 
  uint8_t *src_NA_num;
  uint8_t *dst_NA_num;
  NA *src_NAs;
  NA *dst_NAs;
};  


inline MultiHomingExtHdr::MultiHomingExtHdr(const uint8_t *extHdr,
            uint32_t srcNaNum, uint32_t dstNaNum): ExtensionHeader(extHdr) {
  //set ExtHenader bit and MultiHoming bit
  this->service_id->setExtensionHeader(); 
  this->service_id->setMultiHoming(); 
  *this->next_header_offset = 0;        //no next header by default
  //init options
  options = new MF_MultiHomeOptions(header + EXTHDR_MULTIHOME_OPT_OFFSET); 
  //set # of src NA
  this->src_NA_num = (uint8_t*)(header + EXTHDR_MULTIHOME_SRC_NA_NUM_OFFSET);
  *this->src_NA_num = srcNaNum;
  //set # of dst NA
  this->dst_NA_num = (uint8_t*)(header + EXTHDR_MULTIHOME_DST_NA_NUM_OFFSET);
  *this->dst_NA_num = dstNaNum;
  parseHeader(); 
}

inline MultiHomingExtHdr::MultiHomingExtHdr(
            const uint8_t *extHdr): ExtensionHeader(extHdr) {
  
  //init options
  options = new MF_MultiHomeOptions(header + EXTHDR_MULTIHOME_OPT_OFFSET);
  //set # of src NA
  this->src_NA_num = (uint8_t*)(header + EXTHDR_MULTIHOME_SRC_NA_NUM_OFFSET); 
  this->dst_NA_num = (uint8_t*)(header + EXTHDR_MULTIHOME_DST_NA_NUM_OFFSET);
  parseHeader();
}

inline void MultiHomingExtHdr::parseHeader() {

  src_NAs = new NA[*src_NA_num]();
  for (uint32_t i = 0; i != *src_NA_num; ++i) {
    src_NAs[i].init(header + i * ADDRESS_LENGTH +
                      EXTHDR_MULTIHOME_SRC_NA_OFFSET);
  }
  //set dst NAs
  dst_NAs = new NA[*dst_NA_num]();
  for (uint32_t j = 0; j != *dst_NA_num; ++j) {
    dst_NAs[j].init(header + (*src_NA_num + j) * ADDRESS_LENGTH +
                          EXTHDR_MULTIHOME_SRC_NA_OFFSET);
  }
  headerSize = (*src_NA_num + *dst_NA_num) * ADDRESS_LENGTH + 
                  EXTHDR_MULTIHOME_SRC_NA_OFFSET;
}

inline char * MultiHomingExtHdr::to_log (char* buf) {
  ExtensionHeader::to_log(buf);
  char NA_buf[ADDRESS_LOG_BUF_SIZE]; 
  uint32_t start_pos = strlen(buf);
  sprintf(buf + start_pos, "\n"
    "-----------MultiHoming Extension Header-----------------\n"
    "options       : %u \n"
    "# of Src NA   : %u \n"
    "# of Dst NA   : %u \n",
    options->getOptions(),
    *src_NA_num,
    *dst_NA_num
  );

  for (uint32_t i = 0; i != *src_NA_num; ++i) {
    start_pos = strlen(buf);
    sprintf(buf + start_pos, 
      "Src_NA #%u : %s\n",
      i, src_NAs[i].to_log(NA_buf)); 
  }
  for (uint32_t j = 0; j != *dst_NA_num; ++j) {
    start_pos = strlen(buf);
    sprintf(buf + start_pos, 
      "Dst_NA #%u : %s\n",
      j, dst_NAs[j].to_log(NA_buf)); 
  }
  start_pos = strlen(buf);
  sprintf(buf + start_pos, 
    "--------------------------------------------------------\n"); 
  return buf; 
}

#endif

