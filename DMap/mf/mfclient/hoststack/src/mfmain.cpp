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
 * @file   mfmain.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu), czhang
 * @date   July, 2011
 * @brief  Temp.
 *
 * Temp.
 */

#include "mfsocketfactory.h"
#include "mfmain.h"

void threadExitHandler(int sig) {
	//printf("threadExitHandler(): thead id = %d\n", pthread_self());
	//pthread_exit(0);
}

MF_Main::MF_Main() : _system() {
	_system.setTimeoutManager(new MFTimeoutManager());
	_system.setEventQueue(new MF_EventQueue(&_system));
	_system.setInterfaceManager(new MF_InterfaceManager(&_system));
	_system.setSocketServer(new MF_Server());
	_system.setSocketTable(new MF_SocketTable());
	_system.setRouter(new MF_Router(&_system));
	cont = true;

}

#ifdef __OML__

MF_Main::MF_Main( MF_OML* oml_mps) : _system(){
	_system.setTimeoutManager(new MFTimeoutManager());
	_system.setEventQueue(new MF_EventQueue(&_system));
	_system.setInterfaceManager(new MF_InterfaceManager(&_system));
	_system.setSocketServer(new MF_Server());
	_system.setSocketTable(new MF_SocketTable());
	_system.setRouter(new MF_Router(&_system));
	_system.setOmlMps(oml_mps);
	cont = true;
}
#endif

MF_Main::~MF_Main() {

}

void MF_Main::start(MF_Settings *setts) {
	/*
	 * Start network manager thread
	 */
	MF_Log::mf_log(MF_DEBUG, "MF_Main::start(): starting stack");
	_system.setSettings(setts);
	_system.getInterfaceManager()->setFromSettings();
	_system.getRouter()->setPolicy(_system.getSettings()->getPolicy());
	_system.getRouter()->setDefaultGUID(_system.getSettings()->getDefaultGUID());
	_system.getInterfaceManager()->start();
	if(_system.getEventQueue()->startListener()){
		cont = false;
	}
	if(_system.getTimeoutManager()->StartManager()){
		cont = false;
	}
	_system.getRouter()->init();
	MF_StackEvent *temp;
	while (cont) {
		temp = _system.getEventQueue()->getNext();
		if(temp){
			handleMessage(temp);
		}
	}
}

/*
 * Just a big switch to properly handle each event type.
 * NOTE: after letting the function treat the event, this will be deleted. If the event has to be maintained, it should be copied
 */
void MF_Main::handleMessage(MF_StackEvent *event) {
	switch (event->m_type()) {
	//TODO Order cases by probability to increase performance...
	case EV_OPEN: //from API
		handleOpen((MF_APIOpen *)event);
		break;
	case EV_SEND: //from API
		handleSend((MF_APISend *)event);
		break;
	case EV_RECV: //from API
		handleRecv((MF_APIRecv *)event);
		break;
	case EV_GET: //from API
		handleGet((MF_APIGet *)event);
		break;
	case EV_DO_GET: //from API
		handleDoGet((MF_APIDoGet *)event);
		break;
	case EV_GET_RESPONSE: //from API
		handleGetResponse((MF_APIGetResponse *)event);
		break;
	case EV_ATTACH: //from API
		handleAttach((MF_APIAttach *)event);
		break;
	case EV_DETACH: //from API
		handleDetach((MF_APIDetach *)event);
		break;
	case EV_CLOSE: //from API
		handleClose((MF_APIClose *)event);
		break;
	case EV_CSYN_ACK: //from DeviceManager, (for send)
		handleCsynAck((MF_EvCysnAck *)event);
		break;
	case EV_CSYN: //from DeviceManager, (for send)
		handleCsyn((MF_EvCysn *)event);
		break;
	case EV_CSYN_TIMEOUT: //from Timer thread
		handleCsynTimeout((MF_EvCysnTOut *)event);
		break;
	case EV_IF_STATE_UPDATE: //from NetworkManager thread
		handleIfStateUpdate((MF_EvIfStateUpdate *)event);
		break;
	case EV_ASSOC_TO: //from NetworkManager thread
		handleAssocTO((MF_EvAssocTO *)event);
		break;
	case EV_LP:
		handleLinkProbe((MF_EvLinkProb *)event);
		break;
	case EV_DOWN_API:
		handleDownAPI((MF_EvDownAPI *)event);
		break;
	case EV_DOWN_TRANSP:
		handleDownTransp((MF_EvDownTransp *)event);
		break;
	case EV_DOWN_ROUT:
		handleDownRout((MF_EvDownRout *)event);
		break;
	case EV_UP_IF:
		handleUpIf((MF_EvUpIf *)event);
		break;
	case EV_UP_ROUT:
		handleUpRout((MF_EvUpRout *)event);
		break;
	case EV_UP_TRANSP:
		handleUpTransp((MF_EvUpTransp *)event);
		break;
	case EV_CHK_SENT:
		handleChkSent((MF_EvChkComplete *)event);
		break;
	case EV_TRANS_TO:
		handleTransTOut((MF_EvTransTOut *)event);
		break;
	case EV_SPACE_AVAIL:
		handleSpaceAvail((MF_EvSpaceAvail *)event);
		break;
	case EV_SOCKET_TO:
		handleSocketTO((MF_SocketTOCheck*)event);
		break;
	case EV_SOCKET_CLEAN:
		handleSocketClean((MF_EventSocketClean *)event);
		break;
	default:
		MF_Log::mf_log(MF_WARN, "MF_Main::handleMessage(): wrong message type...");
	}
	delete event;
}

