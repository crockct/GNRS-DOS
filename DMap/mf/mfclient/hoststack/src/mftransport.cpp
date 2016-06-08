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
 * @file   mftransport.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   September, 2013
 * @brief  Implementation of different transport protocols for mobilityfirst.
 *
 * Implementation of different transport protocols for mobilityfirst.
 */

#include <list>
#include <map>

#include <include/mfflags.h>

#include "mfbuffer.h"
#include "mftypes.h"
#include "mfchunkinfo.h"
#include "mfeventqueue.h"
#include "mftransport.h"
#include "mfpacketsupport.h"


bool compChunkInfoPtr(MF_ChunkInfo *a, MF_ChunkInfo *b){
	//return a->getChunkID()<b->getChunkID();
  return a->getStartOffset() < b->getStartOffset();
}

MF_Message::MF_Message(u_short total) : bitmap((total-1)/8 + 1, 0){
  MF_Log::mf_log(MF_DEBUG, "MF_Message:MF_Message() bit map has length: %u", (total-1)/8 + 1);
	mCount = 0;
	mTotal = 0;
	remaining = 0;
	dstGUID = 0;
	srcGUID = 0;
}

MF_Message::~MF_Message(){
	
}

void MF_Message::addChunk(MF_ChunkInfo *c){
	chkList.push_back(c);
	mCount++;
	if(isComplete()){
		MF_Log::mf_log(MF_DEBUG, "MF_Message:addChunk chunk complete, ordering and creating vector of data");
		sortList();
		createData();
	}
}

bool MF_Message::isComplete(){
	return mCount == mTotal;
}

void MF_Message::sortList(){
	chkList.sort(compChunkInfoPtr);
}

void MF_Message::createData(){
	list<MF_ChunkInfo *>::iterator it;
	vector<u_char *> *tempData;
	for(it=chkList.begin(); it!=chkList.end(); ++it){
		tempData = (*it)->getPacketList();
		for(u_int i = 0; i<(*it)->getChunkPktCnt(); i++){
			data.push_back((*tempData)[i]);
		}
		remaining += (*it)->getChunkSize();
	}
}

u_int MF_Message::getData(vector<u_char *> *v, u_int size){
	MF_Log::mf_log(MF_DEBUG, "MF_Message:getData");
	//TODO add support for sending less than the size
	if(size < remaining || remaining == 0){
		MF_Log::mf_log(MF_DEBUG, "MF_Message:getData remaining %u size %u", remaining, size);
		return 0;
	}
	else{
		MF_Log::mf_log(MF_DEBUG, "MF_Message:getData sending back %d", data.size());
		u_int ret = remaining;
		remaining = 0;
		for(u_int i = 0; i< data.size(); i++){
			v->push_back(data[i]);
		}
		return ret;
	}
}


MF_BasicTransport::MF_BasicTransport(){
	
}

MF_BasicTransport::MF_BasicTransport(u_int tid, MF_Buffer *recvBuffer, MF_Buffer *sendBuffer, MFSystem *_system) :
		MF_Transport(recvBuffer, sendBuffer, _system){
	mTID = tid;
	oTID = 0;
	lastChunkID = 0;
	this->recvBuffer = recvBuffer;
	this->sendBuffer = sendBuffer;
	this->_system = _system;
}

MF_BasicTransport::~MF_BasicTransport(){
	
}

/*
 * Count the number of chunks in a message
 */
u_int MF_BasicTransport::computeMsgChkCnt(u_int size) {
	u_int total = size + MF_PacketSupport::HIGH_HEADER_LEN;
	u_int quotient = total / (MF_PacketSupport::MAX_PAYLOAD_SIZE*MAX_CHUNK_SIZE);
	u_int remainder = total % (MF_PacketSupport::MAX_PAYLOAD_SIZE*MAX_CHUNK_SIZE);
	u_int cnt = remainder == 0 ? quotient : (quotient + 1);
	return cnt;
}

/*
 * Count the number of packets in a chunk
 */
u_int MF_BasicTransport::computeChunkPktCnt(u_int size) {
	u_int total = size + MF_PacketSupport::HIGH_HEADER_LEN;
	u_int quotient = total / MF_PacketSupport::MAX_PAYLOAD_SIZE;
	u_int remainder = total % MF_PacketSupport::MAX_PAYLOAD_SIZE;
	u_int cnt = remainder == 0 ? quotient : (quotient + 1);
	return cnt;
}

