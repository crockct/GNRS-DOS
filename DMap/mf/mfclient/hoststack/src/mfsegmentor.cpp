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
 * @file   mfsegmentor.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu), czhang
 * @date   July, 2011
 * @brief  Class that handles the transmission of data chunks, csyn and assoc/dassoc packets.
 *
 * Class that handles the transmission of data chunks, csyn and assoc/dassoc packets.
 */

#include <pcap.h>
#include <vector>
#include <errno.h>
#include <stdlib.h>

#include <include/mfhopmsg.hh>
#include <include/mfgstarctrlmsg.hh>
#include <include/mfroutingheader.hh>
#include <include/mfmhomeextheader.hh>

#include "mfsegmentor.h"
#include "mftypes.h"
#include "mfdeviceinfo.h"
#include "mfchunkinfo.h"
#include "mfipproto.h"
#include "mfetherproto.h"
#include "mflog.h"
#include "mfeventqueue.h"
#include "mfpacketsupport.h"

//TODO: redo this class (at least update parts that are new in stack

MF_Segmentor::MF_Segmentor() {
	srand(time(NULL));
	w_ostd = 0;
	w_sync = 0;
	mLastHopID = (unsigned int)rand() % MAX_HOP_ID;
	pendingChunk = 0;
	isPending = false;
	this->_system = NULL;
	mPcapHandle = NULL;
	timeTimout = 0;
	interfaceID = -1;
}

MF_Segmentor::MF_Segmentor(MFSystem *_system, pcap_t *handle, int devId, int t){
	srand(time(NULL));
	w_ostd = 0;
	w_sync = 0;
	mLastHopID = (unsigned int)rand() % MAX_HOP_ID;
	pendingChunk = 0;
	isPending = false;
	this->_system = _system;
	mPcapHandle = handle;
	timeTimout = t;
	interfaceID = devId;
}

MF_Segmentor::~MF_Segmentor() {

}

bool MF_Segmentor::isEmptyBitmap(unsigned char *bitmap, unsigned int pktCount){
	//TODO: this needs to be dramatically improved
	for (unsigned int i = 0; i < pktCount; i++) {
		if ((bitmap[i / 8] & (0x01 << (i % 8))) != 0) { //not received
			return false;
		}
	}
	return true;
}

void MF_Segmentor::setPCAPHandle(pcap_t *handle){
	mPcapHandle = handle;
}

unsigned int MF_Segmentor::getNewHopID() {
	unsigned int newID = (mLastHopID + 1) % MAX_HOP_ID;
	mLastHopID = newID;
	return newID;
}

bool MF_Segmentor::isAllSent(MFSendingChunk &sendingChunk, u_char *bitmap) {
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:isAllSent check if chunk %u is all sent", sendingChunk.csynSeqNum);
	unsigned int i = 0;
	bool ret = true;
	for (i = 0; i < sendingChunk.chunkInfo->getChunkPktCnt(); i++) {
		if ((bitmap[i / 8] & (0x01 << (i % 8))) == 0) { //not received
			sendingChunk.nonAckedPackets.push(i);
			ret = false;
		}
	}
	return ret;
}

unsigned int MF_Segmentor::countLost(MFSendingChunk &sendingChunk, u_char *bitmap) {
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:isAllSent check if chunk is all sent");
	unsigned int counter = 0;
	for (unsigned int i = 0; i < sendingChunk.chunkInfo->getChunkPktCnt(); i++) {
		if ((bitmap[i / 8] & (0x01 << (i % 8))) == 0) { //not received
			sendingChunk.nonAckedPackets.push(i);
			counter++;
		}
	}
	return counter;
}

