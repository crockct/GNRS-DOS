#ifndef MF_FTP_OPT_MODE_H_
#define MF_FTP_OPT_MODE_H_

#include <dirent.h>

#include "mfftputils.h"
#include "mfftptypes.h"
#include "mfftpcmd.h"

void * optmode_thread (void *arg); 

int list_opt(struct Handle *handle); 
int get_opt(struct Handle *handle);
int put_opt(struct Handle *handle); 


#endif /*MF_FTP_OPT_MODE_H_*/
