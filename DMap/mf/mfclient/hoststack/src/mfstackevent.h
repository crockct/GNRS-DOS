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
 * @file   mfstackevent.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   September, 2013
 * @brief  Classes that describe different stack events.
 *
 * Classes that describe different stack events.
 */

#ifndef MF_STACK_EVENT
#define MF_STACK_EVENT

#include <string.h>
#include <semaphore.h>
#include <vector>
#include <time.h>

#include <include/mfflags.h>
#include <include/mfclientipc.h>

#include "mfevent.h"
#include "mfchunkinfo.h"
#include "mftypes.h"
#include "mfbuffer.h"
#include "mfetherproto.h"
#include "mftransport.h"

using namespace std;

typedef enum {
	//API-stack messages
	EV_OPEN = 1,
	EV_SEND,
	EV_RECV,
	EV_CLOSE,
	EV_ATTACH,
	EV_DETACH,
	EV_GET,
	EV_DO_GET,
	EV_GET_RESPONSE,

	//Stack only messages
	EV_IF_STATE_UPDATE,
	EV_ASSOC_TO,
	EV_POLICY_CHANGE,
	EV_CSYN,
	EV_CSYN_ACK,
	EV_CSYN_TIMEOUT,
	EV_LP,
	EV_UP_IF,
	EV_DOWN_ROUT,
	EV_UP_ROUT,
	EV_DOWN_TRANSP,
	EV_DOWN_API,
	EV_UP_TRANSP,
	EV_REL_CHK,
	EV_SPACE_AVAIL,
	EV_CHK_SENT,
	EV_TRANS_TO,
	EV_SOCKET_TO,
	EV_SOCKET_CLEAN
} evType;

class MF_StackEvent : public MFEvent{
	u_int size;

	inline void NullFunction(){};

public:
	MF_StackEvent();
	MF_StackEvent(evType);
	virtual ~MF_StackEvent();

	u_int getSize();
	void setSize(u_int);
};

/*Messages from API*/
/*  OPEN  */
class MF_APIOpen: public MF_StackEvent{
	u_int UID;
	char profile[LEN_PROF];
	MF_IPCOptionsMap opts;
	u_int srcGUID;

public:
	MF_APIOpen();
	MF_APIOpen(struct MsgOpen *);
	virtual ~MF_APIOpen();

	inline u_int getUID(){
		return UID;
	}

	inline void setUID(u_int u){
		UID = u;
	}

	inline char *getProfile(){
		return profile;
	}

	inline void setProfile(char *prof){
		memcpy(profile, prof, LEN_PROF);
	}

	inline MF_IPCOptionsMap &getOpts(){
		return opts;
	}

	inline void setOpts(MF_IPCOptionsMap &o){
		opts = o;
	}

	inline u_int getSrcGUID(){
		return srcGUID;
	}

	inline void setSrcGUID(u_int s){
		srcGUID = s;
	}

};


/*  SEND  */
class MF_APISend: public MF_StackEvent{
	u_int UID;
	u_int reqID;
	u_int mSize;
	int dstGUID;
	mfflag_t opts;

public:
	MF_APISend();
	MF_APISend(struct MsgSend *);
	virtual ~MF_APISend();

	inline u_int getUID(){
		return UID;
	}

	inline void setUID(u_int u){
		UID = u;
	}

	inline mfflag_t getOpts(){
		return opts;
	}

	inline void setOpts(mfflag_t o){
		opts = o;
	}

	inline u_int getReqID(){
		return reqID;
	}

	inline void setReqID(u_int s){
		reqID = s;
	}

	inline u_int getSize(){
		return mSize;
	}

	inline void setSize(u_int s){
		mSize = s;
	}

	inline int getDstGUID(){
		return dstGUID;
	}

	inline void setDstGUID(int s){
		dstGUID = s;
	}
};


/*  RECEIVE  */
class MF_APIRecv: public MF_StackEvent{
	u_int UID;
	u_int reqID;
	char blk;
	u_int mSize;

public:
	MF_APIRecv();
	MF_APIRecv(struct MsgRecv *);
	virtual ~MF_APIRecv();

	inline u_int getUID(){
		return UID;
	}

	inline void setUID(u_int u){
		UID = u;
	}

	inline u_int getReqID(){
		return reqID;
	}

	inline void setReqID(u_int s){
		reqID = s;
	}

	inline u_int getSize(){
		return mSize;
	}

	inline void setSize(u_int s){
		mSize = s;
	}

	inline char getBlk(){
		return blk;
	}

	inline void setBlk(char b){
		blk = b;
	}
};


