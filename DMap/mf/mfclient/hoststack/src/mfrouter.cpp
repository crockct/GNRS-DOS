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
 * @file   mfrouter.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu), czhang
 * @date   August, 2011
 * @brief  Class that manages available routes for chunks (and relative flows)
 *
 * Class that manages available routes for chunks (and relative flows)
 *      At the moment it only does 2 things:
 *          -for received packets checks if the flow is open otherwise discard the packet
 *          -for packets that have to be sent decides the interface to use and fill the header.
 */

#include <set>
#include <include/mfguid.hh>
#include <include/mfroutingheader.hh>
#include <include/mfmhomeextheader.hh>
#include <include/mfflags.h>

#include "mftypes.h"
#include "mfchunkinfo.h"
#include "mfdevicemanager.h"
#include "mfinterfacemanager.h"
#include "mfsockettable.h"
#include "mfsocketmanager.h"
#include "mfeventqueue.h"
#include "mftransport.h"
#include "mfrouter.h"
#include "mfpacketsupport.h"

MF_Router::MF_Router() {
	this->_system = NULL;
	defaultGUID = 0;
}

MF_Router::MF_Router(MFSystem *_system){
	this->_system = _system;
	defaultGUID = 0;
	inChunks = 0;
	outChunks = 0;
	policy = 0;
	assocTOID = 0;
	assocTOMicros = 0;
}

MF_Router::~MF_Router() {

}

void MF_Router::init() {
	MF_Log::mf_log(MF_DEBUG, "MF_Router:init initializing router component");
	assocTOMicros = _system->getSettings()->getAssocTO();
	assocTOID = _system->getTimeoutManager()->AddTimeout(this, NULL, (long int)(assocTOMicros/1000));
}

void MF_Router::setPolicy(PolicyType pol){
	MF_Log::mf_log(MF_DEBUG, "MF_Router:setPolicy setup system policy by given parameter %d", pol);
	policy = pol;
}

void MF_Router::setDefaultGUID(string fileName){
	ifstream myfile;
	myfile.open(fileName.c_str());
	if(!myfile.is_open()){
		MF_Log::mf_log(MF_ERROR, "Error opening file containing default GUID %s", fileName.c_str());
		return;
	}
	int guid;
	myfile >> guid;
	MF_Log::mf_log(MF_DEBUG, "MF_Router:setDefaultGUID the default GUID is %d", guid);
	defaultGUID = guid;
}

void MF_Router::setDefaultGUID(int guid){
	MF_Log::mf_log(MF_DEBUG, "MF_Router:setDefaultGUID reading default GUID %d", guid);
	defaultGUID = guid;
}

int MF_Router::getDefaultGUID(){
	return defaultGUID;
}

bool MF_Router::procRecv(MF_ChunkInfo *ci) {
	MF_Log::mf_log(MF_DEBUG, "MF_Router:procRecv receiving new chunk");
	//struct routeHeader *route;
    RoutingHeader *route;
	vector <u_char *> *pList = ci->getPacketList();
	if(pList->size() == 0){
		MF_Log::mf_log(MF_WARN, "MF_Router::procRecv: trying to recv EMPTY chunk!");
		return true;
	}
	route = new RoutingHeader(MF_PacketSupport::getNetworkHeaderPtr((*pList)[0]));
        ci->putSrcGUID( route->getSrcGUID().to_int() );
        ci->putDstGUID( route->getDstGUID().to_int() );
        ci->putServiceID( route->getServiceID().to_int() );
        MF_Log::mf_log( MF_DEBUG, "MF_Router::procRecv: SrcGUID: %u, DstGUID:%u, ServiceID: %u", ci->getSrcGUID(), ci->getDstGUID(), ci->getServiceID());
	if(GUIDs.find(ci->getDstGUID()) == GUIDs.end() && (int)ci->getDstGUID()!=defaultGUID){
		MF_Log::mf_log(MF_WARN, "MF_Router::procRecv: no such destination GUID %d!", ci->getDstGUID());
		return true;
	}
	MF_Log::mf_log(MF_DEBUG, "MF_Router:procRecv dstTID == 0 so sending it all sockets listening to GUID=%d", route->getDstGUID().to_int());
	IdMap *m = _system->getSocketTable()->getByGUID(ci->getDstGUID());
	if(m==NULL){
		MF_Log::mf_log(MF_WARN, "MF_Router::procRecv: no socket manager listening to GUID=%d!",route->getDstGUID().to_int());
		return true;
	}
	else if(m->size()>1){
		MF_Log::mf_log(MF_DEBUG, "MF_Router:procRecv %d transports with GUID=%u but sending only to the first one",ci->getDstGUID());
		IdMap::iterator it = m->begin();
		//TODO change 0 with real transport, check if NULL
		MF_EvUpRout *event = new MF_EvUpRout(it->second->getTransportByTID(0), ci);
		_system->getEventQueue()->add(event);
	}
	else{
		MF_Log::mf_log(MF_DEBUG, "MF_Router:procRecv 1 transport with GUID=%u so sending it to appropriate transport",ci->getDstGUID());
		IdMap::iterator it = m->begin();
		//TODO change 0 with real transport, check if NULL
		MF_EvUpRout *event = new MF_EvUpRout(it->second->getTransportByTID(0), ci);
		_system->getEventQueue()->add(event);
	}
#ifdef __OML__
	if(mfoml){
		inChunks++;
		MF_OMLChunkStatistics *omlChunkStatistics;
		omlChunkStatistics = new MF_OMLChunkStatistics((void*)&defaultGUID, 4, inChunks, outChunks);
		_system->getOmlMps()->post(omlChunkStatistics);
	}
#endif
	return false;
}


