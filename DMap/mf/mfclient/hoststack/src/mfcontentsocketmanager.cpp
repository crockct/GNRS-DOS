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
 * @file   mfsocketmanager.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   August, 2013
 * @brief  Class that handles the tresources for each socket.
 *
 * Class that handles the tresources for each socket.
 */

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <list>
#include <vector>
#include <queue>
#include <errno.h>
#include <unistd.h>

#include <include/mfroutingheader.hh>
#include <include/mfguid.hh>
#include <include/mfclientipc.h>

#include "mftypes.h"
#include "mfbuffer.h"
#include "mfchunkinfo.h"
#include "mfserver.h"
#include "mftransport.h"
#include "mfstackevent.h"
#include "mfreliabtransport.h"
#include "mfsockettable.h"
#include "mfsocketmanager.h"
#include "mfcontentsocketmanager.h"
#include "mfpacketsupport.h"


//Select transport based on profile
MF_ContentSocketManager::MF_ContentSocketManager(MFSystem *_system, u_int UID) :
			MF_SocketManager(_system, UID){
	type = 2;
	lastGetRequest = 0;
}

MF_ContentSocketManager::MF_ContentSocketManager(MFSystem *_system, u_int UID, int pCount, TransportSetting ts) :
			MF_SocketManager(_system, UID, pCount, ts) {
	type = 2;
	lastGetRequest = 0;
}

MF_ContentSocketManager::~MF_ContentSocketManager(){

}

int MF_ContentSocketManager::openRequest(MF_APIOpen *event){
	MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:open request analyizing request profile=%s guid=%d", event->getProfile(), event->getSrcGUID());
	if (!strcmp("content", event->getProfile())) {
		type = 2;
	}
	else{
		MF_Log::mf_log(MF_ERROR, "MF_ContentSocketManager:openRequest wrong profile request %s", event->getProfile());
		openReplyError(STACK_ACTION_FAILURE);
		return -1;
	}
  
	if (transSetting.isReliabTrans) {
		trnsp = new MF_ReliabTransport(ID, &recvBuffer, &sendBuffer, _system);
		((MF_ReliabTransport*)(trnsp))->setTransSetting(transSetting);
	} else {
		trnsp = new MF_BasicTransport(ID, &recvBuffer, &sendBuffer, _system);
	}
	trnsp->setMyGUID(event->getSrcGUID());

	_system->getSocketTable()->addManager(this);
	if(event->getSrcGUID()==0){
		MF_Log::mf_log(MF_DEBUG, "MF_BaseSocketManager:openRequest using device default srcGUID");
		_system->getSocketTable()->addGUID(event->getUID(), _system->getRouter()->getDefaultGUID());
	}
	else{
		_system->getSocketTable()->addGUID(event->getUID(), event->getSrcGUID());
		_system->getRouter()->addGUID(event->getSrcGUID());
		GUIDs.push_back(event->getSrcGUID());
	}
	_system->getSocketTable()->addTID(ID, ID);

	startAliveTimer();

	MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:openRequest opened new socket with ID=%u default srcGUID=%d", ID, event->getSrcGUID());
	open = true;
	openReplySuccess();
	connectToClient();
	return 0;
}

void MF_ContentSocketManager::sendMessage(vector<u_char *> *v, u_int UID, u_int reqID, u_int size, u_int dstGUID, mfflag_t opts){
	struct MsgSendReply reply;
	reply.UID = UID;
	reply.reqID = reqID;
	reply.type = MSG_TYPE_SEND_REPLY;
	reply.retValue = STACK_ACTION_SUCCESS;
	_system->getSocketServer()->send((char *)(&reply), sizeof(struct MsgSendReply), &cliAddr);
	MF_Log::mf_log_time("MF_ContentSocketManager:sendMessage SEND_TIME request number [%lu] REPLIED", reqID);
	if(recvData(v, size)){
		MF_Log::mf_log(MF_ERROR, "MF_ContentSocketManager:sendMessage error transferring from API layer");
		return;
	}
	struct MsgSendComplete msc;
	msc.UID = UID;
	msc.reqID = reqID;
	msc.type = MSG_TYPE_SEND_COMPLETE;
	_system->getSocketServer()->send((char *)(&msc), sizeof(struct MsgSendComplete), &cliAddr);
	//pass data to transport
	MF_EvDownAPI *newEvent;
	if(GUIDs.empty()){
		MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:sendMessage passing new message to transport. Size=%u, Src=%d, Dst=%d, opts=%lu", size, 0, dstGUID, opts);
		newEvent = new MF_EvDownAPI(trnsp, v, size, 0, dstGUID, opts);
	}
	else{
		MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:sendMessage passing new message to transport. Size=%u, Src=%d, Dst=%d, opts=%lu", size, GUIDs.front(), dstGUID, opts);
		newEvent = new MF_EvDownAPI(trnsp, v, size, GUIDs.front(), dstGUID, opts);
	}
	_system->getEventQueue()->add(newEvent);
}

