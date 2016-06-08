#include "ping.h"

int debug = 0;

void print_hex(const char *s, int size) {
	int i;
	for(i = 0; i<size; i++)
		printf("%02x", (unsigned int) s[i]);
	printf("\n");
}

double getDouble(char *buffer, int pos){
	char *p = buffer + pos;
	double val;
	memcpy(&val, p, 8);
	return val;
}

int getInt(char *buffer, int pos){
	char *p = buffer + pos;
	int val = 0;
	memcpy(&val, p, 2);
	return val;
}

void putDouble(char *buffer, double n, int pos){
	char *p = buffer + pos;
	memcpy(p, &n, 8);
}

void putInt(char *buffer, int n, int pos){
	char *p = buffer + pos;
	memcpy(p, &n, 2);
}



void *clientRecProc(void *arg){
	if(debug)printf("Started receiving thread\n");
	struct cShared *sh = (struct cShared *)arg;
	char *buffer = (char*) malloc (sizeof(char)*MAX_SIZE);
	if (buffer == NULL) {
		fputs ("Memory error",stderr);
		exit(2);
	}
	int ret;
	struct timeval end;
	int seq_n;
	int i;
	for(i=0; i<sh->max; i++){
		if(debug)printf("Waiting to receive\n");
		ret = mfrecv_blk(&(sh->handle), NULL, buffer, MAX_SIZE, NULL, 0);
		gettimeofday(&end, NULL);
		if(ret < 0){
			printf("ERROR receiving server proc\n");
			continue;
		}
		seq_n = getInt(buffer,1);
		if(debug)printf("Received packet %d of size %d\n",seq_n, ret);
		if(debug){
			printf("Received: ");
			print_hex(buffer, ret);
		}
		sh->eTimes[seq_n] = end.tv_sec*1000.0 + end.tv_usec/1000.0;
		if(debug)printf("Received %d after %2.4f msec", seq_n, sh->eTimes[seq_n]);
		printf("%d bytes received: seq_n=%d, time=%2.4f msec\n",ret,seq_n,(sh->eTimes[i] - sh->sTimes[i]));
	}
	return NULL;
}

void startServer(char *mine, char *other){
	if(debug)printf("Start server\n");
	struct Handle handle;
	char *buffer = (char*) malloc (sizeof(char)*MAX_SIZE);
	if (buffer == NULL) {
		fputs ("Memory error",stderr);
		exit(2);
	}
	int ret;
	if(debug)printf("Opening application\n");
	ret = mfopen(&handle, "basic", 0, atoi(mine));
	if(debug)printf("Application opened\n");
	if (ret!=0) {
		fprintf (stderr,"mfopen error\n");
		exit(-1);
	}
	while(1){
		if(debug)printf("Waiting to receive\n");
		ret = mfrecv_blk(&handle, NULL, buffer, MAX_SIZE, NULL, 0);
		if(ret < 0){
			printf("ERROR receiving server proc\n");
			continue;
		}
		if(debug)printf("Something received\n");
		buffer[0] = 1;
		ret = mfsend(&handle, buffer, ret, atoi(other), 0);
		if(ret < 0){
			printf("ERROR sending server proc\n");
			continue;
		}
	}
}

