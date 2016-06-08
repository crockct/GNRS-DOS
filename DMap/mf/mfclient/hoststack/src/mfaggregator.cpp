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
 * @file   mfaggregator.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   July, 2011
 * @brief  Temp.
 *
 * Temp
 */

//TODO: make this class thread safe

#include <map>

#include <include/mfhopmsg.hh>

#include "mfaggregator.h"
#include "mftypes.h"
#include "mfchunkinfo.h"
#include "mfetherproto.h"
#include "mfipproto.h"
#include "mfbuffer.h"
#include "mftypes.h"
#include "mflog.h"
#include "mfpacketsupport.h"

MF_Aggregator::MF_Aggregator() : mPool(10) {
	lastHopID = 0;
	_system = NULL;
}

MF_Aggregator::MF_Aggregator(MFSystem *_system) : mPool(10) {
	this->_system = _system;
	lastHopID = 0;
}

MF_Aggregator::MF_Aggregator(MFSystem *_system, int bufSize) : mPool(bufSize) {
	this->_system = _system;
	lastHopID = 0;
}

MF_Aggregator::~MF_Aggregator() {
	//Clean buffer
	mPool.dePopulate();
}


/*
 * a data copy happens here
 */
void MF_Aggregator::handleData(const u_char *packet) {
	hop_data_t *hop = (hop_data_t *)packet;
	MF_Log::mf_log(MF_DEBUG, "MF_Aggregator:handleData Recvd data for chunk = %u seqNum=%u", ntohl(hop->hop_ID), ntohl(hop->seq_num));
	map <u_int, MF_ChunkInfo *>::iterator it;
	it = outsChunks.find(ntohl(hop->hop_ID));
	if(it==outsChunks.end()){
		MF_Log::mf_log(MF_WARN, "MF_Aggregator:handleData data received for chunk we are not expecting");
		return;
	}
	else{
		MF_ChunkInfo *ci = it->second;
		vector <u_char *> *pList = ci->getPacketList();
		if(ci) memcpy(((*pList)[ntohl(hop->seq_num)]+ETH_HEADER_SIZE+IP_HEADER_SIZE), packet, HOP_HEADER_SIZE + MF_PacketSupport::MAX_PAYLOAD_SIZE);
	}
}

/*
 * Handle csyn
 */
int MF_Aggregator::handleCsyn(u_int hopID, u_int pktCount) {
	MF_Log::mf_log(MF_INFO, "MF_Aggregator:handleCsyn Received CSYN hop_ID = %u, pkt_count = %u",hopID, pktCount);
	map <u_int, MF_ChunkInfo *>::iterator it;
	if((it = outsChunks.find(hopID)) != outsChunks.end()){
		if(it->second->isDelivered() == false){
			MF_Log::mf_log(MF_DEBUG, "MF_Aggregator:handleCsyn chunk is incomplete or not acknowledged, I will send an ack");
			return 1;
		}
		else {//ack got lost
			MF_Log::mf_log(MF_DEBUG, "MF_Aggregator:handleCsyn chunk was already delivered, I will send a complete ack");
			return 2;
		}
	} else if((it = receivedChunks.find(hopID)) != receivedChunks.end()){
		MF_Log::mf_log(MF_DEBUG, "MF_Aggregator:handleCsyn chunk was already delivered, I will send a complete ack");
		return 2;
	} else {
		MF_Log::mf_log(MF_DEBUG, "MF_Aggregator:handleCsyn received CSYN for OLD chunk. hop_ID = %u", hopID);
	}
	return 0;
}

/*
 * Handle csyn for first csyn message, should be processed in the receiving thread
 */
