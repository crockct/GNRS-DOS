
#include <semaphore.h>
#include <string>
#include <stdio.h>
#include <iostream>

#include "worker.h"

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

void Worker::sendData(){
	struct GetInfo info = task->getInfo();
	int maxsize = 1024*1024*4, readBytes;
	char buffer[maxsize];
	FILE *ifp;
	char *mode = "r";
	ifp = fopen(task->getFilename().c_str(), mode);
	if (ifp == NULL) {
  		fprintf(stderr, "Can't open input file %s\n", task->getFilename().c_str());
  		return;
	}
	readBytes = fread(buffer, 1, maxsize, ifp);
	fclose(ifp);
	//Replace 0 with proper SIDs
	mfget_response(task->getHandle(), &info, buffer, readBytes, 0);
}

void Worker::mainLoop(){
	while(true){
		sem_wait(&wsem);
		std::cout << "Thread " << thread << " performing its task" << std::endl;
		sendData();
		task->getTaskIssuer()->callback(this);
	}
}

void *Worker::startThread(void *argument){
	Worker *worker = (Worker *)argument;
	worker->mainLoop();
	return NULL;
}
