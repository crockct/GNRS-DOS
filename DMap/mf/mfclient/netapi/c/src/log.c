/*
 *  Copyright (c) 2010-2013, Rutgers, The State University of New Jersey
 *  All rights reserved.
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of the organization(s) stated above nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/// @cond API_ONLY

/**
 * @file   log.c
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   June, 2013
 * @brief  Logging methods.
 *
 * Logging methods. Differentiated between UNIX and Android
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include "log.h"

#ifdef API_DEBUG
int mfapi_debug = 1;
#else
int mfapi_debug = 0;
#endif

#ifdef __ANDROID__

/*! Prints a string on the standard output
 * \param str The string to print. */
void stdLog(char *str){
	if(mfapi_debug){
		__android_log_write(ANDROID_LOG_INFO, "MFAPI: ", str);
		printf("Log from MFAPI: %s", str);
	}
}

/*! Prints a string on the standard output and an integer value using printf formatting system
 * \param str The string to print.
 * \param val The integer to print */
void stdLogVal(char *str, int val){
	if(mfapi_debug){
		__android_log_print(ANDROID_LOG_INFO, "MFAPI: ", str, val);
		printf(str, val);
	}
}

/*! Prints a string on the standard output and an other string value using printf formatting system
 * \param str The string to print.
 * \param val The other string to print */
void stdLogStr(char *str, char *val){
	if(mfapi_debug){
		__android_log_print(ANDROID_LOG_INFO, "MFAPI: ", str, val);
		printf(str, val);
	}
}

/*! Prints a string on the error output
 * \param str The string to print. */
void errLog(char *str){
	__android_log_write(ANDROID_LOG_ERROR, "MFAPI: ", str);
	printf(str);
}

/*! Prints a string on the error output and an integer value using printf formatting system
 * \param str The string to print.
 * \param val The integer to print */
void errLogVal(char *str, int val){
	__android_log_print(ANDROID_LOG_ERROR, "MFAPI: ", str, val);
	printf("Error from MFAPI: %s", str, val);
}

/*! Prints a string on the error output and an other string value using printf formatting system
 * \param str The string to print.
 * \param val The other string to print */
void errLogStr(char *str, char *val){
	__android_log_print(ANDROID_LOG_ERROR, "MFAPI: ", str, val);
	printf(str, val);
}

#else

/*! Prints a string on the standard output
 * \param str The string to print. */
void stdLog(char *str){
	if(mfapi_debug) {
		fprintf(stdout, "MFAPI: %s", str);
	}
}

/*! Prints a string on the standard output and an integer value using printf formatting system
 * \param str The string to print.
 * \param val The integer to print */
void stdLogVal(char *str, int val){
	if(mfapi_debug) {
		char strt[256];
		strcpy(strt, "MFAPI: \0");
		fprintf(stdout, strcat(strt, str), val);
	}
}

/*! Prints a string on the standard output and an other string value using printf formatting system
 * \param str The string to print.
 * \param val The other string to print */
void stdLogStr(char *str, char *val){
	if(mfapi_debug) {
		char strt[256];
		strcpy(strt, "MFAPI: \0");
		fprintf(stdout, strcat(strt, str), val);
	}
}

/*! Prints a string on the error output
 * \param str The string to print. */
void errLog(char *str){
	fprintf(stderr, "MFAPI [ERROR]: %s", str);
}

/*! Prints a string on the error output and an integer value using printf formatting system
 * \param str The string to print.
 * \param val The integer to print */
void errLogVal(char *str, int val){
	char strt[256];
	strcpy(strt, "MFAPI [ERROR]: \0");
	fprintf(stderr, strcat(strt, str), val);
}

/*! Prints a string on the error output and an other string value using printf formatting system
 * \param str The string to print.
 * \param val The other string to print */
void errLogStr(char *str, char *val){
	char strt[256];
	strcpy(strt, "MFAPI [ERROR]: \0");
	fprintf(stderr, strcat(strt, str), val);
}

#endif
/// @endcond