/********************************
 Operations to handle the communication with the API
********************************/


/*
 * Handle open request
 */
void MF_Main::handleOpen(MF_APIOpen *event) {
	MF_Log::mf_log(MF_DEBUG, "MF_Main:handleOpen");
	//MF_Log::mf_log_time("MF_Main:handleOpen start of event");
	MF_SocketManager *mSM = _system.getSocketTable()->getById(event->getUID());
	if(mSM!=NULL && mSM->isOpen()){
		MF_Log::mf_log(MF_WARN, "MF_Main:handleOpen An app with UID %lu already exists\n",event->getUID());
		struct MsgOpenReply rcR;
		rcR.type = MSG_TYPE_OPEN_REPLY;
		rcR.retValue = STACK_OPEN_UID_EXISTS;
		_system.getSocketServer()->send((char*)&rcR, sizeof(struct MsgSendReply), event->getUID());
	}
	else{
		//TODO: Add resource management to check if there are enough resources
		mSM = MF_SocketFactory::getInstance( &_system,
				event->getProfile(), event->getUID(), _system.getSettings()->getDefaultBufferSize(),
				_system.getSettings()->getTransSetting());
		if(!mSM->init()) {
			MF_Log::mf_log(MF_WARN, "MF_Main:handleOpen Socket %lu initialization failed\n",event->getUID());
			return;
		}
		if(mSM->openRequest(event)){
			MF_Log::mf_log(MF_WARN, "MF_Main:handleOpen Socket %lu open request failed\n",event->getUID());
			return;
		}
	}
	//MF_Log::mf_log_time("MF_Main:handleOpen end of event");
}


/*
 * Handle a receive request from the API level
 */
void MF_Main::handleRecv(MF_APIRecv *event) {
	MF_Log::mf_log(MF_DEBUG, "MF_Main:handleRecv");
	//MF_Log::mf_log_time("MF_Main:handleRecv start of event");
	MF_SocketManager *mSM = _system.getSocketTable()->getById(event->getUID());
	if(mSM == NULL || !mSM->isOpen()){
		MF_Log::mf_log(MF_DEBUG, "MF_Main:handleRecv no socket manager for this UID");
		struct MsgRecvReply rcR;
		rcR.type = MSG_TYPE_RECV_REPLY;
		rcR.retValue = STACK_OPEN_UID_NEXISTS;
		rcR.reqID = event->getReqID();
		_system.getSocketServer()->send((char*)&rcR, sizeof(struct MsgSendReply), event->getUID());
	}
	else{
		mSM->recvRequest(event);
	}
	//MF_Log::mf_log_time("MF_Main:handleRecv start of event");
}


/*
 * Handles a send request from the API level
 */
