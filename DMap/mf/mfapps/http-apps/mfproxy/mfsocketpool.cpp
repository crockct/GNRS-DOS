#include <mobilityfirst/mfapi.h>
#include <stdlib.h>
#include <iostream>
#include "mfsocketpool.h"

extern bool MFPROXY_DEBUG;

MFSocketPool::MFSocketPool(){
	nSockets = 0;
	initialGuid = 0;
}

MFSocketPool::~MFSocketPool(){
	sem_destroy(&poolAccess);
}

void MFSocketPool::lockPool(){
	sem_wait(&poolAccess);
}

void MFSocketPool::releasePool(){
	sem_post(&poolAccess);
}

int MFSocketPool::init(int nSockets, int initialGuid){
	this->nSockets = 0;
	this->initialGuid = initialGuid;
	sem_init(&poolAccess, 0, 1);
	struct Handle *tempHandle;
	for(int i=initialGuid; i<nSockets+initialGuid; i++){
		//init sockets
		tempHandle = (struct Handle*)malloc(sizeof(struct Handle));
		if(mfopen(tempHandle, "content", 0, i)){
			std::cerr << "ERROR: opening the socket" << std::endl;
			return -1;
		}
		socketPool.push(tempHandle);
		this->nSockets ++;
	}
	return 0;
}

int MFSocketPool::clearPool(){
	lockPool();
	struct Handle *tempHandle;
	while(!socketPool.empty()){
		tempHandle = socketPool.front();
		socketPool.pop();
		mfclose(tempHandle);
		free(tempHandle);
		nSockets --;
	}
	if(nSockets!=0) std::cerr << "ERROR: Some socket were lost along the way" << std::endl;
	releasePool();
	return 0;
}

struct Handle *MFSocketPool::getSocket(){
	lockPool();
	struct Handle *tempHandle = socketPool.front();
	//Check this
	if(tempHandle != NULL) socketPool.pop();
	releasePool();
	return tempHandle;
}

void MFSocketPool::returnSocket(struct Handle *h){
	if(h!=NULL){
		lockPool();
		socketPool.push(h);
		releasePool();
	}
}
