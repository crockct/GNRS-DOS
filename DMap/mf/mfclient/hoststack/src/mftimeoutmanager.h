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
 * @file   mftimeoutmanager.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   May, 2014
 * @brief  Class that handles a system wide timeout system.
 *
 * Class that handles a system wide timeout system.
 *	Glanurality for timeouts is nano seconds.
 */


#ifndef MF_TIMEOUTMANAGER_H_
#define MF_TIMEOUTMANAGER_H_

#include <semaphore.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <queue>
#include <set>

#include "mftimeoutevent.h"
#include "mftimeoutlistener.h"

typedef std::priority_queue<MFTimeoutEvent,std::vector<MFTimeoutEvent>,MFTimeoutLower> TimeoutPQ;


class MFTimeoutManager {

public:
	MFTimeoutManager();
	~MFTimeoutManager();
	int StartManager();
	//timeout is in
	unsigned int AddTimeout(MFTimeoutListenerInterface *listener, void *message, long int timeout);
	int ClearTimeout(unsigned int id);
	int StopManager();
	
	static void *ThreadStart(void *arg);
	
private:
	int MainLoop();
	int QueueLock();
	int QueueUnlock();
	int SleepUntilNextTimeout();
	int NotifyTimeoutThread();
	bool IsEventCleared(const MFTimeoutEvent &event);
	
	TimeoutPQ mqueue_;
	std::set<unsigned int> cleared_set_;
	pthread_mutex_t access_lock_; // QueueLock and QueueUnlock operate on this
	pthread_mutex_t access_time_; // QueueLock and QueueUnlock operate on this
	pthread_cond_t time_cond_;
	pthread_t thread_id_;
	u_int last_id_;
	bool stopped_;
	
	//Have a priority queue for event triggering and map for event clearing
	
};

#endif