void MF_ContentSocketManager::sendRequest(MF_APISend *event){
	MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:sendRequest handle send request from API layer");
	vector<u_char *> *v = new vector<u_char *>();
	//TODO use of HIGH_HEADER_LEN bad
	if(pendingSend.empty()){
		if(sendBuffer.getVectorBySize(v, event->getSize() + MF_PacketSupport::HIGH_HEADER_LEN*MF_BasicTransport::computeMsgChkCnt(event->getSize()))){
			MF_Log::mf_log(MF_WARN, "MF_ContentSocketManager:sendRequest not enough space in buffer");
			//TODO: introduce flag to request non blocking send
			//if non blocking
			if(false){
				struct MsgSendReply reply;
				reply.UID = event->getUID();
				reply.reqID = event->getReqID();
				reply.type = MSG_TYPE_SEND_REPLY;
				reply.retValue = STACK_NO_BUFFER_AVAIL;
				_system->getSocketServer()->send((char *)(&reply), sizeof(struct MsgSendReply), &cliAddr);
			}
			else{
				MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:sendRequest the message will wait until there is memory available");
				MF_APISend *tempEvent = new MF_APISend();
				tempEvent->setDstGUID(event->getDstGUID());
				tempEvent->setOpts(event->getOpts());
				tempEvent->setReqID(event->getReqID());
				tempEvent->setSize(event->getSize());
				tempEvent->setUID(event->getUID());
				pendingSend.push(tempEvent);
			}
		}
		else{
			sendMessage(v, event->getUID(), event->getReqID(), event->getSize(), event->getDstGUID(), event->getOpts());
		}
	}
	else{
		MF_APISend *tempEvent = new MF_APISend();
		tempEvent->setDstGUID(event->getDstGUID());
		tempEvent->setOpts(event->getOpts());
		tempEvent->setReqID(event->getReqID());
		tempEvent->setSize(event->getSize());
		tempEvent->setUID(event->getUID());
		pendingSend.push(tempEvent);
	}
}

