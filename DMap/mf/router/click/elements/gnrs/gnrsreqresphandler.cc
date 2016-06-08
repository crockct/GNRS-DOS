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
#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/ether.h>

#include "mf.hh"
#include "gnrsreqresphandler.hh"
#include "gnrsmessages.hh"
#include "gnrsnetaddr.hh"
#include "mffunctions.hh"
#include "mflogger.hh"

CLICK_DECLS

GNRS_ReqRespHandler::GNRS_ReqRespHandler() 
: _req_id(0), logger(MF_Logger::init()), _reqCache(), _reqPorts() {
	pthread_mutex_init(&reqCacheLock, NULL);
}

GNRS_ReqRespHandler::~GNRS_ReqRespHandler() {
}

/** 
 * Required params: 
 * 1.) MY_GUID: self GUID
 * 2.) NET_ID: network id 
 * 3.) RESP_LISTEN_IP: IP tos listen on for GNRS server responses 
 * 4.) RESP_LISTEN_PORT: port to listen on for GNRS server responses 
 */
int GNRS_ReqRespHandler::configure(Vector<String> &conf, ErrorHandler *errh) {
	if (cp_va_kparse(conf, this, errh,
			"MY_GUID", cpkP+cpkM, cpUnsigned, &my_guid,
			"NET_ID", cpkP+cpkM, cpString, &net_id,
			"RESP_LISTEN_IP", cpkP+cpkM, cpString, &my_ip,
			"RESP_LISTEN_PORT", cpkP+cpkM, cpUnsigned, &my_udp_port,
			"RESP_CACHE", cpkP+cpkM, cpElement, &_respCache,
			cpEnd) < 0) {
		return -1;
	}
	return 0;
}

void GNRS_ReqRespHandler::push(int port, Packet *p) {
	if (port == 0) {
		handleReqPkt(p);
	} else if (port == 1) {
		handleRespPkt(p);
	} else {
		logger.log(MF_ERROR,
				"GNRS_ReqRespHandler: Unrecognized packet on port %d", port);
		p->kill();
	}
}

void GNRS_ReqRespHandler::handleReqPkt(Packet *p, uint32_t resend) {
	//gnrs request from a click element in custom format
	uint8_t portNumber = p->anno_u8(GNRS_SERVICE_ANNO);
	gnrs_req *req = (gnrs_req *)p->data();
	uint32_t req_type = req->type;
	GUID_t guid;
	guid.init(req->GUID);
	WritablePacket *out_req_pkt;
	/*
	 * Prepares GNRS lookup
	 */
	if (req_type == GNRS_LOOKUP) {
		//check first whether it's in cache or not
		gnrs_lkup_resp_t resp;
		if(_respCache->get_response(guid, &resp)){
			//INFO assumes that if multiple requests for the same GUID will request the same types
			logger.log(MF_DEBUG, "GNRS_ReqRespHandler: GUID %d available in cache", guid.to_int());
			out_req_pkt = Packet::make(0, 0, sizeof(gnrs_lkup_resp), 0);
			assert(out_req_pkt);
			memcpy(out_req_pkt->data(), &resp, sizeof(gnrs_lkup_resp));
			out_req_pkt->set_anno_u8(GNRS_SERVICE_ANNO, portNumber);
			output(1).push(out_req_pkt);
		} else {
			//checks whether there are pending requests for the GUID
			ReqPorts::iterator it =_reqPorts.find(guid.to_int());
			if(it!=_reqPorts.end() && !it->second->empty()){
				logger.log(MF_DEBUG,
						"GNRS_ReqRespHandler: Storing query for GUID %d as there is one already pending",
						guid.to_int());
				it->second->push_back(portNumber);
			} else {
				//Create a new lookup request
				if(it==_reqPorts.end()){
					PortsStack *stack = new PortsStack();
					stack->push_back(portNumber);
					_reqPorts.set(guid.to_int(), stack);
					logger.log(MF_DEBUG,
							"GNRS_ReqRespHandler: First request for GUID %d, creating new ports stack",
							guid.to_int());
				} else {
					logger.log(MF_DEBUG,
							"GNRS_ReqRespHandler: Stack already created for GUID %d, creating new query",
							guid.to_int());
					it->second->push_back(portNumber);
				}
				out_req_pkt = createGNRSLookup(req, guid);
				if(out_req_pkt!=NULL){
					store_request(_req_id, p, resend);
					output(0).push(out_req_pkt);
				} else {
					logger.log(MF_ERROR,
							"GNRS_ReqRespHandler: creating lookup query");
					p->kill();
				}
			}
		}

		/*
		 * Prepares INSERT
		 */
	} else if (req_type == GNRS_INSERT) {
		out_req_pkt = createGNRSInsert(req, guid);
		if(out_req_pkt!=NULL){
			store_request(_req_id, p, 0);
			output(0).push(out_req_pkt);
		}

		/*
		 * Prepares UPDATE
		 */
	} else if (req_type == GNRS_UPDATE) {
		out_req_pkt = createGNRSUpdate(req, guid);
		if(out_req_pkt!=NULL){
			store_request(_req_id, p, 0);
			output(0).push(out_req_pkt);
		}

	} else{//unknown
		logger.log(MF_WARN,
				"GNRS_ReqRespHandler: Unknown msg type: %u at in-port 0", req_type);
		p->kill();
		return;
	}
}

