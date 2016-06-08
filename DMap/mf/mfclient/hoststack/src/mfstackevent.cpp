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
 * @file   mfstackevent.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   September, 2013
 * @brief  Classes that describe different stack events.
 *
 * Classes that describe different stack events.
 */

#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <vector>
#include <time.h>

#include <include/mfflags.h>
#include <include/mfclientipc.h>

#include "mfstackevent.h"
#include "mfchunkinfo.h"
#include "mftypes.h"
#include "mfbuffer.h"
#include "mfetherproto.h"
#include "mftransport.h"


/*  TEMPLATE CLASS  */
MF_StackEvent::MF_StackEvent(){
	
}

MF_StackEvent::MF_StackEvent(evType t){
	set_m_type(t);
}

MF_StackEvent::~MF_StackEvent(){
	
}

u_int MF_StackEvent::getSize(){
	return size;
}

void MF_StackEvent::setSize(u_int s){
	size = s;
}



/*  OPEN  */
MF_APIOpen::MF_APIOpen() : MF_StackEvent(EV_OPEN){
	
}

MF_APIOpen::MF_APIOpen(struct MsgOpen *mo) : MF_StackEvent(EV_OPEN){
	UID = mo->UID;
	memcpy(profile, mo->profile, LEN_PROF);
	srcGUID = mo->srcGUID;
	opts.createFromString(mo->opts);
}

MF_APIOpen::~MF_APIOpen(){
	
}



/*  SEND  */
MF_APISend::MF_APISend() : MF_StackEvent(EV_SEND){
	
}

MF_APISend::MF_APISend(struct MsgSend *ms) : MF_StackEvent(EV_SEND){
	UID = ms->UID;
	mSize = ms->size;
	reqID = ms->reqID;
	dstGUID = ms->dst_GUID;
	opts = ms->opts;
}

MF_APISend::~MF_APISend(){
	
}



/*  RECEIVE  */
MF_APIRecv::MF_APIRecv() : MF_StackEvent(EV_RECV){
	
}

MF_APIRecv::MF_APIRecv(struct MsgRecv *mr) : MF_StackEvent(EV_RECV){
	UID = mr->UID;
	mSize = mr->size;
	reqID = mr->reqID;
	blk = mr->blk;
}

MF_APIRecv::~MF_APIRecv(){
	
}



/*  ATTACH  */
MF_APIAttach::MF_APIAttach() : MF_StackEvent(EV_ATTACH){
	
}

MF_APIAttach::MF_APIAttach(struct MsgAttach *ma) : MF_StackEvent(EV_ATTACH){
	UID = ma->UID;
	reqID = ma->reqID;
	nGUID = ma->nGUID;
	for(u_int i = 0; i< nGUID; i++)GUIDs[i]=ma->GUIDs[i];
}

MF_APIAttach::~MF_APIAttach(){
	
}



/*  DETACH  */
MF_APIDetach::MF_APIDetach() : MF_StackEvent(EV_DETACH){
	
}

MF_APIDetach::MF_APIDetach(struct MsgDetach *md) : MF_StackEvent(EV_DETACH){
	UID = md->UID;
	reqID = md->reqID;
	nGUID = md->nGUID;
	for(u_int i = 0; i< nGUID; i++)GUIDs[i]=md->GUIDs[i];
}

MF_APIDetach::~MF_APIDetach(){
	
}



/*  CLOSE  */
MF_APIClose::MF_APIClose() : MF_StackEvent(EV_CLOSE){
	
}

MF_APIClose::MF_APIClose(struct MsgClose *mc) : MF_StackEvent(EV_CLOSE){
	UID = mc->UID;
	reqID = mc->reqID;
}

MF_APIClose::~MF_APIClose(){
	
}

/*  GET  */
MF_APIGet::MF_APIGet() : MF_StackEvent(EV_GET){
	
}

MF_APIGet::MF_APIGet(struct MsgGet *ms) : MF_StackEvent(EV_GET){
	UID = ms->UID;
	mSize = ms->size;
	reqID = ms->reqID;
	dstGUID = ms->dst_GUID;
	opts = ms->opts;
}

MF_APIGet::~MF_APIGet(){
	
}

/*  DO_GET  */
MF_APIDoGet::MF_APIDoGet() : MF_StackEvent(EV_DO_GET){
	
}

MF_APIDoGet::MF_APIDoGet(struct MsgDoGet *mr) : MF_StackEvent(EV_DO_GET){
	UID = mr->UID;
	mSize = mr->size;
	reqID = mr->reqID;
}

