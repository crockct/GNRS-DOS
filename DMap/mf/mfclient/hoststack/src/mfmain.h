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
 * @file   mfmain.h
 * @author wontoniii (bronzino@winlab.rutgers.edu), czhang
 * @date   July, 2011
 * @brief  Temp.
 *
 * Temp.
 */

#ifndef MF_MAIN_H_
#define MF_MAIN_H_

#include <sys/stat.h>
#include <fcntl.h>
#include <string>

#include "mftypes.h"
#include "mfsettings.h"
#include "oml/mfoml.h"
#include "mfbuffer.h"
#include "mftransport.h"
#include "mfsegmentor.h"
#include "mfaggregator.h"
#include "mfinterfacemanager.h"
#include "mfetherproto.h"
#include "mfipproto.h"
#include "mfchunkinfo.h"
#include "mfdeviceinfo.h"
#include "mfdevicemanager.h"
#include "mfrouter.h"
#include "mflog.h"
#include "mfsocketmanager.h"
#include "mfsockettable.h"
#include "mfeventqueue.h"
#include "mfstackevent.h"
#include "mftimeoutmanager.h"
#include "mfreliabtransport.h"
#include "mfsystem.h"


extern "C" void threadExitHandler(int);

class MF_Main {
	MFSystem _system;
	bool cont;

	/********************************
	 Operations to handle the communication with the API
	 ********************************/
	/*
	 EV_OPEN = 1,
	 EV_SEND,
	 EV_RECV,
	 EV_CLOSE,
	 EV_ATTACH,
	 EV_DETACH,
	 */
	void handleMessage(MF_StackEvent *);
	void handleOpen(MF_APIOpen *);
	void handleSend(MF_APISend *);
	void handleRecv(MF_APIRecv *);
	void handleClose(MF_APIClose *);
	void handleAttach(MF_APIAttach *);
	void handleDetach(MF_APIDetach *);
	void handleGet(MF_APIGet *);
	void handleDoGet(MF_APIDoGet *);
	void handleGetResponse(MF_APIGetResponse *);

	/********************************
	 Operations to handle the events from lower layers (network interfaces)
	 ********************************/
	/*
	 EV_INIT_REPORT = 20,
	 EV_IF_STATE_UPDATE,
	 EV_POLICY_CHANGE,
	 EV_CSYN_ACK,
	 EV_CSYN_TIMEOUT,
	 EV_LP,
	 EV_UP_IF,
	 EV_DOWN_ROUT,
	 EV_UP_ROUT,
	 EV_DOWN_TRANSP,
	 EV_DOWN_API,
	 EV_UP_TRANSP,
	 */
	void handleIfStateUpdate(MF_EvIfStateUpdate *);
	void handleCsyn(MF_EvCysn *);
	void handleCsynAck(MF_EvCysnAck *);
	void handleCsynTimeout(MF_EvCysnTOut *);
	void handleLinkProbe(MF_EvLinkProb *);
	void handleChkSent(MF_EvChkComplete *);
	void handleSpaceAvail(MF_EvSpaceAvail *);
	void handleUpIf(MF_EvUpIf *);
	void handleAssocTO(MF_EvAssocTO *);
	void handleDownRout(MF_EvDownRout *);
	void handleUpRout(MF_EvUpRout *);
	void handleDownTransp(MF_EvDownTransp *);
	void handleUpTransp(MF_EvUpTransp *);
	void handleDownAPI(MF_EvDownAPI *);
	void handleTransTOut(MF_EvTransTOut *event);
	void handleSocketTO(MF_SocketTOCheck *event);
	void handleSocketClean(MF_EventSocketClean *event);


	//Still necessary???
	MF_DeviceInfo *IfLookup(struct Options *);
	u_int computeChunkPktCnt (u_int);
	int readPolicy();

public:

	MF_Main();
	MF_Main(MF_OML* oml_mps);
	~MF_Main();
	void start(MF_Settings *setts);

};

#endif /* MF_MAIN_H_ */
