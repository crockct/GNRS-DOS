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
 * @file   mfchunkinfo.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu), czhang
 * @date   July, 2011
 * @brief  Temp.
 *
 * Temp.
 */

#include "mfchunkinfo.h"

MF_ChunkInfo::MF_ChunkInfo() {
	mDstNA = 0; // for network layer, not handled by client, default to 0
	mSrcGUID = 0; // read from file
	mDstGUID = 0;//routing, get from scheme or mfsendto is used
	mServiceID = 0; //routing
	mChunkID = 0; //transport
	mChunkSize = 0; //transport/hop
	mChunkPktCnt = 0; //transport
	mSrcAppID = 0;
	mDstAppID = 0;
	startOffset = 0;
	endOffset = 0;
	mMsgID = 0;
	mHopID = -1; //serve both send and receive for different usage
	mIsChunkDelivered = false;
	mIsReadyForRelease = false;
}

MF_ChunkInfo::MF_ChunkInfo(u_int chkCount) : mPacketList(chkCount){
	mDstNA = 0; // for network layer, not handled by client, default to 0
	mSrcGUID = 0; // read from file
	mDstGUID = 0;//routing, get from scheme or mfsendto is used
	mServiceID = 0; //routing
	mChunkID = 0; //transport
	mChunkSize = 0; //transport/hop
	mChunkPktCnt = 0; //transport
	mSrcAppID = 0;
	mDstAppID = 0;
	mMsgID = 0;
	startOffset = 0;
	endOffset = 0;
	mHopID = -1; //serve both send and receive for different usage
	mIsChunkDelivered = false;
	mIsReadyForRelease = false;
}

MF_ChunkInfo::~MF_ChunkInfo() {
	
}

void MF_ChunkInfo::reset() {
	mDstNA = 0;
	mSrcGUID = 0;
	mDstGUID = 0;//routing, get from scheme or mfsendto is used
	mServiceID = 0; //routing
	mChunkID = 0; //transport
	mChunkSize = 0; //transport
	mSrcAppID = 0;
	mDstAppID = 0;
	mMsgID = 0;
	startOffset = 0;
	endOffset = 0;
	mIsChunkDelivered = false;
	mPacketList.clear();
}

void MF_ChunkInfo::resetReceived() {
	//TODO for now is not implemented
}

u_int MF_ChunkInfo::getPacketCount() {
	return mPacketList.size();
}

u_int MF_ChunkInfo::getChunkPktCnt() {
	return mChunkPktCnt;
}

void MF_ChunkInfo::putChunkPktCnt(u_int cnt) {
	mChunkPktCnt = cnt;
}

void MF_ChunkInfo::putSrcGUID(u_int id) {
	mSrcGUID = id;
}

u_int MF_ChunkInfo::getSrcGUID() {
	return mSrcGUID;
}

void MF_ChunkInfo::putDstGUID(u_int id) {
	mDstGUID = id;
}

u_int MF_ChunkInfo::getDstGUID() {
	return mDstGUID;
}

void MF_ChunkInfo::putServiceID(u_int id) {
	mServiceID = mServiceID | id;
}

u_int MF_ChunkInfo::getServiceID() {
	return mServiceID;
}

void MF_ChunkInfo::putChunkID(u_int id) {
	mChunkID = id;
}

u_int MF_ChunkInfo::getChunkID() {
	return mChunkID;
}

void MF_ChunkInfo::putChunkSize(u_int size) {
	mChunkSize = size;
}

u_int MF_ChunkInfo::getChunkSize() {
	return mChunkSize;
}

void MF_ChunkInfo::putDstNA(u_int na) {
	mDstNA = na;
}

u_int MF_ChunkInfo::getDstNA() {
  return mDstNA;
}

void MF_ChunkInfo::putHopID(u_int id) {
	mHopID = id;
}

u_int MF_ChunkInfo::getHopID() {
	return mHopID;
}

void MF_ChunkInfo::putSrcTID(u_int id) {
	mSrcAppID = id;
}

u_int MF_ChunkInfo::getSrcTID() {
	return mSrcAppID;
}

void MF_ChunkInfo::putDstTID(u_int id) {
	mDstAppID = id;
}

u_int MF_ChunkInfo::getDstTID() {
	return mDstAppID;
}

void MF_ChunkInfo::putMsgID(u_int id) {
	mMsgID = id;
}

u_int MF_ChunkInfo::getMsgID() {
	return mMsgID;
}

void MF_ChunkInfo::putStartOffset(u_short o){
	startOffset = o;
}

u_short MF_ChunkInfo::getStartOffset(){
	return startOffset;
}

void MF_ChunkInfo::putEndOffset(u_short o){
	endOffset = o;
}

u_short MF_ChunkInfo::getEndOffset(){
	return endOffset;
}

bool MF_ChunkInfo::isDelivered(){
	return mIsChunkDelivered;
}

void MF_ChunkInfo::setDelivered(bool val){
	mIsChunkDelivered = val;
}

bool MF_ChunkInfo::isReleased(){
	return mIsReadyForRelease;
}

void MF_ChunkInfo::setReleased(bool val){
	mIsReadyForRelease = val;
}

void MF_ChunkInfo::putPktList(vector<u_char*> *v){
	for(u_int i = 0; i<mChunkPktCnt; i++){
		mPacketList.push_back((*v)[i]);
	}
}

vector<u_char *> *MF_ChunkInfo::getPacketList(){
	return &mPacketList;
}

u_int MF_ChunkInfo::getOwner(){
	return owner;
}
void MF_ChunkInfo::setOwner(u_int o){
	owner = o;
}

mfflag_t MF_ChunkInfo::getOpts(){
	return opts;
}

bool MF_ChunkInfo::isMultihoming(){
	return isMFFlagSet(opts, MF_MULTIHOME);
}

bool MF_ChunkInfo::isMulticast(){
	return isMFFlagSet(opts, MF_MULTICAST);
}

bool MF_ChunkInfo::isBroadcast(){
	return isMFFlagSet(opts, MF_BROADCAST);
}

bool MF_ChunkInfo::isAnycast(){
	return isMFFlagSet(opts, MF_ANYCAST);
}

bool MF_ChunkInfo::isContentRequest(){
	return isMFFlagSet(opts, MF_CONTENT_REQUEST);
}

bool MF_ChunkInfo::isContentResponse(){
	return isMFFlagSet(opts, MF_CONTENT_RESPONSE);
}

void MF_ChunkInfo::setOpts(mfflag_t o){
	opts = o;
}
