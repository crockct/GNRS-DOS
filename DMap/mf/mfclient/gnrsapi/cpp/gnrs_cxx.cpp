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
#include <vector>

#include "gnrs_cxx.h"
#include "exceptions.h"

using namespace std;
using namespace network_exceptions;

list<NetAddr>
Gnrs::lookup(GUID &guid) {

    list<NetAddr> a_list;

    unsigned char buf[2048];
    
    req_t req; 

    req.version = PROTOCOL_VERSION;
    req.type = LOOKUP_REQUEST;
    //set req.len later
    req.id = get_request_id();

    //if no local port was specified for an ipv4:port type, set local port
    //that was assigned at time of local bind
    //WARN: the following code is endpoint implementation specific
    if(local_addr.get_type() == NET_ADDR_TYPE_IPV4_PORT 
        && !local_addr.get_port()) {
        local_addr.set_port(ue.get_local_port());
    }

    //set local endpoint info to receive response from server
    req.src_addr = local_addr.get_tlv();

    //lookup request payload
    lookup_t lkup; 
    memcpy(lkup.guid, guid.getBytes(), GUID_LENGTH);
    req.data_len = sizeof(lookup_t);
    req.data = (void*) &lkup;

    //set up options 
	opt_tlv_t opts[1];
	uint16_t& opts_len = req.opts_len;
	uint16_t& num_opts = req.num_opts;
	opts_len = 0;
	num_opts = 0;

    //redirect option: allow request to be redirected to other GNRS servers
    unsigned char redirect_val[] = {0x00, 0x01};
    opt_tlv_t* redirect_opt = &opts[0];

    redirect_opt->type = OPTION_REQUEST_REDIRECT;
    redirect_opt->len = 2;
    redirect_opt->value = (unsigned char*)&redirect_val;
	opts_len += 2 + redirect_opt->len;
	num_opts++;

    req.opts = opts;

    if ((req.len = GnrsMessageHelper::
        build_request_msg(req, (unsigned char*)&buf, 2048)) < 0) {
#ifdef DEBUG
        cout << "ERROR: gnrs_cxx: Error building lookup req message!" << endl;
#endif
       //TODO throw an exception
		return a_list;
	}
#ifdef DEBUG
    cout << "DEBUG: gnrs_cxx: Built lookup msg, size: " << req.len << endl;
#endif

    //send request to server
    int rc;
    if ((rc = _send((unsigned char*)&buf, req.len)) != req.len) {
#ifdef DEBUG
       cerr << "ERROR: gnrs_cxx: Error in send of lookup request" <<  endl;
#endif
       //TODO throw an exception
        return a_list;
    }
#ifdef DEBUG
    cout << "DEBUG: gnrs_cxx: Sent lookup request to server. " 
        << "msg size: " << req.len << endl;
#endif
    
    //attemp response recv 
    int len;
    if((len = _recv((unsigned char*)&buf, 2048)) < 0){
#ifdef DEBUG
       cerr << "ERROR: gnrs_cxx: Error in recv of lookup response" <<  endl;
       //TODO throw an exception
#endif
        return a_list;
    }

    //parse lookup response and create result 
    resp_t rsp;
    GnrsMessageHelper::parse_response_msg((unsigned char*)&buf, len, rsp);

    if (rsp.req_id == req.id && rsp.type == LOOKUP_RESPONSE) {
#ifdef DEBUG
        cout << "DEBUG: gnrs_cxx: Lookup response code: " << rsp.code << endl;
#endif

        lookup_resp_t* lr = &rsp.lkup_data;

        for (unsigned i = 0; i < lr->size; i++) {
            NetAddr a(lr->addrs[i].type, lr->addrs[i].value, lr->addrs[i].len);
            a_list.push_back(a);
        } 
    } else {
#ifdef DEBUG
        cerr << "ERROR: gnrs_cxx: Mismatch in response;" 
		<< " Expected LOOKUP_RESPONSE got : " 
        	<< rsp.type << "; Expected req id: " << req.id << " got: "
        	<< rsp.req_id << endl;
#endif
        //TODO go back to receive after storing the response
    }

    return a_list;
}

