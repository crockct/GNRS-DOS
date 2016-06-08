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

/**
 * @file   mfetherproto.h
 * @author wontoniii (bronzino@winlab.rutgers.edu), czhang
 * @date   July, 2011
 * @brief  Temp.
 *
 * Temp.
 */

#ifndef MF_ETHERPROTO_H_
#define MF_ETHERPROTO_H_

#include <arpa/inet.h>

#define ETH_HEADER_SIZE 14 //ethernet header size
#define ETHER_ADDR_LEN  6
#define ETHTYPE_IP 0x0800
#define ETHTYPE_MF 0xFFF0

struct etherHeader {
	u_char  ether_dhost[ETHER_ADDR_LEN];    /* destination host address */
	u_char  ether_shost[ETHER_ADDR_LEN];    /* source host address */
	u_short ether_type;                     /* IP? ARP? RARP? etc */
} __attribute__((packed));
struct ether_addr {
	u_char ether_addr_octet[ETHER_ADDR_LEN];
} __attribute__((packed));

class MF_EtherProto {

public:
	MF_EtherProto();
	~MF_EtherProto();
	//void writeHeader(char *);
	static struct ether_addr *ether_aton(const char *);
	static struct ether_addr *ether_aton_r(const char *, struct ether_addr *);
	static char *ether_ntoa(const struct ether_addr *);
	static char *ether_ntoa_r(const struct ether_addr *, char *);
	static void fillHeader(u_char *, u_int, const char *, const char *);
};

#endif /* MF_ETHERPROTO_H_ */