void MF_Main::handleSend(MF_APISend *event) {
	//MF_Log::mf_log_time("MF_Main:handleSend SEND_TIME request number [%lu] INIT",event->getReqID());
	MF_SocketManager *mSM = _system.getSocketTable()->getById(event->getUID());
	if(mSM == NULL || !mSM->isOpen()){
		struct MsgSendReply rsS;
		rsS.type = MSG_TYPE_SEND_REPLY;
		rsS.retValue = STACK_OPEN_UID_NEXISTS;
		rsS.reqID = event->getReqID();
		_system.getSocketServer()->send((char*)&rsS, sizeof(struct MsgSendReply), event->getUID());
	}
	else{
		//TODO
		mSM->sendRequest(event);
	}
	//MF_Log::mf_log_time("MF_Main:handleSend SEND_TIME request number [%lu] END",event->getReqID());
}

/*
 * Handles a get request from the API level
 */
void MF_Main::handleGet(MF_APIGet *event) {
	//MF_Log::mf_log_time("MF_Main:handleGet GET_TIME request number [%lu] INIT",event->getReqID());
	MF_SocketManager *mSM = _system.getSocketTable()->getById(event->getUID());
	if(mSM == NULL || !mSM->isOpen()){
		struct MsgGetReply rsS;
		rsS.type = MSG_TYPE_GET_SEND_REPLY;
		rsS.retValue = STACK_OPEN_UID_NEXISTS;
		rsS.reqID = event->getReqID();
		_system.getSocketServer()->send((char*)&rsS, sizeof(struct MsgGetReply), event->getUID());
	}
	else{
		//TODO
		mSM->getRequest(event);
	}
	//MF_Log::mf_log_time("MF_Main:handleGet GET_TIME request number [%lu] END",event->getReqID());
}

/*
 * Handle a do get request from the API level
 */
void MF_Main::handleDoGet(MF_APIDoGet *event) {
	MF_Log::mf_log(MF_DEBUG, "MF_Main:handleDoGet");
	//MF_Log::mf_log_time("MF_Main:handleRecv start of event");
	MF_SocketManager *mSM = _system.getSocketTable()->getById(event->getUID());
	if(mSM == NULL || !mSM->isOpen()){
		MF_Log::mf_log(MF_DEBUG, "MF_Main:handleDoGet no socket manager for this UID");
		struct MsgDoGetReply rcR;
		rcR.type = MSG_TYPE_DO_GET_REPLY;
		rcR.retValue = STACK_OPEN_UID_NEXISTS;
		rcR.reqID = event->getReqID();
		_system.getSocketServer()->send((char*)&rcR, sizeof(struct MsgDoGetReply), event->getUID());
	}
	else{
		//TODO
		mSM->doGetRequest(event);
	}
	//MF_Log::mf_log_time("MF_Main:handleRecv start of event");
}

/*
 * Handles a get response from the API level
 */
void MF_Main::handleGetResponse(MF_APIGetResponse *event) {
	//MF_Log::mf_log_time("MF_Main:handleGetResponse GET_RESPONSE_TIME request number [%lu] INIT",event->getReqID());
	MF_SocketManager *mSM = _system.getSocketTable()->getById(event->getUID());
	if(mSM == NULL || !mSM->isOpen()){
		struct MsgGetResponseReply rsS;
		rsS.type = MSG_TYPE_GET_RESPONSE_REPLY;
		rsS.retValue = STACK_OPEN_UID_NEXISTS;
		rsS.reqID = event->getReqID();
		_system.getSocketServer()->send((char*)&rsS, sizeof(struct MsgGetResponseReply), event->getUID());
	}
	else{
		//TODO
		mSM->getResponseRequest(event);
	}
	//MF_Log::mf_log_time("MF_Main:handleGetResponse GET_RESPONSE_TIME request number [%lu] END",event->getReqID());
}

/*
 * Handle attach requests from the API
 */
void MF_Main::handleAttach(MF_APIAttach *event) {
	MF_Log::mf_log(MF_DEBUG, "MF_Main: handleAttach");
	//MF_Log::mf_log_time("MF_Main:handleAttach start of event");
	MF_SocketManager *mSM = _system.getSocketTable()->getById(event->getUID());
	if(mSM == NULL || !mSM->isOpen()){
		struct MsgAttachReply mar;
		mar.type = MSG_TYPE_ATTACH_REPLY;
		mar.UID = event->getUID();
		mar.reqID = event->getReqID();
		mar.retValue = STACK_OPEN_UID_NEXISTS;
		_system.getSocketServer()->send((char*)&mar, sizeof(struct MsgAttachReply), event->getUID());
	}
	else{
		mSM->attachRequest(event);
	}
	//MF_Log::mf_log_time("MF_Main:handleAttach end of event");
}

