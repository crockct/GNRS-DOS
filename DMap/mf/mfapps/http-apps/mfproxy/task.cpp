#include <stdio.h>
#include <string.h>
#include <mobilityfirst/mfapi.h>

#include "task.h"

extern bool MFPROXY_DEBUG;


Task::Task(){
	memset (buffer,0,MAX_REQUEST_SIZE);
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

char *Task::getRequest(){
	return buffer;
}

int Task::getRequestSize(){
	return requestSize;
}

void Task::setRequest(const char *request, int requestSize){
	memcpy(buffer, request, requestSize);
	this->requestSize = requestSize;
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

int Task::getPort() const {
	return port;
}

void Task::setPort(int port) {
	this->port = port;
}

const char* Task::getServer() const {
	return server;
}

void Task::setServer(const char *server){
	strcpy(this->server, server);
}

const unsigned int Task::getUniqueID(){
	return uniqueID;
}

void Task::setUniqueID(unsigned int id){
	uniqueID = id;
}
