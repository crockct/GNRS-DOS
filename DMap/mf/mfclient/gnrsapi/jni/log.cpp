/**
 *
 * User: wontoniii
 * Date: 6/13/13
 * File: log.c
 * Description: logging methods. Differentiated between UNIX and Android
 */

#include "log.h"

#ifdef API_DEBUG
int debug = 1;
#else
int debug = 0;
#endif

#ifdef __ANDROID__

void stdLog(const char *str){
	if(debug){
		__android_log_write(ANDROID_LOG_INFO, "Log from MFAPI", str);
		printf("Log from MFAPI: %s", str);
	}
}

void stdLogVal(const char *str, int val){
	if(debug){
		__android_log_print(ANDROID_LOG_INFO, "Log from MFAPI", str, val);
		printf(str, val);
	}
}

void stdLogStr(const char *str, const char *val){
	if(debug){
		__android_log_print(ANDROID_LOG_INFO, "Log from MFAPI", str, val);
		printf(str, val);
	}
}

void errLog(const char *str){
	__android_log_write(ANDROID_LOG_ERROR, "Error from MFAPI", str);
	printf(str);
}

void errLogVal(const char *str, int val){
	__android_log_print(ANDROID_LOG_ERROR, "Error from MFAPI", str, val);
	printf("Error from MFAPI: %s", str, val);
}

void errLogStr(const char *str, const char *val){
	__android_log_print(ANDROID_LOG_ERROR, "Log from MFAPI", str, val);
	printf(str, val);
}

#else

void stdLog(const char *str){
	if(debug) fprintf(stdout, "Log from MFAPI: %s", str);
}

void stdLogVal(const char *str, int val){
	if(debug) fprintf(stdout, str, val);
}

void stdLogStr(const char *str, const char *val){
	if(debug) fprintf(stdout, str, val);
}

void errLog(const char *str){
	fprintf(stderr, "Error from MFAPI: %s", str);
}

void errLogVal(const char *str, int val){
	fprintf(stderr, str, val);
}

void errLogStr(const char *str, const char *val){
	fprintf(stdout, str, val);
}

#endif