int MF_Aggregator::handleCsynInit(const unsigned char *packet) {
	csyn_t *csyn = (csyn_t *)packet;
	unsigned int hopID = ntohl(csyn->hop_ID);
	unsigned int pktCount = ntohl(csyn->chk_pkt_count);
	MF_Log::mf_log(MF_INFO, "MF_Aggregator:handleCsynInit Received CSYN hop_ID = %u, pkt_count = %u",hopID, pktCount);
	MF_ChunkInfo *ci;
	map <unsigned int, MF_ChunkInfo *>::iterator itOuts;
	if((itOuts = outsChunks.find(hopID)) == outsChunks.end()){
		if(receivedChunks.find(hopID) == receivedChunks.end()){
			ci = new MF_ChunkInfo();
			vector<u_char *> *chkpkt = ci->getPacketList();
			/* late reset */
			ci->reset();
			if (mPool.getVectorBySlots(chkpkt, pktCount) < 0) {
				MF_Log::mf_log(MF_WARN, "MF_Aggregator:handleCsynInit Run out of buffer space...");
				return -1; //no ack sent out, sender will timeout
			}
			ci->putHopID(hopID);
			lastHopID = hopID;
			ci->putChunkPktCnt(pktCount);
			outsChunks.insert(make_pair(hopID, ci));
		}
	}
	return 0;
}

//TODO: this can go, it's called only by releaseChunk
void MF_Aggregator::tryRelease(MF_ChunkInfo *ci){
	MF_Log::mf_log(MF_DEBUG, "MF_Aggregator:tryRelease releasing chunk read from upper layers with chk ID %u", ci->getHopID());
	mPool.putVector(ci->getPacketList());
	outsChunks.erase(ci->getHopID());
	receivedChunks.insert(make_pair(ci->getHopID(), ci));
	//Delete all chunks with hopid old enough
	//TODO: do this on a dedicated event
	if (receivedChunks.size() == MAX_OLD_CHUNKS){
		MF_ChunkInfo *ciTemp = NULL;
		std::map<u_int, MF_ChunkInfo *>::iterator it;
		for (int i = 0; i < COMPLETE_CLEAN; i++){
			it = receivedChunks.begin();
			ciTemp = it->second;
			receivedChunks.erase(it->first);
			delete ciTemp;
		}
	}
}

void MF_Aggregator::releaseChunk(u_int chkID){
	MF_Log::mf_log(MF_DEBUG, "MF_Aggregator:releaseChunk releasing chunk read from upper layers with chk ID %u", chkID);
	map <u_int, MF_ChunkInfo *>::iterator it;
	it = outsChunks.find(chkID);
	if(it!=outsChunks.end()){
		MF_ChunkInfo *ci = it->second;
		ci->setReleased(true);
		tryRelease(ci);
	}
}

//TODO: Should not look into both maps.
MF_ChunkInfo *MF_Aggregator::getChunkInfo(u_int hopID){
	MF_Log::mf_log(MF_DEBUG, "MF_Aggregator:getChunkInfo");
	map <u_int, MF_ChunkInfo *>::iterator it;
	it = outsChunks.find(hopID);
	if(it==outsChunks.end()){
		MF_Log::mf_log(MF_DEBUG, "MF_Aggregator:getChunkInfo chunk %u not found in the outstanding chunks", hopID);
		it = receivedChunks.find(hopID);
		if(it==receivedChunks.end()){
			MF_Log::mf_log(MF_DEBUG, "MF_Aggregator:getChunkInfo chunk %u not found in the received chunks", hopID);
			return NULL;
		}
		else{
			return it->second;
		}
	}
	else{
		return it->second;
	}
}

/*
 * Reset the status of the aggregator.
 * 		All pending chunks are removed
 * 		Last received id is set to 0
 */
void MF_Aggregator::reset(){
	map <u_int, MF_ChunkInfo *>::iterator it;
	//release all maps
	lastHopID = 0;
}

void MF_Aggregator::resetStatus() {
	map <u_int, MF_ChunkInfo *>::iterator it;
	for(it = outsChunks.begin(); it != outsChunks.end(); it++) {
	    it->second->resetReceived();
	}
	lastHopID = 0;
}