void MF_BasicTransport::sendData(vector <u_char *> *data, u_int size, int srcGUID, int dstGUID, mfflag_t opts){
	MF_Log::mf_log(MF_DEBUG, "MF_BasicTransport:sendData send data received from API, srcGUID: %u, dstGUID: %u", srcGUID, dstGUID);
	u_int count = computeMsgChkCnt(size);
	MF_Log::mf_log(MF_DEBUG, "MF_BasicTransport:sendData new message of size %u will be composed of %u messages", size, count);
	//Create message
	MF_Message m;
	m.setTotal((u_short)count);
	MF_ChunkInfo *ciTemp;
	vector <u_char *> *tempData;
	int pkts = data->size();
	int rem;
	u_int chkSize = 0, remSize = size;
	u_int msgID = lastChunkID;
	MF_Log::mf_log(MF_DEBUG, "MF_BasicTransport:sendData new message/s. MsgID=%u N_chunks=%u srcTID=%u dstTID=%u", msgID, count, mTID, oTID);
	for(u_int i = 0; i<count; i++){
		ciTemp = new MF_ChunkInfo();
		ciTemp->putDstTID(oTID);
		ciTemp->putSrcTID(mTID);
		ciTemp->putSrcGUID(srcGUID);
		ciTemp->putDstGUID(dstGUID);
		ciTemp->putStartOffset((u_short)i);
		ciTemp->putEndOffset((u_short)(count-i-1));
		tempData = ciTemp->getPacketList();
		//copy pointers in chunk
		rem = (pkts >= MAX_CHUNK_SIZE) ? MAX_CHUNK_SIZE : pkts;
		//If fits, I can use the whole data remaining, otherwise I can use only max minus HIGH_HEADER_LEN
		chkSize = (pkts > MAX_CHUNK_SIZE) ? rem*MF_PacketSupport::MAX_PAYLOAD_SIZE - MF_PacketSupport::HIGH_HEADER_LEN : remSize;
		remSize -= chkSize;
		ciTemp->putChunkSize(chkSize);
		pkts -= rem;
		MF_Log::mf_log(MF_DEBUG, "MF_BasicTransport:sendData new chunk will be composed of %u packets", rem);
		for(int j = 0; j<rem;  j++){
			tempData->push_back((*data)[i*MAX_CHUNK_SIZE + j]);
		}
		ciTemp->putChunkPktCnt(rem);
		ciTemp->putChunkID(lastChunkID);
		lastChunkID++;
		//TODO read options for service ID
		ciTemp->putServiceID(0);
		ciTemp->setOwner(mTID);
		ciTemp->setOpts(opts);
		ciTemp->putMsgID(msgID);
		//prepare a chunk for the router
		fillHeader(ciTemp);
		MF_EvDownTransp *erc = new MF_EvDownTransp(ciTemp);
		_system->getEventQueue()->add(erc);
	}
}

