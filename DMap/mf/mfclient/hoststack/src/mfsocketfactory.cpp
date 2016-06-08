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
 * @file   mfsocketfactory.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   March, 2015
 * @brief  Class that handles instantiation of socket managers.
 *
 * Class that handles instantiation of socket managers. Starting from a profile it generates the proper socketmanager
 */


#include "mflog.h"
#include "mfsocketmanager.h"
#include "mfbasesocketmanager.h"
#include "mfcontentsocketmanager.h"
#include "mfsocketfactory.h"

MF_SocketManager *MF_SocketFactory::getInstance(MFSystem *_system, int type, u_int UID){
	MF_Log::mf_log(MF_ERROR, "MF_SocketFactory::getInstance operation not supported");
	return NULL;
}

MF_SocketManager *MF_SocketFactory::getInstance(MFSystem *_system, int type, u_int UID, int nChunks){
	MF_Log::mf_log(MF_ERROR, "MF_SocketFactory::getInstance operation not supported");
	return NULL;
}

MF_SocketManager *MF_SocketFactory::getInstance(MFSystem *_system, const char *profile, u_int UID){
	if(!strcmp("basic", profile)){
		return new MF_BaseSocketManager(_system, UID);
	} else if (!strcmp("content", profile)) {
		return new MF_ContentSocketManager(_system, UID);
	} else{
		MF_Log::mf_log(MF_ERROR, "MF_SocketFactory::getInstance wrong profile request %s", profile);
		return NULL;
	}
}

MF_SocketManager *MF_SocketFactory::getInstance(MFSystem *_system, const char *profile,
		u_int UID, int nChunks, TransportSetting ts){
	if(!strcmp("basic", profile)){
		return new MF_BaseSocketManager(_system, UID, nChunks, ts);
	} else if (!strcmp("content", profile)) {
		return new MF_ContentSocketManager(_system, UID, nChunks, ts);
	} else{
		MF_Log::mf_log(MF_ERROR, "MF_SocketFactory::getInstance wrong profile request %s", profile);
		return NULL;
	}

}
