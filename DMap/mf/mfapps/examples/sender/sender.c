#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mobilityfirst/mfapi.h>
#include <time.h>
#include <sys/time.h>

#include "sender.h"

int debug = 0;

void usage() {
  fprintf(stderr, "error: unrecognized command-line options\n"); 
  fprintf(stderr, "usage: receiver options:\n"); 
  fprintf(stderr, "\tm\tmy GUID\n"); 
  fprintf(stderr, "\to\tother side GUID\n"); 
  fprintf(stderr, "\ts\tamount of data to send\n"); 					  
  fprintf(stderr, "\tp\tsize per packet (NOTE: packet should be set only to smaller amounts than the max chunk size\n");
  fprintf(stderr, "\tc\tcontinous mode\n"); 
  fprintf(stderr, "\tr\tdirect mode (no request first)\n"); 
  fprintf(stderr, "\ti\tuse specified file as data to send\n"); 
  fprintf(stderr, "\td\tdebug mode\n\n");
}

//Direct send, sends the file without waiting for a request
int sendFile(struct Handle *handle, long psize, int size, int dstGUID, const char *filename){
	char *buffer;
	int ret, sz;
	if(filename!=NULL){
		FILE* pFile;
		pFile = fopen(filename,"rb");
		if (pFile){
			if(psize != 0){
				if(debug)printf("Sending file of size %d in messages of size %lu\n", size, psize);
				if(debug)printf("Sending file of size %d by messages of size %ld\n", size, psize);
				buffer = (char*) malloc (sizeof(char)*psize);
				while((ret = fread(buffer, psize, 1, pFile)) > 0){
					ret = mfsend(handle, buffer, psize, dstGUID, 0);
				}
			}
			else{
				fseek(pFile, 0L, SEEK_END);
				sz = ftell(pFile);
				fseek(pFile, 0L, SEEK_SET);
				if(debug)printf("Sending file of size %d in a unique message\n", sz);
				buffer = (char*) malloc (sizeof(char)*sz);
				if(debug)printf("Sending file of size %d as a single message\n", sz);
				ret = fread(buffer, sz, 1, pFile);
				ret = mfsend(handle, buffer, sz, dstGUID, 0);
			}
		}
		else{
		    return -1;
		}
	}
	else if(psize == 0){
		// allocate memory to contain the whole file:
		buffer = (char*) malloc (sizeof(char)*size);
		if(debug)printf("Sending file of size %d in a unique message\n", size);
		if (buffer == NULL) {
			if(debug)printf("Memory error\n"); 
			return EXIT_FAILURE;
		}
	
		ret = mfsend(handle, buffer, size, dstGUID, 0);
		if (ret < 0) {
			fprintf (stderr,"mfsendmsg error\n");
			return ret;
		}
	}
	else{
		// allocate memory to contain a single packet
		buffer = (char*) malloc (sizeof(char)*psize);
		if(debug)printf("Sending file of size %d in messages of size %lu\n", size, psize);
		if (buffer == NULL) {
			if(debug)printf("Memory error\n"); 
			return EXIT_FAILURE;
		}
		int i = 0;
		for(i=0; i < size/psize; i++){
			ret = mfsend(handle, buffer, psize, dstGUID, 0);
			if (ret < 0) {
				fprintf (stderr,"mfsendmsg error\n");
				return ret;
			}
		}
		
		ret = mfsend(handle, buffer, size%psize, dstGUID, 0);
		if (ret < 0) {
			fprintf (stderr,"mfsendmsg error\n");
			return ret;
		}
	}	
	free(buffer);
	return 1;
}


//Receive a request and respond to it.
int singleSend(char *mine, char *other, long psize, int fsize, int any, const char *filename){
	int ret = 0;
	u_char *buf = (u_char *)calloc(sizeof(u_char),MAX_CHUNK_LEN);
	struct Handle handle;
	struct timeval start, end;
	
	ret = mfopen(&handle, "basic\0", 0, atoi(mine));
	if(ret) {
		if(debug)printf("mfopen error\n"); 
		return EXIT_FAILURE;
	}
	ret = mfrecv_blk(&handle, NULL, buf, MAX_CHUNK_LEN, NULL, 0);
	if(ret < 0){
		if(debug)printf("mfrec error\n"); 
		return EXIT_FAILURE;
	}
	gettimeofday(&start, NULL);
	sendFile(&handle, psize, fsize, atoi(other), filename);
	gettimeofday(&end, NULL);
	mfclose(&handle);
	
	double time_spent = (end.tv_sec*1000.0+end.tv_usec/1000.0)-(start.tv_sec*1000.0-start.tv_usec/1000.0);
	printf("Task succeed in %d milliseconds\n", (int)(end.tv_sec-start.tv_sec));
	printf("Throughput is %.2f bps\n", (8.0*(double)fsize)/(time_spent/1000.0));
	return EXIT_SUCCESS;
}

