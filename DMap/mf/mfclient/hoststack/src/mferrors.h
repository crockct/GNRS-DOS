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
 * @file   mferrors.h
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   July, 2013
 * @brief  Define possible errors occurring in MobilityFirst's API and stack.
 *
 * Define possible errors occurring in MobilityFirst's API and stack.
 */

#ifndef errors_H_
#define errors_H_

/*
 * ERROR: UID already exists
 */
#define UID_EXISTS 1

/*
 * ERROR: UID does not exist
 */
#define UID_NEXISTS 2

/*
 * ERROR: nothing to receive
 */
#define N_RECV_BUF 3

/*
 * ERROR: not enough resources to send
 */
#define N_SEND_BUF 4

/*
 * ERROR description:
 */
#define PCAP_DEVICE_ERROR 60

/*
 * ERROR description:
 */
#define PCAP_FILTER_ERROR 61

/*
 * ERROR description:
 */
#define PCAP_LOOP_ERROR 62

#endif /* errors_H_ */