/*  ATTACH  */
class MF_APIAttach: public MF_StackEvent{
	u_int UID;
	u_int reqID;
	u_int GUIDs[MAX_ATTACH];
	u_int nGUID;

public:
	MF_APIAttach();
	MF_APIAttach(struct MsgAttach *);
	virtual ~MF_APIAttach();

	inline u_int getUID(){
		return UID;
	}

	inline void setUID(u_int u){
		UID = u;
	}

	inline u_int getReqID(){
		return reqID;
	}

	inline void setReqID(u_int s){
		reqID = s;
	}

	inline u_int getNGUID(){
		return nGUID;
	}

	inline void setNGUID(u_int s){
		nGUID = s;
	}

	inline int getGUID(int pos){
		return GUIDs[pos];
	}

	inline void setGUID(int pos, int GUID){
		GUIDs[pos] = GUID;
	}


};


/*  DETACH  */
class MF_APIDetach: public MF_StackEvent{
	u_int UID;
	u_int reqID;
	u_int GUIDs[MAX_ATTACH];
	u_int nGUID;

public:
	MF_APIDetach();
	MF_APIDetach(struct MsgDetach *);
	virtual ~MF_APIDetach();

	inline u_int getUID(){
		return UID;
	}

	inline void setUID(u_int u){
		UID = u;
	}

	inline u_int getReqID(){
		return reqID;
	}

	inline void setReqID(u_int s){
		reqID = s;
	}

	inline u_int getNGUID(){
		return nGUID;
	}

	inline void setNGUID(u_int s){
		nGUID = s;
	}

	inline int getGUID(int pos){
		return GUIDs[pos];
	}

	inline void setGUID(int pos, int GUID){
		GUIDs[pos] = GUID;
	}
};


/*  CLOSE  */
class MF_APIClose: public MF_StackEvent{
	u_int UID;
	u_int reqID;

public:
	MF_APIClose();
	MF_APIClose(struct MsgClose *);
	virtual ~MF_APIClose();

	inline u_int getUID(){
		return UID;
	}

	inline void setUID(u_int u){
		UID = u;
	}

	inline u_int getReqID(){
		return reqID;
	}
};

/*  GET  */
class MF_APIGet: public MF_StackEvent{
	u_int UID;
	u_int reqID;
	u_int mSize;
	int dstGUID;
	mfflag_t opts;

public:
	MF_APIGet();
	MF_APIGet(struct MsgGet *);
	virtual ~MF_APIGet();

	inline u_int getUID(){
		return UID;
	}

	inline void setUID(u_int u){
		UID = u;
	}

	inline mfflag_t getOpts(){
		return opts;
	}

	inline void setOpts(mfflag_t o){
		opts = o;
	}

	inline u_int getReqID(){
		return reqID;
	}

	inline void setReqID(u_int s){
		reqID = s;
	}

	inline u_int getSize(){
		return mSize;
	}

	inline void setSize(u_int s){
		mSize = s;
	}

	inline int getDstGUID(){
		return dstGUID;
	}

	inline void setDstGUID(int s){
		dstGUID = s;
	}
};

/*  DO_GET  */
class MF_APIDoGet: public MF_StackEvent{
	u_int UID;
	u_int reqID;
	u_int mSize;

public:
	MF_APIDoGet();
	MF_APIDoGet(struct MsgDoGet *);
	virtual ~MF_APIDoGet();

	inline u_int getUID(){
		return UID;
	}

	inline void setUID(u_int u){
		UID = u;
	}

	inline u_int getReqID(){
		return reqID;
	}

	inline void setReqID(u_int s){
		reqID = s;
	}

	inline u_int getSize(){
		return mSize;
	}

	inline void setSize(u_int s){
		mSize = s;
	}
};

/*  GET_RESPONSE  */
class MF_APIGetResponse: public MF_StackEvent{
	u_int UID;
	u_int reqID;
	u_int mSize;
	u_int getID;
	u_int srcGUID;
	u_int dstGUID;
	mfflag_t opts;

public:
	MF_APIGetResponse();
	MF_APIGetResponse(struct MsgGetResponse *);
	virtual ~MF_APIGetResponse();

	inline u_int getUID(){
		return UID;
	}

	inline void setUID(u_int u){
		UID = u;
	}

	inline mfflag_t getOpts(){
		return opts;
	}

	inline void setOpts(mfflag_t o){
		opts = o;
	}

	inline u_int getReqID(){
		return reqID;
	}

	inline void setReqID(u_int s){
		reqID = s;
	}

	inline u_int getSize(){
		return mSize;
	}

	inline void setSize(u_int s){
		mSize = s;
	}

	inline int getGetID(){
		return getID;
	}

