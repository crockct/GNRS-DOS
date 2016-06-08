
#ifndef WORKER_H
#define WORKER_H

#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>

#include "task.h"

class Task;

class Worker {
	static const int maxBufferSize = 1024*1024*11;
	sem_t wsem;
	Task *task;
	pthread_t thread;
	char preAllocatedBuffer[maxBufferSize];
	char *tempBuffer;
	int dataSize;

	int retrieveHTTPRequest();
	void sendData();
	void mainLoop();
	//Check if it can be private or has to be public
	static void *startThread(void *argument);

public:
	
	Worker();
	~Worker();
	
	void init();
	void run();
	void assignTask(Task *task);
	void performTask();
	
};

#endif
