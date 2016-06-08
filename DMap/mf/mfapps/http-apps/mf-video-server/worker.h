
#ifndef WORKER_H
#define WORKER_H

#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>

#include "task.h"

class Task;

class Worker {
public:
	
	Worker();
	~Worker();
	
	void init();
	void run();
	void assignTask(Task *task);
	void performTask();
	
private:
	sem_t wsem;
	Task *task;
	pthread_t thread;
	
	void sendData();
	void mainLoop();
	//Check if it can be private or has to be public
	static void *startThread(void *argument);
	
};

#endif