//TODO: minor, the event is not actually stored (or better, it's copied if stored).
//Adjust the fact that we are now passing the size of what is passed in bytes and not number of packets
void MF_ContentSocketManager::recvRequest(MF_APIRecv *event){
	MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:recvRequest handle receive request from API");
	struct MsgRecvReply rcR;
	if(!pendingRec.empty()){
		MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::recvRequest other recv requests are already waiting");
		if(event->getBlk()){
			MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::recvRequest the request is blocking so will wait");
			MF_APIRecv *newEvent = new MF_APIRecv();
			newEvent->setBlk(event->getBlk());
			newEvent->setReqID(event->getReqID());
			newEvent->setSize(event->getSize());
			newEvent->setUID(event->getUID());
			pendingRec.push_back(newEvent);
		}
		else{
			MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::recvRequest the request is non blocking so it will not wait");
			rcR.type = MSG_TYPE_RECV_REPLY;
			rcR.UID = ID;
			rcR.retValue = STACK_NO_BUFFER_AVAIL;
			rcR.reqID = event->getReqID();
			rcR.size = 0;
			_system->getSocketServer()->send((char *)&rcR, sizeof(struct MsgRecvReply), &cliAddr);
		}
	}
	else if(trnsp->dataAvailable()){
		MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::recvRequest data available in transport");
		//check with transport if there is something available
		vector<u_char *> *v = new vector<u_char *>();
		u_int size = trnsp->getData(v, event->getSize());
		if(size){
			MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::recvRequest the buffer is big enough, getting ready to pass data");
			rcR.type = MSG_TYPE_RECV_REPLY;
			rcR.UID = ID;
			rcR.retValue = STACK_ACTION_SUCCESS;
			rcR.reqID = event->getReqID();
			//TODO: check this is correct
			rcR.size = size;
			//rcR.size = MF_BasicTransport::computeChunkPktCnt(size + MF_BasicTransport::computeMsgChkCnt(size)*HIGH_HEADER_LEN);
			RoutingHeader route(MF_PacketSupport::getNetworkHeaderPtr(v->front()));
			rcR.sGUID = route.getSrcGUID().to_int();
			_system->getSocketServer()->send((char *)&rcR, sizeof(struct MsgRecvReply), &cliAddr);
			sendData(v, size);
			recvBuffer.putVector(v);
		}
		else{
			MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::recvRequest the buffer is not big enough, reply will trigger an error");
			rcR.type = MSG_TYPE_RECV_REPLY;
			rcR.UID = ID;
			rcR.retValue = STACK_NO_BUFFER_AVAIL;
			rcR.reqID = event->getReqID();
			rcR.size = 0;
			_system->getSocketServer()->send((char *)&rcR, sizeof(struct MsgRecvReply), &cliAddr);
		}
	}
	else if(event->getBlk()){
		MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::recvRequest no data available and the request is blocking so will wait");
		MF_APIRecv *newEvent = new MF_APIRecv();
		newEvent->setBlk(event->getBlk());
		newEvent->setReqID(event->getReqID());
		newEvent->setSize(event->getSize());
		newEvent->setUID(event->getUID());
		pendingRec.push_back(newEvent);
	}
	else{
		MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::recvRequest no data available and the request is not blocking so will not wait");
		rcR.type = MSG_TYPE_RECV_REPLY;
		rcR.UID = ID;
		rcR.retValue = STACK_NO_BUFFER_AVAIL;
		rcR.reqID = event->getReqID();
		rcR.size = 0;
		_system->getSocketServer()->send((char *)&rcR, sizeof(struct MsgRecvReply), &cliAddr);
	}
}

void MF_ContentSocketManager::sendGetMessage(vector<u_char *> *v, u_int UID, u_int reqID, u_int size, u_int dstGUID, mfflag_t opts){
	struct MsgGetReply reply;
	reply.UID = UID;
	reply.reqID = reqID;
	reply.type = MSG_TYPE_GET_SEND_REPLY;
	reply.retValue = STACK_ACTION_SUCCESS;
	_system->getSocketServer()->send((char *)(&reply), sizeof(struct MsgGetReply), &cliAddr);
	MF_Log::mf_log_time("MF_ContentSocketManager:sendGetMessage SEND_TIME request number [%lu] REPLIED", reqID);
	if(recvData(v, size)){
		MF_Log::mf_log(MF_ERROR, "MF_ContentSocketManager:sendGetMessage error transfering from API layer");
		return;
	}
	struct MsgGetSent msc;
	msc.UID = UID;
	msc.reqID = reqID;
	msc.type = MSG_TYPE_GET_SEND_COMPLETE;
	_system->getSocketServer()->send((char *)(&msc), sizeof(struct MsgGetSent), &cliAddr);
	//Add get to pendingGet
	pendingGet.insert(make_pair(dstGUID, reqID));
	//pass data to transport
	MF_EvDownAPI *newEvent;
	if(GUIDs.empty()){
		MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:sendGetMessage passing new message to transport. Size=%u, Src=%d, Dst=%d, opts=%lu", size, 0, dstGUID, opts);
		newEvent = new MF_EvDownAPI(trnsp, v, size, 0, dstGUID, setMFFlag(opts, MF_CONTENT_REQUEST));
	}
	else{
		MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:sendGetMessage passing new message to transport. Size=%u, Src=%d, Dst=%d, opts=%lu", size, GUIDs.front(), dstGUID, opts);
		newEvent = new MF_EvDownAPI(trnsp, v, size, GUIDs.front(), dstGUID, setMFFlag(opts, MF_CONTENT_REQUEST));
	}
	_system->getEventQueue()->add(newEvent);
}

