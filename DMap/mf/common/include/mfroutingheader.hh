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
#ifndef MF_ROUTINGHEADER_HH_
#define MF_ROUTINGHEADER_HH_

#include "mfguid.hh"
#include "mfaddress.hh"

#include "mfheader.hh"
#include "mfserviceid.hh"
#include "mfextheader.hh"
#include "mfmhomeextheader.hh"

#define ROUTING_HEADER_SIZE 20 + 2*GUID_LENGTH  
#define ROUTING_HEADER_VERSION_OFFSET 0
#define ROUTING_HEADER_SERVICEID_OFFSET 1
#define ROUTING_HEADER_PROTOCOL_OFFSET 3
#define ROUTING_HEADER_PAYLOAD_POSITION_OFFSET 4
#define ROUTING_HEADER_PAYLOAD_SIZE_OFFSET 8
#define ROUTING_HEADER_SRCGUID_OFFSET 12
#define ROUTING_HEADER_SRCNA_OFFSET ROUTING_HEADER_SRCGUID_OFFSET+GUID_LENGTH
#define ROUTING_HEADER_DESTGUID_OFFSET ROUTING_HEADER_SRCNA_OFFSET+4
#define ROUTING_HEADER_DESTNA_OFFSET ROUTING_HEADER_DESTGUID_OFFSET+GUID_LENGTH

