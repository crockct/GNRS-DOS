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
 * @file   mfeventqueue.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   August, 2013
 * @brief  Class that handles the event queue.
 *
 * Class that handles the event queue.
 */

#include <queue>
#include <semaphore.h>
#include <errno.h>

#include <include/mfclientipc.h>

#include "mfstackevent.h"
#include "mftypes.h"
#include "mfbuffer.h"
#include "mflog.h"
#include "mfserver.h"
#include "mfeventqueue.h"

MF_EventQueue::MF_EventQueue(){
	_system = NULL;
	sem_init(&queueAccess, 0, 1);
	sem_init(&queueCounter, 0, 0);
	cont = true;
}

MF_EventQueue::MF_EventQueue(MFSystem *system){
	_system = system;
	sem_init(&queueAccess, 0, 1);
	sem_init(&queueCounter, 0, 0);
	cont = true;
}

MF_EventQueue::~MF_EventQueue(){
	
}

void MF_EventQueue::mainLoop(){
	u_int bufSize = 65536;
	char buffer[bufSize];
	int ret;
	while(cont){
		if(_system->getSocketServer() == NULL){
			MF_Log::mf_log(MF_CRITICAL, "MF_EventQueue::mainLoop(): there's no server");
			return;
		}
		ret = _system->getSocketServer()->receive(buffer, bufSize);
		if (ret < 0) {
			MF_Log::mf_log(MF_CRITICAL, "MF_EventQueue::mainLoop(): recv() error:%s\n", strerror(errno));
			return;
		}
		struct Msg* m = (struct Msg*)buffer;
		MF_StackEvent *se = NULL;
		if(m->type == MSG_TYPE_SEND){
			se = new MF_APISend((MsgSend*)m);
		}
		else if(m->type == MSG_TYPE_RECV){
			se = new MF_APIRecv((MsgRecv*)m);
		}
		else if(m->type == MSG_TYPE_GET_SEND){
			se = new MF_APIGet((MsgGet*)m);
		}
		else if(m->type == MSG_TYPE_DO_GET){
			se = new MF_APIDoGet((MsgDoGet*)m);
		}
		else if(m->type == MSG_TYPE_GET_RESPONSE){
			se = new MF_APIGetResponse((MsgGetResponse*)m);
		}
		else if(m->type == MSG_TYPE_OPEN){
			se = new MF_APIOpen((MsgOpen*)m);
		}
		else if(m->type == MSG_TYPE_ATTACH){
			se = new MF_APIAttach((MsgAttach*)m);
		}
		else if(m->type == MSG_TYPE_DETACH){
			se = new MF_APIDetach((MsgDetach*)m);
		}
		else if(m->type == MSG_TYPE_CLOSE){
			se = new MF_APIClose((MsgClose*)m);
		}
		else{
			MF_Log::mf_log(MF_ERROR, "Unrecognized msg from API of type: %d. Buffer size %d", m->type, ret);
		}
		if(se!=NULL) add(se);
		se = NULL;
	}
}

int MF_EventQueue::startListener(){
	int error = pthread_create(&tid, NULL, event_queue_thread, this);
	if (error) {
		MF_Log::mf_log(MF_ERROR, "MF_EventQueue: ERROR starting listener: %d\n",error);
		return error;
	}
	return 0;
}

MF_StackEvent *MF_EventQueue::getNext(){
	MF_StackEvent *ret = NULL;
	sem_wait(&queueCounter);
	sem_wait(&queueAccess);
	if(!high_eq.empty()){
		MF_Log::mf_log(MF_DEBUG, "MF_EventQueue:getNext new high priority event available");
		ret = high_eq.front();
		high_eq.pop();
	}
	else{
		MF_Log::mf_log(MF_DEBUG, "MF_EventQueue:getNext new low priority event available");
		ret = low_eq.front();
		low_eq.pop();
	}
	sem_post(&queueAccess);
	return ret;
}

void MF_EventQueue::add(MF_StackEvent *se){
	sem_wait(&queueAccess);
	low_eq.push(se);
	sem_post(&queueCounter);
	sem_post(&queueAccess);
}

void MF_EventQueue::add(priority_level_t p, MF_StackEvent *se){
	sem_wait(&queueAccess);
	if(p==MF_LOW_PRIORITY)low_eq.push(se);
	else if(p==MF_HIGH_PRIORITY) high_eq.push(se);
	else low_eq.push(se);
	sem_post(&queueCounter);
	sem_post(&queueAccess);
}

void *event_queue_thread(void *eq_ptr){
	MF_EventQueue *eq = (MF_EventQueue*)eq_ptr;
	eq->mainLoop();
	return NULL;
}
