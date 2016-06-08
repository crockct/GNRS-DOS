#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mobilityfirst/mfapi.h>
#include <time.h>
#include <sys/time.h>

#include "receiver.h"

int debug = 0;

void usage() {
  fprintf(stderr, "error: unrecognized command-line options\n");
  fprintf(stderr, "usage: receiver options:\n");
  fprintf(stderr, "\tm\tmy GUID\n");
  fprintf(stderr, "\to\tother side GUID\n");
  fprintf(stderr, "\ts\tamount of data to receive\n");					  
  fprintf(stderr, "\tp\tsize per packet\n");
  fprintf(stderr, "\ta\trequest via anycast\n");
  fprintf(stderr, "\tr\tdirect mode (no request first)\n");
  fprintf(stderr, "\tO\toutput file to store the received data\n");
  fprintf(stderr, "\td\tdebug mode\n\n");
}

//This function requests file using anycast or unicast and then receives it
int receiveFile(char *mine, char *other, long size, long psize, int any){
	struct Handle handle;
	int recvSize = 0; //for debug
	int recvCnt = 0; //for debug
	int ret = 0;
	int first = 1;
	long reminSize = size;
	u_char *buf = (u_char *)calloc(sizeof(u_char),MAX_CHUNK_LEN);
	u_char b[32];

	struct timeval start, end, th;
	
	memset(buf, 0, MAX_CHUNK_LEN);
	
	ret = mfopen(&handle, "basic\0", 0, atoi(mine));
	if(ret) {
		if(debug)printf("receiver: mfopen error\n");
		return (EXIT_FAILURE);
	}
	
	gettimeofday(&start, NULL);

	ret = mfsend(&handle, &b, 32, atoi(other), 0);
	if (ret < 0) {
		fprintf (stderr,"mfsendmsg error\n");
		return EXIT_FAILURE;
	}

	while(1) {
		if (reminSize <= psize) {
			ret = mfrecv_blk(&handle, NULL, buf, MAX_CHUNK_LEN, NULL, 0);
			if(debug)printf("Received %d data message\n", ret);
			recvSize += ret;
			recvCnt++;
			if(debug)printf("Total %ld bytes, received %d bytes, chunk count = %d\n", size, recvSize, recvCnt);
			break;
		}
		ret = mfrecv_blk(&handle, NULL, buf, MAX_CHUNK_LEN, NULL, 0);
		if(debug)printf("Received %d data message\n", ret);
		recvSize += ret;
		recvCnt++;
		if(debug)printf("Total %ld bytes, received %d bytes, chunk count = %d\n", size, recvSize, recvCnt);
		reminSize -= ret;
		if(first){
			gettimeofday(&th, NULL);
			first = 0;
		}
	}
	gettimeofday(&end, NULL);
	double time_spent = (end.tv_sec*1000.0+end.tv_usec/1000.0)-(start.tv_sec*1000.0-start.tv_usec/1000.0);
	double th_time_spent = (end.tv_sec*1000.0+end.tv_usec/1000.0)-(th.tv_sec*1000.0-th.tv_usec/1000.0);
   
	mfclose(&handle);
	printf("Task succeed in %d milliseconds\n", (int)(time_spent));
	printf("Throughput is %.2f bps\n", (8.0*(double)size)/(th_time_spent/1000.0));
	return EXIT_SUCCESS;
}

//This function
int directReceiveFile(char *mine, char *other, long size, long psize, int any, const char *filename){
	struct Handle handle;
	int recvSize = 0; //for debug
	int recvCnt = 0; //for debug
	int ret = 0;
	int first = 1;
	long reminSize = size;
	u_char *buf = (u_char *)calloc(sizeof(u_char),psize);
	FILE* pFile;
	if(filename != NULL){
		pFile = fopen(filename,"wb");
	}

	struct timeval start, end;
	
	memset(buf, 0, psize);
	
	ret = mfopen(&handle, "basic\0", 0, atoi(mine));
	if(ret) {
		if(debug)printf("receiver: mfopen error\n");
		return (EXIT_FAILURE);
	}
	
	while(1) {
		if (reminSize <= psize) {
			ret = mfrecv_blk(&handle, NULL, buf, psize, NULL, 0);
			if(debug)printf("Received %d data message\n", ret);
			if(pFile!=NULL){
				fwrite(buf, ret, 1, pFile);
			}
			recvSize += ret;
			recvCnt++;
			if(debug)printf("Total %ld bytes, received %d bytes, chunk count = %d\n", size, recvSize, recvCnt);
			break;
		}
		ret = mfrecv_blk(&handle, NULL, buf, psize, NULL, 0);
		if(debug)printf("Received %d data message\n", ret);
		if(pFile!=NULL){
			fwrite(buf, ret, 1, pFile);
		}
		recvSize += ret;
		recvCnt++;
		if(debug)printf("Total %ld bytes, received %d bytes, chunk count = %d\n", size, recvSize, recvCnt);
		reminSize -= ret;
		if(first){
			if(debug)printf("Received first chunk\n");
			gettimeofday(&start, NULL);
			first = 0;
		}
	}
	gettimeofday(&end, NULL);
	double time_spent = (end.tv_sec*1000.0+end.tv_usec/1000.0)-(start.tv_sec*1000.0-start.tv_usec/1000.0);
   
	mfclose(&handle);
	printf("Task succeed in %d milliseconds\n", (int)(end.tv_sec-start.tv_sec));
	printf("Throughput is %.2f bps\n", (8.0*(double)size)/(time_spent/1000.0));
	return EXIT_SUCCESS;
}