void GNRS_ReqRespHandler::handleRespPkt(Packet *p) {
	//udp response packet from GNRS server
	resp_t rsp;

	GnrsMessageHelper::parse_response_msg((unsigned char*)p->data(), p->length(), rsp);

	if (rsp.status == RESPONSE_INCOMPLETE) {
		logger.log(MF_ERROR,
				"GNRS_ReqRespHandler: Received incomplete response! len: %u", p->length());
		p->kill();
		return;
	}

	gnrs_req_t * req = get_request(rsp.req_id);
	if (req == NULL) {
		logger.log(MF_ERROR,
				"GNRS_ReqRespHandler: can't find matching request for resp; req id: %u", rsp.req_id);
		p->kill();
		return;
	}
	GUID_t guid_c;
	guid_c.init(req->GUID);
	GUID_t guid(guid_c);
	char buf[GUID_LOG_BUF_SIZE];

	if (rsp.type == INSERT_RESPONSE) {
		if (rsp.code == RESPONSE_FAILURE) {
			logger.log(MF_ERROR,
					"GNRS_ReqRespHandler: insert failed for req id: %u, response code: %d",
					rsp.req_id, rsp.code);
			//TODO decide based on response code if should retry
		}
		cleanup_request(rsp.req_id);
	} else if (rsp.type == UPDATE_RESPONSE) {
		if (rsp.code == RESPONSE_FAILURE) {
			logger.log(MF_ERROR,
					"GNRS_ReqRespHandler: insert failed for req id: %u, response code: %d",
					rsp.req_id, rsp.code);
			//TODO decide based on response code if should retry
		}
		cleanup_request(rsp.req_id);
	} else if(rsp.type == LOOKUP_RESPONSE){
		if (rsp.code == RESPONSE_FAILURE) {
			logger.log(MF_ERROR,
					"GNRS_ReqRespHandler: Lookup failed for req id: %u, response code: %d",
					rsp.req_id, rsp.code);
			//TODO decide based on response code if should retry
			cleanup_request(rsp.req_id);
			p->kill();
			return;
		}
		logger.log(MF_DEBUG,
				"GNRS_ReqRespHandler: recvd lookup response req id: %u, response code: %d",
				rsp.req_id, rsp.code);
		/* prepare internal resp message and send to requestor element */
		ReqPorts::iterator it =_reqPorts.find(guid.to_int());
		if(it == _reqPorts.end() || it->second->empty()){
			logger.log(MF_WARN,
					"GNRS_ReqRespHandler: no pending lookup for guid: %d",
					guid.to_int());
			p->kill();
			return;
		}
		WritablePacket *out_resp_pkt = NULL;
		out_resp_pkt = Packet::make(0, 0, sizeof(gnrs_lkup_resp), 0);
		assert(out_resp_pkt);
		memset(out_resp_pkt->data(), 0, out_resp_pkt->length());
		gnrs_lkup_resp_t  *resp = (gnrs_lkup_resp_t*)out_resp_pkt->data();
		memcpy(resp->GUID, guid.getGUID(), GUID_LENGTH);
		int num_na = 0;
		for (int i = 0; (i < rsp.lkup_data.size) && (num_na < LOOKUP_MAX_NA); ++i) {
			//Currently, NA is being encoded as a GUID, so all
			//responses are expected to in TYPE GUID
			if (rsp.lkup_data.addrs[i].type == NET_ADDR_TYPE_GUID) {
				//TODO since we overload NA and GUID types due
				//to GNRS server limitation, the handling will
				//be identical. It works since the GUID currently
				//is a 4 byte encoded as last 4 bytes of 20 bytes,
				//and NA is net(16):local(4), where local is essentially
				//a guid.
				NetAddr na(NET_ADDR_TYPE_NA,
						rsp.lkup_data.addrs[i].value,
						rsp.lkup_data.addrs[i].len);
				resp->NAs[num_na].net_addr = na.netaddr_local;
				//resp->NAs[num_na].TTL =
				//resp->NAs[num_na].weight =

				logger.log(MF_DEBUG,
						"GNRS_ReqRespHandler: lookup resp for guid: %s -> LA: %u",
						guid.to_log(buf), resp->NAs[num_na].net_addr);
				num_na++;

			} else if (rsp.lkup_data.addrs[i].type == NET_ADDR_TYPE_NA) {

				NetAddr na(NET_ADDR_TYPE_NA, rsp.lkup_data.addrs[i].value,
						rsp.lkup_data.addrs[i].len);
				resp->NAs[num_na].net_addr = na.netaddr_local;
				//resp->NAs[num_na].TTL =
				//resp->NAs[num_na].weight =
				logger.log(MF_DEBUG,
						"GNRS_ReqRespHandler: lookup resp for guid: %u -> NA/LA: %s/%u",
						resp->GUID, na.netaddr_net.c_str(), resp->NAs[num_na].net_addr);
				num_na++;

			} else {
				logger.log(MF_WARN,
						"GNRS_ReqRespHandler: Unsupported addr encoding in lookup response; type: %d",
						rsp.lkup_data.addrs[i].type);
				//skip this entry
				continue;
			}
		}
		resp->na_num = num_na;
		//push gnrs lookup response to element downstream on port 1
		_respCache->insert(guid, out_resp_pkt);
		PortsStack::iterator portsIterator = it->second->begin();
		out_resp_pkt->set_anno_u8(GNRS_SERVICE_ANNO, *portsIterator);
		output(1).push(out_resp_pkt);
		portsIterator++;
		while(portsIterator!=it->second->end()){
			Packet *cloned_pkt = out_resp_pkt->clone();
			out_resp_pkt->set_anno_u8(GNRS_SERVICE_ANNO, *portsIterator);
			output(1).push(out_resp_pkt);
		}
		it->second->clear();
		cleanup_request(rsp.req_id);
	} else{//unknown
		logger.log(MF_WARN,"GNRS_ReqRespHandler: Got unknown msg type at in-port 1");
	}
	p->kill();
}

