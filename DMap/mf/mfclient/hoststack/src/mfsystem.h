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
 * @file   mfsystem.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   May, 2014
 * @brief  Class that contains the management classes of the system.
 *
 * Class that contains the management classes of the system.
 */


#ifndef MF_SYSTEM_H
#define MF_SYSTEM_H


#include "mftimeoutmanager.h"
#include "mfserver.h"
#include "mfeventqueue.h"
#include "mfinterfacemanager.h"
#include "mfrouter.h"
#include "oml/mfoml.h"
#include "mfsettings.h"

class MF_InterfaceManager;
class MF_SocketTable;

class MFSystem {

public:
	MFSystem();
	~MFSystem();

	inline  MF_EventQueue* getEventQueue()  {
		return _eventQueue;
	}

	void setEventQueue(MF_EventQueue* eventQueue) {
		_eventQueue = eventQueue;
	}

	inline  MF_InterfaceManager* getInterfaceManager()  {
		return _interfaceManager;
	}

	void setInterfaceManager(MF_InterfaceManager* interfaceManager) {
		_interfaceManager = interfaceManager;
	}

	inline  MF_Router* getRouter()  {
		return _router;
	}

	void setRouter(MF_Router* router) {
		_router = router;
	}

	inline  MF_Server* getSocketServer()  {
		return _socketServer;
	}

	void setSocketServer(MF_Server* socketServer) {
		_socketServer = socketServer;
	}

	inline  MF_SocketTable* getSocketTable()  {
		return _socketTable;
	}

	void setSocketTable(MF_SocketTable* socketTable) {
		_socketTable = socketTable;
	}

	inline  MFTimeoutManager* getTimeoutManager()  {
		return _timeoutManager;
	}

	void setTimeoutManager(MFTimeoutManager* timeoutManager) {
		_timeoutManager = timeoutManager;
	}

	inline bool isInizialized()  {
		return initialized;
	}

	void setInizialized(bool inizialized) {
		this->initialized = inizialized;
	}

	inline  MF_Settings* getSettings()  {
		return _settings;
	}

	void setSettings(MF_Settings* settings) {
		_settings = settings;
	}

	inline MF_OML* getOmlMps()  {
		return _oml_mps;
	}

	void setOmlMps(MF_OML* omlMps) {
		_oml_mps = omlMps;
	}

private:

	bool initialized;
	MF_EventQueue *_eventQueue;
	MFTimeoutManager *_timeoutManager;
	MF_SocketTable *_socketTable;
	MF_InterfaceManager *_interfaceManager;
	MF_Server *_socketServer;
	MF_Router *_router;
	MF_OML *_oml_mps;
	MF_Settings *_settings;


};

#endif
