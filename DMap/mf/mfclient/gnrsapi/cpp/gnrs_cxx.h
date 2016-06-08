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
#ifndef GNRS_CXX_H
#define GNRS_CXX_H


#ifndef __ANDROID__
#include <string>
#endif

#include <list>
#include <string.h>
#include <include/gnrsmessages.hh>
#include <include/mfguid.hh>

#include "common.h"
#include "net_addr.h"
#include "udpipv4_endpoint.h"
#include "exceptions.h"

using namespace network_exceptions;

class Gnrs{

    public:

        /**
         * Constructor that sets up service access and local addresses.
         *
         * 
         * Refer to wire protocol documentation to learn about address
         * types supported by the current implementation.
         */
        Gnrs(NetAddr server, NetAddr local): 
            server_addr(server), local_addr(local), request_id(1),
		ue(server_addr, local_addr) {}

        /**
         * Destructor
         * 
         */
        ~Gnrs() {
        }

        /**
         * Retreive address(es) currently mapped to given guid.
         *
         */

        list<NetAddr> lookup(GUID &guid);

        /** 
         * Add guid to address(es) mapping. 
         *
         * If a mapping already exists at the time the request is processed 
         * (at the server), then the set of new addresses is added to the
         * mapping - not replaced.
         *
         */
        void add(GUID &guid, list<NetAddr>& addrs);

        /**
         * Replace existing mapping (if any) with new set of addresses.
         *
         * The old mappings (set of addresses, and any related options)
         * are deleted from the directory. If no mapping exists at the time
         * request is serviced, then this is equivalent to the add(...)
         * operation.
         *
         */
        void update(GUID &guid, list<NetAddr>& addrs);

    private:
        NetAddr server_addr;
        NetAddr local_addr;
        uint32_t request_id;
        uint16_t req_len_offset;
        UdpIpv4Endpoint ue;

        int _send(unsigned char* buf, int len) {
            //UDP/IP default transport
            return ue._send(buf, len);
        }

        int _recv(unsigned char* buf, int len) {
            return ue._recv(buf, len);
        }

        //TODO: make thread safe
        uint32_t get_request_id() { return request_id++;}

};

#endif //GNRS_CXX_H
