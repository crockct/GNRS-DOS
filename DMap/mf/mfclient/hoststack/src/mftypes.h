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
 * @file   mftypes.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   July, 2013
 * @brief  Temp.
 *
 * Temp.
 */

#ifndef MFTYPES_H_
#define MFTYPES_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <include/mfroutingheader.hh>
#include <include/mfhopmsg.hh>
#include <include/mftransportheader.hh>
#include <include/mfmhomeextheader.hh>
#include <include/mfflags.h>
#include "mfipproto.h"
#include "mfetherproto.h"

//#define MAX_PAYLOAD_SIZE 1340
#define MAX_CHUNK_SIZE 750 //number of packets, this, with 1400B per pkt, makes a chunk 1MB

#define CSYN_TIMEOUT_STACK 5

#ifndef u_int
typedef uint32_t u_int;
#endif
#ifndef u_short
typedef uint16_t u_short;
#endif
#ifndef u_char
typedef unsigned char u_char;
#endif

#define DEV_NAME_LEN 8
#define SWITCH_HOLD -95
#define VERSION "1.0"

typedef enum {
	WIFI,
	WIMAX,
	ETHER,
	OTHER
} IfType;

typedef enum {
	AUTO,
	MANUAL
} IfMode;

typedef enum {
	POLICY_BESTPERFORMANCE = 1,
	POLICY_WIFIONLY, //wifi 1, wimax 0 (wifi only)
	POLICY_WIMAXONLY, //wifi 0, wimax 1 (wimax only)
	POLICY_WEIGHTED11, //wifi 1, wimax 1
	POLICY_WEIGHTED31, //wifi 3, wimax 1
	POLICY_WEIGHTED00,
	POLICY_ETHERONLY
    
} PolicyType;

#endif
