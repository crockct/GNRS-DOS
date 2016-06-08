
#ifndef TASK_H
#define TASK_H

#include <string>
#include <mobilityfirst/mfapi.h>

#include "itaskissuer.h"

class ITaskIssuer;

class Task{
public:
	Task();
	~Task();
	
	struct GetInfo getInfo();
	void setInfo(struct GetInfo info);
	std::string getFilename();
	void setFilename(std::string filename);
	struct Handle *getHandle();
	void setHandle(struct Handle *mfHandle);
	ITaskIssuer *getTaskIssuer();
	void setTaskIssuer(ITaskIssuer *taskIssuer);
	

private:
	struct Handle *mfHandle;
	struct GetInfo info;
	std::string filename;
	ITaskIssuer *taskIssuer;
};

#endif