	inline void setGetID(int s){
		getID = s;
	}

	inline int getSrcGUID(){
		return srcGUID;
	}

	inline void setSrcGUID(int s){
		srcGUID = s;
	}

	inline int getDstGUID(){
		return dstGUID;
	}

	inline void setDstGUID(int s){
		dstGUID = s;
	}
};

/*Messages stack to stack*/

/*  INTERFACE UPDATE FROM NETWORK MANAGER  */
class MF_EvIfStateUpdate : public MF_StackEvent{
	//list <int> enabledIfs;

public:
	MF_EvIfStateUpdate();
	virtual ~MF_EvIfStateUpdate();
};

/*  INTERFACE UPDATE FROM NETWORK MANAGER  */
class MF_EvAssocTO : public MF_StackEvent{
	//list <int> enabledIfs;

public:
	MF_EvAssocTO();
	virtual ~MF_EvAssocTO();
};


/*  CSYN RECEIVED  */
class MF_EvCysn : public MF_StackEvent{
	int interface;
	u_int hop_ID;
	u_int pkt_count;

public:
	MF_EvCysn();
	MF_EvCysn(struct csyn *pkt, int interface);
	virtual ~MF_EvCysn();

	inline int getInterface(){
		return interface;
	}

	inline u_int getHopID(){
		return hop_ID;
	}

	inline u_int getPktCount(){
		return pkt_count;
	}
};


/*  CSYN ACK RECEIVED  */
class MF_EvCysnAck : public MF_StackEvent{
	int interface;
	u_int hop_ID;
	u_int pkt_count;
	u_char *bitmap;

public:
	MF_EvCysnAck();
	MF_EvCysnAck(struct csyn_ack *ack, int interface);
	virtual ~MF_EvCysnAck();

	inline int getInterface(){
		return interface;
	}

	inline u_int getHopID(){
		return hop_ID;
	}

	inline u_int getPktCount(){
		return pkt_count;
	}

	inline u_char *getBitmap(){
		return bitmap;
	}
};


/*  NEW LINK PROBE MESSAGE RECEIVED  */
class MF_EvLinkProb : public MF_StackEvent{
	int devID;
	char ether_dhost[18]; //ether source
	unsigned long s_addr; //ip source
	//lp content
	u_int sourceLP;
	u_int seqNum;
	time_t timestamp;

public:
	MF_EvLinkProb();
	MF_EvLinkProb(int, const char*, unsigned long, u_int, u_int, time_t);
	virtual ~MF_EvLinkProb();

	inline int getDevID(){
		return devID;
	}
	inline char *getEth(){
		return ether_dhost;
	}

	inline unsigned long getIP(){
		return s_addr;
	}

	inline u_int getSLP(){
		return sourceLP;
	}

	inline time_t getTimestamp(){
		return timestamp;
	}

	inline u_int getSeqNum(){
		return seqNum;
	}
};


/*  CSYN TIMEOUT  */
class MF_EvCysnTOut : public MF_StackEvent{
	int interface;
	u_int chnkID;

public:
	MF_EvCysnTOut();
	MF_EvCysnTOut(int, u_int);
	virtual ~MF_EvCysnTOut();

	inline int getInterface(){
		return interface;
	}

	inline u_int getChunkID(){
		return chnkID;
	}
};


/*  Message completely sent  */
class MF_EvChkComplete : public MF_StackEvent{
	MF_ChunkInfo *ci;
	int interface;

public:
	MF_EvChkComplete();
	MF_EvChkComplete(int, MF_ChunkInfo *);
	virtual ~MF_EvChkComplete();

	inline int getInterface(){
		return interface;
	}

	inline MF_ChunkInfo * getChunkInfo(){
		return ci;
	}
};

/*  Space available in buffer  */
class MF_EvSpaceAvail : public MF_StackEvent{
	unsigned int sm;
public:
	MF_EvSpaceAvail();
	MF_EvSpaceAvail(unsigned int );
	virtual ~MF_EvSpaceAvail();

	inline unsigned int getID(){
		return sm;
	}
};

/*
 * Name convention for data flow:
 *		the event carries the name of the layer its data is coming from and in which direction (up or down)
 */


/*  RECEIVED COMPLETE CHUNK ON AN INTERFACE. DELIVER TO ROUTER  */
class MF_EvUpIf : public MF_StackEvent{
	int devID;
	MF_ChunkInfo *ci;

public:
	MF_EvUpIf();
	MF_EvUpIf(int, MF_ChunkInfo *);
	virtual ~MF_EvUpIf();

	inline int getDevID(){
		return devID;
	}

	inline MF_ChunkInfo *getChunkInfo(){
		return ci;
	}
};