void MF_ContentSocketManager::getRequest(MF_APIGet *event){
	//TODO: if space available for sending request sends, otherwise returns error
	MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:getRequest handle get request from API layer");
	vector<u_char *> *v = new vector<u_char *>();
	//TODO use of HIGH_HEADER_LEN bad
	if(sendBuffer.getVectorBySize(v, event->getSize() + MF_PacketSupport::HIGH_HEADER_LEN*MF_BasicTransport::computeMsgChkCnt(event->getSize()))){
			MF_Log::mf_log(MF_WARN, "MF_ContentSocketManager:getRequest not enough space in buffer");
			//send back error to the API
		struct MsgGetReply reply;
		reply.UID = event->getUID();
		reply.reqID = event->getReqID();
		reply.type = MSG_TYPE_GET_SEND_REPLY;
		reply.retValue = STACK_NO_BUFFER_AVAIL;
		_system->getSocketServer()->send((char *)(&reply), sizeof(struct MsgGetReply), &cliAddr);
	}
	else{
		sendGetMessage(v, event->getUID(), event->getReqID(), event->getSize(), event->getDstGUID(), event->getOpts());
	}
}

//TODO: minor not storing the request
void MF_ContentSocketManager::replyDoGetRequest(MF_APIDoGet *event){
	MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:replyDoGetRequest");
	struct MsgDoGetReply rcR;
	//check with transport if there is something available
	vector<u_char *> *v = pendingGetRequest.front();
	pendingGetRequest.pop();
	u_int size = pendingGetRequestSizes.front();
	pendingGetRequestSizes.pop();
	rcR.type = MSG_TYPE_DO_GET_REPLY;
	rcR.UID = event->getUID();
	rcR.retValue = STACK_ACTION_SUCCESS;
	rcR.reqID = event->getReqID();
	lastGetRequest++;
	rcR.getID = lastGetRequest;
	rcR.size = size;
	//rcR.size = MF_BasicTransport::computeChunkPktCnt(size + MF_BasicTransport::computeMsgChkCnt(size)*HIGH_HEADER_LEN);
	RoutingHeader route(MF_PacketSupport::getNetworkHeaderPtr(v->front()));
	rcR.srcGUID = route.getSrcGUID().to_int();
	rcR.dstGUID = route.getDstGUID().to_int();
	_system->getSocketServer()->send((char *)&rcR, sizeof(struct MsgDoGetReply), &cliAddr);
	//TODO socket failure
	sendData(v, size);
	recvBuffer.putVector(v);
}

void MF_ContentSocketManager::doGetRequest(MF_APIDoGet *event){
	//TODO: very similar a recv, if there are some messages pending it will push them up, otherwise will start waiting
	MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:doGetRequest handle do_get request from API");
	if(!pendingGetRequest.empty()){
		replyDoGetRequest(event);
	}
	else{
		MF_APIDoGet *tempEvent = new MF_APIDoGet();
		tempEvent->setReqID(event->getReqID());
		tempEvent->setSize(event->getSize());
		tempEvent->setUID(event->getUID());
		pendingDoGet.push(tempEvent);
	}
}

void MF_ContentSocketManager::sendGetResponseMessage(vector<u_char *> *v, u_int UID, u_int reqID, u_int size, u_int srcGUID, u_int dstGUID, mfflag_t opts){
	struct MsgGetResponseReply reply;
	reply.UID = UID;
	reply.reqID = reqID;
	reply.type = MSG_TYPE_GET_RESPONSE_REPLY;
	reply.retValue = STACK_ACTION_SUCCESS;
	_system->getSocketServer()->send((char *)(&reply), sizeof(struct MsgGetResponseReply), &cliAddr);
	MF_Log::mf_log_time("MF_ContentSocketManager:sendGetResponseMessage GET_RESPONSE_TIME request number [%lu] REPLIED", reqID);
	if(recvData(v, size)){
		MF_Log::mf_log(MF_ERROR, "MF_ContentSocketManager:sendGetResponseMessage error transfering from API layer");
		return;
	}
	struct MsgGetResponseComplete msc;
	msc.UID = UID;
	msc.reqID = reqID;
	msc.type = MSG_TYPE_GET_RESPONSE_COMPLETE;
	_system->getSocketServer()->send((char *)(&msc), sizeof(struct MsgGetResponseComplete), &cliAddr);
	//pass data to transport
	MF_EvDownAPI *newEvent;
	//NOTE: now srcGUID and dstGUID are getting swapped because they still are the original src and dst of the get request
	//TODO: make this swapping more clear
	MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:sendGetResponseMessage passing new message to transport. Size=%u, Src=%d, Dst=%d, opts=%lu", size, dstGUID, srcGUID, opts);
	newEvent = new MF_EvDownAPI(trnsp, v, size, dstGUID, srcGUID, setMFFlag(opts, MF_CONTENT_RESPONSE));
	_system->getEventQueue()->add(newEvent);
}