void MF_Segmentor::fillHeader(MF_ChunkInfo *ci, MF_DeviceInfo *di) {
	unsigned int reminSize = ci->getChunkSize() + MF_PacketSupport::HIGH_HEADER_LEN;
	unsigned int seqNum = 0;
	unsigned int pldSize = 0;
	unsigned int i;
	vector <u_char *> *pList = ci->getPacketList();
	for (i = 0; i < ci->getChunkPktCnt(); i++) {
		pldSize = (reminSize > (unsigned int)MF_PacketSupport::MAX_PAYLOAD_SIZE) ?(unsigned int)MF_PacketSupport::MAX_PAYLOAD_SIZE : reminSize;
		MF_EtherProto::fillHeader((*pList)[i], 0, di->getSMAC().c_str(), di->getAPMAC().c_str());
		MF_IPProto::fillHeader((*pList)[i], ETH_HEADER_SIZE, pldSize + HOP_HEADER_SIZE, di->getSIP().c_str(), di->getAPIP().c_str());
		fillHopHeader((*pList)[i], ci->getHopID(), pldSize, seqNum);
		seqNum++;
		reminSize -= pldSize;
	}
}

void MF_Segmentor::fillHopHeader(u_char *buffer, unsigned int hopID, unsigned int pldSize, unsigned int seqNum) {
	struct hop_data *hop;
	hop = (struct hop_data *) MF_PacketSupport::getHopHeaderPtr(buffer);
	hop->type = htonl(DATA_PKT);
	hop->hop_ID = htonl(hopID);
	hop->pld_size = htonl(pldSize);
	hop->seq_num = htonl(seqNum);
}

void MF_Segmentor::sendCsyn(MF_ChunkInfo *ci, MF_DeviceInfo *di) {
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:sendCsyn send csyn packet");
	u_char buffer[256];
	int ret = 0;
	struct csyn *csyn;
	csyn = (struct csyn *) MF_PacketSupport::getHopHeaderPtr(buffer);
	csyn->type = htonl(CSYN_PKT);
	csyn->hop_ID = htonl(ci->getHopID());
	csyn->chk_pkt_count = htonl(ci->getChunkPktCnt());
	MF_IPProto::fillHeader(buffer, ETH_HEADER_SIZE, sizeof(struct csyn), di->getSIP().c_str(), di->getAPIP().c_str());
	MF_EtherProto::fillHeader(buffer, 0, di->getSMAC().c_str(), di->getAPMAC().c_str());
	if ((ret = pcap_inject(mPcapHandle, (void *) buffer, ETH_HEADER_SIZE + IP_HEADER_SIZE + sizeof(struct csyn))) < 0) {
		MF_Log::mf_log(MF_ERROR, "pcap_sendpacket error, errno=%d", errno);
		return;
	}
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor: Sent CSYN hop_ID=%u, pkt_count=%u, if = %s", ci->getHopID(), ci->getChunkPktCnt(), di->getName().c_str());

#ifdef __OML__
	di->increaseOutPackets();
	di->increaseOutBytesByInt(ETH_HEADER_SIZE + IP_HEADER_SIZE + sizeof(struct csyn));
#endif
}

