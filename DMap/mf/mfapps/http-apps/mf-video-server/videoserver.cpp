
#include <mobilityfirst/mfapi.h>
#include <list>
#include <iostream>
#include <name-resolver/nameresolver.h>

#include "worker.h"
#include "workerlist.h"
#include "task.h"
#include "videoserver.h"

VideoServer::VideoServer(){
	nWorkers = 1;
	defaultGuid = 1;
}

VideoServer::~VideoServer(){
	
}

void VideoServer::setNumberOfWorkers(int nWorkers){
	this->nWorkers = nWorkers;
}

void VideoServer::setMappingsFile(std::string filename){
	nameResolver.init(filename);
}

void VideoServer::init(){
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

void VideoServer::run(){
	cont = true;
	int size;
	while(cont){
		size = mfdo_get(&mfHandle, &mfInfo, &buffer, (u_int)65536);
		Worker *selectedWorker = workerList.pullWorker();
		if(selectedWorker!=NULL){
			Task *task = new Task();
			task->setInfo(mfInfo);
			task->setFilename(nameResolver.getName(mfInfo.dstGuid));
			task->setHandle(&mfHandle);
			task->setTaskIssuer(this);
			selectedWorker->assignTask(task);
			selectedWorker->performTask();
		}
	}
}

void VideoServer::stop(){
	cont = false;
}

void VideoServer::callback(Worker *worker){
	workerList.addWorker(worker);
}