WritablePacket *GNRS_ReqRespHandler::createGNRSLookup(gnrs_req *req, GUID &guid){
	char buf[GUID_LOG_BUF_SIZE];
	//TODO define NA
	uint16_t weight = req->weight;
	//uint32_t ttl = DEFAULT_MAPPING_TTL_SECS;

	WritablePacket *out_req_pkt = NULL;
	req_t req_msg;

	req_msg.version = PROTOCOL_VERSION;
	req_msg.type = LOOKUP_REQUEST;
	//set len later
	req_msg.id = get_request_id();
	char tmp[32];
	sprintf(tmp, "%s:%u", my_ip.c_str(), my_udp_port);
	string addr_s(tmp);
	NetAddr local_addr(NET_ADDR_TYPE_IPV4_PORT, addr_s);
	req_msg.src_addr = local_addr.get_tlv();

	//lookup request payload
	lookup_t lkup;
	memcpy(lkup.guid, guid.getGUID(), GUID_LENGTH);
	req_msg.data_len = sizeof(lookup_t);
	req_msg.data = (void*) &lkup;

	//set up options
	opt_tlv_t opts[1];
	uint16_t& opts_len = req_msg.opts_len;
	uint16_t& num_opts = req_msg.num_opts;
	opts_len = 0;
	num_opts = 0;
	//redirect option: allow request redirection to other GNRS servers
	unsigned char redirect_val[] = {0x00, 0x01};
	opt_tlv_t* redirect_opt = &opts[0];
	redirect_opt->type = OPTION_REQUEST_REDIRECT;
	redirect_opt->len = 2;
	redirect_opt->value = (unsigned char*)&redirect_val;
	opts_len += 2 + redirect_opt->len;
	num_opts++;

	req_msg.opts = opts;

	uint16_t msg_len = 16 + req_msg.src_addr.len
			+ req_msg.data_len + req_msg.opts_len;
	out_req_pkt = Packet::make(0, 0, msg_len, 0);
	assert(out_req_pkt);
	if ((GnrsMessageHelper::build_request_msg(req_msg,
			out_req_pkt->data(), out_req_pkt->length())) < 0) {
		/* error building message */
		logger.log(MF_ERROR,
				"GNRS_ReqRespHandler: error building lookup message of len: %u, "
				"using pkt buff of size: %u or guid: %s req id: %u",
				msg_len, out_req_pkt->length(), guid.to_log(buf), req_msg.id);
		out_req_pkt->kill();
		return NULL;

	} else {
		logger.log(MF_DEBUG,
				"GNRS_ReqRespHandler: sent lookup for guid: %s req id: %u",
				guid.to_log(buf), _req_id);
		return out_req_pkt;
	}
}

