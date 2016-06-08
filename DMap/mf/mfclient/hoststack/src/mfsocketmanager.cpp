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

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <list>
#include <vector>
#include <queue>
#include <map>
#include <list>

#include <include/mfclientipc.h>

#include "mfsocketmanager.h"


MF_SocketManager::MF_SocketManager(MFSystem *_system, u_int UID) : recvBuffer(10), sendBuffer(10){
	ID = UID;
	this->_system = _system;
	open = false;
	transSetting.isReliabTrans = false;
	trnsp = NULL;
	mConnSock = 0;
	mDataSock = 0;
	type = 0;
	activeTimeout = 0;
}

MF_SocketManager::MF_SocketManager(MFSystem *_system, u_int UID, int bufferSize, TransportSetting ts) :
			recvBuffer(bufferSize),sendBuffer(bufferSize),transSetting(ts) {
	ID = UID;
	this->_system = _system;
	open = false;
	trnsp = NULL;
	mConnSock = 0;
	mDataSock = 0;
	type = 0;
	activeTimeout = 0;
}

MF_SocketManager::~MF_SocketManager(){}

bool MF_SocketManager::init(){
	MF_Log::mf_log(MF_DEBUG, "MF_SocketManager:init Initializing socket manager with ID: %lu",ID);
	cliAddr.sun_family = AF_LOCAL;
	sprintf(cliAddr.sun_path, "%s%010x", CLIENT_PATH, ID);
	mConnSock = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (mConnSock < 0) {
		MF_Log::mf_log(MF_ERROR,"socket() error:%s\n",strerror(errno));
		//TODO: send error message to api layer
		return false;
	}
	struct sockaddr_un dataAddr;
	dataAddr.sun_family = AF_LOCAL;
	sprintf(dataAddr.sun_path, "%s%010x", SERVER_DATA_PATH, ID);
	unlink(dataAddr.sun_path);
	if (bind(mConnSock, (struct sockaddr *)&dataAddr, sizeof(dataAddr)) < 0 ){
		MF_Log::mf_log(MF_ERROR, "MF_SocketManager:init bind error constructing server datapath :%s",strerror(errno));
		close(mConnSock);
		//TODO: send error message to api layer
		openReplyError(STACK_ACTION_FAILURE);
		return false;
	}
#ifdef __ANDROID__
	char command[128];
	strcpy(command, "chmod 777 ");
	strcat(command, dataAddr.sun_path);
	system(command);
#endif
	if (listen(mConnSock, 1) < 0) {
		MF_Log::mf_log(MF_ERROR, "MF_SocketManager:init listen error constructing server datapath :%s",strerror(errno));
		close(mConnSock);
		openReplyError(STACK_ACTION_FAILURE);
		return false;
	}
	return true;
}

int MF_SocketManager::connectToClient(){
	if((mDataSock = accept(mConnSock, NULL, NULL)) < 0) {
		MF_Log::mf_log(MF_ERROR, "MF_SocketManager:connectionToClient accept error constructing server datapath :%s",strerror(errno));
		return -1;
	}
	return 0;
}

void MF_SocketManager::openReplySuccess() {
	MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:openSuccess a socket manager is opened with UID %lu",ID);
	struct MsgOpenReply openR;
	openR.type = MSG_TYPE_OPEN_REPLY;
	openR.retValue = STACK_ACTION_SUCCESS;
	if(_system->getSocketServer()->send((char *)(&openR), sizeof(struct MsgOpenReply), &cliAddr)<=0){
		MF_Log::mf_log(MF_ERROR, "MF_ContentSocketManager:openSuccess error replying to API");
	}
}

void MF_SocketManager::openReplyError(int reason){
	MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:openError a socket manager is already opened with UID %lu",ID);
	struct MsgOpenReply openR;
	openR.type = MSG_TYPE_OPEN_REPLY;
	openR.retValue = reason;
	if(_system->getSocketServer()->send((char *)(&openR), sizeof(struct MsgOpenReply), &cliAddr)<=0){
		MF_Log::mf_log(MF_ERROR, "MF_ContentSocketManager:openSuccess error replying to API");
	}
}