unsigned int MF_Segmentor::sendData(MFSendingChunk &sendingChunk, MF_DeviceInfo *di) {
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:sendData send data chunk");
	int ret = 0;
	struct hop_data *hop;
	vector <u_char *> *pList = sendingChunk.chunkInfo->getPacketList();
	//Transmits only the max amount of allowed packets
	unsigned int maxPackets = MAX_OSTD - w_ostd;
	unsigned int min = maxPackets > sendingChunk.nonAckedPackets.size() ? sendingChunk.nonAckedPackets.size() : maxPackets;
	unsigned int index;
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:sendData sending %u packets given window size %u for hop_ID=%u", min, w_ostd, sendingChunk.csynSeqNum);
	unsigned int total_bytes = 0;
	for (unsigned int i = 0; i < min; i++) {
		hop = (struct hop_data *) MF_PacketSupport::getHopHeaderPtr((*pList)[i]);
		index = sendingChunk.nonAckedPackets.front();
		sendingChunk.nonAckedPackets.pop();
		//cout << "Sending packet " << index << endl;
		ret = pcap_inject(mPcapHandle, (void *) ((*pList)[index]), MF_PacketSupport::LOW_HEADER_LEN + ntohl(hop->pld_size));
		if (ret < 0) {
			MF_Log::mf_log(MF_ERROR, "MF_Segmentor::sendData(): pcap_inject() error, errno=%d", errno);
		}
		total_bytes += MF_PacketSupport::LOW_HEADER_LEN + ntohl(hop->pld_size);
	}
#ifdef __OML__
	di->increaseOutPacketsByInt(min);
	di->increaseOutBytesByInt(total_bytes);
	di->sendOMLUpdate();
#endif

	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor::sendData(): Sent %u packets. hop_ID=%u",
			min, sendingChunk.chunkInfo->getHopID());
	//MF_Log::mf_log_time("MF_Segmentor:procSend SEND_CSYN INIT chunk %lu interface %d", sendingChunk.chunkInfo->getChunkID(), di->getID());
	if(sendingChunk.nonAckedPackets.size() == 0){
		//cout << "Sent out all packets for chunk " << sendingChunk.csynSeqNum << endl;
		MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:sendData send all packets for hop_ID=%u",sendingChunk.csynSeqNum);
		sendCsyn(sendingChunk.chunkInfo, di);
		sendingChunk.isCsynSent = true;
		unsigned int *tempInt = (unsigned int*)calloc(1, sizeof(unsigned int));
		*tempInt = sendingChunk.csynSeqNum;
		sendingChunk.TOID = _system->getTimeoutManager()->AddTimeout(this, (void *)(tempInt), (long int)timeTimout);
		if(sendingChunk.csynSeqNum == pendingChunk && isPending){
			isPending = false;
		}
	} else {
		//cout << "Missing chunks for chunk " << sendingChunk.csynSeqNum << " will have to wait for availability in window" << endl;
		MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:sendData hop_ID=%u partially sent",sendingChunk.csynSeqNum);
		isPending = true;
		pendingChunk = sendingChunk.csynSeqNum;
	}
	return min;
}

void MF_Segmentor::startSendingChunk(MF_ChunkInfo *ci, MF_DeviceInfo *di) {
	//TODO: If enough, send depending on weather is a short chunk or long chunk
	unsigned int hopID = getNewHopID();
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:startSendingChunk starting to send new chunk with hop_ID=%u", hopID);
	MFSendingChunk sendingChunk;
	sendingChunk.chunkInfo = ci;
	sendingChunk.csynSeqNum = hopID;
	sendingChunk.isCsynSent = false;
	sendingChunk.isGranted = false;
	sendingChunk.transmitted = 0;
	if(ci->getChunkPktCnt() <= SMALL_PKT_SIZE){
		MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:sendData hop_ID=%u is a short chunk",sendingChunk.csynSeqNum);
		sendingChunk.isShortChunk = true;
	} else {
		MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:sendData hop_ID=%u is a long chunk",sendingChunk.csynSeqNum);
		sendingChunk.isShortChunk = false;
	}
	ci->putHopID(hopID);
	fillHeader(ci, di);
	//MF_Log::mf_log_time("MF_Segmentor:procSend SEND_CSYN INIT chunk hop_ID=%u interface=%d", ci->getChunkID(), di->getID());
	sendCsyn(ci, di);
	sendingChunk.isCsynSent = true;
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:sendData w_sync goes from %u",w_sync);
	w_sync += ci->getChunkPktCnt();
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:sendData to %u",w_sync);
	if(sendingChunk.isShortChunk){
		for(unsigned int i = 0; i < ci->getChunkPktCnt(); i++) {
			sendingChunk.nonAckedPackets.push(i);
		}
		w_ostd += sendData(sendingChunk, di);
		MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:sendData w_ostd is now %u",w_ostd);
	} else {
		MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:sendData not a short chunk so adding csyn TO for hop_ID=%u",sendingChunk.csynSeqNum);
		unsigned int *tempInt = (unsigned int*)calloc(1, sizeof(unsigned int));
		*tempInt = sendingChunk.csynSeqNum;
		sendingChunk.TOID = _system->getTimeoutManager()->AddTimeout(this, (void *)(tempInt), (long int)timeTimout);
	}
	outstandingChunks.insert(std::make_pair(hopID, sendingChunk));
}

