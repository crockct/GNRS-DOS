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
 * @file   mfsocketmanagerinterface.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   March, 2015
 * @brief  Abstract class that defines socket manager interface.
 *
 * Abstract class that defines socket manager interface.
 */

#ifndef MFSOCKETMANAGERINTERFACE_H_
#define MFSOCKETMANAGERINTERFACE_H_

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <list>
#include <vector>
#include <queue>
#include <map>
#include <list>

#include "mfsystem.h"
#include "mferrors.h"
#include "mftypes.h"
#include "mfbuffer.h"
#include "mfchunkinfo.h"
#include "mfserver.h"
#include "mftransport.h"
#include "mfstackevent.h"
#include "mfsockettable.h"
#include "mfsettings.h"
#include "mftimeoutlistener.h"
using namespace std;

class MF_SocketTable;
class MF_Transport;
class MF_System;
struct TransportSetting;
class MF_EvUpTransp;
class MF_APIOpen;
class MF_APISend;
class MF_APIRecv;
class MF_APIGet;
class MF_APIDoGet;
class MF_APIGetResponse;
class MF_APIAttach;
class MF_APIDetach;
class MF_APIClose;



class MF_SocketManager : public MFTimeoutListenerInterface{

protected:
	//Resources to communicate with api
	struct sockaddr_un cliAddr;
	int mDataSock, mConnSock;
	u_int ID;
	bool open;
	unsigned int activeTimeout;

	//Socket resources
	//TODO at the moment it distinguish only between content socket and data socket, this will need a new design
	int type; //1 data 2 content
	MF_Buffer recvBuffer;
	MF_Buffer sendBuffer;
	list <int> GUIDs;
	list <u_int> TIDs;

	//Other classes involved
	MFSystem *_system;
	MF_Transport *trnsp; //transport. At the moment it will only be a single transport (basic). In the future the transport/s will be selected based on the policy
	TransportSetting transSetting;


public:

	MF_SocketManager(MFSystem *_system, u_int UID);

	MF_SocketManager(MFSystem *_system, u_int UID, int bufferSize, TransportSetting ts);

	virtual ~MF_SocketManager();

	inline bool isOpen(){
		return open;
	}

	inline u_int getID(){
		return ID;
	}

	inline void setID(u_int ID){
		this->ID = ID;
	}

	inline void addGUID(int guid){
		GUIDs.push_back(guid);
	}

	inline void removeGUID(int guid){
		GUIDs.remove(guid);
	}

	inline void addTID(u_int tid){
		TIDs.push_back(tid);
	}

	inline void removeTID(u_int tid){
		TIDs.remove(tid);
	}


	inline list<int>::iterator getGUIDBegin(){
		return GUIDs.begin();
	}

	inline list<int>::iterator getGUIDEnd(){
		return GUIDs.end();
	}

	inline list<u_int>::iterator getTIDBegin(){
		return TIDs.begin();
	}

	inline list<u_int>::iterator getTIDEnd(){
		return TIDs.end();
	}

	bool init();

	int connectToClient();

	void openReplySuccess();

	void openReplyError(int reason);

	int recvSinglePacket(u_char *buf, int size, int offset);

	int recvData(vector<u_char *> *chkPkt, u_int chunkSize);

	void sendData(vector<u_char *> *chkPkt, u_int size);

	virtual void msgAvailable(MF_EvUpTransp *){}

	virtual int openRequest(MF_APIOpen *) = 0;
	virtual void sendRequest(MF_APISend *){}
	virtual void recvRequest(MF_APIRecv *){}
	virtual void getRequest(MF_APIGet *){}
	virtual void doGetRequest(MF_APIDoGet *){}
	virtual void getResponseRequest(MF_APIGetResponse *){}
	virtual void attachRequest(MF_APIAttach *event){}
	virtual void detachRequest(MF_APIDetach *event){}
	virtual void closeRequest(MF_APIClose *event){}

	MF_Transport *getTransportByTID(u_int tid);
	virtual void releaseChunk(MF_ChunkInfo *ci);
	virtual void newSpaceAvail(){}

	virtual void startAliveTimer();
	virtual void checkSocketAlive();
	virtual void cleanSocket();
	virtual void OnTimeout(void *message, unsigned int id);
};

#endif /* MFSOCKETMANAGERINTERFACE_H_ */
