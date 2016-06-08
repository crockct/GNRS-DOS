#include <queue>
#include <semaphore.h>
#include "server.h"

class MFHandler{
public:
	MFHandler();
	~MFHandler();

	int init(const char **options, const char *serverName, int serverPort, int nWorkers, int myGuid);

	int run();

private:
	char *webserver;
	const char** options;
	int myGuid;
	Server server;
};
