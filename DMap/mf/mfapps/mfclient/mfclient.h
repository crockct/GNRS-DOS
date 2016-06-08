#ifndef MFCLIENT_H_
#define MFCLIENT_H_
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mobilityfirst/mfapi.h>
#include <arpa/inet.h>
#include <pthread.h>

#define VERSION 1.0

#define ARGUMENT_ERROR -1
#define MFOPEN_ERROR -2
#define MFATTACH_ERROR -3

#define MAX_MSG_LEN 256
#define MAX_FILENAME_LEN 256

//#define CHUNK_LEN MAX_PAYLOAD_SIZE*1000-ROUTE_HEADER_LEN-TRANS_HEADER_LEN-NEXT_HEADER_RESERVED_SIZE
#define CHUNK_LEN 1024000

#define FILE_DATA 0
#define FILE_META 1
#define MSG 2

typedef struct {
  struct Handle *handle;
  bool sendflag;
  bool receiveflag; 
} option_thread_arg_t;

typedef struct {
  uint32_t type;
  uint32_t file_size; 
  char filename[MAX_FILENAME_LEN];
} trans_file_t;

typedef struct {
  uint32_t type;
  char msg[MAX_MSG_LEN];  
} trans_msg_t;

typedef struct {
  uint32_t type;
  char *file_data;
} trans_file_data_t; 

static void *receive_thread_start(void *arg); 
static void *option_thread_start(void *arg); 

int send_msg(struct Handle *handle);
int send_file(struct Handle *handle); 
int attach_guid(struct Handle *handle);

void gen_random_filename(char*, const int len);
int* parseAttachedGUIDs (char *guids, int *guid_num);
#endif
