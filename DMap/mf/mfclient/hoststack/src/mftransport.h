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
 * @file   mftransport.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   Septmber, 2013
 * @brief  Temp.
 *
 * Temp.
 */

#ifndef MF_TRANSPORT_H
#define MF_TRANSPORT_H

#include <list>
#include <map>

#include "mfsystem.h"
#include "mfbuffer.h"
#include "mfchunkinfo.h"
#include "mfeventqueue.h"
#include "mftimeoutmanager.h"


class MFSystem;
class MF_EventQueue;

class MF_Message {
	list<MF_ChunkInfo *> chkList;
	vector<u_char *> data;
	u_int mID;
	u_short mCount;
	u_short mTotal;
	u_int remaining;
	u_int srcGUID;
	u_int dstGUID;

	// added for reliab transp
	vector<char> bitmap;
	u_int start_seq;


	void sortList();
	void createData();

public:
	//MF_Message();
	MF_Message(u_short total = 0);
	~MF_Message();

	void addChunk(MF_ChunkInfo *);

	bool isComplete();

	u_int getData(vector<u_char *> *v, u_int size);

	inline  void setMID(u_int s){
		mID = s;
	}

	inline u_int getMID(){
		return mID;
	}

	inline void setCount(u_short s){
		mCount = s;
	}

	inline u_short getCount(){
		return mCount;
	}

	inline void setTotal(u_short s){
		mTotal = s;
	}

	inline u_short getTotal(){
		return mTotal;
	}

	inline u_int getRemaining(){
		return remaining;
	}

	inline void setStartSeq(u_int s) {
		start_seq = s;
	}
	inline u_int getStartSeq() {
		return start_seq;
	}

	inline void setBitmapBit(u_short start_offset) {
		MF_Log::mf_log(MF_DEBUG, "Message::setBitmapBit() called by ReliabTrans, set for start_offset: %u, cur bitmap char: %u",
				start_offset, bitmap[(start_offset) / 8]);
		bitmap[(start_offset) / 8] |= (0x01 << ((start_offset) % 8));
		MF_Log::mf_log(MF_DEBUG, "Message::setBitmapBit() called by ReliabTrans, set for start_offset: %u, cur bitmap char: %u",
				start_offset, bitmap[(start_offset) / 8]);
	}

	// caller is resiponsible for deallocating memory
	inline void copyBitmap(u_char *ptr) {
		std::copy(bitmap.begin(), bitmap.end(), ptr);
	}
};

/*
 * Abstract class that defines the basic interfaces that each transport should provide
 */
class MF_Transport {

protected:
	MF_Buffer *recvBuffer;
	MF_Buffer *sendBuffer;
	MFSystem *_system;
	u_int mGUID;

	virtual void fillHeader(MF_ChunkInfo *){}
public:
	MF_Transport(){}

	MF_Transport(MF_Buffer *recvBuffer, MF_Buffer *sendBuffer, MFSystem *_system){}

	virtual ~MF_Transport(){}

	virtual bool dataAvailable(){return false;}
	virtual u_int getData(vector<u_char *> *v, u_int size){return 0;}
	virtual int releaseChunk(MF_ChunkInfo *ci){return 0;}

	virtual void sendData(vector <u_char *> *data, u_int size, int srcGUI, int dstGUID, mfflag_t opts){}
	virtual void recvData(MF_ChunkInfo *) = 0;
	inline void setMyGUID(u_int g) { mGUID = g; }
	inline u_int getMyGUID() { return mGUID; }
};

/*
 * Basic transport class. No added reliability provided. Only chunking and re-aggregation of application level messages.
 */
class MF_BasicTransport : public MF_Transport {
	//u_int mTID;
	u_int oTID;
	u_int lastChunkID;
	map <u_int, MF_Message *> incoming;


	void fillHeader(MF_ChunkInfo *);

protected:
	list <MF_Message *> complete;
	u_int mTID;

public:
	MF_BasicTransport();
	MF_BasicTransport(u_int, MF_Buffer *recvBuffer, MF_Buffer *sendBuffer, MFSystem *_system);
	virtual ~MF_BasicTransport();
	virtual void sendData(vector <u_char *> *data, u_int size, int srcGUID, int dstGUID, mfflag_t opts);
	virtual void recvData(MF_ChunkInfo *);

	//temp
	virtual bool dataAvailable();
	virtual u_int getData(vector<u_char *> *v, u_int size);
	virtual int releaseChunk(MF_ChunkInfo *ci);
	static u_int computeMsgChkCnt(u_int size);
	static u_int computeChunkPktCnt(u_int size);
};

#endif /* MF_TRANSPORT_H */
