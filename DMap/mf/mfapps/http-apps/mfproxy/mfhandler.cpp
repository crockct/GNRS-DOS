#include <stdio.h>
#include <string.h>
#include <mobilityfirst/mfapi.h>
#include <vector>

#include "mongoose.h"
#include "mfhandler.h"
#include "server.h"

extern bool MFPROXY_DEBUG;

MFHandler::MFHandler() : server(){
}

MFHandler::~MFHandler(){
}

int MFHandler::init(const char **options, const char *serverName, int serverPort, int nWorkers, int myGuid) {
	this->options = options;
	this->myGuid = myGuid;
//	server.setMappingsFile(filename);
	server.setNumberOfWorkers(nWorkers);
	server.setServerName(serverName);
	server.setServerPort(serverPort);
	server.setDefaultGuid(myGuid);
	server.init();
	return 0;
}

int MFHandler::run(){
	server.run(false);
	printf("Press ENTER to stop\n");
	while(1){
    sleep(10);
  }
  server.stop();
	return 0;
}
