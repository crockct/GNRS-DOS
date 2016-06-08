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
 * @file   mfsettings.cpp
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   September, 2013
 * @brief  Classes that contains the parser of the settings file and the functions to access it.
 *
 * Classes that contains the parser of the settings file and the functions to access it.
 */


#include "mfsettings.h"

MF_Settings::MF_Settings(){
	defaultBufferSize = DEFAULT_BUFFER_SIZE;
	policy = DEFAULT_POLICY;
	defaultGUID = 0;
	if_scan_period = DEFAULT_NET_MAN_LOOPTIME;
	csynTOMicros = DEFAULT_CSYN_TO_MICROS;
	assocTOMicros = DEFAULT_ASSOC_TO_MICROS;
	errorOccurred = false;
	socketAliveTimeout = 10000; //Seconds
}

MF_Settings::~MF_Settings(){

}

char *MF_Settings::removeSpaces(char *ptr){
	char *init = ptr, *end;
	while(*init && (*init == ' '))init++;
	end = init;
	while(*end && *end != ' ' && *end != '\t')end++;
	if(*end == ' ' || *end == '\t' || *end == '\n') *(end) = '\0';
	return init;
}

InterfaceDescr *MF_Settings::parseInterface(char *ptr){
	char *temp;
	InterfaceDescr *newInt = new InterfaceDescr();
	temp = strtok(ptr, ",");
	temp = removeSpaces(temp);
	if(!strcmp(temp,"wifi") || !strcmp(temp, "WIFI")){
		newInt->setType(WIFI);
	}
	else if(!strcmp(temp,"wimax") || !strcmp(temp, "WIMAX")){
		newInt->setType(WIMAX);
	}
	else if(!strcmp(temp,"ether") || !strcmp(temp, "ETHER")){
		newInt->setType(ETHER);
	}
	else{
		errorOccurred = true;
		errDescr.append("error: wrong interface type ");
		errDescr.append(temp);
	}
	temp = strtok(NULL, ",");
	temp = removeSpaces(temp);
	newInt->setName(temp);
	temp = strtok(NULL, ",");
	temp = removeSpaces(temp);
	if(!strcmp(temp,"auto") || !strcmp(temp, "AUTO")){
		newInt->setMode(AUTO);
	}
	else if(!strcmp(temp,"manual") || !strcmp(temp, "MANUAL")){
		newInt->setMode(MANUAL);
		temp = strtok(NULL, ",");
		temp = removeSpaces(temp);
		newInt->setApip(temp);
		temp = strtok(NULL, " \n\t#");
		temp = removeSpaces(temp);
		newInt->setApmac(temp);
	}
	else{
		errorOccurred = true;
		errDescr.append("error: wrong mode ");
		errDescr.append(temp);
	}
	if(errorOccurred){
		delete newInt;
		newInt = NULL;
	}
	return newInt;
}

PolicyType MF_Settings::parsePolicy(char *ptr){
	char *temp;
	PolicyType pol;
	temp = strtok(ptr, " \n\t#");
	while(*temp && (*temp == ' '))temp++;
	if(!strcmp(temp,"bestperformance") || !strcmp(temp, "BESTPERFORMANCE")){
		pol = POLICY_BESTPERFORMANCE;
	}
	else if(!strcmp(temp,"wifionly") || !strcmp(temp, "WIFIONLY")){
		pol = POLICY_WIFIONLY;
	}
	else if(!strcmp(temp,"wimaxonly") || !strcmp(temp, "WIMAXONLY")){
		pol = POLICY_WIMAXONLY;
	}
	else if(!strcmp(temp,"etheronly") || !strcmp(temp, "ETHERONLY")){
		pol = POLICY_ETHERONLY;
	}
	else if(!strcmp(temp,"weighted11") || !strcmp(temp, "WEIGHTED11")){
		pol = POLICY_WEIGHTED11;
	}
	else if(!strcmp(temp,"weighted31") || !strcmp(temp, "WEIGHTED31")){
		pol = POLICY_WEIGHTED31;
	}
	else{
		errorOccurred = true;
		errDescr.append("error: wrong mode ");
		errDescr.append(temp);
		pol = POLICY_BESTPERFORMANCE;
	}
	return pol;
}

