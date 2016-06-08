#include <mobilityfirst/mfapi.h>
#include <queue>
#include <semaphore.h>

#ifndef MFSOCKETPOOL_H
#define MFSOCKETPOOL_H

class MFSocketPool{
	sem_t poolAccess;
	int nSockets;
	int initialGuid;
	std::queue<struct Handle *> socketPool;

	void lockPool();
	void releasePool();

public:
	MFSocketPool();
	~MFSocketPool();
	int init(int nSockets, int initialGuid);
	int clearPool();
	struct Handle *getSocket();
	void returnSocket(struct Handle *h);
};

#endif
