#include <queue>
#include <semaphore.h>
#include "server.h"
#include "mfsocketpool.h"
#include "../name-resolver/nameresolver.h"
#include "mongoose.h"

class HTTPHandler{
public:
	HTTPHandler();
	~HTTPHandler();

	//Handlers for mongoose callbacks
	static int  begin_request(struct mg_connection *conn);
	static void end_request(const struct mg_connection *conn, int reply_status_code);
	static int log_message(const struct mg_connection *conn, const char *message);

	int initSocket();
	int stopSocket();

	int initNameResolver(const char *filename);

	int initAllocatedBuffers(int n);

	void freeAllocatedBuffers();

	char *getPreAllocatedBuffer();

	void returnPreAllocatedBuffer(char *buffer);

	int init(const char *options[], const char *nameResolver, int startingGuid, int nSockets);

	int init(const char *options[], int fixedDst, int startingGuid, int nSockets);

	int run();

private:
	//mf socket handle
	//struct Handle h;
	MFSocketPool socketPool;
	NameResolver nameResolver;
	sem_t wsem;
	std::queue<char *> preAllocatedBuffers;
	static const int maxBufferSize = 1024*1024*11;
	int nBuffers;
	int nSockets;
	const char **options;
	bool fixed;
	int fixedGuid;
	int startingGuid;
	volatile unsigned int lastReqID;

	const char *extractHostname(struct mg_connection *conn);
	int processRequest(struct mg_connection *conn, char *buffer, int bufferSize);
};
