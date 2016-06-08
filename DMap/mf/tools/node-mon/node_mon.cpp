/*
 * Node monitor with OML backend for state persistence
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <time.h>
#include <sstream>
#include <fstream>
#include <sys/time.h>

#include <math.h>
#include <unistd.h>

#include <oml2/omlc.h>
#include "config.h"

#include "node_stats.h"

#define MON_PERIOD_SECS 1

using namespace std;


void print_usage(){

	printf("./node-mon <guid> --oml-config <oml-config-file>\n");
}

int main(int argc, const char *argv[]){

    if(argc < 3) {
        print_usage();
        exit(0);
    }

    if(omlc_init("node_mon", &argc, argv, NULL) == -1) {
        cout << "FATAL: " << "Unable to init OML client" << endl;
        exit(1);
    }

    //register OML measurement points
    node_stats nd_stats(argv[1]);

    if(omlc_start() == -1) {
        cout << "FATAL: " << "Unable start OML stream" << endl;
        exit(1);
    }

    unsigned mp_index = 0;
    for(;;mp_index++) {
        nd_stats.read();	
        nd_stats.inject_into_oml(mp_index);
        sleep(MON_PERIOD_SECS);
    }
    omlc_close();

    return(0);
}
