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
 * @file   mfomlmessage.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   July, 2013
 * @brief  Temp.
 *
 * Temp
 */

#ifndef MF_OML_Message_H_
#define MF_OML_Message_H_

#include <iostream>
#include <stdint.h>

#if !defined(__ANDROID__) && defined(__OML__)
enum oml_message_type {
    BASIC,
    IF_STATS,
    CHK_STATS
	
};
#endif

class MF_OMLMessage {
	
#if !defined(__ANDROID__) && defined(__OML__)
    enum oml_message_type t;
#endif

public:
	
#if !defined(__ANDROID__) && defined(__OML__)
	MF_OMLMessage();
	MF_OMLMessage(enum oml_message_type type);
	enum oml_message_type getType(){
		return t;
	}
#endif
};


#if !defined(__ANDROID__) && defined(__OML__)
/*  MF_OMLInterfaceStatistics  */
class MF_OMLInterfaceStatistics : public MF_OMLMessage {
	uint32_t dID;
	const char * dName;
	const char * mac;
	uint32_t inPcks;
	uint32_t outPcks;
	uint32_t inBytes;
	uint32_t outBytes;
	
public:
	MF_OMLInterfaceStatistics(uint32_t dID, const char * dName, const char * mac, uint32_t inPcks, uint32_t outPcks, uint32_t inBytes, uint32_t outBytes);
	uint32_t getDID();
	const char *getDName();
	const char *getMac();
	uint32_t getInPcks();
	uint32_t getOutPcks();
	uint32_t getInBytes();
	uint32_t getOutBytes();
};


/*  MF_OMLChunkStatistics  */
class MF_OMLChunkStatistics : public MF_OMLMessage {
	//TODO: These two will need to be changed into a GUID object
	void *GUID;
	size_t GUIDLen;
	uint32_t inChunks;
	uint32_t outChunks;
	
public:
	MF_OMLChunkStatistics(void *GUID, size_t GUIDLen, uint32_t inChunks, uint32_t outChunks);
    void *getGUID();
	size_t getGUIDLen();
	uint32_t getInChunks();
	uint32_t getOutChunks();
};

#endif

#endif /* defined(__excluded_svn_folder__MF_OML_Message__) */
