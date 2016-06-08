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
#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/select.h>
#include <unistd.h>

#include "constants.h"
#include "net_addr.h"
#include "exceptions.h"
#include "udpipv4_endpoint.h"

using namespace std;
using namespace network_exceptions;


UdpIpv4Endpoint::UdpIpv4Endpoint(NetAddr dest_addr, NetAddr local_addr) : 
    recv_timeout_sec(DEFAULT_RECV_TIMEOUT_SEC),  
    recv_timeout_usec(DEFAULT_RECV_TIMEOUT_USEC) {
    
    if(dest_addr.get_type() != NET_ADDR_TYPE_IPV4_PORT ||
        local_addr.get_type() != NET_ADDR_TYPE_IPV4_PORT) {
        stringstream msg;
        msg << "One or both address types of dest/lcl: '" 
            << dest_addr.get_type()
            << "/" + local_addr.get_type()
            << "' not supported. Only ipv4:port type!";
#ifdef DEBUG
        cerr << "ERROR: udpipv4: " << msg.str() << endl;
#endif
        throw NetAddrTypeException(msg.str());
    }

    struct hostent *dest_h;
    if (!(dest_h = gethostbyname(
                    dest_addr.get_hostname_or_ip().c_str()))) {
        stringstream msg;
        msg << "gethostbyname failed for: " 
            << dest_addr.get_hostname_or_ip();
#ifdef DEBUG
        cerr << "ERROR: udpipv4: " << msg.str() << endl;
#endif
        throw ResolveException(msg.str());
    }

    dest.sin_family = AF_INET;
    memcpy((char*)&dest.sin_addr.s_addr, dest_h->h_addr_list[0], 
                                            dest_h->h_length);
    dest.sin_port = htons(dest_addr.get_port());

    struct hostent *local_h;
    if (!(local_h = gethostbyname(
                    local_addr.get_hostname_or_ip().c_str()))) {
        stringstream msg;
        msg << "gethostbyname failed for: " 
            << local_addr.get_hostname_or_ip();
#ifdef DEBUG
        cerr << "ERROR: udpipv4: " << msg.str() << endl;
#endif
        throw ResolveException(msg.str());
    }

    local.sin_family = AF_INET;
    memcpy((char*)&local.sin_addr.s_addr, local_h->h_addr_list[0], 
                                            local_h->h_length);
    local.sin_port = htons(local_addr.get_port());

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        stringstream msg;
        msg << "socket create failed, err: " << strerror(errno);
#ifdef DEBUG
        cerr << "ERROR: udpipv4: " << msg.str() << endl;
#endif
        throw SocketException(msg.str());
    }

    //bind local addr/port
    if (bind(sock, (struct sockaddr *) &local, sizeof(local)) < 0) {
        stringstream msg;
        msg << "local bind failed, err: " << strerror(errno);
#ifdef DEBUG
        cerr << "ERROR: udpipv4: " << msg.str() << endl;
#endif
        throw ResolveException(msg.str());
    }

    //retrieve local port if supplied was 0
    if (!local_addr.get_port()) {
        socklen_t addr_len = sizeof(local);
        if (getsockname(sock, (struct sockaddr *)&local, &addr_len)) {
            stringstream msg;
            msg << "getsockname failed, err: " << strerror(errno);
#ifdef DEBUG
            cerr << "ERROR: udpipv4: " << msg.str() << endl;
#endif
            throw SocketException(msg.str());
        } else {
#ifdef DEBUG
            cout << "INFO: udpipv4: Local endpoint bound to port " 
                << ntohs(local.sin_port) << endl;
#endif
        }
    }
}

UdpIpv4Endpoint::~UdpIpv4Endpoint() throw () {

    int rc;
    
    if ((rc = close(sock))) {
#ifdef DEBUG
        cerr << "ERROR: udpipv4: sock close failed, err: " << strerror(errno) 
            << endl;
#endif
    }
}


int UdpIpv4Endpoint::_send(unsigned char* buf, int len) {

    int rc;

    if ((rc = sendto(sock, buf, len, 0, (struct sockaddr *) & dest,
                                            sizeof(dest))) < 0) {
        stringstream msg;
        msg << "sendto failed, err: " << strerror(errno);
#ifdef DEBUG
        cerr << "ERROR: udpipv4: " << msg.str() << endl;
#endif
        throw NetIOException(msg.str());
    }
    return rc;
}

int UdpIpv4Endpoint::_recv(unsigned char* buf, int len) {

    int rc;
    check_recv_ready();
    if ((rc = recv(sock, buf, len, 0)) < 0) {
        stringstream msg;
        msg << "recv failed, err: " << strerror(errno) << endl;
#ifdef DEBUG
        cerr << "ERROR: udpipv4: " << msg.str() << endl;
#endif
        throw NetIOException(msg.str());
    }
    return rc;
}

int UdpIpv4Endpoint::get_local_port() {

    return ntohs(local.sin_port);
}

/* set timeout for recv in milliseconds */
void UdpIpv4Endpoint::set_recv_timeout(long ms) {

    recv_timeout_sec = ms/1000;
    recv_timeout_usec = (ms % 1000) * 1000;
}

void UdpIpv4Endpoint::check_recv_ready() {

   fd_set read_fds;
   struct timeval tv;
   int rc;

   FD_ZERO(&read_fds);
   FD_SET(sock, &read_fds);

   tv.tv_sec = recv_timeout_sec;
   tv.tv_usec = recv_timeout_usec;

   if((rc = select(sock + 1, &read_fds, NULL, NULL, &tv)) == -1) {
        stringstream msg;
        msg << "select failed, err: " << strerror(errno);
#ifdef DEBUG
        cerr << "ERROR: udpipv4: " << msg.str() << endl;
#endif
        throw NetIOException(msg.str());
    } else if (!rc) {
        throw TimeoutException();
    }
}
