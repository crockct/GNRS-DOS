#ifndef MF_FTPCMD_H_
#define MF_FTPCMD_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <netinet/in.h>
#include <mobilityfirst/mfapi.h>
#include <time.h>
#include <sys/time.h>

#include "mfftputils.h"
#include "mfftptypes.h"

#define LIST 1
#define GET  2 
#define PUT  3

#define CHUNK_SIZE 1024000

struct timeval start; 

int send_list_msg(struct Handle *handle, uint32_t my_guid, 
                  uint32_t dst_guid, uint32_t svc_opt); 
int send_get_msg(struct Handle *handle, uint32_t my_guid, uint32_t dst_guid,
                 char *filename);
int send_put_msg(struct Handle *handle, uint32_t my_guid, uint32_t dst_guid,
                 char *filename); 

int send_no_file_msg(struct Handle *handle, uint32_t dst_guid, 
                     char *filename);

int send_file_info(struct Handle *handle, uint32_t dst_guid, char *filename); 

int send_file(struct Handle *handle, uint32_t dst_guid, char* filename, 
              uint32_t svc_opt);

#endif