//This function
int receivePerfFile(char *mine, char *other, long size, long psize, int any, const char *filename){
	if(debug)printf("receiver: receivePerfFile\n");
	if(debug)printf("receiver: data run:\n");
	if(debug)printf("\tmyGUID %d\n",atoi(mine));
	if(debug)printf("\totherGUID %d\n",atoi(other));
	if(debug)printf("\tsize %ld\n",size);
	if(debug)printf("\tpsize %ld\n",psize);
	struct Handle handle;
	int recvSize = 0; //for debug
	int recvCnt = 0; //for debug
	int ret = 0;
	int first = 1;
	long reminSize = size;
	u_char dummy[64];
	u_char *buf = (u_char *)malloc(psize);
	FILE* pFile = NULL;
	if(filename != NULL){
		pFile = fopen(filename,"wb");
	}
	
	struct timeval start, end;
	
	memset(buf, 0, psize);
	
	ret = mfopen(&handle, "basic\0", 0, atoi(mine));
	if(ret) {
		if(debug)printf("receiver: mfopen error\n");
		return (EXIT_FAILURE);
	}
	
	while(1) {
		if(first){
			ret = mfrecv_blk(&handle, NULL, dummy, 64, NULL, 0);
			if(debug)printf("Received first chunk\n");
			gettimeofday(&start, NULL);
			first = 0;
			continue;
		}
		if (reminSize <= psize) {
			ret = mfrecv_blk(&handle, NULL, buf, psize, NULL, 0);
			if(pFile!=NULL){
				fwrite(buf, ret, 1, pFile);
			}
			recvSize += ret;
			recvCnt++;
			if(debug)printf("Total %ld bytes, received %d bytes, chunk count = %d\n", size, recvSize, recvCnt);
			break;
		}
		ret = mfrecv_blk(&handle, NULL, buf, psize, NULL, 0);
		if(pFile!=NULL){
			fwrite(buf, ret, 1, pFile);
		}
		recvSize += ret;
		recvCnt++;
		if(debug)printf("Total %ld bytes, received %d bytes, chunk count = %d\n", size, recvSize, recvCnt);
		reminSize -= ret;
	}
	gettimeofday(&end, NULL);
	double time_spent = (end.tv_sec*1000.0+end.tv_usec/1000.0)-(start.tv_sec*1000.0-start.tv_usec/1000.0);
   
	mfclose(&handle);
	printf("Task succeed in %.2f milliseconds\n", time_spent);
	printf("Throughput is %.2f mbps\n", (8.0*(double)recvSize)/(time_spent/1000.0));
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
	char *mine = NULL, *other = NULL, *filename = NULL;
	int i, any = 0, direct = 0, perf = 0;
	long size = 0, psize = 0;
	while((i = getopt(argc,argv,"hdo:m:s:arp:PO:")) != -1){
		switch(i){
		case 'm':
			mine = optarg;
			break;
		case 'o':
			other  = optarg;
			break;
		case 'O':
			filename = optarg;
			break;
		case 's':
			size = atol(optarg);
			break;
		case 'p':
			psize = atol(optarg);
			break;
		case 'd':
			debug = 1;
			break;
		case 'a':
			any = 1;
			break;
		case 'r':
			direct = 1;
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
	
	if(psize == 0){
		psize = size;
	}
	
	if(perf) return receivePerfFile(mine, other, size, psize, any, filename);
	else if(!direct)return receiveFile(mine, other, size, psize, any);
	else return directReceiveFile(mine, other, size, psize, any, filename);
}

