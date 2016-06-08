#include "mfftputils.h"

//get the file name from the path
const char* extract_filename(const char *path) {
  const char *p =  path;
  while(*p != '\0') {
    p++;
  }
  while(*p != '/') {
    p--;
  }
  p++;
  return p;
}

int gen_server_path(const char* filename, const char* dir, char *path) {
  strcpy(path, dir);
  strcat(path, filename);
  return 0; 
}