//TODO: making a copy of the event
void MF_ContentSocketManager::getResponseRequest(MF_APIGetResponse *event){
	//When a do_get is pushed up, an entry will be created in the map for when the responsed is actually pushed
	//Modify from send
	MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:getResponseRequest handle send request from API layer");
	vector<u_char *> *v = new vector<u_char *>();
	//TODO use of HIGH_HEADER_LEN bad
	if(pendingGetResponse.empty()){
		if(sendBuffer.getVectorBySize(v, event->getSize() + MF_PacketSupport::HIGH_HEADER_LEN*MF_BasicTransport::computeMsgChkCnt(event->getSize()))){
			MF_Log::mf_log(MF_WARN, "MF_ContentSocketManager:getResponseRequest not enough space in buffer");
			MF_APIGetResponse *tempEvent = new MF_APIGetResponse();
			tempEvent->setDstGUID(event->getDstGUID());
			tempEvent->setGetID(event->getGetID());
			tempEvent->setOpts(event->getOpts());
			tempEvent->setReqID(event->getReqID());
			tempEvent->setSize(event->getSize());
			tempEvent->setSrcGUID(event->getSrcGUID());
			tempEvent->setUID(event->getUID());
			pendingGetResponse.push(tempEvent);
		}
		else{
			sendGetResponseMessage(v, event->getUID(), event->getReqID(), event->getSize(), event->getSrcGUID(), event->getDstGUID(), event->getOpts());
		}
	}
	else{
		MF_APIGetResponse *tempEvent = new MF_APIGetResponse();
		tempEvent->setDstGUID(event->getDstGUID());
		tempEvent->setGetID(event->getGetID());
		tempEvent->setOpts(event->getOpts());
		tempEvent->setReqID(event->getReqID());
		tempEvent->setSize(event->getSize());
		tempEvent->setSrcGUID(event->getSrcGUID());
		tempEvent->setUID(event->getUID());
		pendingGetResponse.push(tempEvent);
	}
}

void MF_ContentSocketManager::closeRequest(MF_APIClose *event){
	MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:closeRequest handle close request from API");
	//TODO:clean things if there is something to clean
	open = false;
	_system->getTimeoutManager()->ClearTimeout(activeTimeout);
	MF_EventSocketClean *cleanEvent = new MF_EventSocketClean(ID);
	_system->getEventQueue()->add(cleanEvent);
	struct MsgCloseReply cloR;
	cloR.type = MSG_TYPE_CLOSE_REPLY;
	cloR.UID = event->getUID();
	cloR.reqID = event->getReqID();
	cloR.retValue = STACK_ACTION_SUCCESS;
	_system->getSocketServer()->send((char *)&cloR, sizeof(struct MsgCloseReply), event->getUID());
}

