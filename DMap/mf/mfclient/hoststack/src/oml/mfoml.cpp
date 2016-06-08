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
 * @file   mfoml.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   July, 2013
 * @brief  Temp.
 *
 * Temp.
 */

//TODO: write debug messages

#include "mfoml.h"

#if defined(__ANDROID__) || !defined(__OML__)
//Defined only as dummy functions not to have compatibility problems
MF_OML::MF_OML(){
    
}

int MF_OML::init(const char* name, int* argc_ptr, const char** argv){
    return 0;
}

int MF_OML::start(){
    return 0;
}

int MF_OML::stop(){
    return 0;
}

void MF_OML::post(MF_OMLMessage *m){
    return;
}

void run(){
    return;
}

void MF_OML::inject_if_stats(uint32_t interface_ID, const char *ifname, const char *mac, uint32_t inpackets, uint32_t outpackets, uint32_t inbytes, uint32_t outbytes){
	return;
}

void MF_OML::inject_chk_stats(const void *defaultGUID, size_t defaultGUID_len, uint32_t inchunks, uint32_t outchunks){
	return;
}

#else

//Real definition used if OML is enabled

#define USE_OPTS /* Include command line parsing code*/
#include "mfstack_popt.h"

#define OML_FROM_MAIN /* Define storage for some global variables; #define this in only one file */
#include "mfstack_oml.h"

#include "config.h"


MF_OML::MF_OML(){
    cont = 1;
}

int MF_OML::oml_init(const char* name, int* argc, const char** argv){
	return omlc_init(name, argc, argv, NULL);
}

void MF_OML::poptGetContxt(int argc, const char **argv){
	optCon = poptGetContext(NULL,argc, argv, options, 0);
}

int MF_OML::poptGetNxtOpt(){
	return poptGetNextOpt(optCon);
}

void MF_OML::register_mps(){
	oml_register_mps();
}


void MF_OML::inject_if_stats(uint32_t interface_ID, const char *ifname, const char *mac, uint32_t inpackets, uint32_t outpackets, uint32_t inbytes, uint32_t outbytes){
	oml_inject_interface_statistics(g_oml_mps_mfstack->interface_statistics, interface_ID, ifname, mac, inpackets, outpackets, inbytes, outbytes);
}

void MF_OML::inject_chk_stats(const void *defaultGUID, size_t defaultGUID_len, uint32_t inchunks, uint32_t outchunks){
	oml_inject_chunks_statistics(g_oml_mps_mfstack->chunks_statistics, defaultGUID, defaultGUID_len, inchunks, outchunks);
}

int MF_OML::init(const char* name, int* argc_ptr, const char** argv){
	int ret, c;
    if((ret = oml_init("mfstack", argc_ptr, argv)) < 0) {
		logerror("Could not initialize OML\n");
		return -1;
	}
    /* Parse command line arguments */
	poptGetContxt(*argc_ptr, argv); /* options is defined in mfstack_popt.h */
	while ((c = poptGetNxtOpt()) > 0) {printf("Option??? %d", c);}
    register_mps(); /* Defined in mfstack_oml.h */
    //last operations on local variables
    if(ret == 0){
		sem_init(&qLock, 0, 1);
		sem_init(&count, 0, 0);
		if(start()) {
			logerror("Could not start OML\n");
			return -1;
		}
	}
	return ret;
}

int MF_OML::start(){
	omlc_start();
    int error = pthread_create(&tid, NULL, &run, this);
	return error;
}

int MF_OML::stop(){
	return omlc_close();
}

void MF_OML::post(MF_OMLMessage *m){
    sem_wait(&qLock);
    q.push(m);
    sem_post(&qLock);
    sem_post(&count);
}

void MF_OML::handleMessage(MF_OMLMessage *m){
	MF_OMLInterfaceStatistics *ifs;
	MF_OMLChunkStatistics *chs;
    switch (m->getType()) {
        case BASIC:
            //Nothing to do with it
            break;
        case IF_STATS:
			ifs = (MF_OMLInterfaceStatistics *)m;
			inject_if_stats(ifs->getDID(), ifs->getDName(), ifs->getMac(), ifs->getInPcks(), ifs->getOutPcks(), ifs->getInBytes(), ifs->getOutBytes());
            break;
        case CHK_STATS:
			chs = (MF_OMLChunkStatistics *)m;
            	inject_chk_stats(chs->getGUID(), chs->getGUIDLen(), chs->getInChunks(), chs->getOutChunks());
            break;
        default:
            break;
    }
}

void MF_OML::mainLoop() {
    MF_OMLMessage *temp;
	while(cont){
        sem_wait(&count);
        sem_wait(&qLock);
        temp = q.front();
        q.pop();
        handleMessage(temp);
        sem_post(&qLock);
    }
}

void *run(void *arg) {
    MF_OML *mfoml = (MF_OML *)arg;
    mfoml->mainLoop();
	return NULL;
}

#endif