/*
 * triggers when a new chunk is passed down from the routing layer
 */
void MF_Segmentor::procSend(MF_ChunkInfo *ci, MF_DeviceInfo *di) {
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:procSend send new chunk");
	//TODO: First check weather there is enough space to send chunk, otherwise queue
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:procSend sending new chunk %u",ci->getChunkID());
	if(w_sync >= MAX_OSTD + MAX_EXTRA_SYNC){
		MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:procSend Current w_sync %u full",w_sync);
		chunksQueue.push_back(ci);
		return;
	} else{
		MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:procSend Current w_sync %u has space",w_sync);
		startSendingChunk(ci, di);
	}
}

/*
 * triggers when a csyn timeout expires
 */
void MF_Segmentor::csynTO(unsigned int chunkID, MF_DeviceInfo *di){
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:csynTO TO has triggered for hop_ID=%u", chunkID);
	std::map<unsigned int,MFSendingChunk>::iterator it;
	it = outstandingChunks.find(chunkID);
	if(it!=outstandingChunks.end() && it->second.isCsynSent){
		MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:csynTO has to resend the csyn for hop_ID=%u because it got lost", chunkID);
		sendCsyn(it->second.chunkInfo, di);
		it->second.isCsynSent = true;
		unsigned int *tempInt = (unsigned int*)calloc(1, sizeof(unsigned int));
		*tempInt = it->second.csynSeqNum;
		it->second.TOID = _system->getTimeoutManager()->AddTimeout(this, (void *)tempInt, (long int)timeTimout);
		//TODO: if small chunk retransmit data too?
	}
	else{
		MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:csynTO nothing to do. Should have not triggered for hop_ID=%u", chunkID);
	}
}

/*
 * triggers when a csyn ack is received
 */
