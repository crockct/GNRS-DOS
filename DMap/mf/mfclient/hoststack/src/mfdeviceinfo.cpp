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
 * @file   mfdeviceinfo.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu), czhang
 * @date   August, 2011
 * @brief  Class that contains the information regarding an interface.
 *
 * Class that contains the information regarding an interface. Moreover intialize the pcap filter used to receive and transmit packets.
 */

#include "mfdeviceinfo.h"

MF_DeviceInfo::MF_DeviceInfo() {
	sem_init(&access, 0, 1);
}

MF_DeviceInfo::MF_DeviceInfo(MFSystem *_system, int type, string name, int ifID) {
	mType = type;
	mName = name;
	ID = ifID;
	lastProbe = 0;
	manual = false;
	this->_system = _system;
	inPackets = 0;
	outPackets = 0;
	inBytes = 0;
	outBytes = 0;
	status = IF_INACTIVE;
	sem_init(&access, 0, 1);
}

MF_DeviceInfo::MF_DeviceInfo(MFSystem *_system, int type, string name, string mac, string apmac, string ip, string apip, bool enabled) {
	mType = type;
	mName = name;
	mMac = mac;
	mAPMac = apmac;
	mIP = ip;
	mAPIP = apip;
	this->_system = _system;
	inPackets = 0;
	outPackets = 0;
	inBytes = 0;
	outBytes = 0;
	mSignalLevel = -200;
	status = 0;
	lastProbe = 0;
	manual = false;
	status = IF_INACTIVE;
	sem_init(&access, 0, 1);
}

MF_DeviceInfo::~MF_DeviceInfo() {
}

bool MF_DeviceInfo::isEnabled() {
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceInfo:isEnabled checking if interface is enabled");
	sem_wait(&access);
	bool ret = status == IF_ENABLED;
	sem_post(&access);
	return ret;
}

void MF_DeviceInfo::disable() {
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceInfo:disable disable the interface");
	sem_wait(&access);
	if(status == IF_ENABLED) status = IF_ACTIVE;
	sem_post(&access);
}

void MF_DeviceInfo::enable() {
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceInfo:enable enable the interface");
	sem_wait(&access);
	status = IF_ENABLED;
	sem_post(&access);
}

bool MF_DeviceInfo::isActive() {
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceInfo:isActive checking if interface is active");
	sem_wait(&access);
	bool ret = status == IF_ACTIVE || status == IF_ENABLED;
	sem_post(&access);
	return ret;
}

void MF_DeviceInfo::deactivate() {
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceInfo:deactivate deactivate the interface");
	sem_wait(&access);
	if(status == IF_ACTIVE || status == IF_ENABLED) status = IF_INACTIVE;
	sem_post(&access);
}

void MF_DeviceInfo::activate() {
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceInfo:activate activate the interface");
	sem_wait(&access);
	if(status == IF_INACTIVE) status = IF_ACTIVE;
	sem_post(&access);
}

#ifdef __OML__
void MF_DeviceInfo::sendOMLUpdate(){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceInfo: update OML stats in server");
	MF_OMLInterfaceStatistics *omlInterfaceStatistics;
	if(_system->getOmlMps()){
		sem_wait(&access);
		omlInterfaceStatistics = new MF_OMLInterfaceStatistics(ID, mName.c_str(), mMac.c_str(), inPackets, outPackets, inBytes, outBytes);
		_system->getOmlMps()->post(omlInterfaceStatistics);
		sem_post(&access);
	}
}

#endif //OML
