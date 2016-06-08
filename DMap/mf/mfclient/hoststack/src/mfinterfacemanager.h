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
 * @file   mfnetworkmanager.h
 * @author wontoniii (bronzino@winlab.rutgers.edu), czhang
 * @date   July, 2011
 * @brief  Class that keeps the status of the different interfaces updated.
 *
 * Class that keeps the status of the different interfaces updated.
 */

#ifndef MF_NETWORKMANAGER_H_
#define MF_NETWORKMANAGER_H_

#include <vector>
#include <semaphore.h>
#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/ioctl.h> 
#include <net/if.h> 
#if !defined(__ANDROID__)
#include <ifaddrs.h>
#else
#include "ifaddrs-android.h"
#endif
#include <sys/un.h>
#include <map>

#include "mfdevicemanager.h"
#include "mfsystem.h"
#include "mftypes.h"
#include "mflog.h"

using namespace std;

class MF_DeviceManager;

class MF_InterfaceManager {
	int mSock;
	pthread_t mTid;
	//Needs to be extended to support multiple interfaces of the same type
	vector<MF_DeviceManager *> ifs;
	map <string,int> nameToID;
	int wifi;
	int wimax;
	int ether;
	sem_t ifAccess;
	MFSystem *_system;
	int scanPeriod;
	
	void scanDevices();
	int matchText(char *, u_int, char *);
	int getWifiSignal(char *);
	char *getActiveEssid();
	int extractSignalValue(char *, int);
	
protected:
	void managerLoop();
	
public:
	MF_InterfaceManager();
	MF_InterfaceManager(MFSystem *_system);
	~MF_InterfaceManager();
	void setFromSettings();
	void start();

	//Functions to get the different interfaces depending on policy, policy, index, etc.
	MF_DeviceManager *getWifi();
	MF_DeviceManager *getWimax();
	MF_DeviceManager *getEther();
	MF_DeviceManager *getBest();
	MF_DeviceManager *getByIndex(int index);

	friend void *netManProc(void *arg);
	
};

void *netManProc(void *arg);

#endif /* MF_NETWORKMANAGER_H_ */
