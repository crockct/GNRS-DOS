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
#ifndef MF_SERVICEID_HH_
#define MF_SERVICEID_HH_

#define SID_LOG_BUF_SIZE 17     //16bypes + '\0'

class MF_ServiceID {
public:
  MF_ServiceID(uint16_t *sid) : service_id(sid) {
  }
  MF_ServiceID(const uint8_t* sid): service_id((uint16_t*)sid) {
  }
  ~MF_ServiceID() {
  }

  inline bool isExtensionHeader() {
    uint16_t sid = htons(SID_EXTHEADER); 
    return (*this->service_id & sid) == sid? true : false; 
  }
  inline bool isMultiCast() {
    uint16_t sid = htons(SID_MULTICAST); 
    return (*this->service_id & sid) == sid? true : false; 
  }
  inline bool isAnyCast() {
    uint16_t sid = htons(SID_ANYCAST); 
    return (*this->service_id & sid) == sid? true : false; 
  }
  inline bool isMultiHoming() {
    uint16_t sid = htons(SID_MULTIHOMING); 
    return (*this->service_id & sid) == sid? true : false; 
  }
  inline bool isContentRequest() {
	uint16_t sid = htons(SID_CONTENT_REQUEST);
	return (*this->service_id & sid) == sid? true : false;
  }
  inline bool isContentResponse() {
	uint16_t sid = htons(SID_CONTENT_RESPONSE);
	return (*this->service_id & sid) == sid? true : false;
  }

  //set
  inline MF_ServiceID set(uint16_t sid) {
    *this->service_id |= htons(sid);
    return *this; 
  }
  inline MF_ServiceID setExtensionHeader() {
    *this->service_id |= htons(SID_EXTHEADER);
    return *this; 
  }
  inline MF_ServiceID setMultiCast() {
    *this->service_id |= htons(SID_MULTICAST);
    return *this; 
  }
  inline MF_ServiceID setAnyCast() {
    *this->service_id |= htons(SID_ANYCAST);
    return *this; 
  }
  inline MF_ServiceID setMultiHoming() {
    *this->service_id |= htons(SID_MULTIHOMING);
     return *this;
  }
  inline MF_ServiceID setContentRequest() {
	*this->service_id |= htons(SID_CONTENT_REQUEST);
	return *this;
  }
  inline MF_ServiceID setContentResponse() {
	*this->service_id |= htons(SID_CONTENT_RESPONSE);
	return *this;
  }
  //reset
  inline MF_ServiceID resetAll() {
    *this->service_id = 0;
    return *this; 
  }
  
  inline MF_ServiceID reset(uint16_t sid) {
    *this->service_id &= htons(~sid);
    return *this; 
  }
  inline MF_ServiceID resetExtensionHeader() {
    *this->service_id &= htons((uint16_t)(~SID_EXTHEADER));
    return *this; 
  }
  inline MF_ServiceID resetMultiCast() {
    *this->service_id &= htons(~SID_MULTICAST);
    return *this; 
  }
  inline MF_ServiceID resetAnyCast() {
    *this->service_id &= htons(~SID_ANYCAST);
    return *this; 
  }
  inline MF_ServiceID resetMultiHoming() {
    *this->service_id &= htons(~SID_MULTIHOMING);
    return *this; 
  }

  //operator: assign
  inline bool operator=(const uint8_t* sid) {
    service_id = (uint16_t*)sid;
    return true; 
  }
  
  //to_log
  inline char* to_log(char* buf) {
    uint16_t temp = ntohs(*service_id); 
    for (uint32_t i = 0; i != SID_LOG_BUF_SIZE; ++i) {
      temp & 0x8000 ? buf[i] = '1' : buf[i] = '0';
      temp <<= 1; 
    }
    buf[SID_LOG_BUF_SIZE - 1] = '\0'; 
    return buf; 
  }
  
  inline uint16_t to_int() {
    uint16_t temp = *service_id;
    temp &= htons((uint16_t)(~SID_EXTHEADER));  //block next_header bit
    return (uint16_t)ntohs(temp);
  }
  
  static const uint16_t SID_EXTHEADER = 0x8000;  //1000 0000 0000 0000 next_header
  static const uint16_t SID_UNICAST = 0x0000;     //0000 0000 0000 0000 uincast
  static const uint16_t SID_MULTICAST = 0x0001;   //0000 0000 0000 0001 multicast
  static const uint16_t SID_ANYCAST = 0x0002;     //0000 0000 0000 0010 anycast
  static const uint16_t SID_MULTIHOMING = 0x0004; //0000 0000 0000 0100 multihoming
  static const uint16_t SID_BROADCAST = 0x2000; //0010 0000 0000 0000 end point broadcast
  static const uint16_t SID_CONTENT_REQUEST = 0x0200; //0000 0010 0000 0000 multihoming
  static const uint16_t SID_CONTENT_RESPONSE = 0x0400; //0000 0100 0000 0000 multihoming

private:
  uint16_t *service_id;                      //note: this is in network order
}; 

#endif   /*MF_SERVICEID_HH_*/
