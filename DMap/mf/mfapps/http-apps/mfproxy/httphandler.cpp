#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mobilityfirst/mfapi.h>
#include <vector>
#include <sys/time.h>

#include "mongoose.h"
#include "server.h"
#include "httphandler.h"
#include "mfsocketpool.h"

extern bool MFPROXY_DEBUG;

HTTPHandler::HTTPHandler(){
	fixed = false;
	nBuffers = 0;
	startingGuid = 0;
	nSockets = 0;
	fixedGuid = 0;
	lastReqID = 0;
	options = NULL;
}

HTTPHandler::~HTTPHandler(){
	//I can run it if it was left behind...
	freeAllocatedBuffers();
	sem_destroy(&wsem);
}

// Called when mongoose has received new HTTP request.
// If callback returns non-zero,
// callback must process the request by sending valid HTTP headers and body,
// and mongoose will not do any further processing.
// If callback returns 0, mongoose processes the request itself. In this case,
// callback must not send any data to the client.
int  HTTPHandler::begin_request(struct mg_connection *conn){
	if(MFPROXY_DEBUG) {
		std::cout << "Begin_request proxying" << std::endl;
	}
	int maxsize = maxBufferSize, readBytes, wroteBytes;
	char *buffer;

	struct mg_request_info *info = mg_get_request_info(conn);

	HTTPHandler *httpHandler = (HTTPHandler*)(info->user_data);

	buffer = httpHandler->getPreAllocatedBuffer();

	if(buffer!=NULL){
		readBytes = httpHandler->processRequest(conn, buffer, maxsize);
		if (readBytes>0) {
			if(MFPROXY_DEBUG) std::cout << "Sending back " << readBytes << " bytes" << std::endl;
			wroteBytes = mg_write(conn, buffer, readBytes);
			if (readBytes != wroteBytes) std::cerr << "WARNING: sent bytes " << wroteBytes << " different from read bytes " << readBytes << std::endl;
		}
		else{
			std::cerr << "WARNING: Received an empty request" << std::endl;
			mg_printf(conn,
					"HTTP/1.1 503 Service Unavailable\r\n\r\n");
		}
		httpHandler->returnPreAllocatedBuffer(buffer);
	}
	else{
		mg_printf(conn,
				  "HTTP/1.1 503 Service Unavailable\r\n\r\n");
	}
	return 1;
}

// Called when mongoose has finished processing request.
void HTTPHandler::end_request(const struct mg_connection *conn, int reply_status_code){
	if(MFPROXY_DEBUG) std::cout << "end_request" << std::endl;
}

// Called when mongoose is about to log a message. If callback returns
// non-zero, mongoose does not log anything.
int HTTPHandler::log_message(const struct mg_connection *conn, const char *message){
	return 0;
}

int HTTPHandler::initSocket(){
	//TODO number of guids is harcoded
	//return socketPool.init(20, startingGuid) ;
	return socketPool.init(2, startingGuid) ;
}

int HTTPHandler::stopSocket(){
	//return mfclose(&h);
	return socketPool.clearPool();
}

int HTTPHandler::initNameResolver(const char *filename){
	std::string name(filename);
	nameResolver.init(name);
	return 0;
}

int HTTPHandler::initAllocatedBuffers(int n){
	nBuffers = n;
	char *temp;
	for(int i=0; i<n; i++){
		temp = new char[maxBufferSize];
		preAllocatedBuffers.push(temp);
	}
	return 0;
}

void HTTPHandler::freeAllocatedBuffers(){
	char *temp;
	for(int i=0; i<nBuffers; i++){
		temp = preAllocatedBuffers.front();
		if(temp!=NULL){
			delete []temp;
			preAllocatedBuffers.pop();
		}
	}
}

char *HTTPHandler::getPreAllocatedBuffer(){
	sem_wait(&wsem);
	char *temp = preAllocatedBuffers.front();
	if(temp!=NULL){
		preAllocatedBuffers.pop();
	}
	sem_post(&wsem);
	return temp;
}

void HTTPHandler::returnPreAllocatedBuffer(char *buffer){
	sem_wait(&wsem);
	preAllocatedBuffers.push(buffer);
	sem_post(&wsem);
}

const char *HTTPHandler::extractHostname(struct mg_connection *conn){
	const char *hostname = mg_get_header(conn, "Host");
	const char *dstName;
	if(hostname != NULL){
		dstName = hostname;
	}
	else{
		//TODO: for now assumes that the Host header is specified
		if(MFPROXY_DEBUG) std::cout << "WARNING: Host header has not been specified" << std::endl;
		dstName = NULL;
		return 0;
		//struct mg_request_info *info = mg_get_request_info(conn);
		//struct in_addr addr;
		//addr.s_addr = info->remote_ip;
	}
	return dstName;
}

