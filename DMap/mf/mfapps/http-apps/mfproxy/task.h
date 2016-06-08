
#ifndef TASK_H
#define TASK_H

#include <string>
#include <mobilityfirst/mfapi.h>

#include "itaskissuer.h"

#define MAX_REQUEST_SIZE 51200

class ITaskIssuer;

class Task{
public:
	Task();
	~Task();
	
	struct GetInfo getInfo();
	void setInfo(struct GetInfo info);
	char *getRequest();
	int getRequestSize();
	void setRequest(const char *request, int requestSize);
	std::string getFilename();
	void setFilename(std::string filename);
	struct Handle *getHandle();
	void setHandle(struct Handle *mfHandle);
	ITaskIssuer *getTaskIssuer();
	void setTaskIssuer(ITaskIssuer *taskIssuer);
	int getPort() const;
	void setPort(int port);
	const char* getServer() const;
	void setServer(const char *server);
	const unsigned int getUniqueID();
	void setUniqueID(unsigned int id);

private:
	struct Handle *mfHandle;
	struct GetInfo info;
	//Max size for server name
	char server[64];
	int port;
	unsigned int uniqueID;
	char buffer[MAX_REQUEST_SIZE];
	int requestSize;
	std::string filename;
	ITaskIssuer *taskIssuer;
};

#endif
