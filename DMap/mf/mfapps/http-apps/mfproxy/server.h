/*
 *
 */

#ifndef VIDEOSERVER_H
#define VIDEOSERVER_H

#include <string>
#include "../name-resolver/nameresolver.h"

#include "itaskissuer.h"
#include "worker.h"
#include "workerlist.h"
#include "task.h"

class Server : public ITaskIssuer {
public:
	Server();
	~Server();
	
	void setNumberOfWorkers(int nWorkers);
	void setMappingsFile(std::string filename);
	
	void callback(Worker *worker);
	void init();
	void run(bool detached);
	void stop();

	const char* getServerName() const;
	void setServerName(const char *serverName);
	int getServerPort() const;
	void setServerPort(int serverPort);
	int getDefaultGuid() const;
	void setDefaultGuid(int defaultGUID);

private:
	int cont;
	int nWorkers;
	int defaultGuid;
	unsigned int lastReqID;
	WorkerList workerList;
	NameResolver nameResolver;
	struct Handle mfHandle;
	struct GetInfo mfInfo;
	char buffer[65536];
	char serverName[64];
	int serverPort;
};

#endif
