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
#ifndef MF_PACKETSUPPORT_HH_
#define MF_PACKETSUPPORT_HH_

#include <include/mfroutingheader.hh>
#include <include/mfhopmsg.hh>
#include <include/mfmhomeextheader.hh>
#include <include/mftransportheader.hh>

#include "mftypes.h"
#include "mfetherproto.h"
#include "mfipproto.h"

//#define TRANS_HEADER_SIZE sizeof(transport_header)
//#define EXTHDR_MULTIHOME_MAX_SIZE 88
//#define ROUTING_HEADER_SIZE 20 + 2*GUID_LENGTH
//#define HOP_HEADER_SIZE 16
//#define IP_HEADER_SIZE 20
//#define ETH_HEADER_SIZE 14

class MF_PacketSupport {
public:
	
	const static int MAX_PAYLOAD_SIZE = 1400;
	const static int TOTAL_HEADER_LEN = ETH_HEADER_SIZE+IP_HEADER_SIZE+HOP_HEADER_SIZE+ROUTING_HEADER_SIZE+EXTHDR_MULTIHOME_MAX_SIZE+TRANS_BASE_HEADER_SIZE;
	//static int LOW_HEADER_LEN = ETH_HEADER_SIZE + IP_HEADER_SIZE + TRANS_HEADER_SIZE;
	const static int LOW_HEADER_LEN = ETH_HEADER_SIZE + IP_HEADER_SIZE + HOP_HEADER_SIZE;
	const static int HIGH_HEADER_LEN = ROUTING_HEADER_SIZE + EXTHDR_MULTIHOME_MAX_SIZE+ TRANS_BASE_HEADER_SIZE;
	const static int PACKET_SIZE = MAX_PAYLOAD_SIZE - LOW_HEADER_LEN;
	
	static inline u_char *getEthernetHeaderPtr(u_char *packet){
		return packet;
	}
	
	static inline u_char *getIPHeaderPtr(u_char *packet){
		return packet + ETH_HEADER_SIZE;
	}
	
	static inline u_char *getHopHeaderPtr(u_char *packet){
		return packet + ETH_HEADER_SIZE + IP_HEADER_SIZE;
	}
	
	static inline u_char *getNetworkHeaderPtr(u_char *packet){
		return packet + ETH_HEADER_SIZE + IP_HEADER_SIZE + HOP_HEADER_SIZE;
	}
	
	//bool is true if it's first packet of the chunk
	static inline u_char *getTransportHeaderPtr(u_char *packet, bool first){
		if(first){
			return packet + ETH_HEADER_SIZE + IP_HEADER_SIZE + HOP_HEADER_SIZE + ROUTING_HEADER_SIZE + EXTHDR_MULTIHOME_MAX_SIZE;
		}
		else {
			return packet + ETH_HEADER_SIZE + IP_HEADER_SIZE + HOP_HEADER_SIZE;
		}
	}
	
	//bool is true if it's first packet of the chunk
	static inline u_char *getData(u_char *packet, bool first){
		if(first){
			return packet + ETH_HEADER_SIZE + IP_HEADER_SIZE + HOP_HEADER_SIZE + ROUTING_HEADER_SIZE + EXTHDR_MULTIHOME_MAX_SIZE + TRANS_BASE_HEADER_SIZE;
		}
		else {
			return packet + ETH_HEADER_SIZE + IP_HEADER_SIZE + HOP_HEADER_SIZE;
		}
	}
    
private:
	
};




#endif /*MF_ROUTINGHEADER_HH_*/
