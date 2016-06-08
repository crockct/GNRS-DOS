
#include <semaphore.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <sys/time.h>
#include <stdlib.h>

#include "worker.h"
#include "mongoose.h"

extern bool MFPROXY_DEBUG;

Worker::Worker(){
	
}

Worker::~Worker(){
	
}

void Worker::init(){
	sem_init(&wsem, 0, 0);
	task = NULL;
}

void Worker::run(){
	int error = pthread_create(&thread, NULL, &startThread, this);
	if (error) {
		std::cerr << "Thread creation failed" << std::endl;
	}
}

void Worker::assignTask(Task *task){
	this->task = task;
}

void Worker::performTask(){
	sem_post(&wsem);
}

int Worker::retrieveHTTPRequest(){
	if(MFPROXY_DEBUG){
		std::cout << "[" << thread << "] Received request of size: " << task->getRequestSize() << std::endl;
		int max_size = task->getRequestSize() > 500 ? 500 : task->getRequestSize();
		for (int i = 0; i< max_size; i++) {
			std::cout << task->getRequest()[i];
		}
		std::cout << std::endl;
	}
	char  ebuf[100];
	struct mg_connection *conn;
	conn = mg_download(task->getServer(), task->getPort(), 0, ebuf, sizeof(ebuf), "%s", task->getRequest());
	//TODO: download the content part and the
	if(conn==NULL){
		std::cerr << "ERROR: mg_download returned NULL value" << ebuf << std::endl;
		memcpy(preAllocatedBuffer, "503\n\r\n\r", 7);
		dataSize = 7;
	}
	struct mg_request_info *info = mg_get_request_info(conn);
	if(strcmp(info->uri, "200")){
		if(MFPROXY_DEBUG) std::cout << "Server returned error: " << info->uri << std::endl;
		memcpy(preAllocatedBuffer, info->raw_request, info->raw_request_size);
		mg_close_connection(conn);
		dataSize = info->raw_request_size;
	}
	else{
		if(MFPROXY_DEBUG) std::cout << "[" << thread << "] Received response: ";
		for (int i = 0; i< info->raw_request_size; i++) {
			if(MFPROXY_DEBUG) std::cout << info->raw_request[i];
		}
		if(MFPROXY_DEBUG) std::cout << std::endl;
		memcpy(preAllocatedBuffer, info->raw_request, info->raw_request_size);
		int readBytes = info->raw_request_size;
		int len = 0;
		while ((len = mg_read(conn, preAllocatedBuffer + readBytes, maxBufferSize - readBytes)) > 0) {
			readBytes += len;
		}
		mg_close_connection(conn);
		dataSize = readBytes;
	}
	return 0;
}

void Worker::sendData(){
	//TODO: max size for content responses is 4MB. Should be allocated dynamically if needed.
	struct GetInfo info = task->getInfo();
	int bufferSwitch = retrieveHTTPRequest();
	if(!bufferSwitch){
		mfget_response(task->getHandle(), &info, preAllocatedBuffer, dataSize, 0);
	} else {
		mfget_response(task->getHandle(), &info, tempBuffer, dataSize, 0);
		free(tempBuffer);
	}
	if(MFPROXY_DEBUG){
		struct timeval time;
		gettimeofday(&time, NULL);
		std::cout << "[" << task->getUniqueID() << "][" << time.tv_sec*1000000+time.tv_usec << "] Response sent" << std::endl;
	}
}

void Worker::mainLoop(){
	while(true){
		sem_wait(&wsem);
		struct timeval ts;
		gettimeofday(&ts, NULL);
		long int t = ts.tv_sec*1000 + ts.tv_usec/1000;
		if(MFPROXY_DEBUG) std::cout << "Thread " << thread << " starting its task at time: " << t << std::endl;
		sendData();
		t = ts.tv_sec*1000 + ts.tv_usec/1000;
		if(MFPROXY_DEBUG) std::cout << "Thread " << thread << " completed its task at time: " << t << std::endl;
		task->getTaskIssuer()->callback(this);
	}
}

void *Worker::startThread(void *argument){
	Worker *worker = (Worker *)argument;
	worker->mainLoop();
	return NULL;
}