/*
 * Handle detach requests from the API
 */
void MF_Main::handleDetach(MF_APIDetach *event) {
	MF_Log::mf_log(MF_DEBUG, "MF_Main:handleDetach");
	//MF_Log::mf_log_time("MF_Main:handleDetach start of event");
	MF_SocketManager *mSM = _system.getSocketTable()->getById(event->getUID());
	if(mSM == NULL || !mSM->isOpen()){
		struct MsgDetachReply mdr;
		mdr.type = MSG_TYPE_DETACH_REPLY;
		mdr.UID = event->getUID();
		mdr.reqID = event->getReqID();
		mdr.retValue = STACK_OPEN_UID_NEXISTS;
		_system.getSocketServer()->send((char*)&mdr, sizeof(struct MsgDetachReply), event->getUID());
	}
	else{
		mSM->detachRequest(event);
	}
	//MF_Log::mf_log_time("MF_Main:handleDetach end of event");
}

/*
 * Close the socket
 */
void MF_Main::handleClose(MF_APIClose *event) {
	MF_Log::mf_log(MF_DEBUG, "MF_Main:handleClose");
	//MF_Log::mf_log_time("MF_Main:handleClose start of event");
	MF_SocketManager *mSM = _system.getSocketTable()->getById(event->getUID());
	if(mSM == NULL || !mSM->isOpen()){
		struct MsgCloseReply cloR;
		cloR.type = MSG_TYPE_CLOSE_REPLY;
		cloR.UID = event->getUID();
		cloR.reqID = event->getReqID();
		cloR.retValue = STACK_OPEN_UID_NEXISTS;
		_system.getSocketServer()->send((char *)&cloR, sizeof(struct MsgCloseReply), event->getUID());
	}
	else{
		mSM->closeRequest(event);
	}
	//MF_Log::mf_log_time("MF_Main:handleClose end of event");
}



/********************************
 Operations to handle the events from lower layers (network interfaces)
********************************/


void MF_Main::handleIfStateUpdate(MF_EvIfStateUpdate *event) {
	MF_Log::mf_log(MF_DEBUG, "MF_Main:handleIfStateUpdate");
	//MF_Log::mf_log_time("MF_Main:handleIfStateUpdate start of event");
	_system.getRouter()->sendAssocReqs();
	//MF_Log::mf_log_time("MF_Main:handleIfStateUpdate end of event");
}

void MF_Main::handleAssocTO(MF_EvAssocTO *event) {
	MF_Log::mf_log(MF_DEBUG, "MF_Main:handleAssocTO");
	//MF_Log::mf_log_time("MF_Main:handleIfStateUpdate start of event");
	_system.getRouter()->sendAssocReqs();
	//MF_Log::mf_log_time("MF_Main:handleIfStateUpdate end of event");
}

/*
 * Handles a csyn
 */
void MF_Main::handleCsyn(MF_EvCysn *event) {
	MF_Log::mf_log(MF_DEBUG, "MF_Main:handleCsyn");
	MF_DeviceManager *dm = _system.getInterfaceManager()->getByIndex(event->getInterface());
	if(dm!=NULL && dm->isActive()){
		dm->handleCsyn(event->getHopID(), event->getPktCount());
	}
	else{
		MF_Log::mf_log(MF_INFO, "MF_Main::handleCsyn the interface specified is not available");
	}
}

/*
 * Handles a csyn ack
 */
void MF_Main::handleCsynAck(MF_EvCysnAck *event) {
	//MF_Log::mf_log_time("MF_Main:handleCsynAck SEND_CSYN END ack received for interface %d", event->getInterface());
	MF_DeviceManager *dm = _system.getInterfaceManager()->getByIndex(event->getInterface());
	if(dm!=NULL && dm->isActive()){
		dm->handleCsynAck(event->getHopID(), event->getPktCount(), event->getBitmap());
	}
	else{
		MF_Log::mf_log(MF_INFO, "MF_Main:handleCsynAck the interface specified is not available");
	}
}

