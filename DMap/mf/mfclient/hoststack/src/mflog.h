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

/**
 * @file   mflog.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   June, 2013
 * @brief  Implement a unified log manager that prints to std output.
 *
 * Implement a unified log manager that prints to std output.
 */

#ifndef MF_LOG_H_
#define MF_LOG_H_

#include <iostream>

#include <cstdarg>
#include <cstdlib>
#include <sys/time.h>
#include <string.h>

#include "mftypes.h"

// Log levels definition
enum log_level_t {
	MF_DEBUG = 1,
	MF_INFO = 2,
	MF_TIME = 3,
	MF_WARN = 4,
	MF_ERROR = 5,
	MF_CRITICAL = 6,
	MF_FATAL = 7
};

using namespace std;

class MF_Log {
	static int mLogLevel;
	static int mTimeLog;
	int logLevel;
	char className[64];

    
public:

	MF_Log();
	void init(int level, const char *className);
	void init(int level, string &className);
	static void init_mf_log();
	static void init_mf_log(log_level_t lvl);
	static void enable_mf_time_log();
	static log_level_t get_log_level(int lvl);
	static void set_log_level_str(log_level_t lvl, char* str);
	static void mf_log(log_level_t msg_type, const char* fmt, ...);
	static void mf_log_time(const char* fmt, ...);
	void init_logger();
	void init_logger(log_level_t lvl);
	void log_d(const char* fmt, ...);
	void log_i(const char* fmt, ...);
	void log_w(const char* fmt, ...);
	void log_e(const char* fmt, ...);
	void log_c(const char* fmt, ...);
	void log_f(const char* fmt, ...);
	void log_t(const char* fmt, ...);


};

#endif /* MF_LOG_H_ */
