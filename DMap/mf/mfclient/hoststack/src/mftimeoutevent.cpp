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
 * @file   mftimeoutevent.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   May, 2014
 * @brief  Class that contains information about the actions a timeout should trigger.
 *
 * Class that contains information about the actions a timeout should trigger.
 */

#include <unistd.h>
#include <sys/time.h>

#include "mftimeoutevent.h"
#include "mfevent.h"
#include "mftimeoutlistener.h"

MFTimeoutEvent::MFTimeoutEvent(){
	cleared_ = false;
	message_ = NULL;
	timeout_value_ = 0;
}

MFTimeoutEvent::~MFTimeoutEvent(){
	
}

long int MFTimeoutEvent::timeout_value() const{
	return timeout_value_;
}

void MFTimeoutEvent::set_timeout_value(long int value){
	timeout_value_ = value;
}

bool MFTimeoutEvent::cleared() const{
	return cleared_;
}
void MFTimeoutEvent::set_cleared(bool cleared){
	cleared_ = cleared;
}
void *MFTimeoutEvent::message() const{
	return message_;
}
void MFTimeoutEvent::set_message(void *message){
	message_ = message;
}
MFTimeoutListenerInterface *MFTimeoutEvent::listener() const{
	return listener_;
}
void MFTimeoutEvent::set_listener(MFTimeoutListenerInterface *listener){
	listener_ = listener;
}

bool MFTimeoutEvent::Triggered() const{
	struct timeval actual_time;
	long int a_time;
	gettimeofday(&actual_time, NULL);
	a_time = actual_time.tv_sec * 1000 + actual_time.tv_usec/1000;
	a_time = timeout_value_ - a_time;
	return a_time <= 0;
}