void 
Gnrs::add(GUID &guid, list<NetAddr>& addrs) {

    unsigned char buf[2048];
    req_t req;

    req.version = PROTOCOL_VERSION;
    req.type = INSERT_REQUEST;
    //set req.len later
    req.id = get_request_id();

    req.src_addr = local_addr.get_tlv();

    //insert request payload
    upsert_t ups; 
    memcpy(ups.guid, guid.getBytes(), GUID_LENGTH);
    //count data len
    int data_len = GUID_LENGTH + 4;
    //set the address entries
    vector<addr_tlv_t> addrs_v;
    for (list<NetAddr>::iterator it = addrs.begin(); it != addrs.end(); it++) {
        addr_tlv_t at;
        addrs_v.push_back(at = (*it).get_tlv());
        data_len += at.len + 4;
    }
    ups.addrs = &addrs_v[0];
    ups.size = addrs_v.size();

    req.data = (void*) &ups;
    req.data_len = data_len;

    //set up options 
    opt_tlv_t opts[1];
    uint16_t& opts_len = req.opts_len;
    uint16_t& num_opts = req.num_opts;
    opts_len = 0;
    num_opts = 0;

    //redirect option: allow request to be redirected to other GNRS servers
    unsigned char redirect_val[] = {0x00, 0x01};
    opt_tlv_t* redirect_opt = &opts[0];

    redirect_opt->type = OPTION_REQUEST_REDIRECT;
    redirect_opt->len = 2;
    redirect_opt->value = (unsigned char*)&redirect_val;
    opts_len += 2 + redirect_opt->len;
    num_opts++;

    req.opts = opts;

    if ((req.len = GnrsMessageHelper::
        build_request_msg(req, (unsigned char*)&buf, 2048)) < 0) {
#ifdef DEBUG
        cout << "ERROR: gnrs_cxx: Error building insert req message!" << endl;
#endif
       //TODO throw an exception
		return;
	}
#ifdef DEBUG
    cout << "DEBUG: gnrs_cxx: Built insert msg, size: " << req.len << endl;
#endif

    //send request to server
    int rc;
    if ((rc = _send((unsigned char*)&buf, req.len)) != req.len) {
#ifdef DEBUG
       cerr << "ERROR: gnrs_cxx: Error in sending insert msg" <<  endl;
#endif
       //TODO throw an exception
        return;
    }
#ifdef DEBUG
    cout << "DEBUG: gnrs_cxx: Sent insert request to server. "
        << "msg size: " << req.len << endl;
#endif
    
    //attemp response recv 
    int len;
    if((len = _recv((unsigned char*)&buf, 2048)) < 0){
#ifdef DEBUG
       cerr << "ERROR: gnrs_cxx: Error in recv of insert response" <<  endl;
#endif
       //TODO throw an exception
        return;
    }

    //parse lookup response and create result 

    resp_t rsp; 
    
    GnrsMessageHelper::parse_response_msg((unsigned char*)&buf, len, rsp);

    if (rsp.req_id == req.id && rsp.type == INSERT_RESPONSE) {
#ifdef DEBUG
        cout << "DEBUG: gnrs_cxx: Insert result code: " << rsp.code << endl;
#endif
    } else {
#ifdef DEBUG
        cerr << "ERROR: gnrs_cxx: Mismatch in response;" 
		<< " Expected INSERT_RESPONSE got : " 
        	<< rsp.type << "; expected req id: " << req.id 
		<< " got: " << rsp.req_id << endl;
#endif
        //TODO go back to receive correct resp after storing this one
    }
}


void 
Gnrs::update(GUID &guid, list<NetAddr>& addrs) {
    unsigned char buf[2048];
    req_t req;

    req.version = PROTOCOL_VERSION;
    req.type = UPDATE_REQUEST;
    //set req.len later
    req.id = get_request_id();

    req.src_addr = local_addr.get_tlv();

    //insert request payload
    upsert_t ups; 
    memcpy(ups.guid, guid.getBytes(), GUID_LENGTH);
    //count data len
    int data_len = GUID_LENGTH + 4;
    //set the address entries
    vector<addr_tlv_t> addrs_v;
    for (list<NetAddr>::iterator it = addrs.begin(); it != addrs.end(); it++) {
        addr_tlv_t at;
        addrs_v.push_back(at = (*it).get_tlv());
        data_len += at.len + 4;
    }
    ups.addrs = &addrs_v[0];
    ups.size = addrs_v.size();

    req.data = (void*) &ups;
    req.data_len = data_len;

    //set up options 
    opt_tlv_t opts[1];
    uint16_t& opts_len = req.opts_len;
    uint16_t& num_opts = req.num_opts;
    opts_len = 0;
    num_opts = 0;

    //redirect option: allow request to be redirected to other GNRS servers
    unsigned char redirect_val[] = {0x00, 0x01};
    opt_tlv_t* redirect_opt = &opts[0];

    redirect_opt->type = OPTION_REQUEST_REDIRECT;
    redirect_opt->len = 2;
    redirect_opt->value = (unsigned char*)&redirect_val;
    opts_len += 2 + redirect_opt->len;
    num_opts++;

    req.opts = opts;

    if ((req.len = GnrsMessageHelper::
        build_request_msg(req, (unsigned char*)&buf, 2048)) < 0) {
#ifdef DEBUG
        cout << "ERROR: gnrs_cxx: Error building update req message!" << endl;
#endif
       //TODO throw an exception
		return;
	}
#ifdef DEBUG
    cout << "DEBUG: gnrs_cxx: Built update msg, size: " << req.len << endl;
#endif

    //send request to server
    int rc;
    if ((rc = _send((unsigned char*)&buf, req.len)) != req.len) {
#ifdef DEBUG
       cerr << "ERROR: gnrs_cxx: Error in sending update msg" <<  endl;
#endif
       //TODO throw an exception
        return;
    }
#ifdef DEBUG
    cout << "DEBUG: gnrs_cxx: Sent update request to server. "
        << "msg size: " << req.len << endl;
#endif
    
    //attemp response recv 
    int len;
    if((len = _recv((unsigned char*)&buf, 2048)) < 0){
#ifdef DEBUG
       cerr << "ERROR: gnrs_cxx: Error in recv of update response" <<  endl;
#endif
       //TODO throw an exception
        return;
    }

    //parse lookup response and create result 

    resp_t rsp; 
    
    GnrsMessageHelper::parse_response_msg((unsigned char*)&buf, len, rsp);

    if (rsp.req_id == req.id && rsp.type == UPDATE_RESPONSE) {
#ifdef DEBUG
        cout << "DEBUG: gnrs_cxx: Update result code: " << rsp.code << endl;
#endif
    } else {
#ifdef DEBUG
        cerr << "ERROR: gnrs_cxx: Mismatch in response;" 
		<< " Expected UPDATE_RESPONSE got : " 
        	<< rsp.type << "; expected req id: " << req.id 
		<< " got: " << rsp.req_id << endl;
#endif
        //TODO go back to receive correct resp after storing this one
    }
}
