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
 * @file   mfdevicemanager.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu), czhang
 * @date   August, 2011
 * @brief  Temp.
 *
 * Temp.
 */

#include <stdio.h>
#include <pcap.h>
#include <errno.h>
#include <string>
#include <time.h>
#include <list>
#include <vector>

#include <include/mfhopmsg.hh>
#include <include/mfgstarctrlmsg.hh>

#include "mfdevicemanager.h"
#include "mfdeviceinfo.h"
#include "mftypes.h"
#include "mfchunkinfo.h"
#include "mfetherproto.h"
#include "mfipproto.h"
#include "mfbuffer.h"
#include "mflog.h"
#include "mfaggregator.h"
#include "mfsegmentor.h"
#include "mfstackevent.h"
#include "mfpacketsupport.h"
#include "mfdeviceinfo.h"

MF_DeviceManager::MF_DeviceManager(MFSystem *_system, int type, string devName, int ifID, int t) {
	mDevInfo = new MF_DeviceInfo(_system, type, devName, ifID);
	mAggregator = new MF_Aggregator(_system);
	mSegmentor = new MF_Segmentor(_system, mPcapHandle, ifID, t);
	this->_system = _system;
}

MF_DeviceManager::MF_DeviceManager(MFSystem *_system, int type, string devName, int ifID, int bufSize,
		int t) {
	mDevInfo = new MF_DeviceInfo(_system, type, devName, ifID);
	mAggregator = new MF_Aggregator(_system, bufSize);
	mSegmentor = new MF_Segmentor(_system, mPcapHandle, ifID, t);
	this->_system = _system;
}

MF_DeviceManager::~MF_DeviceManager() {

}

void MF_DeviceManager::gotPacket(const struct pcap_pkthdr *header, const u_char *packet) {
	u_int len = header->caplen;
	u_int *type = (u_int *) (MF_PacketSupport::getHopHeaderPtr((u_char *)packet));
	switch (ntohl(*type)) {
	case DATA_PKT:
		dataHandler(MF_PacketSupport::getHopHeaderPtr((u_char *)packet));
		break;
	case CSYN_PKT:
		csynHandler(MF_PacketSupport::getHopHeaderPtr((u_char *)packet));
		break;
	case CSYN_ACK_PKT:
		ackHandler(MF_PacketSupport::getHopHeaderPtr((u_char *)packet), len - ETH_HEADER_SIZE - IP_HEADER_SIZE);
		break;
	case LP_PKT:
		lpHandler(packet);
		break;
	case ASSOC_PKT:
		MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager: Received a association request packet, ignore...");
		break;
	case DASSOC_PKT:
		MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager: Received a d-association request packet, ignore...");
		break;
	default:
		MF_Log::mf_log(MF_WARN, "MF_DeviceManager: unknown type: %u ", ntohl(*type));
	}
#ifdef __OML__
	mDevInfo->increaseInBytesByInt(len);
	mDevInfo->increaseInPackets();
#endif
}