/*
 * Do the work of router: decide which interface to use.
 * The decision is based on user policy
 */
void MF_Router::procSend(MF_ChunkInfo *chunkInfo) {
	MF_Log::mf_log(MF_DEBUG, "MF_Router:procSend sending new chunk");
	MF_DeviceManager *dm;
	switch (policy) {
		case POLICY_BESTPERFORMANCE:
			dm = _system->getInterfaceManager()->getBest();
			break;
		case POLICY_WIFIONLY:
			dm = _system->getInterfaceManager()->getWifi();
			break;
		case POLICY_WIMAXONLY:
			dm = _system->getInterfaceManager()->getWimax();
			break;
		case POLICY_ETHERONLY:
			dm = _system->getInterfaceManager()->getEther();
			break;
		default:
			dm = _system->getInterfaceManager()->getBest();
			break;
	}
	if (dm!=NULL) {
		if(chunkInfo->getSrcGUID()==0){
			chunkInfo->putSrcGUID(defaultGUID);
		}
		MF_Log::mf_log(MF_DEBUG, "MF_Router:procSend chunk info srcGUID=%d dstGUID=%d services=%u", chunkInfo->getSrcGUID(), chunkInfo->getDstGUID(), chunkInfo->getServiceID());
		//TODO: better organize management of service IDs
		if(chunkInfo->isMultihoming()){
			chunkInfo->putServiceID(MF_ServiceID::SID_MULTIHOMING);
		}
		else if(chunkInfo->isAnycast()) {
			chunkInfo->putServiceID(MF_ServiceID::SID_ANYCAST);
		}
		else if(chunkInfo->isBroadcast()){
			chunkInfo->putServiceID(MF_ServiceID::SID_BROADCAST);
		}
		else if(chunkInfo->isMulticast()){
			chunkInfo->putServiceID(MF_ServiceID::SID_MULTICAST);
		}
		if(chunkInfo->isContentRequest()) {
			chunkInfo->putServiceID(MF_ServiceID::SID_CONTENT_REQUEST);
		}
		else if(chunkInfo->isContentResponse()) {
			chunkInfo->putServiceID(MF_ServiceID::SID_CONTENT_RESPONSE);
		}
		fillHeader(chunkInfo);
		MF_EvDownRout *edc = new MF_EvDownRout(chunkInfo, dm->getID());
		_system->getEventQueue()->add(edc);
#ifdef __OML__
		if(mfoml){
			outChunks++;
			MF_OMLChunkStatistics *omlChunkStatistics;
			omlChunkStatistics = new MF_OMLChunkStatistics((void*)&defaultGUID, 4, inChunks, outChunks);
			_system->getOmlMps()->post(omlChunkStatistics);
		}
#endif
	}
	else{
		//TODO: clean up eventual problem
		//MF_EvChkComplete *event = new MF_EvChkComplete(-1, chunkInfo);
		//eq->add(event);
		MF_Log::mf_log(MF_ERROR, "MF_Router::procSend policy error, the given policy %d generate NULL Dev Manager", policy);
	}
}

