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
 * @file   mflog.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   June, 2013
 * @brief  Implement a unified log manager that prints to std output.
 *
 * Implement a unified log manager that prints to std output.
 */

#include "mflog.h"

int MF_Log::mLogLevel;
int MF_Log::mTimeLog;

MF_Log::MF_Log(){
	logLevel = MF_DEBUG;
	className[0] = '\0';
}

void MF_Log::init(int level, const char *className) {
	this->logLevel = level;
	strcpy(this->className, className);
}

void MF_Log::init(int level, string &className) {
	this->logLevel = level;
	strcpy(this->className, className.c_str());
}

void MF_Log::init_mf_log(){

	char * mf_ll;
	mf_ll = getenv("MF_CLICK_LOG_LEVEL");
	if(mf_ll != NULL){
		mLogLevel = get_log_level(atoi(mf_ll));
	}else{
		mLogLevel = MF_ERROR;
		fprintf(stdout,
				"WARN: MF_CLICK_LOG_LEVEL not set, "
				"setting to default %d == ERROR\n", mLogLevel);
	}
	mTimeLog = 0;
}

void MF_Log::init_mf_log(log_level_t lvl){
	mLogLevel = lvl;
	mTimeLog = 0;
}

void MF_Log::enable_mf_time_log(){
	mTimeLog = 1;
}

log_level_t MF_Log::get_log_level(int lvl){

	switch(lvl){

	case MF_DEBUG: return MF_DEBUG;
	case MF_INFO: return MF_INFO;
	case MF_TIME: return MF_TIME;
	case MF_WARN: return MF_WARN;
	case MF_ERROR: return MF_ERROR;
	case MF_CRITICAL: return MF_CRITICAL;
	case MF_FATAL: return MF_FATAL;
	default: return MF_ERROR;
	}

}

void MF_Log::set_log_level_str(log_level_t lvl, char* str){

	switch(lvl){

	case MF_DEBUG: sprintf(str, "DEBUG: ");break;
	case MF_INFO: sprintf(str, "INFO: ");break;
	case MF_TIME: sprintf(str, "TIME: ");break;
	case MF_WARN: sprintf(str, "WARN: ");break;
	case MF_ERROR: sprintf(str, "ERROR: ");break;
	case MF_CRITICAL: sprintf(str, "CRITICAL: ");break;
	case MF_FATAL: sprintf(str, "FATAL: ");break;
	//case default: sprintf(str, "DEBUG: ");break;
	}
}

void MF_Log::mf_log(log_level_t msg_type,
		const char* fmt, ...){

	if(msg_type < mLogLevel)return;

	va_list val;
	va_start(val, fmt);

	//truncate after about 3 lines (each of 80 chars)
	char buf[256];
	set_log_level_str(msg_type, buf);
	int start_index = strlen(buf);
	int max = 256 - start_index - 1;
	int req; //no of bytes vsnprint would write if space allows
	int last_index = 253; //position of last char of log message
	if((req = vsnprintf(buf+start_index, max, fmt, val)) < max){
		last_index = start_index - 1 + req;
	}
	//buf[last_index + 1] = '\n';
	buf[last_index + 2] ='\0';
	std::cout << buf << std::endl;

	//fputs((const char*) buf, stdout);
	va_end(val);
}

void MF_Log::mf_log_time(const char* fmt, ...){

	if(mTimeLog){
		va_list val;
		va_start(val, fmt);

		//truncate after about 3 lines (each of 80 chars)
		char buf[256];
		struct timeval ts;
		gettimeofday(&ts, NULL);
		sprintf(buf, "TIME: [%lu]: ",
				ts.tv_sec*1000000 + ts.tv_usec);
		int start_index = strlen(buf);
		int max = 256 - start_index - 1;
		int req; //no of bytes vsnprint would write if space allows
		int last_index = 253; //position of last char of log message
		if((req = vsnprintf(buf+start_index, max, fmt, val)) < max){
			last_index = start_index - 1 + req;
		}
		//buf[last_index + 1] = '\n';
		buf[last_index + 2] ='\0';
		std::cout << buf << std::endl;

		//fputs((const char*) buf, stdout);
		va_end(val);
	}
}

void MF_Log::log_d(const char* fmt, ...){
	if(logLevel <= MF_DEBUG)return;

	va_list val;
	va_start(val, fmt);

	//truncate after about 3 lines (each of 80 chars)
	char buf[256];
	sprintf(buf, "[DEBUG][%s]",className);
	int start_index = strlen(buf);
	int max = 256 - start_index - 1;
	int req; //no of bytes vsnprint would write if space allows
	int last_index = 253; //position of last char of log message
	if((req = vsnprintf(buf+start_index, max, fmt, val)) < max){
		last_index = start_index - 1 + req;
	}
	//buf[last_index + 1] = '\n';
	buf[last_index + 2] ='\0';
	std::cout << buf << std::endl;

	//fputs((const char*) buf, stdout);
	va_end(val);
}

