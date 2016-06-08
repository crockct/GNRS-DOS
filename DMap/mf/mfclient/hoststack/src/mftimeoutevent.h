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
 * @file   mftimeoutevent.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   May, 2014
 * @brief  Class that contains information about the actions a timeout should trigger.
 *
 * Class that contains information about the actions a timeout should trigger.
 */


#ifndef MFTimeoutEvent_H_
#define MFTimeoutEvent_H_

#include <unistd.h>

#include "mfevent.h"
#include "mftimeoutlistener.h"

class MFTimeoutEvent : public MFEvent {
	
public:
	MFTimeoutEvent();
	~MFTimeoutEvent();
	
	long int timeout_value() const;
	void set_timeout_value(long int value);
	bool cleared() const;
	void set_cleared(bool cleared);
	void *message() const;
	void set_message(void *message);
	MFTimeoutListenerInterface *listener() const;
	void set_listener(MFTimeoutListenerInterface *listener);
	bool Triggered() const;
	
private:
	
	virtual void NullFunction(){};
	
	long int timeout_value_;
	bool cleared_;
	void *message_;
	MFTimeoutListenerInterface *listener_;
	
};


/**
 * Class that implements the lower comparison for timeouts
 * Check the fact that is going to be a priority list on pointers
 */
class MFTimeoutLower{
	
public:
	MFTimeoutLower(){}
	bool operator() (const MFTimeoutEvent& to_event_a, const MFTimeoutEvent&to_event_b) const {
		return to_event_a.timeout_value() > to_event_b.timeout_value();
	}
};

#endif