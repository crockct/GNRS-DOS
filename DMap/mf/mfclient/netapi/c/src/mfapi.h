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
 * @file   mfapi.h
 * @author wontoniii (bronzino@winlab.rutgers.edu), czhang
 * @date   July, 2011
 * @brief  MobilityFirst's API.
 *
 * C/C++ MobilityFirst API. Used to communicate with MobilityFirst's network protocol stack
 */

/// @cond API_ONLY

#ifndef MFAPI_H_
#define MFAPI_H_
#include <semaphore.h>
#include <pthread.h>

#ifndef __linux
#include <sys/endian.h>
#include <sys/types.h>
#endif

#include <include/mfflags.h>

#ifdef __cplusplus
extern "C" {
#endif

struct WaitingTIDs{
	u_int tid;
	sem_t wsem;
	char elected;
	u_int msg;
	u_int GUID;
	u_int dstGUID;
	u_int opID;
	u_int size;
	struct WaitingTIDs *next;
};

struct Handle{
	sem_t hsem;
	u_int UID;
	int sock; //Control path socket. Uses datagram sockets for message based communications with the stack
	int datasock; //Data path socket. Uses stream sockets for data exchange between client and stack
	char waiting; //Variable that stores a flag on weather a leader for the waiting process has been elected or not
	u_int lastReq; //Last request number used
	struct WaitingTIDs *first; //List of waiting requests
	struct WaitingTIDs *garbage; //Garbage collector used for WaitingTIDs
};
	
struct GetInfo{
	u_int UID;
	int dstGuid;
	int srcGuid;
};

/// @endcond

//@Deprecated
int mfopenOptions(struct Handle *h, const char *profile, mfflag_t opts, const int src_GUID);
int mfopen(struct Handle *h, const char *profile, const char *opts, const int src_GUID);
int mfsend(struct Handle *h, void *buffer, u_int size, const int dst_GUID, mfflag_t opts);
int mfrecv(struct Handle *h, int *sGUID, void *buffer, u_int size, const int *src_GUID, const int n_GUID);
int mfrecv_blk(struct Handle *h, int *sGUID, void *buffer, u_int size, const int *src_GUID, const int n_GUID);
int mfattach(struct Handle *h, const int *GUIDs, const int nGUID);
int mfdetach(struct Handle *h, const int *GUIDs, const int nGUID);
//content
int mfget(struct Handle *h, const int dst_GUID, void *request, u_int requestSize, void *buffer, u_int bufferSize, mfflag_t opts);
int mfdo_get(struct Handle *h, struct GetInfo *info, void *buffer, u_int size);
int mfget_response(struct Handle *h, struct GetInfo *info, void *buffer, u_int size, mfflag_t opts);
int mfclose(struct Handle *h);

#ifdef __cplusplus
}
#endif
	
#endif /* MFAPI_H_*/
