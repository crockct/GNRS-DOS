/**
 * Copyright (c) 2013, Rutgers, The State University of New Jersey
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the organization(s) stated above nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef MESSAGES_H
#define MESSAGES_H

#include "gnrscommon.hh"
//#include "gnrsguid.hh"
#include "mfguid.hh"

#define INSERT_REQUEST (unsigned char)0
#define LOOKUP_REQUEST (unsigned char)1
#define UPDATE_REQUEST (unsigned char)2

#define INSERT_RESPONSE (unsigned char)0x80
#define LOOKUP_RESPONSE (unsigned char)0x81
#define UPDATE_RESPONSE (unsigned char)0x82

#define PROTOCOL_VERSION 0 //for debug

//options
#define OPTION_REQUEST_REDIRECT (uint8_t)0
#define OPTION_NA_EXPIRATION (uint8_t)1
#define OPTION_NA_TTL (uint8_t)2

#define FINAL_OPTION_FLAG (uint8_t)0x80

//response codes
#define RESPONSE_SUCCESS 0
#define RESPONSE_FAILURE 1

//response status
#define RESPONSE_INCOMPLETE 0
#define RESPONSE_COMPLETE 1

#define MAX_INSERT_REQUEST_ADDRS 64

//response containers for easy unpacking
#define MAX_RESPONSE_OPTS 64
#define MAX_LOOKUP_RESPONSE_ADDRS 64

#define GUID_BINARY_SIZE GUID_LENGTH

const long long GNRS_ENTRY_EXPIRATION_LENGTH = 15000; // in millisecond
/**
 * Note: The following data structures are for use on host only.
 * For on the wire transmission, these need to be serialized with correct byte
 * ordering. Refer to the wire protocol documentation for correct field
 * ordering in packet
 * https://bitbucket.org/romoore/gnrs/wiki/Home
 */

typedef struct{
	uint8_t version;
	char type;
	uint16_t len;
	uint32_t id;
	addr_tlv_t src_addr;
	uint16_t data_len;
	void* data; /* request-type specific payload */
	uint16_t num_opts;
	opt_tlv_t* opts;
	uint16_t opts_len;
}req_t;

typedef struct{
	uint8_t guid[GUID_BINARY_SIZE];    /* binary encoded guid */
}lookup_t;

typedef struct{
	uint8_t guid[GUID_BINARY_SIZE];
	uint32_t size;
	addr_tlv_t* addrs; /* points to vector of addresses */
}upsert_t;


typedef struct{
	uint32_t size;
	addr_tlv_t addrs[MAX_LOOKUP_RESPONSE_ADDRS]; /* points to vector of addresses */
}lookup_resp_t; 

/*
typedef struct{
}upsert_resp_t;
 */

typedef struct{
	uint8_t status;// RESPONSE_COMPLETE, RESPONSE_INCOMPLETE
	uint8_t version;
	unsigned char type;
	uint16_t len;
	uint32_t req_id;
	addr_tlv_t src_addr;
	uint16_t code;
	uint16_t data_len;
	lookup_resp_t lkup_data; /* request-type specific payload */
	//upsert_resp_t ups_data; /* request-type specific payload */
	uint16_t num_opts;
	opt_tlv_t opts[MAX_RESPONSE_OPTS];
}resp_t;

class GnrsMessageHelper{

public:
	/* returns written bytes on successful build, else returns -1 */
	static int16_t build_request_msg(req_t req, unsigned char* buf,
			uint16_t max_len) {

		uint16_t i = 0;
		//TODO should validate data_len and opts_len
		if (max_len < (16 + req.src_addr.len + req.data_len
				+ req.opts_len)) {
			return -1;
		}
		//version
		*(buf + i) = req.version;
		i++;
		//type
		*(buf + i) = req.type;
		i++;
		//len fill after counting
		//*(uint16_t*)(buf + i) = htons(req.len);
		uint16_t req_len_offset = i;
		i += 2;
		//request id
		*(uint32_t*)(buf + i) = htonl(req.id);
		i += 4;
		//options offset
		*(uint16_t*)(buf + i) = htons(12 + 4 + req.src_addr.len
				+ req.data_len);
		i += 2;
		//data offset
		*(uint16_t*)(buf + i) = htons(12 + 4 + req.src_addr.len);
		i += 2;

		//requestor address
		*(uint16_t*)(buf + i) = htons(req.src_addr.type);
		i += 2;
		*(uint16_t*)(buf + i) = htons(req.src_addr.len);
		i += 2;
		memcpy(buf + i, req.src_addr.value, req.src_addr.len);
		i += req.src_addr.len;

		//request payload
		if(req.type == LOOKUP_REQUEST){

			if (req.data_len < GUID_BINARY_SIZE) return -1;
			lookup_t* lkup = (lookup_t*)req.data;
			memcpy(buf + i, lkup->guid, GUID_BINARY_SIZE);
			i += GUID_BINARY_SIZE;

		}else if(req.type == INSERT_REQUEST ||
				req.type == UPDATE_REQUEST) {

			uint16_t data_len = 0;
			if (req.data_len < (GUID_BINARY_SIZE + 4)) return -1;
			upsert_t* ups = (upsert_t*)req.data;
			memcpy(buf + i, ups->guid, GUID_BINARY_SIZE);
			i += GUID_BINARY_SIZE;
			data_len += GUID_BINARY_SIZE;
			*(uint32_t*)(buf + i) = htonl(ups->size);
			i += 4;
			data_len += 4;

			//address entries
			addr_tlv_t* addrs = ups->addrs;
			for(unsigned j = 0; j < ups->size; j++) {
				if (req.data_len < (data_len + 4 + (addrs+j)->len))
					return -1;
				*(uint16_t*)(buf + i) = htons((addrs + j)->type);
				i += 2;
				*(uint16_t*)(buf + i) = htons((addrs + j)->len);
				i += 2;
				memcpy(buf + i, (addrs + j)->value, (addrs + j)->len);
				i += (addrs+j)->len;
				data_len += 4 + (addrs+j)->len;
			}
		}

		//options
		opt_tlv_t* opts = req.opts;
		uint16_t opts_len = 0;
		for (int j = 0; j < req.num_opts; j++) {
			if (req.opts_len < (opts_len + 2 + (opts+j)->len))
				return -1;
			if (j == (req.num_opts - 1)) {
				/* last option, set 'final' bit 0x80 */
				*(uint8_t*)(buf + i) =
						(opts + j)->type | FINAL_OPTION_FLAG;
			} else {
				*(uint8_t*)(buf + i) = (opts + j)->type;
			}
			i++;
			*(uint8_t*)(buf + i) = (opts + j)->len;
			i++;
			memcpy(buf + i, (opts + j)->value, (opts + j)->len);
			i += (opts+j)->len;
			opts_len += 2 + (opts+j)->len;
		}
		//fill in len
		*(uint16_t*)(buf + req_len_offset) = htons(i);
		return i;
	}