void MF_Segmentor::handleAck(unsigned int hop_ID, unsigned int pkt_count, u_char *bitmap, MF_DeviceInfo *di) {
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:handleAck Recvd csyn-ack for hop_ID=%u, pkt_count = %u, if = %s", hop_ID, pkt_count, di->getName().c_str());
	std::map<unsigned int,MFSendingChunk>::iterator it;
	it = outstandingChunks.find(hop_ID);
	if(it == outstandingChunks.end()){
		MF_Log::mf_log(MF_WARN, "MF_Segmentor:handleAck received ack for chunk not outstanding hop_ID=%u", hop_ID);
		return;
	}
	char tempBitmap[pkt_count/8+1];
	unsigned int n;
	for (n = 0; (n < (pkt_count - 1) / 8 + 1) && n < 512; n++) {
		sprintf(tempBitmap + n, "%x", bitmap[n]);
	}
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:handleAck hop_ID=%u received bitmap: %s", hop_ID, tempBitmap);
	if ((it->second.isCsynSent == false) || (hop_ID > mLastHopID)) { //wrong state, return directly
		MF_Log::mf_log(MF_WARN, "MF_Segmentor:handleAck(): unexpected csyn-ack: expected hopID=%u, got %u", mLastHopID, hop_ID);
		return;
	}
	else if(!it->second.isGranted && it->second.isShortChunk && isEmptyBitmap(bitmap, pkt_count)){
		MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:handleAck received initial ack for short hop_ID%u", hop_ID);
		it->second.isGranted=true;
		return;
	} else {
		//what is this case for?
	}

	_system->getTimeoutManager()->ClearTimeout(it->second.TOID);
	it->second.isCsynSent = false;
	unsigned int lost;

	if(!(lost = countLost(it->second, bitmap))) {
		MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:handleAck chunk with hop_ID=%u has been successfully transfered", hop_ID);
		MF_EvChkComplete *event = new MF_EvChkComplete(di->getID(), it->second.chunkInfo);
		_system->getEventQueue()->add(event);
		unsigned int oldTransmitted = it->second.transmitted;
		it->second.transmitted = it->second.chunkInfo->getPacketCount();
		//cout << w_ostd << " " << it->second.transmitted << " " << oldTransmitted << " " << lost << endl;
		w_ostd -= it->second.transmitted - oldTransmitted;
		MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:handleAck reducing w_ostd by %u to %u", it->second.transmitted - oldTransmitted, w_ostd);
		w_sync -= it->second.chunkInfo->getPacketCount();
		MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:handleAck reducing w_sync by %u to %u", it->second.chunkInfo->getPacketCount(), w_sync);
		outstandingChunks.erase(it->first);
	} else {
		if(it->second.isGranted==false){
			it->second.isGranted=true;
			MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:handleAck chunk with hop_ID=%u granted space", hop_ID);
			w_ostd += sendData(it->second, di);
			MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:handleAck w_ostd increased to %u", w_ostd);
		} else {
			MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:handleAck chunk with hop_ID=%u had %u losses", hop_ID, lost);
			unsigned int oldTransmitted = it->second.transmitted;
			it->second.transmitted = it->second.chunkInfo->getPacketCount() - lost;
			//cout << w_ostd << " " << it->second.transmitted << " " << oldTransmitted << " " << lost << endl;
			w_ostd -= it->second.transmitted - oldTransmitted + lost;
			MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:handleAck reducing w_ostd by %u to %u", it->second.transmitted - oldTransmitted, w_ostd);
			//MF_Log::mf_log_time("MF_Segmentor:handleAck SEND_DATA INIT hop_id %u", hop_ID);
			w_ostd += sendData(it->second, di);
			//MF_Log::mf_log_time("MF_Segmentor:handleAck SEND_DATA END hop_id %u", hop_ID);
		}
	}
	//TODO: try to send new if available
	if(isPending){
		MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:handleAck There are pending chunks that have been already synced");
		it = outstandingChunks.find(pendingChunk);
		w_ostd += sendData(it->second, di);
		MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:handleAck w_ostd increased to %u", w_ostd);
	}
	MF_ChunkInfo *nextChunk;
	while (w_sync < MAX_OSTD + MAX_EXTRA_SYNC && !chunksQueue.empty()){
		MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:handleAck w_sync size %u enough to send pending chunk", w_sync);
		nextChunk = chunksQueue.front();
		chunksQueue.pop_front();
		startSendingChunk(nextChunk, di);
	}
	return;
}

/*
 * This function should reset the status of the segmentor.
 * 		-remove outstanding chunks and push them back up the stack.
 */
void MF_Segmentor::reset(){
	return;
}

void MF_Segmentor::OnTimeout(void *message, unsigned int id){
	MF_EvCysnTOut *to = new MF_EvCysnTOut(interfaceID, *((unsigned int *)message));
	_system->getEventQueue()->add(to);
	free(message);
}

/*
 * From now on these functions are unrelated to the transmitting protocol
 */

