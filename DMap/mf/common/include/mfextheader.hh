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
#ifndef MF_NEXTHEADER_HH
#define MF_NEXTHEADER_HH

#include "mfheader.hh"

#define EXTHEADER_SERVICEID_OFFSET 0
#define EXTHEADER_EXTHEADER_OFFSET 2

class ExtensionHeader: public PacketHeader {
public:
  ExtensionHeader(const uint8_t *extHdr) : PacketHeader(extHdr){
    parseHeader(); 
  }
  ~ExtensionHeader() {
    
  }
 
  MF_ServiceID getServiceID() {
    return *service_id; 
  }
  
  void setServiceID(uint16_t serviceID) {
    this->service_id->set(serviceID); 
  } 
  
  void resetServiceID(uint16_t serviceID) {
    this->service_id->reset(serviceID); 
  }

  uint16_t getNextHeaderOffset() {
    return ntohs(*this->next_header_offset); 
  }

  void setNextHeaderOffset(uint16_t nextHdr) {
    *this->next_header_offset = htons(nextHdr); 
  }
  
  //virtual bool isStandardSid() {  
    //if first bit is set, this is a standard sid, which every bit indicates a service
  //  return ((*service_id & htons(0x8000)) == 0x8000) ? true: false; 
  //}

  virtual bool hasNextHeader(){
    //has next header is next_header is not zero
    return *next_header_offset != 0 ? true : false; 
  }
  
  //get next extension header 
  virtual ExtensionHeader* getNextHeader(); 
  //for logging
  virtual char* to_log(char* buf); 
protected:
  virtual void parseHeader();
  MF_ServiceID *service_id; 
  uint16_t* next_header_offset; 
}; 

inline void ExtensionHeader::parseHeader() {
  service_id = new MF_ServiceID(header);
  service_id->setExtensionHeader(); 
  next_header_offset = (uint16_t*)(header + EXTHEADER_EXTHEADER_OFFSET);
}

inline ExtensionHeader* ExtensionHeader::getNextHeader() {
  ExtensionHeader *ext_hdr = NULL; 
  if (hasNextHeader()) {
    ext_hdr = new ExtensionHeader(header + *next_header_offset);
  }
  return ext_hdr;  
}

inline char* ExtensionHeader::to_log (char * buf) {
  char sid_buf[SID_LOG_BUF_SIZE];
  sprintf(buf, "\n"
    "----------------Extension Heander-----------------------\n"
    "Service ID    : %s \n"
    "Next Header   : %x \n"
    "--------------------------------------------------------\n",
    service_id->to_log(sid_buf),
    ntohs(*next_header_offset));
  return buf; 
}

#endif   /*MF_NEXTHEADER_HH*/