int MF_SocketManager::recvSinglePacket(u_char *buf, int size, int offset){
	//MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:recvSinglePacket receiving %d bytes with offset for buffer %d", size, offset);
	int rec = 0, totRec = 0;
	while(totRec!=size){
		rec = recv(mDataSock, buf + rec + offset, size - rec, 0);
		//MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:recvSinglePacket received %d bytes", rec);
		if(rec<=0){
			MF_Log::mf_log(MF_ERROR, "MF_SocketManager:recvSinglePacket errno: %d: %s", errno, strerror(errno));
			return rec;
		}
		totRec+=rec;
	}
	return totRec;
}

int MF_SocketManager::recvData(vector<u_char *> *chkPkt, u_int chunkSize){
	MF_Log::mf_log(MF_DEBUG, "MF_SocketManager:recvData Receiving a message (size = %d) from app for ID %u", chunkSize, ID);
	int size = 0; //for debug only
	int recSize = 0; //for debug only
	int reminSize = (int)chunkSize;
	int i = 0;
	while(recSize!=(int)chunkSize){
		if ((reminSize <= (int)(MF_PacketSupport::MAX_PAYLOAD_SIZE - MF_PacketSupport::HIGH_HEADER_LEN)) && (i%MAX_CHUNK_SIZE == 0)) {
			size = recvSinglePacket((*chkPkt)[i], reminSize, MF_PacketSupport::TOTAL_HEADER_LEN);
			//MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:recvData Last loop first chunk Receiving %d received %d", reminSize, size);
			recSize += size;
			if(size < 0) return -1;
		}
		else if ((reminSize > (int)(MF_PacketSupport::MF_PacketSupport::MAX_PAYLOAD_SIZE - MF_PacketSupport::HIGH_HEADER_LEN)) && (i%MAX_CHUNK_SIZE == 0)) {
			size = recvSinglePacket((*chkPkt)[i], MF_PacketSupport::MAX_PAYLOAD_SIZE - MF_PacketSupport::HIGH_HEADER_LEN, MF_PacketSupport::TOTAL_HEADER_LEN);
			//MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:recvData First packet of chunk Receiving %d received %d", MF_PacketSupport::MAX_PAYLOAD_SIZE - MF_PacketSupport::HIGH_HEADER_LEN, size);
			recSize += size;
			reminSize = reminSize - (MF_PacketSupport::MAX_PAYLOAD_SIZE - MF_PacketSupport::HIGH_HEADER_LEN);
			i++;
			if(size < 0) return -1;
		}
		else if ((reminSize <= MF_PacketSupport::MAX_PAYLOAD_SIZE) && (i%MAX_CHUNK_SIZE != 0)) {
			size = recvSinglePacket((*chkPkt)[i], reminSize, MF_PacketSupport::LOW_HEADER_LEN);
			//MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:recvData Last loop not first packet of chunk Receiving %d received %d", reminSize, size);
			recSize += size;
			if(size < 0) return -1;
		}
		else if ((reminSize > MF_PacketSupport::MAX_PAYLOAD_SIZE) && (i%MAX_CHUNK_SIZE != 0)) {
			size = recvSinglePacket((*chkPkt)[i], MF_PacketSupport::MAX_PAYLOAD_SIZE, MF_PacketSupport::LOW_HEADER_LEN);
			//MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:recvData not first Receiving %d received %d", MF_PacketSupport::MAX_PAYLOAD_SIZE, size);
			recSize += size;
			reminSize = reminSize - MF_PacketSupport::MAX_PAYLOAD_SIZE;
			i++;
			if(size < 0) return -1;
		}
	}
	return 0;
}