WritablePacket *GNRS_ReqRespHandler::createGNRSInsert(gnrs_req *req, GUID &guid){
	char buf[GUID_LOG_BUF_SIZE];
	//TODO define NA
	uint16_t weight = req->weight;
	//uint32_t ttl = DEFAULT_MAPPING_TTL_SECS;
	WritablePacket *out_req_pkt = NULL;
	req_t req_msg;
	req_msg.version = PROTOCOL_VERSION;
	//req_msg.type = INSERT_REQUEST;
	req_msg.type = INSERT_REQUEST;

	//set req_msg.len later
	req_msg.id = get_request_id();

	char tmp[32];
	sprintf(tmp, "%s:%u", my_ip.c_str(), my_udp_port);
	string addr_s(tmp);
	NetAddr local_addr(NET_ADDR_TYPE_IPV4_PORT, addr_s);
	req_msg.src_addr = local_addr.get_tlv();

	//insert request payload
	upsert_t ups;
	memcpy(ups.guid, guid.getGUID(), GUID_LENGTH);
	int data_len = GUID_LENGTH + 4;

	//set the one address entry corresponding to this router
	//TODO maybe the NA ought to arrive in the request, and not
	//determined here
	addr_tlv_t addrs_v[1];
	sprintf(tmp, "%s:%u", net_id.c_str(), my_guid);
	addr_s.assign(tmp);
	NetAddr na(NET_ADDR_TYPE_NA, addr_s);
	addrs_v[0] = na.get_tlv();
	data_len += na.len + 4;

	ups.addrs = &addrs_v[0];
	ups.size = 1;

	req_msg.data = (void*) &ups;
	req_msg.data_len = data_len;

	//set up options
	opt_tlv_t opts[2]; // 1 for redirection, another for binding expiration time

	// redirection opt
	uint16_t& opts_len = req_msg.opts_len;
	uint16_t& num_opts = req_msg.num_opts;
	opts_len = 0;
	num_opts = 0;

	//redirect option: allow request redirection to other GNRS servers
	unsigned char redirect_val[] = {0x00, 0x01};
	opt_tlv_t* redirect_opt = &opts[0];
	redirect_opt->type = OPTION_REQUEST_REDIRECT;
	redirect_opt->len = 2;
	redirect_opt->value = (unsigned char*)&redirect_val;

	// binding expiration time opt
	opt_tlv_t *expopt = &opts[1];
	expopt->type = OPTION_NA_EXPIRATION;
	expopt->len = 8 * 1; // currently router deals with one binding at a time

	struct timeval tp;
	gettimeofday(&tp, NULL);

	// make sure uint64_t is actually long long
	uint64_t exptime = ((uint64_t) tp.tv_sec * 1000L + tp.tv_usec / 1000) + GNRS_ENTRY_EXPIRATION_LENGTH;

	expopt->value = (uint8_t*) &exptime;
	MF_Functions::byteswap(expopt->value, 8);

	opts_len += (2 + redirect_opt->len) + (2 + expopt->len) ;
	num_opts += 2;

	req_msg.opts = opts;

	uint16_t msg_len = 16 + req_msg.src_addr.len
			+ req_msg.data_len + req_msg.opts_len;

	/*
		    logger.log(MF_DEBUG, "gnrs_rrh: req message to be generated with: type: %d, exptime: %lld"
		              "\n", req_msg.type, exptime);
		    logger.log(MF_DEBUG, "gnrs_rrh: req opt exp time in char int: ");
		    for (int i=0; i<8; ++i) logger.log(MF_DEBUG, "gnrs_rrh: %u", *(expopt->value + i));
		    char buff[1000];
		    memset(buff, 0, 1000);
		    logger.log(MF_DEBUG, "%s", to_log(req_msg, buff));
	 */

	out_req_pkt = Packet::make(0, 0, msg_len, 0);
	assert(out_req_pkt);
	if ((GnrsMessageHelper::build_request_msg(req_msg,
			out_req_pkt->data(), out_req_pkt->length())) < 0) {
		/* error building message */
		logger.log(MF_ERROR,
				"GNRS_ReqRespHandler: error building insert message of len: %u, "
				"using pkt buff of size: %u or guid: %s req id: %u",
				msg_len, out_req_pkt->length(), guid.to_log(buf), req_msg.id);
		out_req_pkt->kill();
		return NULL;
	} else {
		logger.log(MF_DEBUG,
				"gnrs_rrh: prepared insert for guid: %s with req id: %u weight: %u",
				guid.to_log(buf), _req_id, weight);
		return out_req_pkt;
	}
}