//TODO: minor, not making a copy
void MF_ContentSocketManager::basicMsgAvailable(MF_EvUpTransp *event){
	if(!pendingRec.empty()){
		MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::msgAvailable there are pending recv requests");
		vector<MF_APIRecv *>::iterator it = pendingRec.begin();
		bool cont = true;
		while (it!=pendingRec.end() && cont) {
			MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::msgAvailable checking among pending recv request %u", (*it)->getReqID());
			if((*it)->getSize() < event->getSize()){
				MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::msgAvailable request %u has buffer of size %u too small for message of size %u", (*it)->getReqID(), (*it)->getSize(), event->getSize());
				it++;
			}
			else{
				MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::msgAvailable sending data to request %u", (*it)->getReqID());
				struct MsgRecvReply rcR;
				cont = false;
				MF_APIRecv *temp = (*it);
				pendingRec.erase(it);
				rcR.type = MSG_TYPE_RECV_REPLY;
				rcR.retValue = STACK_ACTION_SUCCESS;
				rcR.UID = ID;
				rcR.reqID = temp->getReqID();
				vector<u_char *> *v = new vector<u_char *>();
				u_int avail = trnsp->getData(v, temp->getSize());
				if(avail>0){
					//TODO: change this to get the info from the transport and not the packet
					RoutingHeader route(MF_PacketSupport::getNetworkHeaderPtr(v->front()));
					rcR.sGUID = route.getSrcGUID().to_int();
					//TODO: check this is correct
					rcR.size = avail;
					//rcR.size = MF_BasicTransport::computeChunkPktCnt(avail+MF_BasicTransport::computeMsgChkCnt(avail)*HIGH_HEADER_LEN);
					MF_Log::mf_log(MF_INFO, "MF_ContentSocketManager::msgAvailable replying to request %u with %u bytes of data", (*it)->getReqID(), avail);
					_system->getSocketServer()->send((char *)&rcR, sizeof(struct MsgRecvReply), &cliAddr);
					//MF_Log::mf_log_time("MF_Main:handleUpTransp RECV_TIME REPLIED");
					sendData(v, avail);
					recvBuffer.putVector(v);
				}
				else {
					MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::msgAvailable cannot send message because returned size from transport is 0");
				}
				delete temp;
			}
		}
		if(cont==false){
			//MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::msgAvailable: No waiting receive has enough buffer");
		}
	}
	else{
		MF_Log::mf_log(MF_INFO, "MF_ContentSocketManager::msgAvailable no recv requests pending for UID %u", ID);
	}
}

//TODO: minor, not making a copy (it's actually not using it at all..)
void MF_ContentSocketManager::contentMsgAvailable(MF_EvUpTransp *event){
	//TODO: check, if it's a content request, it checks if there is an available do_get pending and pushes up otherwise waits.
	//In any case it takes it back from the transport

	vector<u_char *> *v = new vector<u_char *>();
	//TODO 99999999 is just a place holder to make sure the msg is passed up from transport
	u_int avail = trnsp->getData(v, 999999999);
	//TODO I forgot that this could also be the answer for a sent get, but also a request to pass to do_get if any pending
	if(avail>0){
		RoutingHeader route(MF_PacketSupport::getNetworkHeaderPtr(v->front()));
		if(route.getServiceID().isContentResponse()){
			MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::contentMsgAvailable received a new content response");
			u_int srcGUID = (u_int)route.getSrcGUID().to_int();
			map<unsigned int, unsigned int>::iterator it = pendingGet.find(srcGUID);
			if (it!=pendingGet.end()) {
				struct MsgGetComplete mgc;
				mgc.type = MSG_TYPE_GET_SEND_RECV;
				mgc.UID = ID;
				mgc.reqID = it->second;
				mgc.retValue = STACK_ACTION_SUCCESS;
				mgc.size = avail;
				_system->getSocketServer()->send((char *)&mgc, sizeof(struct MsgGetComplete), &cliAddr);
				sendData(v, avail);
				recvBuffer.putVector(v);
				pendingGet.erase(srcGUID);
			}
			else{
				MF_Log::mf_log(MF_WARN, "MF_ContentSocketManager::contentMsgAvailable no pending request for guid %u", srcGUID);
				recvBuffer.putVector(v);
			}
		}
		else if(route.getServiceID().isContentRequest()){
			MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::contentMsgAvailable received a new content request");
			pendingGetRequest.push(v);
			pendingGetRequestSizes.push(avail);
			if(!pendingDoGet.empty()){
				MF_APIDoGet *doGetEvent = pendingDoGet.front();
				pendingDoGet.pop();
				replyDoGetRequest(doGetEvent);
				delete doGetEvent;
			}
		}
		else{
			MF_Log::mf_log(MF_WARN, "MF_ContentSocketManager::contentMsgAvailable I am working on content but the message was not a content request or response");
			recvBuffer.putVector(v);
		}
	}
	else {
		MF_Log::mf_log(MF_ERROR, "MF_ContentSocketManager::contentMsgAvailable cannot send message because returned size from transport is 0");
		recvBuffer.putVector(v);
	}
}

