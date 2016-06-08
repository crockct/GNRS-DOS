#ifndef MF_FTP_UTILS_H_
#define MF_FTP_UTILS_H_

#include <string.h>

const char* extract_filename(const char *path); 
int gen_server_path(const char* filename, const char * dir, char *path); 

#endif /*MF_FTP_UTIL_H_*/
