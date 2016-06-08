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
 * @file   mfoml.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   July, 2013
 * @brief  Temp.
 *
 * Temp.
 */


#ifndef MFOML_H_
#define MFOML_H_

#if !defined(__ANDROID__) && defined(__OML__)
#include <popt.h>
#include <oml2/omlc.h>
#endif

#include <queue>
#include <semaphore.h>
#include <pthread.h>
#include <stdint.h>

#include "mfomlmessage.h"

using namespace std;

class MF_OML{
#if !defined(__ANDROID__) && defined(__OML__)
	poptContext optCon;
	queue<MF_OMLMessage *> q;
	sem_t count;
	sem_t qLock;
	pthread_t tid;
	int cont;
	int oml_init(const char* name, int* argc_ptr, const char** argv);
	void poptGetContxt(int , const char **);
	int poptGetNxtOpt();
	void register_mps();
    
protected:
	void mainLoop();
#endif

public:
	MF_OML();
	int init(const char* name, int* argc_ptr, const char** argv);
	int start();
	int stop();
	void post(MF_OMLMessage *);
	void handleMessage(MF_OMLMessage *m);
	void inject_if_stats(uint32_t interface_ID, const char *ifname, const char *mac, uint32_t inpackets, uint32_t outpackets, uint32_t inbytes, uint32_t outbytes);
	void inject_chk_stats(const void *defaultGUID, size_t defaultGUID_len, uint32_t inchunks, uint32_t outchunks);
	friend void *run(void *arg);
};

void *run(void *arg);

#endif