//TODO: minor, not making a copy
//wrong behaviour it should send some error message
void MF_ContentSocketManager::msgAvailable(MF_EvUpTransp *event){
	MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::msgAvailable new message available to be delivered to API from socket %u", ID);
	if(type == 1){
		basicMsgAvailable(event);
	}
	else{
		contentMsgAvailable(event);
	}
}

void MF_ContentSocketManager::releaseChunk(MF_ChunkInfo *ci){
	MF_SocketManager::releaseChunk(ci);
}

//TODO: do we want to do this operation in a new event in the stack (think so)
void MF_ContentSocketManager::newSpaceAvail(){
	MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:newSpaceAvail some buffer has been freed up; checking if there are pending send requests");
	vector<u_char *> *v = new vector<u_char *>();
	bool enoughSpace = true;
	if(type == 1){
		while(!pendingSend.empty() && enoughSpace){
			MF_APISend *event = pendingSend.front();
			//TODO use of HIGH_HEADER_LEN bad
			if(sendBuffer.getVectorBySize(v, event->getSize() + MF_PacketSupport::HIGH_HEADER_LEN*MF_BasicTransport::computeMsgChkCnt(event->getSize()))){
				MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:sendRequest the message will wait until there is memory available");
				enoughSpace = false;
			}
			else{
				pendingSend.pop();
				sendMessage(v, event->getUID(), event->getReqID(), event->getSize(), event->getDstGUID(), event->getOpts());
				//TODO: clean this behavior
				delete event;
			}
		}
	} else{
		while(!pendingGetResponse.empty() && enoughSpace){
			MF_APIGetResponse *event = pendingGetResponse.front();
			//TODO use of HIGH_HEADER_LEN bad
			if(sendBuffer.getVectorBySize(v, event->getSize() + MF_PacketSupport::HIGH_HEADER_LEN*MF_BasicTransport::computeMsgChkCnt(event->getSize()))){
				MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager:sendRequest the message will wait until there is memory available");
				enoughSpace = false;
			}
			else{
				pendingGetResponse.pop();
				sendGetResponseMessage(v, event->getUID(), event->getReqID(), event->getSize(), event->getSrcGUID(), event->getDstGUID(), event->getOpts());
				//TODO: clean this behavior
				delete event;
			}
		}
	}
}

void MF_ContentSocketManager::attachRequest(MF_APIAttach *event){
	MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::attachRequest");
	struct MsgAttachReply mar;
	for(int i=0; i<(int)event->getNGUID(); i++){
		_system->getSocketTable()->addGUID(event->getUID(), event->getGUID(i));
		_system->getRouter()->addGUID(event->getGUID(i));
		GUIDs.push_back(i);
	}
	mar.type = MSG_TYPE_ATTACH_REPLY;
	mar.UID = event->getUID();
	mar.reqID = event->getReqID();
	mar.retValue = STACK_ACTION_SUCCESS;
	_system->getSocketServer()->send((char*)&mar, sizeof(struct MsgAttachReply), event->getUID());
}

void MF_ContentSocketManager::detachRequest(MF_APIDetach *event){
	MF_Log::mf_log(MF_DEBUG, "MF_ContentSocketManager::detachRequest");
	struct MsgDetachReply mdr;
	for(int i=0; i<(int)event->getNGUID(); i++){
		_system->getSocketTable()->removeGUID(event->getUID(), event->getGUID(i));
		_system->getRouter()->removeGUID(i);
		GUIDs.remove(i);
	}
	mdr.type = MSG_TYPE_DETACH_REPLY;
	mdr.UID = event->getUID();
	mdr.reqID = event->getReqID();
	mdr.retValue = STACK_ACTION_SUCCESS;
	_system->getSocketServer()->send((char*)&mdr, sizeof(struct MsgDetachReply), event->getUID());
}

void MF_ContentSocketManager::cleanSocket() {
	MF_SocketManager::cleanSocket();
	//TODO check whether something else has to be cleaned
}
