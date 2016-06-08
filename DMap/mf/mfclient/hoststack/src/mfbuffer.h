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
 * @file   mfbuffer.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   July, 2011
 * @brief  Buffer manager.
 *
 * Buffer manager. One assigned to each socket.
 */

#ifndef MF_BUFFER_H_
#define MF_BUFFER_H_

#include <queue>
#include <vector>
#include <semaphore.h>

#include "mftypes.h"
#include "mfpacketsupport.h"



using namespace std;

class MF_Buffer {
	queue<u_char*> mBufferPool;
	sem_t buf_acc;
	unsigned int totalSize;

public:
	const static u_int SLOT_SIZE = MF_PacketSupport::MAX_PAYLOAD_SIZE + MF_PacketSupport::LOW_HEADER_LEN;
	
	MF_Buffer(u_int);
	~MF_Buffer();
	void populate(u_int);
	void dePopulate();
	void putVector(vector<u_char*> *);
	unsigned int getSize();
	unsigned int getTotalSizePackets();
	unsigned int getTotalSizeBytes();
	int getVectorBySize(vector<u_char *> *v, u_int size);
	int getVectorBySlots(vector<u_char *> *v, u_int count);
};

#endif /* MF_BUFFER_H_ */
