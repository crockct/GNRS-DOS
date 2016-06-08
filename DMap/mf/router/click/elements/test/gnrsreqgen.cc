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
#include <click/standard/scheduleinfo.hh>
#include <click/vector.hh>
#include <stdio.h>
#include <string.h>

#include "mf.hh"
#include "gnrsreqgen.hh"
#include "mffunctions.hh"
#include "mflogger.hh"

CLICK_DECLS

GNRS_ReqGen::GNRS_ReqGen() : _iter(0), _timer(this), logger(MF_Logger::init()) {
	_delay = 1;
	_period = 1;
	_iter = 1;
	_mode = RANDOM;
	_offset = 0;
}

GNRS_ReqGen::~GNRS_ReqGen(){
}

int GNRS_ReqGen::configure(Vector<String> &conf, ErrorHandler *errh) {

  if (cp_va_kparse(conf, this, errh,
                   "DELAY", cpkP+cpkM, cpUnsigned, &_delay,
                   "PERIOD", cpkP+cpkM, cpUnsigned, &_period,
                   "OFFSET", cpkP, cpUnsigned, &_offset,
                   cpEnd)<0) {
    return -1;
  }
  return 0;
}

int GNRS_ReqGen::initialize(ErrorHandler *errh){

  _timer.initialize(this, errh);
  //timer used to allow for the time it takes for the routing table to converge
  _timer.schedule_after_msec(_delay*1000);
  return 0;
}

void GNRS_ReqGen::push(int port, Packet *p){
	//Check response
	if(port != 0) {
		p->kill();
		return;
	}
	gnrs_lkup_resp_t *resp = (gnrs_lkup_resp_t *)(p->data());
	GUID guid;
	guid.init(resp->GUID);
	logger.log(MF_INFO,
			"GNRS_ReqGen: Got GNRS resp: qry GUID: %u, num_na: %u",
			guid.to_int(), resp->na_num);
	//push if port 2 is set
	if(noutputs()>1) {
		output(1).push(p);
	} else {
		p->kill();
	}
}

void GNRS_ReqGen::runFromTrace() {
	//TODO
	logger.log(MF_WARN, "GNRS_ReqGen: Currently not implemented");
}

void GNRS_ReqGen::runRandom() {
	GUID guid;
	WritablePacket *p = Packet::make(sizeof(click_ether), 0, sizeof(gnrs_req), 0);
	assert(p); assert(p->data());
	memset(p->data(), 0, p->length());

	gnrs_req *gnrs_p = (gnrs_req *)(p->data());
	/* alternate between INSERT & LOOKUP - TODO use trace */
	if (!(_iter % 2)) {
		/* do lookup on guid inserted in prev iter */
		gnrs_p->type = GNRS_LOOKUP;
		guid.init(GENERATOR_GUID_START + _offset + _iter -1);
		char buf[GUID_LOG_BUF_SIZE];
		memcpy(gnrs_p->GUID, guid.getGUID(), GUID_LENGTH);
		logger.log(MF_INFO,
		  "GNRS_ReqGen: generating a LOOKUP on guid: %s", guid.to_log(buf));
	} else {
		gnrs_p->type = GNRS_INSERT;
		guid.init(GENERATOR_GUID_START + _offset + _iter);
		char buf[GUID_LOG_BUF_SIZE];
		memcpy(gnrs_p->GUID, guid.getGUID(), GUID_LENGTH);
		gnrs_p->net_addr = _iter;
		gnrs_p->weight = 1;
		logger.log(MF_INFO,
		  "GNRS_ReqGen: generating a INSERT for guid: %s, netaddr: %u, weight: %u",
		  guid.to_log(buf), gnrs_p->net_addr, gnrs_p->weight);
	}

	output(0).push(p);
}

void GNRS_ReqGen::run_timer(Timer *){

  /* Read request trace, create and push gnrs request packets */
  if(_mode == RANDOM) {
	  runRandom();
  } else if (_mode == TRACE) {
	  runFromTrace();
  } else {
	  logger.log(MF_ERROR, "GNRS_ReqGen: Wrong GNRS request generator mode");
	  return;
  }

  /* reschedule if repeat requested */
  _iter++;
  if(_period > 0)_timer.reschedule_after_sec(_period);
}

CLICK_DECLS
EXPORT_ELEMENT(GNRS_ReqGen)
ELEMENT_REQUIRES(userlevel)