WritablePacket *GNRS_ReqRespHandler::createGNRSUpdate(gnrs_req *req, GUID &guid){
	char buf[GUID_LOG_BUF_SIZE];
	//TODO define NA
	uint16_t weight = req->weight;
	//uint32_t ttl = DEFAULT_MAPPING_TTL_SECS;
	WritablePacket *out_req_pkt = NULL;
	req_t req_msg;
	req_msg.version = PROTOCOL_VERSION;
	//req_msg.type = INSERT_REQUEST;
	req_msg.type = UPDATE_REQUEST;

	//set req_msg.len later
	req_msg.id = get_request_id();

	char tmp[32];
	sprintf(tmp, "%s:%u", my_ip.c_str(), my_udp_port);
	string addr_s(tmp);
	NetAddr local_addr(NET_ADDR_TYPE_IPV4_PORT, addr_s);
	req_msg.src_addr = local_addr.get_tlv();

	//insert request payload
	upsert_t ups;
	memcpy(ups.guid, guid.getGUID(), GUID_LENGTH);
	int data_len = GUID_LENGTH + 4;

	//set the one address entry corresponding to this router
	//TODO maybe the NA ought to arrive in the request, and not
	//determined here
	addr_tlv_t addrs_v[1];
	sprintf(tmp, "%s:%u", net_id.c_str(), my_guid);
	addr_s.assign(tmp);
	NetAddr na(NET_ADDR_TYPE_NA, addr_s);
	addrs_v[0] = na.get_tlv();
	data_len += na.len + 4;

	ups.addrs = &addrs_v[0];
	ups.size = 1;

	req_msg.data = (void*) &ups;
	req_msg.data_len = data_len;

	//set up options
	opt_tlv_t opts[2]; // 1 for redirection, another for binding expiration time

	// redirection opt
	uint16_t& opts_len = req_msg.opts_len;
	uint16_t& num_opts = req_msg.num_opts;
	opts_len = 0;
	num_opts = 0;

	//redirect option: allow request redirection to other GNRS servers
	unsigned char redirect_val[] = {0x00, 0x01};
	opt_tlv_t* redirect_opt = &opts[0];
	redirect_opt->type = OPTION_REQUEST_REDIRECT;
	redirect_opt->len = 2;
	redirect_opt->value = (unsigned char*)&redirect_val;

	// binding expiration time opt
	opt_tlv_t *expopt = &opts[1];
	expopt->type = OPTION_NA_EXPIRATION;
	expopt->len = 8 * 1; // currently router deals with one binding at a time

	struct timeval tp;
	gettimeofday(&tp, NULL);

	// make sure uint64_t is actually long long
	uint64_t exptime = ((uint64_t) tp.tv_sec * 1000L + tp.tv_usec / 1000) + GNRS_ENTRY_EXPIRATION_LENGTH;

	expopt->value = (uint8_t*) &exptime;
	MF_Functions::byteswap(expopt->value, 8);

	opts_len += (2 + redirect_opt->len) + (2 + expopt->len) ;
	num_opts += 2;

	req_msg.opts = opts;

	uint16_t msg_len = 16 + req_msg.src_addr.len
			+ req_msg.data_len + req_msg.opts_len;

	/*
	    logger.log(MF_DEBUG, "gnrs_rrh: req message to be generated with: type: %d, exptime: %lld"
	              "\n", req_msg.type, exptime);
	    logger.log(MF_DEBUG, "gnrs_rrh: req opt exp time in char int: ");
	    for (int i=0; i<8; ++i) logger.log(MF_DEBUG, "gnrs_rrh: %u", *(expopt->value + i));
	    char buff[1000];
	    memset(buff, 0, 1000);
	    logger.log(MF_DEBUG, "%s", to_log(req_msg, buff));
	 */

	out_req_pkt = Packet::make(0, 0, msg_len, 0);
	assert(out_req_pkt);
	if ((GnrsMessageHelper::build_request_msg(req_msg,
			out_req_pkt->data(), out_req_pkt->length())) < 0) {
		/* error building message */
		logger.log(MF_ERROR,
				"GNRS_ReqRespHandler: error building update message of len: %u, "
				"using pkt buff of size: %u or guid: %s req id: %u",
				msg_len, out_req_pkt->length(), guid.to_log(buf), req_msg.id);
		out_req_pkt->kill();
		return NULL;
	} else {
		logger.log(MF_DEBUG,
				"gnrs_rrh: prepared update for guid: %s with req id: %u weight: %u",
				guid.to_log(buf), _req_id, weight);
		return out_req_pkt;
	}
}

