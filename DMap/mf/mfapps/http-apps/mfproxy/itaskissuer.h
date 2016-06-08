/*
 * Interface for who's issueing a task to a worker.
 * Only needed for having a general callback to the server
 */

#ifndef ITASKISSUER_H
#define ITASKISSUER_H

#include "worker.h"

class Worker;

class ITaskIssuer{
public:
	 virtual void callback(Worker *worker) = 0;

private:

};

#endif