int HTTPHandler::processRequest(struct mg_connection *conn, char *buffer, int bufferSize){
	struct mg_request_info *info = mg_get_request_info(conn);
	unsigned int reqID;
	char *request;
	const char *cLenStr;
	int requestSize = 0, len = 0, totalSize = 0, readBytes = 0;
	requestSize = info->raw_request_size;
	cLenStr=mg_get_header(conn, "Content-Length");
	if (cLenStr != NULL){
		totalSize = requestSize + atoi(cLenStr);
	}
	else{
		totalSize = requestSize;
	}
	request = (char*)malloc(totalSize);
	memcpy(request, info->raw_request, info->raw_request_size);
	//if is a POST request and there is content, copy the content
	if(!strcmp(info->request_method, "POST") && totalSize!=requestSize){
		len = 1;
		readBytes = requestSize;
		while (len > 0) {
			len = mg_read(conn, request + readBytes, totalSize - readBytes);
			readBytes += len;
		}
	}
	if(MFPROXY_DEBUG) {
		struct timeval time;
		gettimeofday(&time, NULL);
		reqID = lastReqID++;
		std::cout << "[" << reqID << "][" << time.tv_sec*1000000+time.tv_usec << "] Sending request of size: " << totalSize << std::endl;
		for (int i = 0; i< requestSize; i++) std::cout << request[i];
		std::cout << std::endl;
	}
	struct Handle *handle = socketPool.getSocket();
	if(handle == NULL){
		std::cerr << "ERROR: no socket available" << std::endl;
		readBytes = -1;
	}
	//I can preassign a destination GUID
	else if(fixed){
		readBytes = mfget(handle, fixedGuid, request, requestSize, buffer, bufferSize, 0);
		if(MFPROXY_DEBUG) {
			struct timeval time;
			gettimeofday(&time, NULL);
			std::cout << "[" << reqID << "][" << time.tv_sec*1000000+time.tv_usec << "] Received answer of size: " << readBytes << std::endl;
		}
		socketPool.returnSocket(handle);
	}
	//Or resolve it dynamically through a nameResolver
	else{
		const char *hostname = extractHostname(conn);
		if(hostname == NULL){
			readBytes = 0;
		}
		else{
			std::string name(hostname);
			std::vector <int> *guid = nameResolver.getGuids(name);
			if(guid == NULL){
				readBytes = 0;
			}
			else{
				readBytes = mfget(handle, (*guid)[0], request, requestSize, buffer, bufferSize, 0);
				if(MFPROXY_DEBUG) {
					struct timeval time;
					gettimeofday(&time, NULL);
					std::cout << "[" << reqID << "][" << time.tv_sec*1000000+time.tv_usec << "] Received answer of size: " << readBytes << std::endl;
				}
			}
		}
		socketPool.returnSocket(handle);
	}
	free(request);
	return readBytes;
}

int HTTPHandler::init(const char *options[], const char *nameResolver, int startingGuid, int nSockets) {
	initNameResolver(nameResolver);
	initAllocatedBuffers(20);
	this->options = options;
	this->startingGuid = startingGuid;
	this->nSockets = nSockets;
	initSocket();
	return 0;
}

int HTTPHandler::init(const char *options[], int fixedDst, int startingGuid, int nSockets) {
	initAllocatedBuffers(20);
	this->options = options;
	this->fixedGuid = fixedDst;
	this->startingGuid = startingGuid;
	this->nSockets = nSockets;
	initSocket();
	fixed = true;
	return 0;
}

int HTTPHandler::run(){
	sem_init(&wsem, 0, 1);
	struct mg_context *ctx;

	struct mg_callbacks callbacks;
	callbacks.begin_request = &HTTPHandler::begin_request;
	callbacks.end_request = &HTTPHandler::end_request;
	callbacks.log_message = &HTTPHandler::log_message;
	callbacks.init_ssl = NULL;
	callbacks.websocket_connect = NULL;
	callbacks.websocket_ready = NULL;
	callbacks.websocket_data = NULL;
	callbacks.open_file = NULL;
	callbacks.init_lua = NULL;
	callbacks.upload = NULL;
	callbacks.thread_start = NULL;
	callbacks.thread_stop = NULL;

	ctx = mg_start(&callbacks, (void *)this, (const char**)options);
	if(ctx==NULL){
		std::cerr << "ERROR: mg_start returned NULL value" << std::endl;
		return 0;
	}
	if(MFPROXY_DEBUG) std::cout << "Press ENTER to stop" << std::endl;
	while(1){
    sleep(10);
  }
  mg_stop(ctx);
	stopSocket();
	return 0;
}