int MF_Settings::parseInt(char *ptr){
	char *temp;
	temp = strtok(ptr, " \n\t#");
	while(*temp && (*temp == ' '))temp++;
	return atoi(temp);
}

float MF_Settings::parseFloat(char *ptr){
	char *temp;
	temp = strtok(ptr, " \n\t#");
	while(*temp && (*temp == ' '))temp++;
	return 0;
}

double MF_Settings::parseDouble(char *ptr){
	char *temp;
	temp = strtok(ptr, " \n\t#");
	while(*temp && (*temp == ' '))temp++;
	return atof(temp);
}

bool MF_Settings::parseBoolean(char *ptr) {
	char *temp = strtok(ptr, " \n\t#");
	while (*temp && *temp == ' ') ++temp;
	char *str_p = temp;
	while (*str_p) {
		*str_p = std::tolower(*str_p);
		++str_p;
	}
	return strlen(temp) >= 4 && strncmp(temp, "true", 4) == 0;
}

void MF_Settings::init(const char *filename){
	ifstream in(filename, ios::in);
	if (!in){
		MF_Log::mf_log(MF_ERROR, "error opening file %s",filename);
		errDescr.append("error opening file ");
		errDescr.append(filename);
		errorOccurred = true;
		return;
	}
	char *ptr,buffer[512],*key,*value;
	InterfaceDescr *temp;
	while (!in.eof()){
		//extracting all pairs key-value
		in.getline(buffer,512);
		if(!*buffer || *buffer == '#')continue;
		ptr = buffer;
		while(*ptr && (*ptr == ' ' ||*ptr == '\t'))ptr++;
		key = strtok(ptr, "=");
		value = strtok(NULL, "=");
		switch(*key){
		case 'A':
			if(!strncmp(buffer,"ASSOC_TO",8)){
				assocTOMicros = parseInt(value);
				if(!errorOccurred){
					MF_Log::mf_log(MF_DEBUG, "assoc TO parsed: %d", assocTOMicros);
				}
			}
			else{
				errDescr.append("warning: invalid label ");
				errDescr.append(buffer);
				errorOccurred = true;
			}
			break;
		case 'B':
			if(!strncmp(buffer,"BUFFER_SIZE",11)){
				defaultBufferSize = parseInt(value);
				if(!errorOccurred){
					MF_Log::mf_log(MF_DEBUG, "Default buffer size parsed: %d", defaultBufferSize);
				}
			}
			else{
				errDescr.append("warning: invalid label ");
				errDescr.append(buffer);
				errorOccurred = true;
			}
			break;
		case 'C':
			if(!strncmp(buffer,"CSYN_TO",7)){
				csynTOMicros = parseInt(value);
				if(!errorOccurred){
					MF_Log::mf_log(MF_DEBUG, "csyn TO parsed: %d", csynTOMicros);
				}
			}
			else{
				errDescr.append("warning: invalid label ");
				errDescr.append(buffer);
				errorOccurred = true;
			}
			break;
		case 'D':
			if(!strncmp(buffer,"DEFAULT_GUID",12)){
				defaultGUID = parseInt(value);
				if(!errorOccurred){
					MF_Log::mf_log(MF_DEBUG, "Default GUID parsed: %d", defaultGUID);
				}
			}
			else{
				errDescr.append("warning: invalid label ");
				errDescr.append(buffer);
				errorOccurred = true;
			}
			break;
		case 'I':
			if(!strncmp(buffer,"INTERFACE",9)){
				temp = parseInterface(value);
				if(!errorOccurred){
					ifList.push_back(temp);
					MF_Log::mf_log(MF_DEBUG, "New interface parsed:");
					temp->printInterface();
				}
			}
			else if(!strncmp(buffer,"IF_SCAN_PERIOD",14)){
				if_scan_period = parseInt(value);
				if(!errorOccurred){
					MF_Log::mf_log(MF_DEBUG, "Policy parsed: %d", policy);
				}
			}
			else{
				errDescr.append("warning: invalid label ");
				errDescr.append(buffer);
				errorOccurred = true;
			}
			break;
		case 'P':
			if(!strncmp(buffer,"POLICY",6)){
				policy = parsePolicy(value);
				if(!errorOccurred){
					MF_Log::mf_log(MF_DEBUG, "Policy parsed: %d", policy);
				}
			}
			else{
				errDescr.append("warning: invalid label ");
				errDescr.append(buffer);
				errorOccurred = true;
			}
			break;
		case 'S':
			if(!strncmp(buffer,"SOCKET_TO",9)){
				socketAliveTimeout = parseInt(value);
				if(!errorOccurred){
					MF_Log::mf_log(MF_DEBUG, "Socket timeout: %d", socketAliveTimeout);
				}
			}
			else{
				errDescr.append("warning: invalid label ");
				errDescr.append(buffer);
				errorOccurred = true;
			}
			break;
		case 'T':
			if (!strncmp(buffer, "TRANS_RELIABLE", strlen("TRANS_RELIABLE"))) {
				transSetting.isReliabTrans = parseBoolean(value);
				if (!errorOccurred) {
					MF_Log::mf_log(MF_DEBUG, "TRANS_RELIABLE parsed: %d", transSetting.isReliabTrans);
				}
			} else if (!strncmp(buffer, "TRANS_CHK_COUNT_NACK_THRESH", strlen("TRANS_CHK_COUNT_NACK_THRESH"))) {
				transSetting.chkCountNACKThresh = parseInt(value);
				if (!errorOccurred)
					MF_Log::mf_log(MF_DEBUG, "TRANS_CHK_COUNT_NACK_THRESH parsed: %d", transSetting.chkCountNACKThresh);
			} else if (!strncmp(buffer, "TRANS_SEND_NACK_TIMEOUT", strlen("TRANS_SEND_NACK_TIMEOUT"))) {
				transSetting.sendNACKTOutPerChk = parseInt(value);
				if (!errorOccurred)
					MF_Log::mf_log(MF_DEBUG, "TRANS_SEND_NACK_TIMEOUT parsed: %d", transSetting.sendNACKTOutPerChk);
			} else if (!strncmp(buffer, "TRANS_RECV_NACK_TIMEOUT", strlen("TRANS_RECV_NACK_TIMEOUT"))) {
				transSetting.recvNACKTOutPerChk = parseInt(value);
				if (!errorOccurred)
					MF_Log::mf_log(MF_DEBUG, "TRANS_RECV_NACK_TIMEOUT parsed: %d", transSetting.recvNACKTOutPerChk);
			} else {
				errDescr.append("warning: invalid label ");
				errDescr.append(buffer);
				errorOccurred = true;
			}
			break;
		default:
			errDescr.append("warning: invalid label ");
			errDescr.append(buffer, 1);
			errorOccurred = true;
			break;
		}
		if(errorOccurred){
			MF_Log::mf_log(MF_ERROR, "Error parsing settings: %s", errDescr.c_str());
			break;
		}
		memset(buffer, 0, 512);
	}
	in.close();
	if(!errorOccurred && ifList.size() == 0){
		errDescr.append("Error: no interface specified");
		errorOccurred = true;
		MF_Log::mf_log(MF_ERROR, "No interface specified in settings file");
	}
	else if(!errorOccurred && defaultGUID == 0){
		errDescr.append("Error: no default GUID specified");
		errorOccurred = true;
		MF_Log::mf_log(MF_ERROR, "No default GUID specified in settings file");
	}
}

void MF_Settings::init(){
	init(".settings");
}

void MF_Settings::init(string filename){
	init(filename.c_str());
}
