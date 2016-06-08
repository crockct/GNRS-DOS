/*
 *
 */

#ifndef VIDEOSERVER_H
#define VIDEOSERVER_H

#include <string>
#include <name-resolver/nameresolver.h>

#include "itaskissuer.h"
#include "worker.h"
#include "workerlist.h"
#include "task.h"

class VideoServer : public ITaskIssuer {
public:
	VideoServer();
	~VideoServer();
	
	void setNumberOfWorkers(int nWorkers);
	void setMappingsFile(std::string filename);
	
	void callback(Worker *worker);
	void init();
	void run();
	void stop();

private:
	int cont;
	int nWorkers;
	int defaultGuid;
	WorkerList workerList;
	NameResolver nameResolver;
	struct Handle mfHandle;
	struct GetInfo mfInfo;
	char buffer[65536];
};

#endif
