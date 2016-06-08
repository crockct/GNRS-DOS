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
 * @file   mfnetworkmanager.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu), czhang
 * @date   July, 2011
 * @brief  Class that keeps the status of the different interfaces updated.
 *
 * Class that keeps the status of the different interfaces updated.
 */

//TODO: do we want to run the scanner in the main thread instead of in the network manager thread?

#include "mfinterfacemanager.h"

/*
 * Deprecated
 */
MF_InterfaceManager::MF_InterfaceManager() {
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager: initializing");
	mTid = 0;
	mSock = socket(AF_LOCAL, SOCK_DGRAM, 0);
	if ( mSock<0 ) {
		printf ("MF_InterfaceManager::MF_InterfaceManager(): socket error\n");
	}
	sem_init(&ifAccess, 0, 1);
	wifi = -1;
	wimax = -1;
	ether = -1;
}


MF_InterfaceManager::MF_InterfaceManager(MFSystem *_system){
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager: initializing");
	this->_system = _system;
	wifi = -1;
	wimax = -1;
	ether = -1;
}


MF_InterfaceManager::~MF_InterfaceManager() {

}


void MF_InterfaceManager::managerLoop(){
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager: starting network manager loop");
	//bool first = true;
	//TODO: at the moment seems not to be doing anything special
	while(1){
		sleep(scanPeriod);
		scanDevices();
		/* Do something to check the quality of the interfaces */
	}
}

void MF_InterfaceManager::setFromSettings() {
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:setFromSettings");
	int ifsn = 0;
	scanPeriod = _system->getSettings()->getIfScanPeriod();
	wifi = -1;
	wimax = -1;
	ether = -1;
	_system->getSettings()->setInterfaceIterator();
	InterfaceDescr *id;
	MF_DeviceManager *tempDM;
	while((id = _system->getSettings()->getNextInterface())!=NULL){
		if(id->getType() == WIFI){
			string name = id->getName();
			MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:setFromSettings read interface %s", name.c_str());
			wifi = ifsn;
			ifsn ++;
			nameToID.insert(make_pair(name, wifi));
			ifs.push_back((tempDM = new MF_DeviceManager(_system, WIFI, name, wifi,
					_system->getSettings()->getDefaultBufferSize(), _system->getSettings()->getCsynTO())));
			if(id->getMode() == MANUAL){
				tempDM->updateManually(id->getApmac(), id->getApip());
				tempDM->setManual();
			}
		}
		else if(id->getType() == WIMAX){
			string name = id->getName();
			MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:setFromSettings read interface %s", name.c_str());
			wimax = ifsn;
			ifsn ++;
			nameToID.insert(make_pair(name, wimax));
			ifs.push_back((tempDM = new MF_DeviceManager(_system, WIMAX, name, wimax,
					_system->getSettings()->getDefaultBufferSize(), _system->getSettings()->getCsynTO())));
			if(id->getMode() == MANUAL){
				tempDM->updateManually(id->getApmac(), id->getApip());
				tempDM->setManual();
			}
		}
		else if(id->getType() == ETHER){
			string name = id->getName();
			MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:setFromSettings read interface %s", name.c_str());
			ether = ifsn;
			ifsn ++;
			nameToID.insert(make_pair(name, ether));
			ifs.push_back((tempDM = new MF_DeviceManager(_system, ETHER, name, ether,
					_system->getSettings()->getDefaultBufferSize(), _system->getSettings()->getCsynTO())));
			if(id->getMode() == MANUAL){
				tempDM->updateManually(id->getApmac(), id->getApip());
				tempDM->setManual();
			}
		}
		else{
			MF_Log::mf_log(MF_ERROR, "MF_InterfaceManager:setFromSettings Interface type not known");
		}
	}
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager::setupFromSettings() finished reading interfaces");
}


void MF_InterfaceManager::start() {
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:start starting nm process");
	scanDevices();
	int error = pthread_create(&mTid, NULL, &netManProc, this);
	if (error){
		MF_Log::mf_log(MF_ERROR, "MF_InterfaceManager: Thread creation failed...");
		return;
	}
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager::start(): Network Manager started");
	for(u_int i = 0; i<ifs.size(); i++){
		ifs[i]->start();
	}
}


int MF_InterfaceManager::matchText(char *buffer, u_int index, char *text) {
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:matchText");
	u_int N = strlen(text);
	if((index + N) >= strlen(buffer)) {
		return -1;
	}
	u_int i;
	for(i = 0; i < N; i++) {
		if(buffer[index + i] != text[i]) {
			return -1;
		}
	}
	return 0;
}

int MF_InterfaceManager::extractSignalValue(char *buffer, int index) {
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:extractSignalValue");
	int start_index = index - 80;
	char signal_buffer[7];
	signal_buffer[0] = '-';
	int i = 1;
	while(index < (int)strlen(buffer) && buffer[start_index] != '\n') {

		while(buffer[start_index] >= '0' && buffer[start_index] <= '9') {
			signal_buffer[i] = buffer[start_index];
			start_index++;
			i++;
		}
		start_index++;
	}
	signal_buffer[i] = '\0';

	if (i>=3) {
		return atoi(signal_buffer);
	}
	return 0;
}

char *MF_InterfaceManager::getActiveEssid() {
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:getActiveEssid");
	//TODO: hard code for test
	//char active_essid = "mfclient";
	//return active_essid;
	return NULL;
}

