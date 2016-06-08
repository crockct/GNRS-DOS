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
 * @file   mfrouter.h
 * @author wontoniii (bronzino@winlab.rutgers.edu), czhang
 * @date   August, 2011
 * @brief  Class that manages available routes for chunks (and relative flows)
 *
 * Class that manages available routes for chunks (and relative flows)
 *      At the moment it only does 2 things:
 *          -for received packets checks if the flow is open otherwise discard the packet
 *          -for packets that have to be sent decides the interface to use and fill the header.
 */

#ifndef MF_ROUTER_H_
#define MF_ROUTER_H_

#include <set>
#include <include/mfguid.hh>
#include <include/mfroutingheader.hh>
#include <include/mfflags.h>

#include "mfsystem.h"
#include "mftypes.h"
#include "mfchunkinfo.h"
#include "mfdevicemanager.h"
#include "mfinterfacemanager.h"
#include "mfsockettable.h"
#include "mfsocketmanager.h"
#include "mfeventqueue.h"
#include "mftransport.h"
#include "mftimeoutmanager.h"
#include "oml/mfoml.h"

using namespace std;

class MF_Router : MFTimeoutListenerInterface {
	set <int> GUIDs;
	MFSystem *_system;
	int policy;
	int defaultGUID;
	unsigned int assocTOID;
	int assocTOMicros;
	
	//OML Statistics
	MF_OML* mfoml;
	uint32_t inChunks;
	uint32_t outChunks;
	
public:
	MF_Router();
	MF_Router(MFSystem *_system);
	~MF_Router();
	
	void init();

	void setPolicy(PolicyType pol);
	void setDefaultGUID(string guid);
	void setDefaultGUID(int guid);
	int getDefaultGUID();
	
	void procSend(MF_ChunkInfo *);
	/*
	 * does nothing right, DTN routing code may be placed here,
	 * return true if it decides to pass data to transport layer,
	 * return false if it decides to store
	 */
	bool procRecv(MF_ChunkInfo *);
	void fillHeader(MF_ChunkInfo *);
	void addGUID(int GUID);
	void removeGUID(int GUID);
	
	void sendAssocReqs();

	virtual void OnTimeout(void *message, unsigned int id);
};

#endif /* MF_ROUTER_H_ */
