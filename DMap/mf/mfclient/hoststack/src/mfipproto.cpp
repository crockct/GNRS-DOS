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
 * @file   mfipproto.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu), czhang
 * @date   July, 2011
 * @brief  Temp.
 *
 * Temp.
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mfipproto.h"

MF_IPProto::MF_IPProto() {
	

}

MF_IPProto::~MF_IPProto() {
	
}

void MF_IPProto::fillHeader(u_char *buffer, u_int offset, u_int plsize, const char * source_ip, const char * destination_ip) {
	struct ipHeader *ip;
	ip = (struct ipHeader *)(buffer + offset);
		ip->ip_hl = 5;
		ip->ip_v = 4;
		ip->ip_tos = 0;
		ip->ip_len = htons(IP_HEADER_SIZE+plsize);
		ip->ip_id = 0;
		ip->ip_off = 0;
		ip->ip_ttl = 255;
		ip->ip_p = IPPROTO_MF; //IPPROTO_TCP = 6, defined in linux/in.h, use 5 for MF
		ip->ip_sum = 0;
		ip->ip_src.s_addr = inet_addr(source_ip);
		ip->ip_dst.s_addr = inet_addr(destination_ip);
		/*
		 * last to calculate ip checksum
		 */
		ip->ip_sum = in_cksum ((u_short *)ip , sizeof(*ip));
}

unsigned short MF_IPProto::in_cksum(unsigned short *addr, int len){
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;
    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }
    if (nleft == 1) {
        *(unsigned char *) (&answer) = *(unsigned char *) w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    answer = ~sum;
    return (answer);
}
