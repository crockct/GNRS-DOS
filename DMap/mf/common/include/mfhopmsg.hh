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
#ifndef MF_HOP_MSG_HH_
#define MF_HOP_MSG_HH_

#include <stdint.h>

#define CSYN_SIZE sizeof(csyn_t)
#define CSYN_ACK_SIZE sizeof(csyn_ack_t)
#define HOP_DATA_PKT_SIZE sizeof(hop_data_t)
#define HOP_HEADER_SIZE 16
#define CSYN_HEADER_ACK_SIZE sizeof(csyn_ack_t)

#define CSYN_TIMEOUT 2000 // 2000 ms

//packet types
enum {
	DATA_PKT,
	CSYN_PKT,
	CSYN_ACK_PKT
};

//hop protocol data packet
struct hop_data{
  uint32_t type;
  uint32_t pld_size;               //payload size in bytes
  uint32_t seq_num;
  uint32_t hop_ID;                 //start from 0
  char data[];
};
typedef struct hop_data hop_data_t;

// Hop protocol signalling packets
struct csyn{
        uint32_t type;
        uint32_t hop_ID;
        uint32_t chk_pkt_count;    //number of pac the chunk has
};
typedef struct csyn csyn_t;

struct csyn_ack{
        uint32_t type;
        uint32_t hop_ID;
        uint32_t chk_pkt_count;
        char bitmap[];
};
typedef struct csyn_ack csyn_ack_t;

#endif /*MF_HOP_MSG_HH_*/
