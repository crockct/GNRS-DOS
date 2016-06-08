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
 * @file   mfchunkinfo.h
 * @author wontoniii (bronzino@winlab.rutgers.edu), czhang
 * @date   July, 2011
 * @brief  Temp.
 *
 * Temp.
 */

#ifndef MF_CHUNKINFO_H_
#define MF_CHUNKINFO_H_

#include <vector>
#include <list>

#include <include/mfflags.h>

#include "mftypes.h"
#include "mflog.h"

using namespace std;

class MF_ChunkInfo {
	unsigned int mDstNA; //routing, returned by netmanager
	unsigned int mSrcGUID; //routing, get from a file
	unsigned int mDstGUID; //routing, get from scheme or mfsendto is used
	unsigned int mServiceID; //routing
	unsigned int mChunkID; //transport
	unsigned int mChunkSize; //transport
	unsigned int mChunkPktCnt;
	unsigned int mSrcAppID;
	unsigned int mDstAppID;
	unsigned int mMsgID;
	unsigned short startOffset;
	unsigned short endOffset;
	unsigned int mHopID; //hop, hold for easy access, need to be updated when re-transmission happen
	bool mIsChunkDelivered; //used for DeviceManager to determine what to do when received a csyn packet
	bool mIsReadyForRelease; //used for DeviceManager to determine what to do when received a csyn packet

	vector<u_char *> mPacketList;
	unsigned int owner;
	mfflag_t opts;
  unsigned int seq;
public:
  void putTransSeq(unsigned int s) { seq = s; }
  unsigned int getTransSeq() { return seq; }
  	
	MF_ChunkInfo();
	MF_ChunkInfo(unsigned int);
	~MF_ChunkInfo();
	unsigned int getPacketCount();
	unsigned int getChunkPktCnt();
	void putChunkPktCnt(unsigned int);
	void reset();
	void resetReceived();
	void putSrcGUID(unsigned int);
	unsigned int getSrcGUID();
	void putDstGUID(unsigned int);
	unsigned int getDstGUID();
	void putServiceID(unsigned int);
	unsigned int getServiceID();
	void putChunkID(unsigned int);
	unsigned int getChunkID();
	void putChunkSize(unsigned int);
	unsigned int getChunkSize();
	void putDstNA(unsigned int);
  unsigned int getDstNA();
	void putHopID(unsigned int);
	unsigned int getHopID();
	void putSrcTID(unsigned int);
	unsigned int getSrcTID();
	void putDstTID(unsigned int);
	unsigned int getDstTID();
	void putMsgID(unsigned int);
	unsigned int getMsgID();
	void putMsgNum(unsigned short);
	unsigned short getMsgNum();
	void putStartOffset(unsigned short);
	unsigned short getStartOffset();
	void putEndOffset(unsigned short);
	unsigned short getEndOffset();
	bool isDelivered();
	void setDelivered(bool val);
	bool isReleased();
	void setReleased(bool val);
	void putPktList(vector<u_char*> *);
	vector<u_char *> *getPacketList();
	unsigned int getOwner();
	void setOwner(unsigned int);
	mfflag_t getOpts();
	bool isMultihoming();
	bool isMulticast();
	bool isBroadcast();
	bool isAnycast();
	bool isContentRequest();
	bool isContentResponse();
	void setOpts(mfflag_t);
};

#endif /* MF_CHUNKINFO_H_ */
