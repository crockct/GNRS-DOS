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
 * @file   mfaggregator.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   July, 2011
 * @brief  Temp.
 *
 * Temp
 */

#ifndef MF_AGGREGATOR_H_
#define MF_AGGREGATOR_H_

#include <map>

#include "mfsystem.h"
#include "mftypes.h"
#include "mfchunkinfo.h"
#include "mfbuffer.h"

using namespace std;

class MF_Aggregator {
	//TODO: remove this
	static const int COMPLETE_CLEAN = 20;
	static const int MAX_OLD_CHUNKS = 50;

	//Using a buffer to store data for the incoming chunks
	MF_Buffer mPool;
	//Outstanding chunks on the interface. The key is the hopID
	map <unsigned int, MF_ChunkInfo *> outsChunks;
	//Complete chunks on the interface. The key is the hopID. Periodically should be cleaned up
	map <unsigned int, MF_ChunkInfo *> receivedChunks;
	unsigned int lastHopID;
	MFSystem *_system;
	
	void tryRelease(MF_ChunkInfo *);
	
public:
	MF_Aggregator();
	MF_Aggregator(MFSystem *_system);
	MF_Aggregator(MFSystem *_system, int buffSize);
	~MF_Aggregator();
	void handleData(const unsigned char *);
	int handleCsyn(unsigned int, unsigned int);
	int handleCsynInit(const unsigned char *packet);
	void sendAck();
	void sendCtlMsg(unsigned char *, unsigned int);
	MF_ChunkInfo *getChunkInfo(unsigned int);
	void releaseChunk(unsigned int);
	void reset();
	void resetStatus();
};

#endif /* MF_AGGREGATOR_H_ */