void MF_DeviceManager::start() {
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:start starting interface");
	char errbuf[PCAP_ERRBUF_SIZE];
	struct bpf_program fp;  /* The compiled filter expression */
	char filter_exp[100] = "ip proto 5 and not ether src ";	 /* The filter expression */
	strcat(filter_exp, mDevInfo->getSMAC().c_str());
	mPcapHandle = pcap_open_live(mDevInfo->getName().c_str(), BUFSIZ, 1, 0, errbuf);
	if (mPcapHandle == NULL) {
		MF_Log::mf_log(MF_ERROR, "Couldn't open device %s: %s", mDevInfo->getName().c_str(), errbuf);
		mDevInfo->setStatus(IF_PCAP_ERROR);
		mDevInfo->setPcapError(PCAP_DEVICE_ERROR);
		return;
	}
	if (pcap_compile(mPcapHandle, &fp, filter_exp, 0, 0) == -1) {
		MF_Log::mf_log(MF_ERROR, "Couldn't parse filter %s: %s", filter_exp, pcap_geterr(mPcapHandle));
		mDevInfo->setStatus(IF_PCAP_ERROR);
		mDevInfo->setPcapError(PCAP_FILTER_ERROR);
		return;
	}
	if (pcap_setfilter(mPcapHandle, &fp) == -1) {
		MF_Log::mf_log(MF_ERROR, "Couldn't install filter %s: %s", filter_exp, pcap_geterr(mPcapHandle));
		mDevInfo->setStatus(IF_PCAP_ERROR);
		mDevInfo->setPcapError(PCAP_FILTER_ERROR);
		return;
	}
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager: initialized device: %s",  mDevInfo->getName().c_str());
	int error = pthread_create(&tid, NULL, &dmProc, this);
	if (error) {
		MF_Log::mf_log(MF_ERROR, "MF_DeviceManager: Thread creation failed!!!");
		mDevInfo->deactivate();
		return;
	}
	mSegmentor->setPCAPHandle(mPcapHandle);
	mDevInfo->activate();
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager: new thread is created for device %s", mDevInfo->getName().c_str());
}

/*
 * The probe can not be replied here using a global variable.
 * It has to be passed to upper layers to decide what to do with them.
 */
void MF_DeviceManager::lpHandler(const unsigned char *packet) {
	link_probe_t *lp  = (link_probe_t *) MF_PacketSupport::getHopHeaderPtr((unsigned char *)packet);
	struct etherHeader *eh = (struct etherHeader *)MF_PacketSupport::getEthernetHeaderPtr((unsigned char *)packet);
	struct ipHeader *ih = (struct ipHeader*)MF_PacketSupport::getIPHeaderPtr((unsigned char *)packet);
	struct ether_addr tempAddr;
	char srcEthAddr[18];
	memcpy(&(tempAddr.ether_addr_octet), eh->ether_shost, ETHER_ADDR_LEN);
	MF_EtherProto::ether_ntoa_r(&tempAddr,srcEthAddr);
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:lpHHandler received probe from %s ", srcEthAddr);
	MF_EvLinkProb *elp = new MF_EvLinkProb(mDevInfo->getID(), srcEthAddr, ih->ip_src.s_addr, ntohl(lp->sourceLP),ntohl(lp->seq), time(NULL));
	_system->getEventQueue()->add(elp);
	MF_Log::mf_log(MF_DEBUG,"Stack replied a link probe message. remote GUID = %d, seq = %d", ntohl(lp->sourceLP), ntohl(lp->seq));
}

void MF_DeviceManager::csynHandler(const unsigned char *packet) {
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:csynHandler new csyn received");
	if(!mAggregator->handleCsynInit(packet)){
		struct csyn *csyn = (struct csyn *) packet;
		MF_EvCysn *event = new MF_EvCysn(csyn, mDevInfo->getID());
		_system->getEventQueue()->add(event);
	}
}

void MF_DeviceManager::handleCsyn(u_int hopID, u_int pktCount){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:cysnMain handling new csyn received");
	int ret = mAggregator->handleCsyn(hopID, pktCount);
	MF_ChunkInfo *ci;
	if (ret == 1){
		ci = mAggregator->getChunkInfo(hopID);
		if(ci==NULL){
			MF_Log::mf_log(MF_ERROR, "MF_DeviceManager:csynHandler something went wrong with the aggregator");
		}
		else if(mSegmentor->sendAck(ci, mDevInfo) == true) {//received the whole chunk
			deliverChunk(ci);
		}
#ifdef __OML__
		mDevInfo->sendOMLUpdate();
#endif
	}
	else if (ret == 2) {//ack got lost
		ci = mAggregator->getChunkInfo(hopID);
		if(ci!=NULL) mSegmentor->sendAck(ci, mDevInfo);
	}
}