/*
 * Handles a message generated when a csyn message has timed out
 */
void MF_Main::handleCsynTimeout(MF_EvCysnTOut *event) {
	MF_Log::mf_log(MF_DEBUG, "MF_Main:handleCsynTimeout");
	MF_DeviceManager *dm = _system.getInterfaceManager()->getByIndex(event->getInterface());
	//TODO Manage possible switch of interface
	if(dm!=NULL){
		dm->handleCsynTimeOut(event->getChunkID());
	}
}


/*
 * Handles a new Link Probe from an interface
 */
void MF_Main::handleLinkProbe(MF_EvLinkProb *event){
	MF_Log::mf_log(MF_DEBUG, "MF_Main:handleLinkProbe");
	//MF_Log::mf_log_time("MF_Main:handleLinkProbe start of event");
	MF_DeviceManager *dm = _system.getInterfaceManager()->getByIndex(event->getDevID());
	if(dm==NULL){
		MF_Log::mf_log(MF_INFO, "MF_Main:handleLinkProbe no device with if number %d", event->getDevID());
		return;
	}
	string mac((const char*)event->getEth());
	struct in_addr addr;
	addr.s_addr = event->getIP();
	string ip(inet_ntoa(addr));
	if(dm->updateProb(mac, ip, event->getTimestamp())){
		dm->sendLinkProb(_system.getRouter()->getDefaultGUID(), event->getSLP(), event->getSeqNum());
	}
	//MF_Log::mf_log_time("MF_Main:handleLinkProbe end of event");
}

void MF_Main::handleChkSent(MF_EvChkComplete *event){
	//MF_Log::mf_log_time("MF_Main:handleChkSent TRANSMIT_TIME END chunk %lu interface %d", event->getChunkInfo()->getChunkID(), event->getInterface());
	MF_SocketManager *sm = _system.getSocketTable()->getByTID(event->getChunkInfo()->getOwner());
	if(sm==NULL){
		MF_Log::mf_log(MF_DEBUG, "MF_Main:handleChkSent chunk for transport that doesn't exist\n");
		//TODO free the memory of the chunk or will be lost unused
		return;
	}
	MF_Transport *t = sm->getTransportByTID(0);
	t->releaseChunk(event->getChunkInfo());
}

void MF_Main::handleSpaceAvail(MF_EvSpaceAvail *event) {
	MF_SocketManager *sm = _system.getSocketTable()->getByTID(event->getID());
	if(sm==NULL){
		MF_Log::mf_log(MF_DEBUG, "MF_Main:handleSpaceAvail SocketManager doesn't exist\n");
		return;
	}
	sm->newSpaceAvail();
}


/*
 * Handles a message that says that a new chunk has been received
 */
void MF_Main::handleUpIf(MF_EvUpIf *event){
	//MF_Log::mf_log_time("MF_Main:handleUpIf TRAVERSE_UP_TIME INIT");
	//MF_Log::mf_log_time("MF_Main:handleUpIf ROUTER_UP_TIME INIT");
	bool ret = _system.getRouter()->procRecv(event->getChunkInfo());
	if(ret){
		MF_DeviceManager *mdm = _system.getInterfaceManager()->getByIndex((int)event->getChunkInfo()->getOwner());
		if(mdm!=NULL){
			mdm->releaseChunk(event->getChunkInfo()->getHopID());
		}
	}
	//MF_Log::mf_log_time("MF_Main:handleUpIf ROUTER_UP_TIME END");
}


/*
 * From Router to Transport
 */
void MF_Main::handleUpRout(MF_EvUpRout *event){
	//MF_Log::mf_log_time("MF_Main:handleUpRout TRANSPORT_UP_TIME INIT");
	MF_Transport *t = event->getTransport();
	if(t == NULL){
		MF_Log::mf_log(MF_WARN, "MF_Main:handleUpRout the transport does not exist");
		//MF_Log::mf_log_time("MF_Main:handleUpRout end of event");
		return;
	}
	t->recvData(event->getChunkInfo());
	MF_DeviceManager *mdm = _system.getInterfaceManager()->getByIndex((int)event->getChunkInfo()->getOwner());
	if(mdm!=NULL){
		mdm->releaseChunk(event->getChunkInfo()->getHopID());
	}
	//MF_Log::mf_log_time("MF_Main:handleUpRout TRANSPORT_UP_TIME END");
}