bool GNRS_ReqRespHandler::store_request(uint32_t req_id, Packet *req_pkt, 
		uint32_t resend) {
	gnrs_req_cache_record_t *record =
			(gnrs_req_cache_record_t*)malloc(sizeof(gnrs_req_cache_record_t));
	record->req_pkt = req_pkt;
	timer_data_t *timerdata = new timer_data_t();
	timerdata->me = this;
	timerdata->req_id = req_id;
	timerdata->resend = resend;
	record->expiry = new Timer(&GNRS_ReqRespHandler::handleReqExpiry, timerdata);
	record->expiry->initialize(this);
	record->expiry->schedule_after_sec(GNRS_REQ_TIMEOUT_SEC);

	logger.log(MF_DEBUG,
			"GNRS_ReqRespHandler: insert req_id %u", req_id);

	pthread_mutex_lock(&reqCacheLock);
	bool ret = _reqCache.set(req_id, record);
	pthread_mutex_unlock(&reqCacheLock);
	return ret;
}

gnrs_req_t * GNRS_ReqRespHandler::get_request(uint32_t req_id) {
	ReqCache::iterator it = _reqCache.find(req_id);
	if (it == _reqCache.end()) {
		logger.log(MF_DEBUG, "GNRS_ReqRespHandler: no req_id %u entry", req_id);
		return NULL;
	} else {
		logger.log(MF_DEBUG,
				"GNRS_ReqRespHandler: find req_id %u record in reqCache", req_id);

		return (gnrs_req_t*)it.value()->req_pkt->data();
	}
}

