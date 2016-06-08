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
 * @file   mfomlmessage.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   July, 2013
 * @brief  Temp.
 *
 * Temp
 */

//TODO: write debug messages

#include "mfomlmessage.h"

#ifndef __ANDROID__
#ifdef __OML__

MF_OMLMessage::MF_OMLMessage(){
    //Nothing to do
}

MF_OMLMessage::MF_OMLMessage(enum oml_message_type type){
	t = type;
}


/*  OMLReceivedPackets  */
MF_OMLInterfaceStatistics::MF_OMLInterfaceStatistics(uint32_t dID, const char * dName, const char * mac, uint32_t inPcks, uint32_t outPcks, uint32_t inBytes, uint32_t outBytes) : MF_OMLMessage(IF_STATS) {
	this->dID = dID;
	this->dName = dName;
	this->mac = mac;
	this->inPcks = inPcks;
	this->outPcks = outPcks;
	this->inBytes = inBytes;
	this->outBytes = outBytes;
}

uint32_t MF_OMLInterfaceStatistics::getDID(){
	return dID;
}

const char *MF_OMLInterfaceStatistics::getDName(){
    return dName;
}

const char *MF_OMLInterfaceStatistics::getMac(){
	return mac;
}

uint32_t MF_OMLInterfaceStatistics::getInPcks(){
	return inPcks;
}

uint32_t MF_OMLInterfaceStatistics::getOutPcks(){
	return outPcks;
}

uint32_t MF_OMLInterfaceStatistics::getInBytes(){
	return inBytes;
}

uint32_t MF_OMLInterfaceStatistics::getOutBytes(){
	return outBytes;
}



/*  MF_OMLChunkStatistics  */
MF_OMLChunkStatistics::MF_OMLChunkStatistics(void *GUID, size_t GUIDLen, uint32_t inChunks, uint32_t outChunks) : MF_OMLMessage(CHK_STATS) {
	this->GUID = GUID;
	this->GUIDLen = GUIDLen;
	this->inChunks = inChunks;
	this->outChunks = outChunks;
}

void *MF_OMLChunkStatistics::getGUID(){
	return GUID;
}

size_t MF_OMLChunkStatistics::getGUIDLen(){
	return GUIDLen;
}

uint32_t MF_OMLChunkStatistics::getInChunks(){
	return inChunks;
}

uint32_t MF_OMLChunkStatistics::getOutChunks(){
	return outChunks;
}


#endif
#endif