/*
 * From Transport to API
 */
void MF_Main::handleUpTransp(MF_EvUpTransp *event){
	//MF_Log::mf_log_time("MF_Main:handleUpTransp RECV_TIME INIT");
	MF_SocketManager *sm = _system.getSocketTable()->getByTID(event->getTID());
	if(!sm){
		MF_Log::mf_log(MF_DEBUG, "MF_Main: chunk for transport that doesn't exist\n");
		//MF_Log::mf_log_time("MF_Main:handleUpTransp end of event");
		return;
	}
	//add chunk to available ones for the socket manager
	//other option is to keep them in the transport and get them once they can be transferd up to the api
	//TODO
	sm->msgAvailable(event);
	//MF_Log::mf_log_time("MF_Main:handleUpTransp RECV_TIME END");
}


/*
 * From API to Transport
 */
void MF_Main::handleDownAPI(MF_EvDownAPI *event){
	//MF_Log::mf_log_time("MF_Main:handleDownAPI TRAVERSE_DOWN_TIME INIT");
	//MF_Log::mf_log_time("MF_Main:handleDownAPI TRANSPORT_DOWN_TIME INIT");
	MF_Transport *t = event->getTransport();
	t->sendData(event->getData(), event->getSize(), event->getSrcGUID(), event->getDstGUID(), event->getOpts());
	//MF_Log::mf_log_time("MF_Main:handleDownAPI TRANSPORT_DOWN_TIME END");
}


/*
 * From Transport to Router
 */
void MF_Main::handleDownTransp(MF_EvDownTransp *event){
	//MF_Log::mf_log_time("MF_Main:handleDownTransp ROUTER_DOWN_TIME INIT");
	_system.getRouter()->procSend(event->getChunkInfo());
	//MF_Log::mf_log_time("MF_Main:handleDownTransp ROUTER_DOWN_COMPLETE END");
	//MF_Log::mf_log_time("MF_Main:handleDownTransp TRAVERSE_DOWN_TIME END");
}


/*
 * From Router to Interface
 */
void MF_Main::handleDownRout(MF_EvDownRout *event){
	//MF_Log::mf_log_time("MF_Main:handleDownRout TRANSMIT_TIME INIT chunk %lu interface %d", event->getChunkInfo()->getChunkID(), event->getDevID());
	MF_DeviceManager *dm = _system.getInterfaceManager()->getByIndex(event->getDevID());
	if(dm){
		dm->sendChunk(event->getChunkInfo());
	}
}


/*
 * Handle timeout which happens in transport
 */
void MF_Main::handleTransTOut(MF_EvTransTOut *event) {
  MF_ReliabTransport *rtr = (MF_ReliabTransport*)(event->getTransport());
  u_char timeout_type = event->getTOutType();
  switch (timeout_type) {
    case RECV_NACK_TIMEOUT_T:
      rtr->handleRecvNACKTOut(event->getDstSeqPair());
      break;
    case SEND_NACK_TIMEOUT_T: // sender nack timeout
      rtr->handleSendNACKTOut(event->getDstSeqPair());
      break;
    case FLOW_CTRL_TIMEOUT_T:
      rtr->handleFCtrlTOut(event->getDstSeqPair());
      break;
    default:
      break;
  }
}

void MF_Main::handleSocketTO(MF_SocketTOCheck *event) {
	MF_SocketManager *sm = _system.getSocketTable()->getById(event->getId());
	if(!sm){
		MF_Log::mf_log(MF_DEBUG, "MF_Main: TO for socket that does not exist\n");
		//MF_Log::mf_log_time("MF_Main:handleUpTransp end of event");
		return;
	}
	sm->checkSocketAlive();
}

void MF_Main::handleSocketClean(MF_EventSocketClean *event) {
	MF_SocketManager *sm = _system.getSocketTable()->getById(event->getId());
	if(!sm){
		MF_Log::mf_log(MF_DEBUG, "MF_Main: Clean request for socket that does not exist\n");
		//MF_Log::mf_log_time("MF_Main:handleUpTransp end of event");
		return;
	}
	sm->cleanSocket();
	_system.getSocketTable()->removeManager(event->getId());
	delete sm;
}