	/* returns with rsp.status set to RESPONSE_INCOMPLETE if theres's
	 * an error while parsing, else it is set to RESPONSE_COMPLETE
	 * before returning
	 */
	static void parse_response_msg(unsigned char* buf, int len,
			resp_t& rsp){

		rsp.status = RESPONSE_INCOMPLETE;
		int i = 0; // index
		//version
		if (len >= (i + 1)) {
			rsp.version = *(buf + i);
		} else {
			return;
		}
		i++;
		//message type
		if (len >= (i+1)) {
			rsp.type = *(buf + i);
		} else {
			return;
		}
		i++;
		//message len
		if (len >= (i+2)) {
			rsp.len = ntohs(*(uint16_t *)(buf + i));
		} else {
			return;
		}
		i += 2;
		//request id
		if (len >= (i+4)) {
			rsp.req_id = ntohl(*(uint32_t *)(buf + i));
		} else {
			return;
		}
		i += 4;

		//options offset
		uint16_t opts_offset;
		if (len >= (i+2)) {
			opts_offset = ntohs(*(uint16_t *)(buf + i));
		} else {
			return;
		}
		i += 2;
		//data offset
		uint16_t data_offset;
		if (len >= (i+2)) {
			data_offset = ntohs(*(uint16_t *)(buf + i));
		} else {
			return;
		}
		i += 2;

		//origin addr
		//T type
		if (len >= (i+2)) {
			rsp.src_addr.type = ntohs(*(uint16_t *)(buf + i));
		} else {
			return;
		}
		i += 2;
		//L len
		if (len >= (i+2)) {
			rsp.src_addr.len = ntohs(*(uint16_t *)(buf + i));
		} else {
			return;
		}
		i += 2;
		//V value
		if (len >= (i + rsp.src_addr.len)) {
			rsp.src_addr.value = buf + i;
		} else {
			return;
		}
		i += rsp.src_addr.len;

		//response code
		if (len >= (i+4)) {
			//16bit value and 2 byte pad - pick up only the first 2
			rsp.code = ntohs(*(uint16_t *)(buf + i));
		} else {
			return;
		}
		i += 4;

		//response payload
		if (rsp.type == LOOKUP_RESPONSE) {
			if (len >= (i+4)) {
				rsp.lkup_data.size = ntohl(*(uint32_t *)(buf + i));
			} else {
				return;
			}
			i += 4;
			for(unsigned j = 0; (j < rsp.lkup_data.size)
			&& (j < MAX_LOOKUP_RESPONSE_ADDRS); j++){

				//addr
				//T type
				if (len >= (i+2)) {
					rsp.lkup_data.addrs[j].type =
							ntohs(*(uint16_t *)(buf + i));
				} else {
					return;
				}
				i += 2;
				//L len
				if (len >= (i+2)) {
					rsp.lkup_data.addrs[j].len =
							ntohs(*(uint16_t *)(buf + i));
				} else {
					return;
				}
				i += 2;
				//V value
				if (len >= (i + rsp.lkup_data.addrs[j].len)) {
					rsp.lkup_data.addrs[j].value = buf + i;
				} else {
					return;
				}
				i += rsp.lkup_data.addrs[j].len;
			}
		} else if (rsp.type == INSERT_RESPONSE) {
			//nothing to do here
		} else if (rsp.type == UPDATE_RESPONSE) {
			//TODO
		}

		//options
		bool opts_done = false;
		int j = 0;
		/*
            while(!opts_done){

                //option
                //T type
                if (len >= (i+1)) {
                    rsp.opts[j].type = *(uint8_t *)(buf + i);
                } else {
                    return;
                }
                i += 1;
                // if this is the last option, 'final' bit 0x80 should be set 
                if(rsp.opts[j].type & 0x80){
                    opts_done = true;
                }

                //L len
                if (len >= (i+1)) {
                    rsp.opts[j].len = *(uint8_t *)(buf + i);
                } else {
                    return;
                }
                i += 1;
                //V value
                if (len >= (i + rsp.opts[j].len)) {
                    rsp.opts[j].value = buf + i;
                } else { 
                    return;
                }
                i += rsp.opts[j].len;

                j++;
            }
		 */
		rsp.status = RESPONSE_COMPLETE;
	}
};
#endif //MESSAGES_H