void MF_BasicTransport::recvData(MF_ChunkInfo *ci){
	MF_Log::mf_log(MF_DEBUG, "MF_BasicTransport:recvData received new chunk from router layer");
	vector<u_char *> *newChunk = ci->getPacketList();
	//Check if I have an outstanding message for this chunk
	TransHeader transpHeader(MF_PacketSupport::getTransportHeaderPtr((*newChunk)[0], true));
	transpHeader.init();
	u_int msgID = transpHeader.getSeq() - transpHeader.getStartOffset();
	map <u_int, MF_Message *>::iterator it = incoming.find(msgID);
	MF_Message *newMsg;
	if(it == incoming.end()){
		MF_Log::mf_log(MF_DEBUG, "MF_BasicTransport::recvData received chunk of new message %hu", msgID);
		newMsg = new MF_Message();
		newMsg->setTotal(transpHeader.getStartOffset() + transpHeader.getEndOffset() + 1);
		MF_Log::mf_log(MF_DEBUG, "MF_BasicTransport::recvData new message will have %hu chunks",newMsg->getTotal());
		incoming.insert(make_pair(msgID, newMsg));
	}
	else{
		MF_Log::mf_log(MF_DEBUG, "MF_BasicTransport::recvData received chunk of incomplete message %hu that will have %u", msgID, transpHeader.getStartOffset() + transpHeader.getEndOffset() + 1);
		newMsg = it->second;
	}
	vector<u_char *> *rec = new vector<u_char *>();
	ci->putChunkSize(transpHeader.getChkSize());
	ci->putStartOffset(transpHeader.getStartOffset());
	ci->putEndOffset(transpHeader.getEndOffset());
	if(recvBuffer->getVectorBySize(rec, ci->getChunkSize() + MF_PacketSupport::HIGH_HEADER_LEN)<0){
		MF_Log::mf_log(MF_WARN, "MF_BasicTransport::recvData no space available in buffer");
		return;
	}
	MF_Log::mf_log_time("MF_BasicTransport::recvData() id=%u source=%u destination=%u size=%u packets=%u",
			ci->getChunkID(), ci->getSrcGUID(), ci->getDstGUID(), ci->getChunkSize(), ci->getChunkPktCnt());
	for(u_int i = 0; i<ci->getChunkPktCnt(); i++){
		//MF_Log::mf_log(MF_DEBUG, "MF_BasicTransport::recvData copying packet %u of %u", i+1, ci->getPacketCount());
		memcpy((*rec)[i], (*newChunk)[i], MF_PacketSupport::MAX_PAYLOAD_SIZE + MF_PacketSupport::LOW_HEADER_LEN);
	}
	MF_ChunkInfo *newCi = new MF_ChunkInfo();
	newCi->putChunkID(ci->getChunkID());
	newCi->putChunkSize(ci->getChunkSize());
	newCi->putChunkPktCnt(ci->getChunkPktCnt());
	newCi->putStartOffset(ci->getStartOffset());
	newCi->putEndOffset(ci->getEndOffset());
	newCi->putPktList(rec);
	newMsg->addChunk(newCi);
	if(newMsg->isComplete()){
		MF_Log::mf_log(MF_DEBUG, "MF_BasicTransport::recvData received whole message, alerting stack manager");
		incoming.erase(msgID);
		complete.push_back(newMsg);
		//Make the parameters to be correct
		MF_EvUpTransp *event = new MF_EvUpTransp(mTID, newMsg->getRemaining(), ci->getDstGUID());
		_system->getEventQueue()->add(event);
	}
}

void MF_BasicTransport::fillHeader(MF_ChunkInfo *ci){
//	struct transport_header *trans;
	vector <u_char *> *pList = ci->getPacketList();
//	trans = (struct transport_header *)MF_PacketSupport::getTransportHeaderPtr(pList->front(), true);
//	trans->chunkID = htonl(ci->getChunkID());
//	trans->chunkPktCnt = htonl(ci->getChunkPktCnt());
//	trans->chunkSize = htonl(ci->getChunkSize());
//	trans->dstTID = htonl(ci->getDstTID());
//	trans->srcTID = htonl(ci->getSrcTID());
//	trans->msgID = htonl(ci->getMsgID());
//	trans->msgNum = htons(ci->getMsgNum());
//	trans->offset = htons(ci->getOffset());
	
	TransHeader transpHeader(MF_PacketSupport::getTransportHeaderPtr(pList->front(), true));
	transpHeader.init();
	transpHeader.setSeq(ci->getChunkID());
	transpHeader.setChkSize(ci->getChunkSize());
	transpHeader.setPktCnt(ci->getChunkPktCnt());
	transpHeader.setStartOffset(ci->getStartOffset());
	transpHeader.setEndOffset(ci->getEndOffset());
	transpHeader.setTransType(DATA_T);
	//Can this be left unset if no congestion mechanism is called?
	//transpHeader.setTransFlag();
	transpHeader.setReliabPref(PREF_DONT_CARE);
	transpHeader.setTransOffset(TRANS_BASE_HEADER_SIZE);
}

bool MF_BasicTransport::dataAvailable(){
	return !complete.empty();
}

u_int MF_BasicTransport::getData(vector<u_char *> *v, u_int size){
	MF_Log::mf_log(MF_DEBUG, "MF_BasicTransport:getData release data for upper layers");
	u_int ret;
	MF_Message *temp = complete.front();
	if(temp == NULL){
		return 0;
	}
	ret = temp->getData(v, size);
	if(ret>0){
		complete.pop_front();
	}
	return ret;
}

int MF_BasicTransport::releaseChunk(MF_ChunkInfo *ci){
	MF_Log::mf_log(MF_DEBUG, "MF_SocketManager::releaseChunk release message sent");
	vector<u_char *> *v = ci->getPacketList();
	if(v!=NULL){
		sendBuffer->putVector(v);
		MF_EvSpaceAvail *event = new MF_EvSpaceAvail(mTID);
		_system->getEventQueue()->add(event);
	}
	else{
		MF_Log::mf_log(MF_WARN, "MF_SocketManager::releaseChunk releasing empty buffer");
	}
}
