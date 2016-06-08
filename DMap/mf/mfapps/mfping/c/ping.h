#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mobilityfirst/mfapi.h>
#include <time.h>
#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>
#include <float.h>
#include <math.h>
#include <signal.h>

#define PING_SIZE 64
#define MAX_SIZE 10*1024*1024

struct cShared{
	struct Handle handle;
	double *sTimes;
	double *eTimes;
	int rec;
	int sent;
	int max;
	sem_t sem;
};