//Receive a request and respond to it, but keeps waiting for new requests.
int continousSend(char *mine, char *other, long psize, int fsize, int any, const char *filename){
	int ret = 0;
	u_char *buf = (u_char *)calloc(sizeof(u_char),MAX_CHUNK_LEN);
	struct Handle handle;
	struct timeval start, end;
	
	ret = mfopen(&handle, "basic\0", 0, atoi(mine));
	if(ret) {
		if(debug)printf("mfopen error\n"); 
		return EXIT_FAILURE;
	}
	while(1){
		ret = mfrecv_blk(&handle, NULL, buf, MAX_CHUNK_LEN, NULL, 0);
		if(ret < 0){
			if(debug)printf("mfrec error\n"); 
			return EXIT_FAILURE;
		}
		gettimeofday(&start, NULL);
		sendFile(&handle, psize, fsize, atoi(other), filename);
		gettimeofday(&end, NULL);
		
		double time_spent = (end.tv_sec*1000.0+end.tv_usec/1000.0)-(start.tv_sec*1000.0-start.tv_usec/1000.0);
		printf("Task succeed in %d milliseconds\n", (int)(end.tv_sec-start.tv_sec));
		printf("Throughput is %.2f bps\n", (8.0*(double)fsize)/(time_spent/1000.0));
	}
	mfclose(&handle);
	return EXIT_SUCCESS;
}

int directSend(char *mine, char *other, long psize, int fsize, int any, const char *filename){
	struct Handle handle;
	struct timeval start, end;
	if(debug)printf("m=%s o=%s\n", mine, other);
	
	if(debug)printf("Opening mf socket\n");
	int ret = mfopen(&handle, "basic\0", 0, atoi(mine));
	if(ret) {
		if(debug)printf("mfopen error\n"); 
		return EXIT_FAILURE;
	}
	if(debug)printf("mf socket open\n");
	gettimeofday(&start, NULL);
	sendFile(&handle, psize, fsize, atoi(other), filename);
	gettimeofday(&end, NULL);
	mfclose(&handle);
	double time_spent = (end.tv_sec*1000.0+end.tv_usec/1000.0)-(start.tv_sec*1000.0-start.tv_usec/1000.0);
	printf("Task succeed in %d milliseconds\n", (int)(end.tv_sec-start.tv_sec));
	printf("Throughput is %.2f bps\n", (8.0*(double)fsize)/(time_spent*1000.0));
	return EXIT_SUCCESS;
}

int perfSend(char *mine, char *other, long psize, int fsize, int any, const char *filename){
	struct Handle handle;
	struct timeval start, end;
	u_char dummy[64];
	if(debug)printf("m=%s o=%s\n", mine, other);
	
	if(debug)printf("Opening mf socket\n");
	int ret = mfopen(&handle, "basic\0", 0, atoi(mine));
	if(ret) {
		if(debug)printf("mfopen error\n"); 
		return EXIT_FAILURE;
	}
	if(debug)printf("mf socket open\n");
	ret = mfsend(&handle, dummy, 64, atoi(other), 0);
	gettimeofday(&start, NULL);
	sendFile(&handle, psize, fsize, atoi(other), filename);
	gettimeofday(&end, NULL);
	mfclose(&handle);
	double time_spent = (end.tv_sec*1000.0+end.tv_usec/1000.0)-(start.tv_sec*1000.0-start.tv_usec/1000.0);
	printf("Task succeed in %d milliseconds\n", (int)(end.tv_sec-start.tv_sec));
	printf("Throughput is %.2f bps\n", (8.0*(double)fsize)/(time_spent/1000.0));
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
	char *mine = NULL, *other = NULL, *filename = NULL;
	int i, cont = 0, direct = 0, any = 0, perf = 0;
	long size = 0, psize = 0;
	while((i = getopt(argc,argv,"hdo:m:acrs:p:aPi:")) != -1){
		switch(i){
		case 'm':
			mine = optarg;
			break;
		case 'o':
			other = optarg;
			break;
		case 'i':
			filename = optarg;
			break;
		case 'd':
			debug = 1;
			break;
		case 'c':
			cont = 1;
			break;
		case 'r':
			direct = 1;
			break;
		case 's':
			size = atol(optarg);
			break;
		case 'p':
			psize = atol(optarg);
			break;
		case 'a':
			any = 1;
			break;
		case 'P':
			perf = 1;
			break;
		case 'h':
			usage();
			exit(1);
		default:
			usage();
			exit(1);
		}
	}

	if(!mine || !other || size == 0){
		usage();
		exit(EXIT_FAILURE);
	}
	
	if(perf){
		return perfSend(mine, other, psize, size, any, filename);
	}
	else if(cont){
		return continousSend(mine, other, psize, size, any, filename);
	}
	else if(direct){
		return directSend(mine, other, psize, size, any, filename);
	}
	else{
		return singleSend(mine, other, psize, size, any, filename);
	}
}

