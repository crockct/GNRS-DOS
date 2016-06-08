#ifndef BITRATE_H_
#define BITRATE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

#define VERSION 1.0

#define MAC_BUF_SIZE 18                    //length of a mac addr string is 18
#define MAC_BYTE_SIZE 6                    //6 bytes mac 

#define IP_BUF_SIZE 16                     //max length of a ip addr string is 16, for example 255.255.255.255
#define BUF_SIZE 1500
#define DEFAULT_SERVER_PORT 6001
#define DEFAULT_CLIENT_PORT 6002

typedef struct {
  uint32_t lookup_id;
  char mac[MAC_BYTE_SIZE];
} bitrate_local_lookup_t;

typedef struct {
  uint32_t lookup_id;
  uint32_t bitrate;
  uint32_t valid_sec;
} bitrate_local_resp_t;

//convert a mac address from bytes to string
void macToString(char *mac, char *mac_str); 
//check whether a mac addr is valid
bool isValidMacAddr(const char *mac);
//check whether a ip addr is valid
bool isValidIpAddr(const char *ip);        //supports IPV4 only

//const char *
typedef struct {
  char *mac_addr;
  char *bitrate;
} mac_bitrate_t;

#endif /*BITRATE_H_*/
