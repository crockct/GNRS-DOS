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
 * @file   mfbuffer.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   July, 2011
 * @brief  Buffer manager.
 *
 * Buffer manager. One assigned to each socket.
 */

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <queue>
#include <vector>
#include <semaphore.h>

#include "mfbuffer.h"
#include "mfpacketsupport.h"
#include "mftypes.h"
#include "mflog.h"

MF_Buffer::MF_Buffer(u_int numOfChunks) {
	/*
	 * ether header 14B
	 * ip header 20B (used to traverse wimax bs only)
	 * hop header 13B
	 * routing header 16B
	 * transport header 4B
	 * payload size: 1024B
	 * totally 1091B
	 * bufferpool size: 1091x1000
	 */
	sem_init(&buf_acc, 0, 1);
	populate(numOfChunks);
	totalSize = numOfChunks*MAX_CHUNK_SIZE*SLOT_SIZE;
}

MF_Buffer::~MF_Buffer() {
	dePopulate();
	sem_destroy(&buf_acc);
}

void MF_Buffer::populate(u_int numOfChunks) {
	sem_wait(&buf_acc);
	for (u_int i=0; i<numOfChunks*MAX_CHUNK_SIZE; i++) {
		u_char *slot = new u_char[SLOT_SIZE];
		memset(slot,0, SLOT_SIZE);
		mBufferPool.push(slot);
	}
	sem_post(&buf_acc);
}

void MF_Buffer::dePopulate() {
	sem_wait(&buf_acc);
	u_char *temp;
	while (!mBufferPool.empty()) {
		temp = mBufferPool.front();
		mBufferPool.pop();
		delete temp;
		
	}
	sem_post(&buf_acc);
}

int MF_Buffer::getVectorBySize(vector<u_char *> *v, u_int size) {
	//checking pars are ok
	if(!v || size <= 0) return -1;
	u_int quotient = size/MF_PacketSupport::MAX_PAYLOAD_SIZE;
	u_int remainder = size%MF_PacketSupport::MAX_PAYLOAD_SIZE;
	u_int slots = remainder==0?quotient:(quotient+1);
	sem_wait(&buf_acc);
	if (mBufferPool.size() >= slots) {
		for (u_int i=0; i<slots; i++) {
			v->push_back(mBufferPool.front());
			mBufferPool.pop();
		}
		sem_post(&buf_acc);
		return 0;
	}
	sem_post(&buf_acc);
	MF_Log::mf_log(MF_WARN, "MF_Buffer:getVectorBySlots not enough sapce to assign vector");
	return -1;
}

int MF_Buffer::getVectorBySlots(vector<u_char *> *v, u_int count) {
	//checking parameters are ok
	if(!v || count <= 0) return -1;
	sem_wait(&buf_acc);
	if (mBufferPool.size() >= count) {
		for (u_int i=0; i<count; i++) {
			v->push_back(mBufferPool.front());
			mBufferPool.pop();
		}
		sem_post(&buf_acc);
		return 0;
	}
	sem_post(&buf_acc);
	MF_Log::mf_log(MF_WARN, "MF_Buffer:getVectorBySlots not enough sapce to assign vector");
	return -1;
}

void MF_Buffer::putVector(vector<u_char *> *v) {
	u_int i;
	sem_wait(&buf_acc);
	for ( i=0; i<v->size(); i++) {
		mBufferPool.push((*v)[i]);
		memset((*v)[i], 0, SLOT_SIZE);
	}
	sem_post(&buf_acc);
}

unsigned int MF_Buffer::getSize() {
	sem_wait(&buf_acc);
	u_int size = mBufferPool.size();
	sem_post(&buf_acc);
	return size;
}

unsigned int MF_Buffer::getTotalSizePackets(){
	return totalSize/SLOT_SIZE;
}

unsigned int MF_Buffer::getTotalSizeBytes(){
	return totalSize;
}
