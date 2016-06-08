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
 * @file   mfsettings.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   September, 2013
 * @brief  Classes that contains the parser of the settings file and the functions to access it.
 *
 * Classes that contains the parser of the settings file and the functions to access it.
 */

#ifndef MF_SETINGS_H_
#define MF_SETINGS_H_

#include <fstream>
#include <list>
#include <string.h>
#include <cctype>
#include "mftypes.h"
#include "mflog.h"

#define DEFAULT_BUFFER_SIZE 10
#define DEFAULT_POLICY POLICY_BESTPERFORMANCE
#define DEFAULT_NET_MAN_LOOPTIME 5
#define DEFAULT_CSYN_TO_MICROS 100000 //100ms
#define DEFAULT_ASSOC_TO_MICROS 10000000 //10s

using namespace std;

struct TransportSetting {
  TransportSetting() : isReliabTrans(false), chkCountNACKThresh(0),
                        sendNACKTOutPerChk(0), recvNACKTOutPerChk(0) {}
  bool isReliabTrans;
  u_int chkCountNACKThresh;
  u_int sendNACKTOutPerChk;
  u_int recvNACKTOutPerChk;
};

class InterfaceDescr{
	IfType type;
	string name;
	IfMode mode;
	string apmac;
	string apip;
	
public:
	
	void setType(IfType t){
		type = t;
	}
	
	IfType getType(){
		return type;
	}
	
	void setName(string n){
		name.assign(n);
	}
	
	void setName(const char *n){
		name.assign(n);
	}
	
	string getName(){
		return name;
	}
	
	void setMode(IfMode m){
		mode = m;
	}
	
	IfMode getMode(){
		return mode;
	}
	
	void setApmac(string a){
		apmac.assign(a);
	}
	
	void setApmac(const char *a){
		apmac.assign(a);
	}
	
	string getApmac(){
		return apmac;
	}
	
	void setApip(string a){
		apip.assign(a);
	}
	
	void setApip(const char *a){
		apip.assign(a);
	}
	
	string getApip(){
		return apip;
	}
	
	void printInterface(){
		MF_Log::mf_log(MF_DEBUG, "Interface %s", name.c_str());
		if(mode == AUTO){
			MF_Log::mf_log(MF_DEBUG, "Mode auto");
		}
		else{
			MF_Log::mf_log(MF_DEBUG, "Mode maunal");
			MF_Log::mf_log(MF_DEBUG, "APIP %s", apip.c_str());
			MF_Log::mf_log(MF_DEBUG, "APMAC %s", apmac.c_str());
		}
	}
};

class MF_Settings {
	
private:
	//Place here all settings to be read
	bool errorOccurred;
	string errDescr;
	
	list <InterfaceDescr *> ifList;
	list<InterfaceDescr *>::iterator it;
	
	PolicyType policy;
	int defaultBufferSize;
	int defaultGUID;
	int if_scan_period;
	int csynTOMicros;
	int assocTOMicros;
	int socketAliveTimeout;
	
	InterfaceDescr *parseInterface(char *ptr);
	PolicyType parsePolicy(char *ptr);
	int parseInt(char *ptr);
	double parseDouble(char *ptr);
	float parseFloat(char *ptr);
  bool parseBoolean(char *ptr);
	char *removeSpaces(char *ptr);
	
  TransportSetting transSetting;	

public:
	MF_Settings();
	~MF_Settings();
	
	void init(const char *filename);
	void init(string filename);
	void init();
	
	PolicyType getPolicy(){
		return policy;
	}
	
  TransportSetting getTransSetting() {
    return transSetting;
  }
	int getDefaultBufferSize(){
		return defaultBufferSize;
	}
	
	int getDefaultGUID(){
		return defaultGUID;
	}
	
	int getIfScanPeriod(){
		return if_scan_period;
	}
	
	int getCsynTO(){
		return csynTOMicros;
	}
	
	int getAssocTO(){
		return assocTOMicros;
	}

	void setInterfaceIterator(){
		it = ifList.begin();
	}
	
	InterfaceDescr *getNextInterface(){
		if(it == ifList.end()){
			return NULL;
		}
		InterfaceDescr *ret = (*it);
		it++;
		return ret;
	}
	
	bool error(){
		return errorOccurred;
	}
	
	const char *errstr(){
		return errDescr.c_str();
	}

	int getSocketAliveTimeout() const {
		return socketAliveTimeout;
	}

	void setSocketAliveTimeout(int socketAliveTimeout) {
		this->socketAliveTimeout = socketAliveTimeout;
	}
};

#endif /* MF_SETINGS_H_ */
