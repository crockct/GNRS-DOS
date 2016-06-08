
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <name-resolver/nameresolver.h>

#include "videoserver.h"

void usage() {
	printf("Usage:\n");
	printf("%s [-h] [-n numberOfThread] [-f mappingsFile]\n\n", "mfserver");
}

int main (int argc, const char **argv) {
	int opt, nWorkers = 1;
	VideoServer videoServer;
	std::string filename("mappings.txt");
    
	while((opt = getopt(argc, (char **)argv, "hn:f:")) != -1) {
		switch(opt) {
			case 'h':
				usage();
				exit(0);
			case 'n':
				nWorkers = atoi(optarg);
				break;
			case 'f':
				filename.assign(optarg);
				break;
			case '?':
				usage();
				exit(-1);
			default:
				break;
		}
	}
	videoServer.setMappingsFile(filename);
	videoServer.setNumberOfWorkers(nWorkers);
	videoServer.init();
	videoServer.run();
	return EXIT_SUCCESS;
	
}
