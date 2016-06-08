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
 * @file   mfsegmentor.h
 * @author wontoniii (bronzino@winlab.rutgers.edu), czhang
 * @date   July, 2011
 * @brief  Class that handles the transmission of data chunks, csyn and assoc/dassoc packets.
 *
 * Class that handles the transmission of data chunks, csyn and assoc/dassoc packets.
 */

#ifndef MF_SEGMENTOR_H_
#define MF_SEGMENTOR_H_

#include <pcap.h>
#include <vector>
#include <list>
#include <map>
#include <climits>

#include "mftypes.h"
#include "mfdeviceinfo.h"
#include "mfchunkinfo.h"
#include "mfeventqueue.h"
#include "mftimeoutmanager.h"
#include "mfsystem.h"

class MF_DeviceInfo;

class MFSendingChunk{
public:
	MF_ChunkInfo *chunkInfo;
	bool isCsynSent;
	bool isShortChunk;
	bool isGranted;
	unsigned int csynSeqNum;
	unsigned int TOID;
	unsigned int transmitted;
	std::queue<unsigned int> nonAckedPackets;
};

class MF_Segmentor : MFTimeoutListenerInterface{

	const static u_int MAX_HOP_ID = ULONG_MAX;
	const static u_int MAX_CSYN_SEQ_NUM = 65535;
	//TODO move this into the configuration file
	static const unsigned int MAX_OSTD = 2000;
	static const unsigned int MAX_EXTRA_SYNC = 0;
	static const unsigned int SMALL_PKT_SIZE = 32;
	int timeTimout;

	//New variables used in transmission
	unsigned int w_ostd;
	unsigned int w_sync;
	//Contains queued chunks that have not been started to be sent yet
	std::list<MF_ChunkInfo *> chunksQueue;
	//Contains chunks that are already on flight
	std::map<unsigned int, MFSendingChunk> outstandingChunks;
	//If a chunk was only partially sent is stored here for sending data asap
	unsigned int pendingChunk;
	bool isPending;
	//increasing counter for next hop id to use
	unsigned int mLastHopID;

	//General class elements
	int interfaceID;
	MFSystem *_system;
	pcap_t *mPcapHandle;
	
	u_int getNewHopID();
	bool isAllSent(MFSendingChunk &sendingChunk, u_char *bitmap);
	unsigned int countLost(MFSendingChunk &sendingChunk, u_char *bitmap);
	void fillHeader(MF_ChunkInfo *, MF_DeviceInfo *);
	void fillHopHeader(u_char *, u_int, u_int, u_int);
	unsigned int sendData(MFSendingChunk &sendingChunk, MF_DeviceInfo *di);
	void startSendingChunk(MF_ChunkInfo *ci, MF_DeviceInfo *di);

	static bool isEmptyBitmap(unsigned char *bitmap, unsigned int pktCount);

public:
	MF_Segmentor();
	MF_Segmentor(MFSystem *_system, pcap_t *handle, int devId, int t);
	~MF_Segmentor();
	void setPCAPHandle(pcap_t *);
	void sendCsyn(MF_ChunkInfo *, MF_DeviceInfo *);
	void csynTO(u_int chunkID, MF_DeviceInfo *);
	void procSend(MF_ChunkInfo *, MF_DeviceInfo *);
	void sendAssoRequest(u_int, u_int, u_short, MF_DeviceInfo *);
	void sendDassoRequest(u_int, u_int, MF_DeviceInfo *);
	void handleAck(u_int hop_ID, u_int pkt_count, u_char *bitmap, MF_DeviceInfo *);
	bool sendAck(MF_ChunkInfo *ci, MF_DeviceInfo *di);
	void sendLPAck(u_int, u_int, u_int, MF_DeviceInfo *);
	void reset();

	virtual void OnTimeout(void *message, unsigned int id);
	void resetStatus();
	void interfaceEnabled(MF_DeviceInfo *di);
};

#endif /* MF_SEGMENTOR_H_ */
