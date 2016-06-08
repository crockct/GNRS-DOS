
#include <string>
#include <mobilityfirst/mfapi.h>

#include "task.h"


Task::Task(){
	
}

Task::~Task(){
	
}

struct GetInfo Task::getInfo(){
	return info;
}

void Task::setInfo(struct GetInfo info){
	this->info.srcGuid = info.srcGuid;
	this->info.dstGuid = info.dstGuid;
	this->info.UID = info.UID;
}

std::string Task::getFilename(){
	return filename;
}

void Task::setFilename(std::string filename){
	this->filename = filename;
}

struct Handle *Task::getHandle(){
	return mfHandle;
}

void Task::setHandle(struct Handle *mfHandle){
	this->mfHandle = mfHandle;
}

ITaskIssuer *Task::getTaskIssuer(){
	return taskIssuer;
}

void Task::setTaskIssuer(ITaskIssuer *taskIssuer){
	this->taskIssuer = taskIssuer;
}