void MF_DeviceManager::deliverChunk(MF_ChunkInfo *ci) {
	MF_Log::mf_log(MF_INFO, "MF_DeviceManager::deliverChunk Recvd complete chunk, interface = %s, hop_ID = %d", mDevInfo->getName().c_str(), ci->getHopID());
	MF_EvUpIf *ecc = new MF_EvUpIf(mDevInfo->getID(), ci);
	vector <u_char *> *v = ci->getPacketList();
	ci->putChunkPktCnt((u_int)v->size());
	ci->setOwner((u_int)mDevInfo->getID());
	ci->setDelivered(true);
	_system->getEventQueue()->add(ecc);
}

void MF_DeviceManager::releaseChunk(u_int chkID) {
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager::releaseChunk releasing chunk %lu", chkID);
	mAggregator->releaseChunk(chkID);
}

void MF_DeviceManager::dataHandler(const u_char *packet) {
	//MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:dataHandler new data packet received");
	mAggregator->handleData(packet);
}

void MF_DeviceManager::ackHandler(const u_char *packet, u_int len) {
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:ackHandler new ack received");
	struct csyn_ack *ack = (struct csyn_ack *)packet;
	MF_EvCysnAck *event = new MF_EvCysnAck(ack, mDevInfo->getID());
	_system->getEventQueue()->add(event);
}

MF_ChunkInfo *MF_DeviceManager::getChunkInfo(u_int id){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:getChunkInfo get info about chunk");
	return mAggregator->getChunkInfo(id);
}

pcap_t *MF_DeviceManager::getPcapHandle(){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:getPcapHandle return pcap handle");
	return mPcapHandle;
}

bool MF_DeviceManager::updateProb(string ether, string ip, time_t t){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:updateProb new info, IP:%s  MAC:%s  tstamp:%d",ip.c_str(), ether.c_str(), t);
	bool sendUpdateAssoc = false;
	if(isManual()){
		if(ether.compare(mDevInfo->getAPMAC())==0 && ip.compare(mDevInfo->getAPIP())==0){
			return true;
		}
		else{
			return false;
		}
	}
	else{
		if(ether.compare(mDevInfo->getAPMAC()) || ip.compare(mDevInfo->getAPIP())){
			sendUpdateAssoc = true;
		}
		mDevInfo->setAPMAC(ether);
		mDevInfo->setAPIP(ip);
		mDevInfo->setLastProbe(t);
		if(!mDevInfo->isEnabled()){
			sendUpdateAssoc = true;
			enable();
		}
		return true;
	}
	if(sendUpdateAssoc){
		MF_EvIfStateUpdate *event = new MF_EvIfStateUpdate();
		_system->getEventQueue()->add(event);
	}
}

void MF_DeviceManager::updateManually(string ether, string ip){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:updateManually new info, IP:%s  MAC:%s",ip.c_str(), ether.c_str());
	mDevInfo->setAPMAC(ether);
	mDevInfo->setAPIP(ip);
}

void MF_DeviceManager::sendLinkProb(u_int sourceLPACK, u_int destinationLPACK, u_int seq_no_cp){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:sendLinkProb");
	mSegmentor->sendLPAck(sourceLPACK, destinationLPACK, seq_no_cp, mDevInfo);
}

void MF_DeviceManager::updateIfInfo(string mac, string ip){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:updateIfInfo update ip and mac of device");
	mDevInfo->setSMAC(mac);
	mDevInfo->setSIP(ip);
	time_t t = time(NULL);
	if((t - mDevInfo->getLastProbe() > IF_INAC_TO) && !isManual() && mDevInfo->isEnabled()){
		disable();
	}
}

void MF_DeviceManager::handleCsynAck(u_int hop_ID, u_int pkt_count, u_char *bitmap){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:cysnReceived new csyn ack received");
	mSegmentor->handleAck(hop_ID, pkt_count, bitmap, mDevInfo);
	//	if(!mSegmentor->isSending() && !outgChunks.empty()){
	//		MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:cysnReceived segmentor is available and there are chunks waiting to be sent");
	//		MF_ChunkInfo *ci = outgChunks.front();
	//		outgChunks.pop_front();
	//		mSegmentor->procSend(ci, mDevInfo);
	//	}
}

