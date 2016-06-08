#ifndef MF_FTPRECEIVING_H_
#define MF_FTPRECEIVING_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <dirent.h>

#include "mfftptypes.h"
#include "mfftpcmd.h"
#include "mfapi.h"

#define MFFTP_DIRECTORY "/root/mfftp_dir/"

extern struct timeval start; 

void *receiving_thread (void *arg); 

int32_t process_data(char *buf, uint32_t size, FILE *fp); 
int32_t process_list_msg (struct Handle *handle, char *buf);
int32_t process_list_resp_msg (char *buf, int size);
int32_t process_get_msg (struct Handle *handle, char *buf);
int32_t process_put_msg (char *buf, uint32_t *file_size, FILE **pf); 
int32_t process_no_file_msg (char *buf); 
int32_t process_file_info_msg(char* buf, uint32_t *file_size, FILE **fp);

#endif