MF_APIDoGet::~MF_APIDoGet(){
	
}

/*  GET_RESPONSE  */
MF_APIGetResponse::MF_APIGetResponse() : MF_StackEvent(EV_GET_RESPONSE){
	
}

MF_APIGetResponse::MF_APIGetResponse(struct MsgGetResponse *ms) : MF_StackEvent(EV_GET_RESPONSE){
	UID = ms->UID;
	mSize = ms->size;
	reqID = ms->reqID;
	getID = ms->getID;
	srcGUID = ms->srcGUID;
	dstGUID = ms->dstGUID;
	opts = ms->opts;
}

MF_APIGetResponse::~MF_APIGetResponse(){
	
}



/*  CSYN ACK RECEIVED  */
MF_EvCysnAck::MF_EvCysnAck() : MF_StackEvent(EV_CSYN_ACK){
	
}

MF_EvCysnAck::MF_EvCysnAck(struct csyn_ack *ack, int interface) : MF_StackEvent(EV_CSYN_ACK){
	this->interface = interface;
	hop_ID = ntohl(ack->hop_ID);
	pkt_count = ntohl(ack->chk_pkt_count);
	int bitmapLength = (pkt_count - 1) / 8 + 1;
	bitmap = new u_char[bitmapLength];
	memcpy(bitmap, ack->bitmap, bitmapLength);
}

MF_EvCysnAck::~MF_EvCysnAck(){
	delete bitmap;
}



/*  CSYN RECEIVED  */
MF_EvCysn::MF_EvCysn() : MF_StackEvent(EV_CSYN){
	
}

MF_EvCysn::MF_EvCysn(struct csyn *pkt, int interface) : MF_StackEvent(EV_CSYN){
	this->interface = interface;
	hop_ID = ntohl(pkt->hop_ID);
	pkt_count = ntohl(pkt->chk_pkt_count);
}

MF_EvCysn::~MF_EvCysn(){
	
}



/*  CSYN TIMEOUT  */
MF_EvCysnTOut::MF_EvCysnTOut() : MF_StackEvent(EV_CSYN_TIMEOUT){
	
}

MF_EvCysnTOut::MF_EvCysnTOut(int i, u_int c) : MF_StackEvent(EV_CSYN_TIMEOUT){
	interface = i;
	chnkID = c;
}

MF_EvCysnTOut::~MF_EvCysnTOut(){
	
}



/*  NEW LINK PROBE MESSAGE RECEIVED  */
MF_EvLinkProb::MF_EvLinkProb() : MF_StackEvent(EV_LP) {
	
}

MF_EvLinkProb::MF_EvLinkProb(int d, const char* eth, unsigned long ip, u_int ls, u_int sn, time_t t) : MF_StackEvent(EV_LP) {
	devID = d;
	memcpy(ether_dhost,eth,18);
	s_addr = ip;
	sourceLP = ls;
	timestamp = t;
	seqNum = sn;
}

MF_EvLinkProb::~MF_EvLinkProb(){
	
}



/*  INTERFACE UPDATE FROM NETWORK MANAGER  */
MF_EvIfStateUpdate::MF_EvIfStateUpdate() : MF_StackEvent(EV_IF_STATE_UPDATE){
	
}

MF_EvIfStateUpdate::~MF_EvIfStateUpdate(){
	
}

/*  INTERFACE UPDATE FROM NETWORK MANAGER  */
MF_EvAssocTO::MF_EvAssocTO() : MF_StackEvent(EV_ASSOC_TO){

}

MF_EvAssocTO::~MF_EvAssocTO(){

}



/*  CHUNK COMPLETELY SENT READY TO BE RELEASED  */
MF_EvChkComplete::MF_EvChkComplete() : MF_StackEvent(EV_CHK_SENT){
	
}

MF_EvChkComplete::MF_EvChkComplete(int i, MF_ChunkInfo *c) : MF_StackEvent(EV_CHK_SENT){
	interface = i;
	ci = c;
}

MF_EvChkComplete::~MF_EvChkComplete(){
	
}

/*  Space available in buffer  */
MF_EvSpaceAvail::MF_EvSpaceAvail() : MF_StackEvent(EV_SPACE_AVAIL) {

}

MF_EvSpaceAvail::MF_EvSpaceAvail(unsigned int sm) : MF_StackEvent(EV_SPACE_AVAIL) {
	this->sm = sm;
}

MF_EvSpaceAvail::~MF_EvSpaceAvail() {

}