void MF_DeviceManager::handleCsynTimeOut(u_int chunk){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:cysnTimeOut timeout of message %u", chunk);
	mSegmentor->csynTO(chunk, mDevInfo);
}

bool MF_DeviceManager::isActive(){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:isActive check if device is active");
	return mDevInfo->getStatus() == IF_ACTIVE || mDevInfo->getStatus() == IF_ENABLED;
}

bool MF_DeviceManager::isEnabled(){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:isEnabled check if device is enabled");
	return mDevInfo->getStatus() == IF_ENABLED;
}

void MF_DeviceManager::disable(){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:disable disable device");
	mDevInfo->disable();
	mAggregator->resetStatus();
	mSegmentor->resetStatus();
}

void MF_DeviceManager::enable(){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:enable enable device");
	mDevInfo->enable();
	mSegmentor->interfaceEnabled(mDevInfo);
}

void MF_DeviceManager::activate(){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:activate activate device");
	mDevInfo->activate();
}

void MF_DeviceManager::deactivate(){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:deactivate deactivate device");
	if(mDevInfo->isEnabled()){
		disable();
	}
	mDevInfo->deactivate();
}

bool MF_DeviceManager::isPcapRunning(){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:isPcapRunning check whether pcap is running for the device");
	return mDevInfo->getStatus() != IF_PCAP_ERROR;
}

int MF_DeviceManager::status(){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:status get device status");
	return mDevInfo->getStatus();
}

void MF_DeviceManager::pcapError(){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:pcapError disable device due to pcap error");
	if(mDevInfo->isEnabled()){
		disable();
	}
	mDevInfo->setStatus(IF_PCAP_ERROR);
	mDevInfo->setPcapError(PCAP_LOOP_ERROR);
}

void MF_DeviceManager::sendChunk(MF_ChunkInfo *ci){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:sendChunk send given chunk");
	//	if(!mSegmentor->isSending() && outgChunks.empty()){
	//		MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:sendChunk the queue is empty, sending out");
	mSegmentor->procSend(ci, mDevInfo);
	//	}
	//	else{
	//		MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:sendChunk the queue is not empty, storing");
	//		outgChunks.push_back(ci);
	//	}
}

void MF_DeviceManager::sendAssocReq(u_int entityGUID, u_int hostGUID, u_short weight){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:sendAssocReq send an association request to access router");
	if(this->isEnabled()){
		mSegmentor->sendAssoRequest(entityGUID, hostGUID, weight, mDevInfo);
	}
}

bool MF_DeviceManager::isManual(){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:isManual checking if interface has manual setup");
	return mDevInfo->getManual();
}

void MF_DeviceManager::setManual(){
	MF_Log::mf_log(MF_DEBUG, "MF_DeviceManager:setManual setting interface to manual mode");
	return mDevInfo->setManual(true);
}

int MF_DeviceManager::getID(){
	return mDevInfo->getID();
}

void handlePacket(u_char *args, const struct pcap_pkthdr *header, const u_char *packet){
	//MF_Log::mf_log(MF_DEBUG, "handlePacket handle new packet in pcap loop");
	MF_DeviceManager *dm = (MF_DeviceManager *)args;
	dm->gotPacket(header, packet);
}

void *dmProc(void *arg) {
	MF_Log::mf_log(MF_DEBUG, "dmProc device manager process starting");
	MF_DeviceManager *dm = (MF_DeviceManager *)arg;
	if (pcap_loop(dm->getPcapHandle(), -1, handlePacket, (u_char *) dm) == -1) {
		MF_Log::mf_log(MF_ERROR, "MF_DeviceManager::proc(): pcap_loop error\n");
		dm->pcapError();
	}
	return NULL;
}