bool MF_Segmentor::sendAck(MF_ChunkInfo *ci, MF_DeviceInfo *di) {
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:sendAck send ack packet for given chunk");
	u_char buffer[65536];// large enough for any size of bitmap
	memset((void *) buffer, 0xff, 65536);
	struct hop_data *hop;
	unsigned int pktCount = ci->getChunkPktCnt();
	struct csyn_ack *ack;
	ack = (struct csyn_ack *) MF_PacketSupport::getHopHeaderPtr(buffer);
	ack->type = htonl(CSYN_ACK_PKT);
	ack->hop_ID = htonl((unsigned int) (ci->getHopID()));
	ack->chk_pkt_count = htonl(pktCount);

	unsigned int i = 0;
	unsigned int j = 0;
	int ret;

	if (ci->isDelivered()) {// last ack get lost
		for (i = 0; i < (pktCount - 1) / 8 + 1; i++) {
			ack->bitmap[i] = 0xff;
		}
	} else {
		vector <u_char *> *pList = ci->getPacketList();
		for (i = 0; i < pktCount; i++) {
			hop = (struct hop_data *) MF_PacketSupport::getHopHeaderPtr((*pList)[i]);
			if (hop->pld_size == 0) { //did not get the packet yet
				*(ack->bitmap + (i / 8)) &= (0xfe << (i % 8));
				j++;
			}
		}
	}
	unsigned int z;
	//TODO remove the harcoded value
	char tempBitmap[512];
	for (z = 0; (z < (pktCount - 1) / 8 + 1) && z<512; z++) {
		sprintf(tempBitmap + z, "%x",ack->bitmap[z]);
	}
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:sendAck sent bitmap: %s", tempBitmap);

	MF_IPProto::fillHeader(buffer, ETH_HEADER_SIZE, CSYN_SIZE + ((pktCount - 1) / 8 + 1), di->getSIP().c_str(), di->getAPIP().c_str());
	MF_EtherProto::fillHeader(buffer, 0, di->getSMAC().c_str(), di->getAPMAC().c_str());

	if ((ret = pcap_inject(mPcapHandle, (void *) buffer, ETH_HEADER_SIZE + IP_HEADER_SIZE + CSYN_SIZE + ((pktCount - 1) / 8 + 1))) < 0) {
		MF_Log::mf_log(MF_ERROR, "MF_Segmentor:sendAck pcap_inject() error, errno=%d", errno);
		return false;
	}
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:sendAck Sent CSYN-ACK hop_ID=%d, recvd/lost/total %d/%d/%d if = %s", ntohl(ack->hop_ID), pktCount-j, j, pktCount, di->getName().c_str());

	//OML
#ifdef __OML__
	di->increaseOutBytesByInt(ETH_HEADER_SIZE + IP_HEADER_SIZE + CSYN_SIZE + ((pktCount - 1) / 8 + 1));
	di->increaseOutPackets();
#endif

	if (j == 0) { //get all the packets
		return true;
	}
	return false;
}

void MF_Segmentor::sendAssoRequest(unsigned int entityGUID, unsigned int hostGUID, u_short weight, MF_DeviceInfo *di) {
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:sendAssoRequest send association request packet");
	u_char buffer[256];
	int ret = 0;
	struct client_assoc_pkt *asso;
	asso = (struct client_assoc_pkt *) MF_PacketSupport::getHopHeaderPtr(buffer);
	asso->type = htonl(ASSOC_PKT);
	asso->client_GUID = htonl(entityGUID);
	asso->host_GUID = htonl(hostGUID);
	asso->weight = htons(weight);
	MF_IPProto::fillHeader(buffer, ETH_HEADER_SIZE, sizeof(struct client_assoc_pkt), di->getSIP().c_str(), di->getAPIP().c_str());
	MF_EtherProto::fillHeader(buffer, 0, di->getSMAC().c_str(), di->getAPMAC().c_str());
	if ((ret = pcap_inject(mPcapHandle, (void *) buffer, ETH_HEADER_SIZE + IP_HEADER_SIZE + sizeof(struct client_assoc_pkt))) < 0) {
		MF_Log::mf_log(MF_ERROR, "pcap_sendpacket error, errno=%d", errno);
		return;
	}
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor: Sent Assoc pkt entity GUID = %u, host GUID = %u, if = %s weight = %u", entityGUID, hostGUID, di->getName().c_str(), weight);

#ifdef __OML__
	di->increaseOutBytesByInt(ETH_HEADER_SIZE + IP_HEADER_SIZE + sizeof(struct client_assoc_pkt));
	di->increaseOutPackets();
#endif
}

