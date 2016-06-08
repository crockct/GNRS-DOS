
#include <mobilityfirst/mfapi.h>
#include <list>
#include <iostream>
#include "../name-resolver/nameresolver.h"
#include <sys/time.h>

#include "worker.h"
#include "workerlist.h"
#include "task.h"
#include "server.h"

extern bool MFPROXY_DEBUG;

Server::Server(){
	nWorkers = 1;
	defaultGuid = 0;
	lastReqID = 0;
}

Server::~Server(){

}

void Server::setNumberOfWorkers(int nWorkers){
	this->nWorkers = nWorkers;
}

void Server::setMappingsFile(std::string filename){
	nameResolver.init(filename);
}

const char* Server::getServerName() const {
	return serverName;
}

int Server::getServerPort() const {
	return serverPort;
}

void Server::setServerName(const char *serverName){
	strcpy(this->serverName, serverName);
}

void Server::setServerPort(int serverPort) {
	this->serverPort = serverPort;
}

int Server::getDefaultGuid() const {
	return defaultGuid;
}

void Server::setDefaultGuid(int defaultGUID){
	this->defaultGuid = defaultGuid;
}

void Server::init(){
	mfopen(&mfHandle, "content", 0, defaultGuid);
	std::list<int> *guidList = nameResolver.getAllGuids();
	std::list<int>::iterator it;
	for (it = guidList->begin(); it!=guidList->end(); it++){
		mfattach(&mfHandle, &(*it), 1);
	}
	for (int i = 0; i<nWorkers; i++) {
		Worker *temp = new Worker();
		temp->init();
		temp->run();
		workerList.addWorker(temp);
	}
}

void Server::run(bool detached){
	if(detached){
		std::cout << "Detached mode not supported for now" << std::endl;
	}
	cont = true;
	int size;
	while(cont){
		size = mfdo_get(&mfHandle, &mfInfo, &buffer, (u_int)65536);
		if(size<=0){
			std::cerr << "ERROR: received request of size " << size << std::endl;
		} else {
			struct timeval time;
			gettimeofday(&time, NULL);
			std::cout << "[" << lastReqID << "][" << time.tv_sec*1000000+time.tv_usec << "] Received request of size: " << size << std::endl;
		}
		Worker *selectedWorker = workerList.pullWorker();
		if(selectedWorker!=NULL){
			Task *task = new Task();
			task->setInfo(mfInfo);
			//TODO: move this to pool of pre-allocated buffers for better performance
			task->setRequest(buffer, size);
			task->setServer(serverName);
			task->setPort(serverPort);
			task->setHandle(&mfHandle);
			task->setTaskIssuer(this);
			task->setUniqueID(lastReqID++);
			selectedWorker->assignTask(task);
			selectedWorker->performTask();
		}
	}
}

void Server::stop(){
	cont = false;
}

void Server::callback(Worker *worker){
	workerList.addWorker(worker);
}