/*
Routing Header Format: (GUID_LENGTH = 20 bytes)
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 Bit
0             1               2               3                 Octet 
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Version   |     Service identifier        |  Protocol     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|        Payload Offset       |         Reserved              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                        Payload Size                         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
~                                                             ~
|                        Source GUID                          |
~                                                             ~
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                        Source NA                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
~                                                             ~
|                        Destination GUID                     |
~                                                             ~
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                        Destination NA                       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
~                                                             ~
|                        Extension Header(s)                  |
~                                                             ~
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
~                                                             ~
|                        Payload                              |
~                                                             ~
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/


class RoutingHeader: public PacketHeader {
public:
  //constructors
  RoutingHeader(const uint8_t* hdr):PacketHeader(hdr) {
	src_GUID = NULL;
	src_NA = NULL;
	dst_GUID = NULL;
	dst_NA = NULL;
    parseHeader(); 
  };
  
  ~RoutingHeader(){
	if(src_GUID != NULL) delete src_GUID;
	if(src_NA != NULL) delete src_NA;
	if(dst_GUID != NULL) delete dst_GUID;
	if(dst_NA != NULL) delete dst_NA;
  }
   
  //getters and setters
  uint8_t getVersion() {
    return *version;
  }

  void setVersion (uint8_t version) {
    *this->version = version;
  }

  MF_ServiceID getServiceID() {
    return *service_id;
  }

  void setServiceID (uint16_t serviceID) {
    this->service_id->set(serviceID);
  }
  void resetServiceID (uint16_t serviceID) {
    this->service_id->reset(serviceID); 
  }
  
  uint8_t getUpperProtocol(){
    return *this->upper_proto;
  }
  void setUpperProtocol(uint8_t protocol) {
    *this->upper_proto = protocol;
  }
  
  uint16_t getPayloadOffset() {
    return ntohs(*pld_position); 
  } 
  void setPayloadOffset(uint16_t offset) {
    *this->pld_position = htons(offset); 
  }
  
  uint32_t getChunkPayloadSize(){
    return ntohl(*pld_size);
  }
  void setChunkPayloadSize(uint32_t size){
    *pld_size = htonl(size); 
  }
  
  GUID& getSrcGUID() {
    return *src_GUID;
  }
  
  void setSrcGUID (GUID &guid) {
    *this->src_GUID = guid;
  }
  void setSrcGUID (uint32_t guid) {
    *this->src_GUID = guid; 
  }
  
  NA& getSrcNA () {
    return *src_NA; 
  }
  void setSrcNA (NA &addr) {
    *this->src_NA = addr; 
  }
  void setSrcNA (uint32_t addr) {
    *this->src_NA = addr; 
  }
  
  GUID& getDstGUID () {
    return *dst_GUID;
  }
  void setDstGUID (GUID &guid) {
    *this->dst_GUID = guid;
  }
  void setDstGUID (uint32_t guid ){
    *this->dst_GUID = guid;
  }
  
  NA& getDstNA () {
    return *dst_NA;
  }
  void setDstNA (NA &addr) {
    *this->dst_NA = addr;
  }
  void setDstNA (uint32_t addr) {
    *this->dst_NA = addr;
  }
  
  
  bool hasExtensionHeader() {
    return service_id->isExtensionHeader(); 
  }
  ExtensionHeader* getExtensionHeader(uint16_t type); 

  //print
  virtual char* to_log(char *buf); 
    
private:
  virtual void parseHeader();
  
  uint8_t *version;
  MF_ServiceID *service_id; 
  uint8_t *upper_proto;
  uint16_t *pld_position;
  uint32_t *pld_size;
  
  GUID *src_GUID;
  NA *src_NA;
  GUID *dst_GUID;
  NA *dst_NA;
}; 


inline void RoutingHeader::parseHeader() {
  headerSize = ROUTING_HEADER_SIZE;

  version = (uint8_t*) header;
  service_id = new MF_ServiceID( header + ROUTING_HEADER_SERVICEID_OFFSET); 
  upper_proto = (uint8_t*)(header + ROUTING_HEADER_PROTOCOL_OFFSET);
  pld_position = (uint16_t*)(header + ROUTING_HEADER_PAYLOAD_POSITION_OFFSET);
  pld_size = (uint32_t*)(header + ROUTING_HEADER_PAYLOAD_SIZE_OFFSET);

  src_GUID = new GUID();
  src_GUID->init( header + ROUTING_HEADER_SRCGUID_OFFSET );
  src_NA = new NA(); 
  src_NA->init(header + ROUTING_HEADER_SRCNA_OFFSET);
  dst_GUID = new GUID();
  dst_GUID->init( header + ROUTING_HEADER_DESTGUID_OFFSET );
  dst_NA = new NA(); 
  dst_NA->init(header + ROUTING_HEADER_DESTNA_OFFSET);
}

inline char* RoutingHeader::to_log( char* buf ) {
  char sid_buf[SID_LOG_BUF_SIZE]; 
  char src_guid_buf[ GUID_LOG_BUF_SIZE ];
  char dst_guid_buf[ GUID_LOG_BUF_SIZE ];
  char src_NA_buf[ ADDRESS_LOG_BUF_SIZE ];
  char dst_NA_buf[ ADDRESS_LOG_BUF_SIZE];
  sprintf( buf,"\n"
    "------------Routing Header------------------------------\n"
    "version       : %x \n" 
    "Service ID    : %s \n" 
    "upper protocol: %x \n"
    "payload pos   : %u \n"
    "payload size  : %u \n"
    "src GUID      : %s \n"
    "src NA        : %s \n"
    "dst GUID      : %s \n"
    "dst NA        : %s \n"
    "--------------------------------------------------------\n", 
    *version, 
    service_id->to_log(sid_buf),  
    *upper_proto, 
    ntohs(*pld_position),
    ntohl(*pld_size),
    src_GUID->to_log( src_guid_buf ), 
    src_NA->to_log( src_NA_buf ), 
    dst_GUID->to_log( dst_guid_buf ), 
    dst_NA->to_log( dst_NA_buf )
  );    
  return buf; 
}

inline ExtensionHeader* RoutingHeader::getExtensionHeader (uint16_t type) {
  if (service_id->isExtensionHeader()) { //if has next header 
    ExtensionHeader *ext_hdr = NULL; 
    uint16_t sid = 0; 
    uint16_t ext_header_pos = headerSize;
    while (ext_header_pos >= headerSize && ext_header_pos < MAX_HEADER_LEN) { 
      ext_hdr = new ExtensionHeader(header + ext_header_pos);
      sid = ext_hdr->getServiceID().to_int();
      if (sid == type) {                 //sid matches desired extension header
        delete ext_hdr;                   
        ext_hdr = NULL;  
        break;                      
      }
      ext_header_pos = ext_hdr->getNextHeaderOffset();
      if (ext_header_pos == 0) {         //if cannot find desired extension header
        return NULL;                     //return NULL
      }
      delete ext_hdr;
      ext_hdr = NULL; 
    }

    switch (sid) {
      case MF_ServiceID::SID_MULTICAST:
        break;
      case MF_ServiceID::SID_ANYCAST:
        break;
      case MF_ServiceID::SID_MULTIHOMING: {
        ext_hdr = new MultiHomingExtHdr(header + ext_header_pos);
        break;
      }
      default:
        return NULL; 
    }
    return ext_hdr;
  }
  return NULL;
}

#endif /*MF_ROUTINGHEADER_HH_*/