int GNRS_ReqRespHandler::cleanup_request(uint32_t req_id) {
	ReqCache::iterator it = _reqCache.find(req_id);
	if (it == _reqCache.end()) {
		logger.log(MF_DEBUG, "GNRS_ReqRespHandler: no req_id %u entry", req_id);
		return 0;
	} else {
		delete it.value()->expiry;
		//kill pakcet
		it.value()->req_pkt->kill();

		logger.log(MF_DEBUG, "GNRS_ReqRespHandler: delete req_id %u", req_id);
		pthread_mutex_lock(&reqCacheLock);
		_reqCache.erase(it);
		pthread_mutex_unlock(&reqCacheLock);
		return 1;
	}
}

void GNRS_ReqRespHandler::handleReqExpiry(Timer *timer, void *data) {
	timer_data_t *timerdata = (timer_data_t*)data;
	assert(timerdata);
	timerdata->me->reqExpire(timerdata->req_id, timerdata);
}

void GNRS_ReqRespHandler::reqExpire(uint32_t req_id, timer_data_t *timerdata) {
	ReqCache::iterator it = _reqCache.find(req_id);
	if (it == _reqCache.end()) {
		logger.log(MF_ERROR,
				"GNRS_ReqRespHandler: cannot find expired req_id %u entry", req_id);
		return;
	} else {
		delete it.value()->expiry;
		if (timerdata->resend <= 0) {
			it.value()->req_pkt->kill();

			logger.log(MF_DEBUG,
					"GNRS_ReqRespHandler: delete expired req_id %u entry", req_id);

			pthread_mutex_lock(&reqCacheLock);
			_reqCache.erase(it);
			pthread_mutex_unlock(&reqCacheLock);
		} else {
			uint32_t resend = --timerdata->resend;

			logger.log(MF_DEBUG,
					"GNRS_ReqRespHandler: delete expired req_id %u entry, and resend", req_id);

			pthread_mutex_lock(&reqCacheLock);
			_reqCache.erase(it);
			pthread_mutex_unlock(&reqCacheLock);
			//resend a new req_id
			handleReqPkt(it.value()->req_pkt, resend);
		}
	}
}

char* GNRS_ReqRespHandler::to_log(req_t &req, char *buf) {
	sprintf(buf, "GNRS_ReqRespHandler: to_log msg: version: %u, type: %c, len: %u, id: %u, \n"
			"addr_tlv: type: %u, len: %u, val: %s\n"
			"data: \n"
			"num_opts: %u\n", req.version, req.type, req.len, req.id,
			req.src_addr.type, req.src_addr.len, req.src_addr.value, req.num_opts);

	for (uint32_t i=0; i<req.num_opts; ++i) {
		uint32_t slen = strlen(buf);
		opt_tlv_t *opt = req.opts + i;
		if (opt->len == 8) {
			sprintf(buf+slen, "opt-%u: type: %u, len: %u, val as long long: %lld\n",
					i, opt->type, opt->len, *(long long *)opt->value);
		} else {
			sprintf(buf+slen, "opt-%u: type: %u, len: %u, val as char array: %s\n",
					i, opt->type, opt->len, opt->value);
		}
	}
	sprintf(buf+strlen(buf), "opts_len: %u\n", req.opts_len);
	return buf;
}

CLICK_ENDDECLS
EXPORT_ELEMENT(GNRS_ReqRespHandler)
ELEMENT_REQUIRES(userlevel)