void MF_SocketManager::sendData(vector<u_char *> *chkPkt, u_int size){
	MF_Log::mf_log(MF_DEBUG, "MF_SocketManager::sendData send data to API layer");
	u_int n;
	u_int reminSize = size;
	for (n = 0; n < chkPkt->size(); n++) {
		if (n%MAX_CHUNK_SIZE == 0) {
			if (reminSize <= MF_PacketSupport::MAX_PAYLOAD_SIZE - MF_PacketSupport::HIGH_HEADER_LEN) {
				send(mDataSock, MF_PacketSupport::getData((*chkPkt)[n], true), reminSize, 0);
				return;
			}
			else if (reminSize > MF_PacketSupport::MAX_PAYLOAD_SIZE - MF_PacketSupport::HIGH_HEADER_LEN) {
				send(mDataSock, MF_PacketSupport::getData((*chkPkt)[n], true), MF_PacketSupport::MAX_PAYLOAD_SIZE - MF_PacketSupport::HIGH_HEADER_LEN, 0);
				reminSize -= MF_PacketSupport::MAX_PAYLOAD_SIZE - MF_PacketSupport::HIGH_HEADER_LEN;
			}
		} else {
			if (reminSize <= (u_int)MF_PacketSupport::MAX_PAYLOAD_SIZE) {
				send(mDataSock, MF_PacketSupport::getData((*chkPkt)[n], false), reminSize, 0);
				return;
			}
			else if (reminSize > (u_int)MF_PacketSupport::MAX_PAYLOAD_SIZE) {
				send(mDataSock, MF_PacketSupport::getData((*chkPkt)[n], false), MF_PacketSupport::MAX_PAYLOAD_SIZE, 0);
				reminSize -= MF_PacketSupport::MAX_PAYLOAD_SIZE;
			}
		}
	}
}

MF_Transport *MF_SocketManager::getTransportByTID(u_int tid){
	MF_Log::mf_log(MF_DEBUG, "MF_SocketManager::getTransoirtByTID return transport object given TID %u", tid);
	//TODO:at the moment only one transport but it should check in the future
	return trnsp;
}

void MF_SocketManager::releaseChunk(MF_ChunkInfo *ci){
	MF_Log::mf_log(MF_DEBUG, "MF_SocketManager::releaseChunk release message sent");
	vector<u_char *> *v = ci->getPacketList();
	if(v!=NULL){
		sendBuffer.putVector(v);
		//Check if there are pending sends to continue
		newSpaceAvail();
	}
	else{
		MF_Log::mf_log(MF_WARN, "MF_ContentSocketManager::releaseChunk releasing empty buffer");
	}
}

void MF_SocketManager::startAliveTimer(){
	MF_Log::mf_log(MF_DEBUG, "MF_SocketManager:startAliveTimer start alive timer");
	activeTimeout = _system->getTimeoutManager()->AddTimeout(
			this, NULL, _system->getSettings()->getSocketAliveTimeout());
}

void MF_SocketManager::checkSocketAlive(){
	struct MsgDummyCheck dummyMessage;
	dummyMessage.type = MSG_TYPE_DUMMY_CHECK;
	if(_system->getSocketServer()->send((char *)(&dummyMessage), sizeof(struct MsgDummyCheck), &cliAddr)<=0){
		MF_Log::mf_log(MF_WARN, "MF_SocketManager:checkSocketAlive socket error, cleaning resources");
		MF_EventSocketClean *event = new MF_EventSocketClean(ID);
		_system->getEventQueue()->add(event);
	} else {
		activeTimeout = _system->getTimeoutManager()->AddTimeout(
				this, NULL, _system->getSettings()->getSocketAliveTimeout());
	}
}

void MF_SocketManager::cleanSocket() {
	//TODO implement for each socket type, this one might only clean common resources
	if(open) {
		open = false;
		_system->getTimeoutManager()->ClearTimeout(activeTimeout);
		std::list <int>::iterator it;
		for(it = GUIDs.begin(); it!=GUIDs.end(); it++ ) {
			_system->getSocketTable()->removeGUID(ID, *it);
			_system->getRouter()->removeGUID(*it);
		}
		GUIDs.clear();
	}
	_system->getSocketTable()->removeManager(ID);
	//TODO remove everything else that was added
	delete trnsp;
	recvBuffer.dePopulate();
	sendBuffer.dePopulate();
	close(mConnSock);
	char tempName[128];
	memset(tempName, 0, 128);
	sprintf(tempName, "%s%010x", SERVER_DATA_PATH, ID);
	unlink(tempName);
}

void MF_SocketManager::OnTimeout(void *message, unsigned int id) {
	//send dummy message to api. If fails consider socket closed.
	MF_SocketTOCheck *event = new MF_SocketTOCheck(ID);
	_system->getEventQueue()->add(event);
}
