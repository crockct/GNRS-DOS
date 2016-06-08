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
#ifndef MF_GSTAR_CTRL_MSG_HH_
#define MF_GSTAR_CTRL_MSG_HH_

//#define BROADCAST_GUID 0 

#define LINKPROBE_PERIOD_MSECS 2000
#define NEIGHBOR_HEARTBEAT_TIMEOUT_MSECS 11000
#define LSA_PERIOD_MSECS 5000
#define LSA_LIVENESS_TIMEOUT_MSECS 11000

//packet types
enum {
	LP_PKT = 3,
	LP_ACK_PKT,
	LSA_PKT,
	ASSOC_PKT,
	DASSOC_PKT
};

struct link_probe {
  uint32_t type;
  uint32_t sourceLP;
  uint32_t destinationLP;
  uint32_t seq;
};
typedef link_probe link_probe_t;

struct link_probe_ACK {
  uint32_t type;
  uint32_t sourceLPACK;
  uint32_t destinationLPACK;
  uint32_t seq_no_cp;
};
typedef link_probe_ACK link_probe_ack_t;

struct LSA {
  uint32_t type;
  uint32_t senderLSA;
  uint32_t sourceLSA;
  uint32_t destinationLSA;
  uint32_t seq;
  uint32_t size;
};
typedef LSA lsa_t;

struct rcvd_pkt {
  uint32_t type;
  uint32_t source;
  uint32_t destination;
  uint32_t seq;
};

//client association request packet
struct client_assoc_pkt{
  uint32_t type;
  uint32_t client_GUID;
  uint32_t host_GUID;
  /* client specified weight for attachment point */
  uint16_t weight;
};
typedef client_assoc_pkt client_assoc_pkt_t;

//client deassociation request packet
struct client_dassoc_pkt{
	uint32_t type;
    uint32_t entity_GUID;
    uint32_t host_GUID;
};
typedef client_dassoc_pkt client_dassoc_pkt_t;

#endif /*MF_GSTAR_CTRL_MSG_HH*/
