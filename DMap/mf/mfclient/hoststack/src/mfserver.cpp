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
 * @file   mfserver.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   September, 2013
 * @brief  Class that handles message communications between the stack and the API layer.
 *
 * Class that handles message communications between from the API layer to the hoststack and vice versa.
 *   A datagram UNIX socket is used to receive messages.
 */


#include <unistd.h>

#include "mfserver.h"
#include "mftypes.h"
#include "mflog.h"

#include <include/mfclientipc.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#ifdef __ANDROID__
#include <stdlib.h>
#endif

using namespace std;

MF_Server::MF_Server(){
	mSock = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (mSock < 0) {
		MF_Log::mf_log(MF_ERROR, "MF_Server::MF_Server(): socket() error:%s", strerror(errno));
	}
	struct sockaddr_un addr;
	addr.sun_family = AF_LOCAL;
	strncpy(addr.sun_path, SERVER_PATH, sizeof(addr.sun_path));
	if (unlink(SERVER_PATH) < 0){
		MF_Log::mf_log(MF_ERROR, "MF_Server::MF_Server(): unlink() error:%s", strerror(errno));
	}
	if (bind(mSock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		MF_Log::mf_log(MF_ERROR, "MF_Server::MF_Server(): bind() error:%s", strerror(errno));
		close(mSock);
		return;
	}
#ifdef __ANDROID__
	//Required for writing from API library
	char command[128];
	strcpy(command, "chmod 777 ");
	strcat(command, addr.sun_path);
	system(command);
#endif
}

MF_Server::~MF_Server(){
	//TODO: close server
}
	
/*
 * Send message to given cliAddr socket address
 */
int MF_Server::send(char *buffer, int size, struct sockaddr_un *cliAddr){
	MF_Log::mf_log(MF_DEBUG, "MF_Server:send send message to api layer");
	return sendto(mSock, buffer, size, 0, (struct sockaddr*) cliAddr,sizeof(struct sockaddr_un));
}

/*
 * Send message to socket address created from a unique identifier
 */
int MF_Server::send(char *buffer, int size, u_int uid){
	MF_Log::mf_log(MF_DEBUG, "MF_Server:send send message to api layer");
	struct sockaddr_un cliAddr;
	cliAddr.sun_family = AF_LOCAL;
	sprintf(cliAddr.sun_path, "%s%010x", CLIENT_PATH, uid);
	return sendto(mSock, buffer, size, 0, (struct sockaddr*) &cliAddr,sizeof(cliAddr));
}

int MF_Server::receive(char *buffer, int size){
	MF_Log::mf_log(MF_DEBUG, "MF_Server:receive receive message from api layer");
	return recv(mSock, (void *) buffer, size, 0);
}