/*  NEW CHUNK FROM ROUTER TO INTERFACE  */
class MF_EvDownRout : public MF_StackEvent{
	MF_ChunkInfo *ci;
	int devID;

public:
	MF_EvDownRout();
	MF_EvDownRout(MF_ChunkInfo *, int);
	virtual ~MF_EvDownRout();

	inline MF_ChunkInfo *getChunkInfo(){
		return ci;
	}

	inline int getDevID(){
		return devID;
	}
};


/*  NEW CHUNK FROM TRANSPORT FOR THE ROUTER  */
class MF_EvDownTransp : public MF_StackEvent{
	MF_ChunkInfo *ci;
	mfflag_t opts;

public:
	MF_EvDownTransp();
	MF_EvDownTransp(MF_ChunkInfo *);
	virtual ~MF_EvDownTransp();

	inline MF_ChunkInfo *getChunkInfo(){
		return ci;
	}

	inline mfflag_t getOpts(){
		return opts;
	}
};


class MF_Transport;
/*  NEW CHUNK FROM ROUTER FOR THE TRANSPORT  */
class MF_EvUpRout : public MF_StackEvent{
	MF_Transport *t;
	MF_ChunkInfo *ci;

public:
	MF_EvUpRout();
	MF_EvUpRout(MF_Transport *t, MF_ChunkInfo *ci);
	virtual ~MF_EvUpRout();

	inline MF_Transport *getTransport(){
		return t;
	}

	inline MF_ChunkInfo *getChunkInfo(){
		return ci;
	}

};

class MF_EvTransTOut : public MF_StackEvent {
	MF_Transport *trans;
	u_char timeout_type;
	pair<u_int, u_int> dstseq_pair;

public: 
	MF_EvTransTOut();
	MF_EvTransTOut(MF_Transport *t, u_char type, pair<u_int, u_int> p);
	virtual ~MF_EvTransTOut();

	inline MF_Transport *getTransport() { return trans; }
	inline u_char getTOutType() { return timeout_type; }
	inline pair<u_int, u_int> getDstSeqPair() { return dstseq_pair; }
};


/*  NEW DATA FOR CHUNK  */
class MF_EvDownAPI : public MF_StackEvent{
	MF_Transport *trnsp;
	vector<u_char *> *data;
	u_int size;
	int src_GUID;
	int dst_GUID;
	mfflag_t opts;

	//something pointing to the new data

public:
	MF_EvDownAPI();
	MF_EvDownAPI(MF_Transport *, vector<u_char *> *, u_int, int, int, mfflag_t);
	virtual ~MF_EvDownAPI();

	inline MF_Transport *getTransport(){
		return trnsp;
	}

	inline vector<u_char *> *getData(){
		return data;
	}

	inline u_int getSize(){
		return size;
	}

	inline int getDstGUID(){
		return dst_GUID;
	}

	inline int getSrcGUID(){
		return src_GUID;
	}

	inline mfflag_t getOpts(){
		return opts;
	}
};


/*  MSG READY FOR SOCKET TO DELIVER TO API (FROM TRANSPORT)  */
class MF_EvUpTransp : public MF_StackEvent{
	u_int TID;
	u_int size;
	int dstGUID;

public:
	MF_EvUpTransp();
	MF_EvUpTransp(u_int, u_int, int);
	virtual ~MF_EvUpTransp();

	inline u_int getTID(){
		return TID;
	}

	inline u_int getSize(){
		return size;
	}

	inline int getDstGUID(){
		return dstGUID;
	}

};



/*  RELEASE CHUNK IN DEVICE  */
class MF_EvRelChk : public MF_StackEvent{
	int devID;
	MF_ChunkInfo *ci;

public:
	MF_EvRelChk();
	MF_EvRelChk(int dev, MF_ChunkInfo *ci);
	virtual ~MF_EvRelChk();
};

/*  TO happened for socket  */
class MF_SocketTOCheck : public MF_StackEvent{
	unsigned int ID;

public:
	MF_SocketTOCheck();
	MF_SocketTOCheck(unsigned int ID);
	virtual ~MF_SocketTOCheck();

	unsigned int getId() const {
		return ID;
	}

	void setId(unsigned int id) {
		ID = id;
	}
};

/*  Clean requesting socket  */
class MF_EventSocketClean : public MF_StackEvent{
	unsigned int ID;

public:
	MF_EventSocketClean();
	MF_EventSocketClean(unsigned int ID);
	virtual ~MF_EventSocketClean();

	unsigned int getId() const {
		return ID;
	}

	void setId(unsigned int id) {
		ID = id;
	}
};

#endif /* MF_STACK_EVENT */
