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
 \mainpage MobilityFirst's API documentation page

 We are building a proof-of-concept protocol stack to include a basic transport layer (message chunking, e-2-e acks, flow control), GUID service layer, storage aware routing, and hop-by-hop link data transport. The clean slate implementation will export a GUID-centered API with message-based communication primitives, and rich delivery service options (e.g., DTN, real-time, multicast, etc). We will also explore a 'network interface manager' functionality to enable policy driven multi-homing that could flexibly address user/application stated goals of performance, reliability, battery lifetime, network svc. payments, etc. The protocol stack will be implemented in C/C++ and will be standalone user-level process, while apps will link to the service API library to use the new stack.
 */

/**
 * @file   testapi.c
 * @author wontoniii (bronzino@winlab.rutgers.edu)
 * @date   January, 2016
 * @brief  Test MobilityFirst's API.
 *
 * Test C/C++ MobilityFirst API. Used to run quick test of functionalities offered by the MobilityFirst's network protocol stack and API.
 */


#include <unistd.h>
#include <stdio.h>
#include <mobilityfirst/mfapi.h>

int test_mfrecv(struct Handle *handle) {
	return 0;
}

int test_mfsend(struct Handle *handle) {
	return 0;
}

int test_mfdetach(struct Handle *handle) {
	int guid = 2;
	int ret = mfdetach(handle, &guid, 1);
	if(ret){
		printf("mfdetach error\n");
		return -1;
	}
	return 0;
}

int test_mfattach (struct Handle *handle) {
	int guid = 2;
	int ret = mfattach(handle, &guid, 1);
	if(ret){
		printf("mfdetach error\n");
		return -1;
	}
	return 0;
}

int test_mfclose(struct Handle *handle) {
	int ret = mfclose(handle);
	if(ret){
		printf("mfclose error\n");
		return -1;
	}
	return 0;
}

//TODO add profile and other options
int test_mfopen(struct Handle *handle) {
	int ret = mfopen(handle, "basic", "Atleast:check,parsing:1,howlong:canbe", 1);
	if(ret) {
		printf("mfopen error\n");
		return -1;
	}
	return 0;
}

int main(int argc, char **argv){
	struct Handle handle;
	test_mfopen(&handle);
	sleep(10);
	test_mfattach(&handle);
	sleep(2);
	test_mfdetach(&handle);
	sleep(2);
	test_mfsend(&handle);
	sleep(2);
	test_mfrecv(&handle);
	sleep(10);
	test_mfclose(&handle);
}
