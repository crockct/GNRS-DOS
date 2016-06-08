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
 * @file   mfdevicemanager.h
 * @author wontoniii (bronzino@winlab.rutgers.edu), czhang
 * @date   August, 2011
 * @brief  Temp.
 *
 * Temp.
 */

#ifndef MF_DEVICEMANAGER_H_
#define MF_DEVICEMANAGER_H_

#include <stdio.h>
#include <pcap.h>
#include <string>

#include "mfsystem.h"
#include "mftypes.h"
#include "mfdeviceinfo.h"
#include "mfchunkinfo.h"
#include "mfetherproto.h"
#include "mfipproto.h"
#include "mfbuffer.h"
#include "mfaggregator.h"
#include "mfsegmentor.h"
#include "mfstackevent.h"


#define IF_INAC_TO 30

class MF_Aggregator;
class MF_Segmentor;
class MF_DeviceInfo;

class MF_DeviceManager {
	MF_DeviceInfo *mDevInfo;
	MF_Aggregator *mAggregator;
	MF_Segmentor *mSegmentor;
	MFSystem *_system;
	pthread_t tid;
	pcap_t *mPcapHandle;
	//list<MF_ChunkInfo *> outgChunks;
	
	//Functions handling received packets
	void dataHandler(const u_char *);
	void csynHandler(const u_char *);
	void ackHandler(const u_char *, u_int);
	void lpHandler(const u_char *);
	
protected:
	void gotPacket(const struct pcap_pkthdr *,const u_char *);
	pcap_t *getPcapHandle();
	void pcapError();

public:
	MF_DeviceManager();
	MF_DeviceManager(MFSystem *_system, int type, string devName, int ifID, int t);
	MF_DeviceManager(MFSystem *_system, int type, string devName, int ifID, int bufSize, int t);
	~MF_DeviceManager();
	//void start(MF_DeviceInfo *);
	void start();
	friend void *dmProc(void *);
	friend void handlePacket(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
	
	bool sendAck(MF_ChunkInfo *);
	void deliverChunk(MF_ChunkInfo *ci);
	void sendChunk(MF_ChunkInfo *c);
	
	void updateIfInfo(string mac, string ip);
	bool isActive();
	bool isEnabled();
	bool isPcapRunning();
	void disable();
	void enable();
	void activate();
	void deactivate();
	int status();
	
	bool updateProb(string ether, string ip, time_t t);
	void updateManually(string ether, string ip);
	void sendLinkProb(u_int sourceLPACK, u_int destinationLPACK, u_int seq_no_cp);
	
	void sendAssocReq(u_int entityGUID, u_int hostGUID, u_short weight);
	
	//Others
	MF_ChunkInfo *getChunkInfo(u_int);
	void releaseChunk(u_int chkID);
	void handleCsyn(u_int hopID, u_int pktCount);
	void handleCsynAck(u_int hop_ID, u_int pkt_count, u_char *bitmap);
	void handleCsynTimeOut(u_int chunk);
	bool isManual();
	void setManual();
	
	int getID();
};

void *dmProc(void *);
void handlePacket(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

#endif /* MF_DEVICEMANAGER_H_ */
