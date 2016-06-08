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
 * @file   mftimeoutmanager.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   May, 2014
 * @brief  Class that handles a system wide timeout system.
 *
 * Class that handles a system wide timeout system.
 */

#include <semaphore.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <queue>
#include <set>


#include "mftimeoutmanager.h"
#include "mftimeoutevent.h"

MFTimeoutManager::MFTimeoutManager(){
	stopped_ = true;
	last_id_ = 0;
}

MFTimeoutManager::~MFTimeoutManager(){
	pthread_mutex_destroy(&access_lock_);
	pthread_cond_destroy(&time_cond_);
}

void *MFTimeoutManager::ThreadStart(void *arg){
	MFTimeoutManager *context = (MFTimeoutManager *)arg;
	context->MainLoop();
	return NULL;
}

int MFTimeoutManager::StartManager(){
	int error = 0;
	if((error = pthread_mutex_init(&access_lock_, NULL)) != 0 ){
		return error;
	}
	if((error = pthread_mutex_init(&access_time_, NULL)) != 0 ){
		return error;
	}
	if((error = pthread_cond_init(&time_cond_ , NULL)) != 0){
		return error;
	}
	stopped_ = false;
	if((error = pthread_create(&thread_id_, NULL, MFTimeoutManager::ThreadStart, this) != 0 )) {
		return error;
	}
	return 0;
}

unsigned int MFTimeoutManager::AddTimeout(MFTimeoutListenerInterface *listener, void *message, long int timeout){
	struct timeval actual_time;
	MFTimeoutEvent new_event;
	new_event.set_listener(listener);
	new_event.set_message(message);
	gettimeofday(&actual_time, NULL);
	new_event.set_timeout_value(actual_time.tv_sec * 1000 + actual_time.tv_usec/1000 + timeout);
	QueueLock();
	new_event.set_unique_id(last_id_);
	last_id_++;
	mqueue_.push(new_event);
	NotifyTimeoutThread();
	QueueUnlock();
	return new_event.unique_id();
}

int MFTimeoutManager::ClearTimeout(unsigned int id){
	QueueLock();
	cleared_set_.insert(id);
	NotifyTimeoutThread();
	QueueUnlock();
	return 0;
}

int MFTimeoutManager::StopManager(){
	QueueLock();
	stopped_ = true;
	NotifyTimeoutThread();
	QueueUnlock();
	return pthread_join(thread_id_, NULL);
}

int MFTimeoutManager::QueueLock(){
	return pthread_mutex_lock(&access_lock_);
}

int MFTimeoutManager::QueueUnlock(){
	return pthread_mutex_unlock(&access_lock_);
}

int MFTimeoutManager::NotifyTimeoutThread(){
	return pthread_cond_signal(&time_cond_);
}

//Run on mutex locked
int MFTimeoutManager::SleepUntilNextTimeout(){
	//QueueLock();
	bool empty = mqueue_.empty();
	if(empty) {
		QueueUnlock();
		pthread_mutex_lock(&access_time_);
		pthread_cond_wait(&time_cond_, &access_time_);
		pthread_mutex_unlock(&access_time_);
    }
    else {
		MFTimeoutEvent event = mqueue_.top();
		QueueUnlock();
		timespec t;
		t.tv_sec = event.timeout_value()/1000;
		t.tv_nsec = event.timeout_value()*1000000;
		pthread_mutex_lock(&access_time_);
		pthread_cond_timedwait(&time_cond_, &access_time_, &t);
		pthread_mutex_unlock(&access_time_);
	}
	return 0;
}

bool MFTimeoutManager::IsEventCleared(const MFTimeoutEvent &event){
	return cleared_set_.find(event.unique_id()) != cleared_set_.end();
}

int MFTimeoutManager::MainLoop(){
	MFTimeoutEvent event;
	QueueLock();
	while(!stopped_){
		//QueueUnlock();
		SleepUntilNextTimeout();
		while(!mqueue_.empty() && mqueue_.top().Triggered() && !stopped_){
			event = mqueue_.top();
			mqueue_.pop();
			if(!IsEventCleared(event)) {
				QueueUnlock();
				//TODO move to handle in main thread
				event.listener()->OnTimeout(event.message(), event.unique_id());
				QueueLock();
			}
		}
	}
	QueueUnlock();
	return 0;
}
