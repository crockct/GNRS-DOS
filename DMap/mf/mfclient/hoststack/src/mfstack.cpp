/*
 *  Copyright (c) 2010-2013, Rutgers, The State University of New Jersey
 *  All rights reserved.
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of the organization(s) stated above nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 \mainpage MobilityFirst's API documentation page
 
 We are building a proof-of-concept protocol stack to include a basic transport layer (message chunking, e-2-e acks, flow control), GUID service layer, storage aware routing, and hop-by-hop link data transport. The clean slate implementation will export a GUID-centered API with message-based communication primitives, and rich delivery service options (e.g., DTN, real-time, multicast, etc). We will also explore a 'network interface manager' functionality to enable policy driven multi-homing that could flexibly address user/application stated goals of performance, reliability, battery lifetime, network svc. payments, etc. The protocol stack will be implemented in C/C++ and will be standalone user-level process, while apps will link to the service API library to use the new stack.
 */

/**
 * @file   mfstack.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   July, 2013
 * @brief  Temp.
 *
 * Temp.
 */

#include "mfstack.h"



void usage() {
	printf("Usage:\n");
#ifdef __OML__
	printf("%s [-h] [-d|i|t|w|e|c|f] [-t] [-O] settings_file\n\n", PACKAGE);
#else
	printf("%s [-h] [-d|i|t|w|e|c|f] [-t] settings_file\n\n", PACKAGE);
#endif 
	printf("  -h\tprint this help and exit\n");
	printf("  -v\tprint version file and exit\n");
	printf("  -d|i|w|e|c|f|t\tset log level(DEBUG|INFO|WARN|ERROR|CRITICAL|FATAL|TIME), default:QUIET\n");
#ifdef __OML__
	printf("  -O\tstart measurements collection with OML\n");
#endif
	printf("  settings_file\trequired\n");
}

int main (int argc, const char **argv) {
	int opt, count = 0;
#ifdef __OML__
	int ret;
	MF_OML* mfoml = new MF_OML();

	if((ret = mfoml->init("mfstack", &argc, argv)) < 0) {
		printf("Could not initialize OML\n");
		return -1;
	}
	bool OML = false;
#endif
	bool logL = false;
	
	if (argc < 2) {
		fprintf(stderr, "%s: Error - incorrect number of command-line options\n\n", PACKAGE);
		usage();
		exit(EXIT_FAILURE);
	}
    
	while((opt = getopt(argc, (char **)argv, "hvditwecfO")) != -1) {
		count ++;
		switch(opt) {
		case 'h':
			usage();
			exit(0);
		case 'v':
			printf("%s %s\n\n", PACKAGE, VERSION);
			exit(0);
		case 'd':
			logL = true;
			MF_Log::init_mf_log(MF_DEBUG);
			break;
		case 'i':
			logL = true;
			MF_Log::init_mf_log(MF_INFO);
				break;
		case 'w':
			logL = true;
			MF_Log::init_mf_log(MF_WARN);
			break;
		case 'e':
            logL = true;
			MF_Log::init_mf_log(MF_ERROR);
			break;
		case 'c':
			logL = true;
			MF_Log::init_mf_log(MF_CRITICAL);
			break;
		case 'f':
			logL = true;
			MF_Log::init_mf_log(MF_FATAL);
			break;
		case 't':
			MF_Log::enable_mf_time_log();
			break;
#ifdef __OML__
		case 'O':
			OML = true;
			break;
#endif
		case ':':
			usage();
			exit(1);
		case '?':
			usage();
			exit(1);
		default:
			break;
		}
	}
    
	if(!logL){
		MF_Log::init_mf_log(MF_INFO);
		MF_Log::mf_log(MF_INFO, "mfstack: No log level specified, setting to INFO\n");
	}
	
	string settings_file(argv[count+1]);
	MF_Settings *setts = new MF_Settings();
	setts->init(settings_file);
	if(setts->error()){
		MF_Log::mf_log(MF_ERROR, setts->errstr());
		exit(EXIT_FAILURE);
	}

	/*
	 * MF_Main runs in current thread and takes care of the commands sent from API
	 */
	MF_Main *m;
#ifdef __OML__
	if(OML){
		mfoml->start();
		m = new MF_Main(mfoml);
	}
	else{
		m = new MF_Main();
	}
#else
	m = new MF_Main();
#endif
	MF_Log::mf_log(MF_DEBUG, "mfstack: ready to start, settings file %s",settings_file.c_str());
	m->start(setts);
#ifdef __OML__
	mfoml->stop();
#endif
	
	return EXIT_SUCCESS;

} /* main */
