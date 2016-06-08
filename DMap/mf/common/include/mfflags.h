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
 * @file   mfflags.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   January, 2014
 * @brief  List of flags in MobilityFirst.
 *
 * List of flags in MobilityFirst. In this list are defined SIDs, flags for API calls, etc.
 */

#ifndef MF_FLAGS_H
#define MF_FLAGS_H

#include <stdint.h>
#include <inttypes.h>

typedef uint32_t mfflag_t;

#define MF_MULTICAST 		0x00000001
#define MF_ANYCAST 		0x00000002
#define MF_MULTIHOME 		0x00000004
#define MF_CONTENT_REQUEST 	0x00000200
#define MF_CONTENT_RESPONSE 	0x00000400
#define MF_BROADCAST 		0x00002000

static inline int isMFFlagSet(mfflag_t f1, mfflag_t f2){
	return f1 & f2;
}

static inline mfflag_t setMFFlag(mfflag_t f1, mfflag_t f2){
	return f1 | f2;
}

#endif