int MF_InterfaceManager::getWifiSignal(char *essid) {
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:getWifiSignal");
	FILE *fp;
	char output[65535];
	fp = popen("iwlist scanning 2>&1", "r");
	if (fp == NULL) {
		MF_Log::mf_log(MF_ERROR, "MF_InterfaceManager::getWifiSignal(): Failed to run command\n" );
		return -1;
	}
	//store the whole file in output for later parsing
	int i = 0;
	while (!feof(fp)) {
		output[i] = fgetc(fp);
		i++;
	}
	output[i] = '\0';

	int signal_value = 1;
	for ( i=0; i<(int)strlen(output); i++) {
		if (matchText(output, i, essid)==0) {
			signal_value = extractSignalValue(output, i);
		}
	}
	pclose(fp);
	return signal_value;
}


void MF_InterfaceManager::scanDevices(){
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:scanDevices scan the status of the devices");
	struct ifaddrs *ifaddr, *ifa;
	int family, s;
	char host[NI_MAXHOST];
	vector <bool> checked;
	for(u_int i = 0; i<ifs.size(); i++){
		checked.push_back(false);
	}
	if (getifaddrs(&ifaddr) == -1) {
		MF_Log::mf_log(MF_ERROR, "MF_InterfaceManager::scanDevices() getifaddrs error");
		return;
	}
	
	/* Walk through linked list, maintaining head pointer so we
	 can free list later */
	int fd;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	MF_DeviceManager *dm;
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;
		family = ifa->ifa_addr->sa_family;
		
		/* For an AF_INET* interface address, display the address */
		if (family == AF_INET) {
			s = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			if (s != 0) {
				MF_Log::mf_log(MF_ERROR, "getnameinfo() failed: %s", gai_strerror(s));
				continue;
			}
			
			/* Check if the interface is among the ones we are "allowed" to use */
			string name(ifa->ifa_name);
			map<string,int>::iterator it = nameToID.find(name);
			if(it == nameToID.end()){
				continue;
			}
			
			/* Retrieve MAC and IP addresses */
			struct ifreq ifr;
			ifr.ifr_addr.sa_family = AF_INET;
			strncpy(ifr.ifr_name , ifa->ifa_name, IFNAMSIZ-1);
			ioctl(fd, SIOCGIFHWADDR, &ifr);
			unsigned char * mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
			MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager::scanDevices() name: %s IP: %s  MAC: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x", ifa->ifa_name, host, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			char c_mac[18];
			sprintf(c_mac, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			string s_mac(c_mac);
			string s_ip(host);
			checked[it->second] = true;
			dm = getByIndex(it->second);
			dm->updateIfInfo(s_mac, s_ip);
		}
	}
	/* Check if some interface has not been encountered */
	bool sendUpdate = false;
	for(u_int i = 0; i<ifs.size(); i++){
		dm = getByIndex(i);
		if(checked[i] == false){
			if(dm->isActive()){
				dm->deactivate();
			}
		}
		else if(checked[i]){
			if(!dm->isPcapRunning()) {
				dm->start();
			}
			if(dm->isManual() && dm->isActive() && !dm->isEnabled()){
				dm->enable();
				sendUpdate = true;
			}
		}
		MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:scanDevices interface %d has status %d", (int)i, dm->status());
	}
	if(sendUpdate) {
		MF_EvIfStateUpdate *event = new MF_EvIfStateUpdate();
		_system->getEventQueue()->add(event);
	}
	freeifaddrs(ifaddr);
	close(fd);
}


MF_DeviceManager *MF_InterfaceManager::getWifi(){
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:getWifi get wifi manager");
	if(wifi>=0 && ifs[wifi]->isEnabled()){
		return ifs[wifi];
	}
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:getWifi no wifi interface available");
	return NULL;
}

MF_DeviceManager *MF_InterfaceManager::getWimax(){
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:getWimax get wimax manager");
	if(wimax>=0 && ifs[wimax]->isEnabled()){
		return ifs[wimax];
	}
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:getWimax no wimax interface available");
	return NULL;
}

MF_DeviceManager *MF_InterfaceManager::getEther(){
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:getEther get ethernet manager");
	if(ether>=0 && ifs[ether]->isEnabled()){
		return ifs[ether];
	}
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:getEther no ether interface available");
	return NULL;
}

//TODO Hardcoded to prioritize in order: ethernet then wifi then wimax
MF_DeviceManager *MF_InterfaceManager::getBest(){
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:getBest get manager of best interface");
	if(ether>=0 && ifs[ether]->isEnabled()){
		return ifs[ether];
	}
	if(wifi>=0 && ifs[wifi]->isEnabled()){
		return ifs[wifi];
	}
	if(wimax>=0 && ifs[wimax]->isEnabled()){
		return ifs[wimax];
	}
	return NULL;
}

MF_DeviceManager *MF_InterfaceManager::getByIndex(int index){
	MF_Log::mf_log(MF_DEBUG, "MF_InterfaceManager:getByIndex get manger of interface with index %d",index);
	if(index>=0 && (int)ifs.size()>index){
		return ifs[index];
	}
	return NULL;
}


void *netManProc(void *arg) {
	MF_Log::mf_log(MF_DEBUG, "netManProc net manager process starting");
	MF_InterfaceManager *dm = (MF_InterfaceManager *)arg;
	dm->managerLoop();
	return NULL;
}