void MF_Segmentor::sendDassoRequest(unsigned int entityGUID, unsigned int hostGUID, MF_DeviceInfo *di) {
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:sendDassoRequest send deassociation request packet");
	u_char buffer[256];
	int ret = 0;
	struct client_dassoc_pkt *asso;
	asso = (struct client_dassoc_pkt *) MF_PacketSupport::getHopHeaderPtr(buffer);
	asso->type = htonl(DASSOC_PKT);
	asso->entity_GUID = htonl(entityGUID);
	asso->host_GUID = htonl(hostGUID);
	MF_IPProto::fillHeader(buffer, ETH_HEADER_SIZE, sizeof(struct client_dassoc_pkt), di->getSIP().c_str(), di->getAPIP().c_str());
	MF_EtherProto::fillHeader(buffer, 0, di->getSMAC().c_str(), di->getAPMAC().c_str());
	if ((ret = pcap_inject(mPcapHandle, (void *) buffer, ETH_HEADER_SIZE + IP_HEADER_SIZE + sizeof(struct client_dassoc_pkt))) < 0) {
		MF_Log::mf_log(MF_ERROR, "pcap_sendpacket error, errno=%d", errno);
		return;
	}
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor: Sent d-asso request for entity GUID = %d, host GUID = %d, if = %s", entityGUID, hostGUID, di->getName().c_str());

#ifdef __OML__
	di->increaseOutBytesByInt(ETH_HEADER_SIZE + IP_HEADER_SIZE + sizeof(struct client_dassoc_pkt));
	di->increaseOutPackets();
#endif
}

void MF_Segmentor::sendLPAck(unsigned int sourceLPACK, unsigned int destinationLPACK, unsigned int seq_no_cp, MF_DeviceInfo *di){
	MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:sendLPAck send deassociation request packet");
	u_char buffer[256];
	int ret;
	struct link_probe_ACK *lpa;
	lpa = (struct link_probe_ACK *)MF_PacketSupport::getHopHeaderPtr(buffer);
	lpa->type = htonl(LP_ACK_PKT);
	lpa->destinationLPACK = htonl(destinationLPACK);
	lpa->sourceLPACK = htonl(sourceLPACK);
	lpa->seq_no_cp = htonl(seq_no_cp);
	MF_IPProto::fillHeader(buffer, ETH_HEADER_SIZE, sizeof(struct link_probe_ACK), di->getSIP().c_str(), di->getAPIP().c_str());
	MF_EtherProto::fillHeader(buffer, 0, di->getSMAC().c_str(), di->getAPMAC().c_str());
	if ((ret = pcap_inject(mPcapHandle, (void *) buffer, ETH_HEADER_SIZE + IP_HEADER_SIZE + sizeof(struct link_probe_ACK))) < 0) {
		MF_Log::mf_log(MF_ERROR, "MF_DeviceManager:sendLPAck pcap_inject() error, errno=%d", errno);
	}
	else{
		MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:sendLPAck link prob ack sent");
	}

#ifdef __OML__
	di->increaseOutBytesByInt(ETH_HEADER_SIZE + IP_HEADER_SIZE + sizeof(struct link_probe_ACK));
	di->increaseOutPackets();
#endif
}

void MF_Segmentor::resetStatus() {
	std::map<unsigned int, MFSendingChunk>::reverse_iterator it;
	for (it = outstandingChunks.rbegin(); it!=outstandingChunks.rend(); it++){
		//push to front
		chunksQueue.push_front(it->second.chunkInfo);
	}
	outstandingChunks.clear();
	w_ostd = 0;
	w_sync = 0;
	isPending = false;
	mLastHopID = 0;
}

void MF_Segmentor::interfaceEnabled(MF_DeviceInfo *di) {
	//TODO: this should not be handled by the segmentor
	while(w_sync < MAX_OSTD + MAX_EXTRA_SYNC && !chunksQueue.empty()){
		MF_Log::mf_log(MF_DEBUG, "MF_Segmentor:procSend Current w_sync %u has space",w_sync);
		MF_ChunkInfo *ci = chunksQueue.front();
		chunksQueue.pop_front();
		startSendingChunk(ci, di);
	}
}
