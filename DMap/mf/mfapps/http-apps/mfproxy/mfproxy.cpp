#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mfproxy.h"
#include "httphandler.h"
#include "mfhandler.h"

bool MFPROXY_DEBUG = false;

void usage(){
	std::cout << "mfproxy -m|p [options]" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "\tl\tLog output file" << std::endl;
	std::cout << "\te\tLog error file" << std::endl;
	std::cout << "\ti\tIP address to be used. For http->mf corresponds to the listening IP. for mf->http corresponds to IP the webserver is listening on." << std::endl;
	std::cout << "\tP\tSame as IP but for the port number" << std::endl;
	std::cout << "\tt\tNumber of threads used. For the http->mf this corresponds to the number of mf sockets used" << std::endl;
	std::cout << "\tg\tLocal Guid. For http->mf the range [x,x + number expressed by the option -t] is used" << std::endl;
	std::cout << "\tN\tMapping file for the local resolver (URL to guid translation and vice-versa). If this option is specified, the option -s is not considered" << std::endl;
	std::cout << "\ts\tGuid for the other end proxy to forward traffic to. Not considered if -N is specified" << std::endl;
	std::cout << "\th\tPrint this message." << std::endl;
}

int main(int argc, char **argv) {
	char *ip = "127.0.0.1\0", *port = "8080\0", ip_port[256];
	char *log_file = "acc.log";
	char *err_file = "err.log";
	char *threads = "1";
	char *nameRes = "mappings.txt";
	int guid = 0;
	int myGuid = 0;
	int startingGuid = 1000;
	int type = 0;
	int nThreads = 1;

	int opt;
	while((opt = getopt(argc, (char **)argv, "dhvmpl:e:i:t:g:P:N:s:")) != -1) {
		switch(opt) {
		case 'd':
			std::cout << "Running in debug mode" << std::endl;
			MFPROXY_DEBUG = true;
			break;
		case 'm':
			std::cout << "Running as mf to http proxy" << std::endl;
			type = 1;
			break;
		case 'p':
			std::cout << "Running as http to mf proxy" << std::endl;
			type = 2;
			break;
		case 'l':
			std::cout << "Log output to file " << optarg << std::endl;
			log_file = optarg;
			break;
		case 'e':
			std::cout << "Err output to file " << optarg << std::endl;
			err_file = optarg;
			break;
		case 'i':
			std::cout << "IP:port pair used " << optarg << std::endl;
			ip = optarg;
			break;
		case 'P':
			std::cout << "IP:port pair used " << optarg << std::endl;
			port = optarg;
			break;
		case 't':
			std::cout << "Number of threads to be used " << optarg << std::endl;
			threads = optarg;
			nThreads = atoi(optarg);
			break;
		case 'g':
			std::cout << "Application GUID to be used " << optarg << std::endl;
			myGuid = atoi(optarg);
			startingGuid = atoi(optarg);
			break;
		case 's':
			std::cout << "GUID to be used " << optarg << std::endl;
			guid = atoi(optarg);
			break;
		case 'N':
			std::cout << "Mappings file for resolver to be used " << optarg << std::endl;
			nameRes = optarg;
			break;
		case 'h':
			usage();
			exit(0);
		default:
			break;
		}
	}

	strcpy(ip_port, ip);
	strncat(ip_port, ":", 1);
	int ports = sizeof(port);
	strncat(ip_port, port, ports);

	const char *options[] = {"document_root", ".", "access_log_file", log_file, "error_log_file", err_file,"listening_ports", ip_port, "num_threads", threads, NULL};
	
	if(argc < 2) {
		usage();
		return -1;
	}
	
	if(type == 1){
		MFHandler *mfHandler = new MFHandler();
		mfHandler->init(options, (const char *)ip, atoi(port), nThreads, myGuid);
		mfHandler->run();
		delete mfHandler;
	} else if (type == 2){
		HTTPHandler *httpHandler = new HTTPHandler();
		if(guid != 0){
			httpHandler->init(options, guid, startingGuid, nThreads);
		}
		else{
			httpHandler->init(options, nameRes, startingGuid, nThreads);
		}
		httpHandler->run();
		delete httpHandler;
	} else{
		std::cerr << "ERROR: No running type selected" << std::endl;
	}

	return 0;
}
