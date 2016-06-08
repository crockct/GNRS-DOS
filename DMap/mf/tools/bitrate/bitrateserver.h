#ifndef BITRATE_SERVER_H_
#define BITRATE_SERVER_H_

#include <time.h>
#include "bitrate.h"

bool debug = false; 

//declaration for wimax
const char* get_wimax_bitrate(char*);
const char* codeToBitrate(char*);

//declaration for wifi 
const char*  get_wifi_bitrate(const char *interface, char* mac);
int matchText(char *buffer, u_int index, const char *text);
char * getMacAddr(char *buffer, int index);
char * extractBitRate(char *buffer, int index);

#endif /*BITRATE_SERVER_H_*/