void startClient(char *mine, char *other, double to, int n, int size){
	if(debug)printf("Start ping client\n");
	char *buffer;
	struct timeval start;
	struct cShared *sh;
	int ret;
	sh = (struct cShared*)malloc(sizeof(struct cShared));
	sh->sTimes = (double *)calloc(n, sizeof(double));
	sh->eTimes = (double *)calloc(n, sizeof(double));
	sh->max = n;
	if(debug)printf("Opening application\n");
	ret = mfopen(&(sh->handle), "basic", 0, atoi(mine));
	if (ret!=0) {
		fprintf (stderr,"mfopen error\n");
		exit(-1);
	}
	if(debug)printf("Application opened\n");
	//sem_init(&(sh->sem), 0, 1);
	pthread_t thread;
	if(debug)printf("Creating receiving thread\n");
	pthread_create(&(thread), 0, &clientRecProc, sh);
	int seq_n;
	if(size == 0){
		size = PING_SIZE;
	}
	buffer = (char*) malloc (sizeof(char)*size);
	memset(buffer, 0, size);
	if (buffer == NULL) {
		fputs ("Memory error",stderr);
		exit(2);
	}
	for(seq_n=0; seq_n<n; seq_n++){
		sh->eTimes[seq_n] = 0.0;
		gettimeofday(&start, NULL);
		sh->sTimes[seq_n] = start.tv_sec*1000.0 + start.tv_usec/1000.0;
		if(debug)printf("Sending packet %d at time %4.2f msec\n",seq_n, sh->sTimes[seq_n]);
		buffer[0] = 0;
		putInt(buffer, seq_n, 1);
		putDouble(buffer, sh->sTimes[seq_n], 5);
		if(debug){
			printf("Sending: ");
			print_hex(buffer, size);
		}
		ret = mfsend(&(sh->handle), buffer, size, atoi(other), 0);
		if(debug)printf("Packet %d sent\n", seq_n);
		if(ret < 0){
			break;
		}
		usleep(to*1000000);
	}
	pthread_join(thread, NULL);
	mfclose(&(sh->handle));
	int i = 0;
	int losses = 0;
	double max = 0, min = DBL_MAX;
	if(debug)printf("Just in case min %8.4f max %8.4f\n", min, max);
	double avg=0, var=0;
	for(i = 0; i<n; i++){
		if(sh->eTimes[i] == 0){
			losses++;
		}
		else{
			if(sh->eTimes[i] - sh->sTimes[i] < min) min = sh->eTimes[i] - sh->sTimes[i];
			if(sh->eTimes[i] - sh->sTimes[i] > max) max = sh->eTimes[i] - sh->sTimes[i];
			avg += (sh->eTimes[i] - sh->sTimes[i]);
			if(debug)printf("Time for packet %d %4.2f %4.2f %4.2f msec\n", i, sh->eTimes[i], sh->sTimes[i], (sh->eTimes[i] - sh->sTimes[i]));
		}
	}
	avg = avg/(n-losses);
	for(i = 0; i<n; i++){
		if(!(sh->eTimes[seq_n] == 0)){
			var += ((sh->eTimes[i] - sh->sTimes[i]) - avg)*
					((sh->eTimes[i] - sh->sTimes[i]) - avg);
		}
	}
	var = var/(n-losses);
	printf("%d packts transmitted, %d received\n",n,n-losses);
	printf("rtt min/avg/max/std = %8.4f/%8.4f/%8.4f/%8.4f msec\n",
			min,avg,max,sqrt(var));
}

void usage() {
  fprintf(stderr, "error: unrecognized command-line options\n");							  
  fprintf(stderr, "usage: receiver options:\n");							  
  fprintf(stderr, "\tm\tmy GUID\n");							  
  fprintf(stderr, "\to\tother side GUID\n");							  
  fprintf(stderr, "\ts\tserver\n");							  
  fprintf(stderr, "\tc\tclient\n");							  
  fprintf(stderr, "\tt\tinterval between pings\n");				  
  fprintf(stderr, "\tn\tnumber of pings to send\n");						  
  fprintf(stderr, "\td\tdebug mode\n\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	int client = 1, max = 1000, size = 0;
	double to = 1.0;
	char *mine = NULL,*other = NULL;
	int i;
	while((i = getopt(argc,argv,"hdcsn:o:m:t:p:")) != -1){
		switch(i){
		case 'n':
			max = atoi(optarg);
			break;
		case 'p':
			size = atoi(optarg);
			break;
		case 'm':
			mine = optarg;
			break;
		case 'o':
			other = optarg;
			break;
		case 't':
			to = atol(optarg);
			break;
		case 'c':
			client = 1;
			break;
		case 'd':
			debug = 1;
			break;
		case 's':
			client = 0;
			break;
		case 'h':
			usage();
			exit(1);
		default:
			usage();
			exit(1);
		}
	}

	if(!mine || !other){
		usage();
	}

	if(client){
		if(debug)printf("It's a client\n");
		startClient(mine, other, to, max, size);
	}
	else{
		if(debug)printf("It's a server\n");
		startServer(mine, other);
	}

	return EXIT_SUCCESS;
}

