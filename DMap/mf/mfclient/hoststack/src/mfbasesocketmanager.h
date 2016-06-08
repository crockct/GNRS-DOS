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
 * @file   mfsocketmanager.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   August, 2013
 * @brief  Class that handles the tresources for each socket.
 *
 * Class that handles the tresources for each socket.
 */

#ifndef MF_SOCKET_MAN
#define MF_SOCKET_MAN

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <list>
#include <vector>
#include <queue>
#include <map>

//TODO: remove once found clean solution to pass info about GUID
#include <include/mfroutingheader.hh>
#include <include/mfguid.hh>

#include "mfsystem.h"
#include "mferrors.h"
#include "mftypes.h"
#include "mfbuffer.h"
#include "mfchunkinfo.h"
#include "mfstackevent.h"
#include "mfserver.h"
#include "mftransport.h"
#include "mfsockettable.h"

using namespace std;

class MF_SocketTable;

class MF_BaseSocketManager : public MF_SocketManager{
	//Resources to communicate with api
	
	//Socket resources
	vector<MF_APIRecv *> pendingRec;
	queue<MF_APISend *> pendingSend;
	
	//Other classes involved

	void sendReply(void *, u_int, u_int);
	void sendReplyRec(void *buffer, u_int size, u_int pid);
	void newSpaceAvail();
	void sendMessage(vector<u_char *> *v, u_int UID, u_int reqID, u_int size, u_int dstGUID, mfflag_t opts);
	void replyDoGetRequest(MF_APIDoGet *event);
	
	//TODO this will need to be removed
	void basicMsgAvailable(MF_EvUpTransp *event);
	
public:
	MF_BaseSocketManager(MFSystem *_system, u_int UID);
	MF_BaseSocketManager(MFSystem *_system, u_int UID, int nChunks, TransportSetting ts);
	~MF_BaseSocketManager();
	
	virtual void msgAvailable(MF_EvUpTransp *);
	
	virtual int openRequest(MF_APIOpen *event);
	virtual void sendRequest(MF_APISend *);
	virtual void recvRequest(MF_APIRecv *);
	virtual void getRequest(MF_APIGet *);
	virtual void doGetRequest(MF_APIDoGet *);
	virtual void getResponseRequest(MF_APIGetResponse *);
	virtual void attachRequest(MF_APIAttach *event);
	virtual void detachRequest(MF_APIDetach *event);
	virtual void closeRequest(MF_APIClose *event);
	
	virtual void releaseChunk(MF_ChunkInfo *ci);
	virtual void cleanSocket();
};

#endif /* MF_SOCKET_MAN */