void MF_Log::log_i(const char* fmt, ...){
	if(logLevel <= MF_INFO)return;

	va_list val;
	va_start(val, fmt);

	//truncate after about 3 lines (each of 80 chars)
	char buf[256];
	sprintf(buf, "[INFO][%s]",className);
	int start_index = strlen(buf);
	int max = 256 - start_index - 1;
	int req; //no of bytes vsnprint would write if space allows
	int last_index = 253; //position of last char of log message
	if((req = vsnprintf(buf+start_index, max, fmt, val)) < max){
		last_index = start_index - 1 + req;
	}
	//buf[last_index + 1] = '\n';
	buf[last_index + 2] ='\0';
	std::cout << buf << std::endl;

	//fputs((const char*) buf, stdout);
	va_end(val);
}

void MF_Log::log_w(const char* fmt, ...){
	if(logLevel <= MF_WARN)return;

	va_list val;
	va_start(val, fmt);

	//truncate after about 3 lines (each of 80 chars)
	char buf[256];
	sprintf(buf, "[WARN][%s]",className);
	int start_index = strlen(buf);
	int max = 256 - start_index - 1;
	int req; //no of bytes vsnprint would write if space allows
	int last_index = 253; //position of last char of log message
	if((req = vsnprintf(buf+start_index, max, fmt, val)) < max){
		last_index = start_index - 1 + req;
	}
	//buf[last_index + 1] = '\n';
	buf[last_index + 2] ='\0';
	std::cout << buf << std::endl;

	//fputs((const char*) buf, stdout);
	va_end(val);
}

void MF_Log::log_e(const char* fmt, ...){
	if(logLevel <= MF_ERROR)return;

	va_list val;
	va_start(val, fmt);

	//truncate after about 3 lines (each of 80 chars)
	char buf[256];
	sprintf(buf, "[ERROR][%s]",className);
	int start_index = strlen(buf);
	int max = 256 - start_index - 1;
	int req; //no of bytes vsnprint would write if space allows
	int last_index = 253; //position of last char of log message
	if((req = vsnprintf(buf+start_index, max, fmt, val)) < max){
		last_index = start_index - 1 + req;
	}
	//buf[last_index + 1] = '\n';
	buf[last_index + 2] ='\0';
	std::cout << buf << std::endl;

	//fputs((const char*) buf, stdout);
	va_end(val);
}

void MF_Log::log_c(const char* fmt, ...){
	if(logLevel <= MF_CRITICAL)return;

	va_list val;
	va_start(val, fmt);

	//truncate after about 3 lines (each of 80 chars)
	char buf[256];
	sprintf(buf, "[CRITICAL][%s]",className);
	int start_index = strlen(buf);
	int max = 256 - start_index - 1;
	int req; //no of bytes vsnprint would write if space allows
	int last_index = 253; //position of last char of log message
	if((req = vsnprintf(buf+start_index, max, fmt, val)) < max){
		last_index = start_index - 1 + req;
	}
	//buf[last_index + 1] = '\n';
	buf[last_index + 2] ='\0';
	std::cout << buf << std::endl;

	//fputs((const char*) buf, stdout);
	va_end(val);
}

void MF_Log::log_f(const char* fmt, ...){
	if(logLevel <= MF_FATAL)return;

	va_list val;
	va_start(val, fmt);

	//truncate after about 3 lines (each of 80 chars)
	char buf[256];
	sprintf(buf, "[FATAL][%s]",className);
	int start_index = strlen(buf);
	int max = 256 - start_index - 1;
	int req; //no of bytes vsnprint would write if space allows
	int last_index = 253; //position of last char of log message
	if((req = vsnprintf(buf+start_index, max, fmt, val)) < max){
		last_index = start_index - 1 + req;
	}
	//buf[last_index + 1] = '\n';
	buf[last_index + 2] ='\0';
	std::cout << buf << std::endl;

	//fputs((const char*) buf, stdout);
	va_end(val);
}

void MF_Log::log_t(const char* fmt, ...){
	if(logLevel <= MF_TIME)return;

	va_list val;
	va_start(val, fmt);

	//truncate after about 3 lines (each of 80 chars)
	char buf[256];
	struct timeval ts;
	sprintf(buf, "[TIME][%lu[%s]",ts.tv_sec*1000000 + ts.tv_usec,className);
	int start_index = strlen(buf);
	int max = 256 - start_index - 1;
	int req; //no of bytes vsnprint would write if space allows
	int last_index = 253; //position of last char of log message
	if((req = vsnprintf(buf+start_index, max, fmt, val)) < max){
		last_index = start_index - 1 + req;
	}
	//buf[last_index + 1] = '\n';
	buf[last_index + 2] ='\0';
	std::cout << buf << std::endl;

	//fputs((const char*) buf, stdout);
	va_end(val);
}
