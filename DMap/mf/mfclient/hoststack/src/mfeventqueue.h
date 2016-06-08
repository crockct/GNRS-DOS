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
 * @file   mfeventqueue.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   August, 2013
 * @brief  Class that handles the event queue.
 *
 * Class that handles the event queue.
 */

#ifndef MF_EVENT_QUEUE
#define MF_EVENT_QUEUE

#include <queue>
#include <semaphore.h>

#include "mfsystem.h"
#include "mfstackevent.h"

using namespace std;

enum priority_level_t {
	MF_HIGH_PRIORITY,
	MF_LOW_PRIORITY
};

class MF_StackEvent;
class MFSystem;

class MF_EventQueue{
	bool cont;
	MFSystem *_system;
	pthread_t tid;
	sem_t queueAccess;
	sem_t queueCounter;
	queue<MF_StackEvent*> low_eq;
	queue<MF_StackEvent*> high_eq;
	
protected:
	void mainLoop();
	
public:
	MF_EventQueue();
	MF_EventQueue(MFSystem *);
	~MF_EventQueue();
	
	int startListener();
	
	MF_StackEvent *getNext();
	void add(MF_StackEvent *se);
	void add(priority_level_t p, MF_StackEvent *se);
	friend void *event_queue_thread(void *eq_ptr);
};

void *event_queue_thread(void *eq_ptr);

#endif /* MF_EVENT_QUEUE */