/*  RECEIVED COMPLETE CHUNK ON AN INTERFACE. DELIVER TO ROUTER  */
MF_EvUpIf::MF_EvUpIf() : MF_StackEvent(EV_UP_IF){
	
}

MF_EvUpIf::MF_EvUpIf(int dev, MF_ChunkInfo *c) : MF_StackEvent(EV_UP_IF){
	ci = c;
	devID = dev;
}

MF_EvUpIf::~MF_EvUpIf(){
	
}



/*  NEW CHUNK FROM ROUTER TO INTERFACE  */
MF_EvDownRout::MF_EvDownRout() : MF_StackEvent(EV_DOWN_ROUT){
	
}

MF_EvDownRout::MF_EvDownRout(MF_ChunkInfo *c, int dev) : MF_StackEvent(EV_DOWN_ROUT){
	ci = c;
	devID = dev;
}

MF_EvDownRout::~MF_EvDownRout(){
	
}



/*  NEW CHUNK FROM TRANSPORT FOR THE ROUTER  */
MF_EvDownTransp::MF_EvDownTransp() : MF_StackEvent(EV_DOWN_TRANSP){
	
}

MF_EvDownTransp::MF_EvDownTransp(MF_ChunkInfo *c) : MF_StackEvent(EV_DOWN_TRANSP){
	ci = c;
}

MF_EvDownTransp::~MF_EvDownTransp(){
	
}



/*  NEW CHUNK FROM ROUTER FOR THE TRANSPORT  */
MF_EvUpRout::MF_EvUpRout() : MF_StackEvent(EV_UP_ROUT){
	
}

MF_EvUpRout::MF_EvUpRout(MF_Transport *t, MF_ChunkInfo *ci) : MF_StackEvent(EV_UP_ROUT) {
	this->t = t;
	this->ci = ci;
}

MF_EvUpRout::~MF_EvUpRout(){
	
}

/* TIMEOUT AT TRANSPORT */
MF_EvTransTOut::MF_EvTransTOut() : MF_StackEvent(EV_TRANS_TO) {

}

MF_EvTransTOut::MF_EvTransTOut(MF_Transport *t, u_char type, pair<u_int, u_int> p) 
      : MF_StackEvent(EV_TRANS_TO), trans(t), timeout_type(type), dstseq_pair(p) {
}

MF_EvTransTOut::~MF_EvTransTOut() {

}


/*  NEW DATA FOR CHUNK  */
MF_EvDownAPI::MF_EvDownAPI() : MF_StackEvent(EV_DOWN_API){
	
}

MF_EvDownAPI::MF_EvDownAPI(MF_Transport *tr, vector<u_char *> *d, u_int s, int sg, int dg, mfflag_t o) : MF_StackEvent(EV_DOWN_API){
	trnsp = tr;
	data = d;
	size = s;
	src_GUID = sg;
	dst_GUID = dg;
	opts = o;
}

MF_EvDownAPI::~MF_EvDownAPI(){
	
}



/*  CHUNK READY FOR SOCKET TO DELIVER TO API (FROM TRANSPORT)  */
MF_EvUpTransp::MF_EvUpTransp() : MF_StackEvent(EV_UP_TRANSP){
	
}

MF_EvUpTransp::MF_EvUpTransp(u_int t, u_int s, int g) : MF_StackEvent(EV_UP_TRANSP){
	size = s;
	dstGUID = g;
	TID = t;
}

MF_EvUpTransp::~MF_EvUpTransp(){
	
}



/*  RELEASE CHUNK IN DEVICE  */
MF_EvRelChk::MF_EvRelChk() : MF_StackEvent(EV_REL_CHK){
	
}

MF_EvRelChk::MF_EvRelChk(int dev, MF_ChunkInfo *ci) : MF_StackEvent(EV_REL_CHK){
	devID = dev;
	this->ci = ci;
}

MF_EvRelChk::~MF_EvRelChk(){
	
}


MF_SocketTOCheck::MF_SocketTOCheck() : MF_StackEvent(EV_SOCKET_TO){

}

MF_SocketTOCheck::MF_SocketTOCheck(unsigned int ID) : MF_StackEvent(EV_SOCKET_TO){
	this->ID = ID;
}

MF_SocketTOCheck::~MF_SocketTOCheck() {

}


MF_EventSocketClean::MF_EventSocketClean() : MF_StackEvent(EV_SOCKET_CLEAN){

}

MF_EventSocketClean::MF_EventSocketClean(unsigned int ID) : MF_StackEvent(EV_SOCKET_CLEAN){
	this->ID = ID;
}

MF_EventSocketClean::~MF_EventSocketClean() {

}
