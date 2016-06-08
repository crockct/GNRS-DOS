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
 * @file   mfdeviceinfo.h
 * @author wontoniii (bronzino@winlab.rutgers.edu), czhang
 * @date   August, 2011
 * @brief  Class that contains the information regarding an interface.
 *
 * Class that contains the information regarding an interface. Moreover intialize the pcap filter used to receive and transmit packets.
 */

#ifndef MF_DEVICEINFO_H_
#define MF_DEVICEINFO_H_

#include <stdint.h>
#include <time.h>
#include <semaphore.h>

#include "mfsystem.h"
#include "mferrors.h"
#include "mftypes.h"
#include "mfbuffer.h"
#include "mflog.h"

typedef int if_status;

#define IF_ENABLED 1 //Ready to transmit
#define IF_ACTIVE 0
#define IF_INACTIVE -1
#define IF_PCAP_ERROR -2

class MFSystem;

class MF_DeviceInfo {
private:
	MFSystem *_system;
	bool manual;
	//Status of the interface. Captures whether the if is enabled, active, inactive, or down due to error
	if_status status;
	int pcap_error;
	int ID;
	int mType;
	string mName;
	string mMac;
	string mAPMac;
	string mIP;
	string mAPIP;
	int mSignalLevel;
	time_t lastProbe;
	sem_t access;
	
	//OML data
	uint32_t inPackets;
	uint32_t outPackets;
	uint32_t inBytes;
	uint32_t outBytes;
	
public:
	
	MF_DeviceInfo();
	MF_DeviceInfo(MFSystem *_system, int type, string name, int ifID);
	MF_DeviceInfo(MFSystem *_system, int, string, string, string, string, string, bool);
	virtual ~MF_DeviceInfo();
	
	bool isEnabled();
	void disable();
	void enable();
	bool isActive();
	void deactivate();
	void activate();
	
	inline int getStatus(){
		sem_wait(&access);
		int ret = status;
		sem_post(&access);
		return ret;
	}
	
	inline void setStatus(int s){
		sem_wait(&access);
		status = s;
		sem_post(&access);
	}
	
	inline int getPcapError(){
		sem_wait(&access);
		int ret = pcap_error;
		sem_post(&access);
		return ret;
	}

	inline void setPcapError(int s){
		sem_wait(&access);
		pcap_error = s;
		sem_post(&access);
	}

	inline uint32_t getInPackets(){
		sem_wait(&access);
		uint32_t ret = inPackets;
		sem_post(&access);
		return ret;
	}
	
	inline void increaseInPackets(){
		sem_wait(&access);
		inPackets++;
		sem_post(&access);
	}
	
	inline void increaseInPacketsByInt(uint32_t n){
		sem_wait(&access);
		inPackets+=n;
		sem_post(&access);
	}
	
	inline uint32_t getOutPackets(){
		sem_wait(&access);
		uint32_t ret = outPackets;
		sem_post(&access);
		return ret;
	}
	
	inline void increaseOutPackets(){
		sem_wait(&access);
		outPackets++;
		sem_post(&access);
	}
	
	inline void increaseOutPacketsByInt(uint32_t n){
		sem_wait(&access);
		outPackets+=n;
		sem_post(&access);
	}
	
	inline uint32_t getInBytes(){
		sem_wait(&access);
		uint32_t ret = inBytes;
		sem_post(&access);
		return ret;
	}
	
	inline void increaseInBytesByInt(uint32_t n){
		sem_wait(&access);
		inBytes+=n;
		sem_post(&access);
	}
	
	inline uint32_t getOutBytes(){
		sem_wait(&access);
		uint32_t ret = outBytes;
		sem_post(&access);
		return ret;
	}
	
	inline void increaseOutBytesByInt(uint32_t n){
		sem_wait(&access);
		outBytes+=n;
		sem_post(&access);
	}
	
	void sendOMLUpdate();
	
	inline int getID(){
		sem_wait(&access);
		int ret = ID;
		sem_post(&access);
		return ret;
	}
	
	inline void setID(int i){
		sem_wait(&access);
		ID = i;
		sem_post(&access);
	}
	
	inline string getName(){
		sem_wait(&access);
		string ret = mName;
		sem_post(&access);
		return ret;
	}
	
	inline void setName(string m){
		sem_wait(&access);
		mName = m;
		sem_post(&access);
	}
	
	inline string getSMAC(){
		sem_wait(&access);
		string ret =  mMac;
		sem_post(&access);
		return ret;
	}
	
	inline void setSMAC(string m){
		sem_wait(&access);
		mMac = m;
		sem_post(&access);
	}
	
	inline string getAPMAC(){
		sem_wait(&access);
		string ret = mAPMac;
		sem_post(&access);
		return ret;
	}
	
	inline void setAPMAC(string m){
		sem_wait(&access);
		mAPMac = m;
		sem_post(&access);
	}
	
	inline string getSIP(){
		sem_wait(&access);
		string ret = mIP;
		sem_post(&access);
		return ret;
	}
	
	inline void setSIP(string m){
		sem_wait(&access);
		mIP = m;
		sem_post(&access);
	}
	
	inline string getAPIP(){
		sem_wait(&access);
		string ret =mAPIP;
		sem_post(&access);
		return ret;
	}
	
	inline void setAPIP(string m){
		sem_wait(&access);
		mAPIP = m;
		sem_post(&access);
	}
	
	inline time_t getLastProbe(){
		sem_wait(&access);
		time_t ret;
		ret = lastProbe;
		sem_post(&access);
		return ret;
	}
	
	inline void setLastProbe(time_t t){
		sem_wait(&access);
		lastProbe = t;
		sem_post(&access);
	}
	
	inline bool getManual(){
		return manual;
	}
	
	inline void setManual(bool m){
		manual = m;
	}

};

#endif /* MF_DEVICEINFO_H_ */
