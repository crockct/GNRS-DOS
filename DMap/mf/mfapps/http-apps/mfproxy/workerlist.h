

#ifndef WORKERLIST_H
#define WORKERLIST_H

#include <list>
#include <semaphore.h>
#include "worker.h"

class WorkerList{
public:
	
	WorkerList();
	~WorkerList();
	
	void addWorker(Worker *worker);
	Worker *pullWorker();

private:
	std::list <Worker *> workersList;
	sem_t wsem;
};

#endif