void MF_Router::fillHeader(MF_ChunkInfo *ci) {
	MF_Log::mf_log(MF_DEBUG, "MF_Router:fillHeader");
	//struct routeHeader *route;
	RoutingHeader *route;
	vector <u_char *> *pList = ci->getPacketList();
	route = new RoutingHeader( MF_PacketSupport::getNetworkHeaderPtr(pList->front()));
	route->setServiceID( ci->getServiceID() );

  if (ci->getDstNA() > 0) route->setDstNA(ci->getDstNA());
	else route->setDstNA( 0 );

	route->setDstGUID( ci->getDstGUID() );
	route->setSrcGUID( ci->getSrcGUID() );
//TODO replace 60 with the next header reserved space
	route->setPayloadOffset( ROUTING_HEADER_SIZE + EXTHDR_MULTIHOME_MAX_SIZE);
	route->setChunkPayloadSize( (unsigned int)(ci->getChunkSize() + TRANS_HEADER_SIZE));
	MF_Log::mf_log(MF_DEBUG, "MF_Router: fillHeader: srcGUID: %u, DstGUID: %u, ServiceID %u",
	ci->getSrcGUID(), ci->getDstGUID(), ci->getServiceID() );
}


void MF_Router::addGUID(int GUID){
	MF_Log::mf_log(MF_DEBUG, "MF_Router:addGUID add GUID to local GUID list");
	set <int>::iterator it = GUIDs.find(GUID);
	if(it==GUIDs.end()){
		GUIDs.insert(GUID);
	}
}

void MF_Router::removeGUID(int GUID){
	MF_Log::mf_log(MF_DEBUG, "MF_Router:removeGUID remove GUID from local GUID list");
	GUIDs.erase(GUID);
}

void MF_Router::sendAssocReqs(){
	MF_Log::mf_log(MF_DEBUG, "MF_Router:sendAssocReqs update associations with routers");
	if ( policy == POLICY_BESTPERFORMANCE || policy == POLICY_WEIGHTED11 || policy == POLICY_WEIGHTED31 || policy == POLICY_WEIGHTED00) {
		MF_DeviceManager *wifi = _system->getInterfaceManager()->getWifi();
		MF_DeviceManager *wimax = _system->getInterfaceManager()->getWimax();
		if(wifi!=NULL) wifi->sendAssocReq(defaultGUID,defaultGUID,1);
		if(wimax!=NULL) wimax->sendAssocReq(defaultGUID,defaultGUID,1);
		for(set <int>::iterator it = GUIDs.begin(); it!=GUIDs.end(); ++it){
			if(wifi!=NULL) wifi->sendAssocReq((*it), defaultGUID, 1);
			if(wimax!=NULL) wimax->sendAssocReq((*it), defaultGUID, 1);
		}
	}
	else if ( policy == POLICY_WIFIONLY ) {
		MF_DeviceManager *wifi = _system->getInterfaceManager()->getWifi();
		if(wifi==NULL) return;
		wifi->sendAssocReq(defaultGUID,defaultGUID,1);
		for(set <int>::iterator it = GUIDs.begin(); it!=GUIDs.end(); ++it){
			wifi->sendAssocReq((*it), defaultGUID, 1);
		}
	}
	
	else if ( policy == POLICY_WIMAXONLY ) {
		MF_DeviceManager *wimax = _system->getInterfaceManager()->getWimax();
		if(wimax==NULL) return;
		wimax->sendAssocReq(defaultGUID,defaultGUID,1);
		for(set <int>::iterator it = GUIDs.begin(); it!=GUIDs.end(); ++it){
			wimax->sendAssocReq((*it), defaultGUID,1);
		}
	}
	
	else if ( policy == POLICY_ETHERONLY ) {
		MF_DeviceManager *ether = _system->getInterfaceManager()->getEther();
		if(ether==NULL) return;
		ether->sendAssocReq(defaultGUID,defaultGUID,1);
		for(set <int>::iterator it = GUIDs.begin(); it!=GUIDs.end(); ++it){
			ether->sendAssocReq((*it), defaultGUID,1);
		}
	}
	else{
		MF_Log::mf_log(MF_ERROR, "MF_Router:sendAssocReqs policy error");
	}
}

void MF_Router::OnTimeout(void *message, unsigned int id){
	MF_EvAssocTO *to = new MF_EvAssocTO();
	_system->getEventQueue()->add(to);
	MF_Log::mf_log(MF_ERROR, "MF_Router:OnTimeout rescheduling TO after %d micros", assocTOMicros);
	assocTOID = _system->getTimeoutManager()->AddTimeout(this, NULL, (long int)(assocTOMicros/1000));
}
