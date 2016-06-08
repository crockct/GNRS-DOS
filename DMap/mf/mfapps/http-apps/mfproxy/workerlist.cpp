#include <list>
#include <semaphore.h>

#include "workerlist.h"
#include "worker.h"

extern bool MFPROXY_DEBUG;

WorkerList::WorkerList(){
	sem_init(&wsem, 0, 1);
}

WorkerList::~WorkerList(){
	//Decide on whether destroy the workers here or not
	sem_destroy(&wsem);
}

void WorkerList::addWorker(Worker *worker){
	sem_wait(&wsem);
	workersList.push_back(worker);
	sem_post(&wsem);
}

Worker *WorkerList::pullWorker(){
	Worker *ret;
	sem_wait(&wsem);
	if(!workersList.empty()){
		ret = workersList.front();
		workersList.pop_front();
	}
	else{
		ret = NULL;
	}
	sem_post(&wsem);
	return ret;
}
