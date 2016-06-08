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
 * @file   mfetherproto.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   July, 2013
 * @brief  Temp.
 *
 * Temp.
 */

#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "mfetherproto.h"

MF_EtherProto::MF_EtherProto() {
	
}

MF_EtherProto::~MF_EtherProto() {
	
}
void MF_EtherProto::fillHeader(u_char *buffer, u_int offset, const char *source_MAC, const char * destination_MAC) {
	//MF_Log::mf_log(MF_DEBUG, "MF_EtherProto:fillHeader");
	struct etherHeader *ether;
	struct ether_addr *d, *s;
	ether = (struct etherHeader *)(buffer + offset);
	d = MF_EtherProto::ether_aton(destination_MAC);
	memcpy (ether->ether_dhost, d->ether_addr_octet, 6);
	s = MF_EtherProto::ether_aton(source_MAC);
	memcpy (ether->ether_shost, s->ether_addr_octet, 6);
	ether->ether_type = htons(ETHTYPE_IP);
}

struct ether_addr *MF_EtherProto::ether_aton_r(const char *asc, struct ether_addr *addr){
	//MF_Log::mf_log(MF_DEBUG, "MF_EtherProto:ether_aton_r");
	size_t cnt;

	for (cnt = 0; cnt < 6; ++cnt) {
		unsigned int number;
		char ch;

		ch = tolower (*asc);
		asc++;
		if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f'))
			return NULL;
		number = isdigit (ch) ? (ch - '0') : (ch - 'a' + 10);

		ch = tolower (*asc);
		if ((cnt < 5 && ch != ':') || (cnt == 5 && ch != '\0' && !isspace (ch))) {
			++asc;
			if ((ch < '0' || ch > '9') && (ch < 'a' || ch > 'f'))
				return NULL;
			number <<= 4;
			number += isdigit (ch) ? (ch - '0') : (ch - 'a' + 10);

			ch = *asc;
			if (cnt < 5 && ch != ':')
				return NULL;
		}

		/* Store result.  */
		addr->ether_addr_octet[cnt] = (unsigned char) number;

		/* Skip ':'.  */
		++asc;
	}
	return addr;
}

struct ether_addr *MF_EtherProto::ether_aton(const char *asc) {
	//MF_Log::mf_log(MF_DEBUG, "MF_EtherProto:ether_aton");
    static struct ether_addr result;
    return ether_aton_r(asc, &result);
}

char *MF_EtherProto::ether_ntoa_r (const struct ether_addr *addr, char *buf) {
	//MF_Log::mf_log(MF_DEBUG, "MF_EtherProto:ether_ntoa_r");
	sprintf (buf, "%02x:%02x:%02x:%02x:%02x:%02x",
			addr->ether_addr_octet[0], addr->ether_addr_octet[1],
			addr->ether_addr_octet[2], addr->ether_addr_octet[3],
			addr->ether_addr_octet[4], addr->ether_addr_octet[5]);
	return buf;
}

char *MF_EtherProto::ether_ntoa (const struct ether_addr *addr) {
	static char asc[18];
	return ether_ntoa_r (addr, asc);
}

