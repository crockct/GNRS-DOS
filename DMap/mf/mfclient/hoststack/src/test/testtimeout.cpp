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


#include <unistd.h>
#include <iostream>
#include <stdio.h>

#include "mftimeoutmanager.h"
#include "mftimeoutlistener.h"

class DummyListener : public MFTimeoutListenerInterface {
public:
	DummyListener(){}
	
	virtual ~DummyListener(){}
	
	virtual void OnTimeout(void *message, unsigned int id) {
		struct timeval *original_time = (struct timeval *)message;
		struct timeval actual_time;
		long int a_time;
		gettimeofday(&actual_time, NULL);
		a_time = actual_time.tv_sec * 1000 + actual_time.tv_usec/1000;
		printf("Timeout triggered TIME: [%lu]: \n",
				actual_time.tv_sec*1000000 + actual_time.tv_usec);
		a_time = a_time - (original_time->tv_sec * 1000 + original_time->tv_usec/1000);
		std::cout << "Timeout triggered after " << a_time << std::endl;
	};
};

int main(int argc, char **argv){
	MFTimeoutManager tm;
	struct timeval time;
	struct timeval time2;
	struct timeval time3;
	struct timeval time4;
	DummyListener dl;
	unsigned int lastId;
	tm.StartManager();
	struct timeval temptime;
	gettimeofday(&time, NULL);
	gettimeofday(&temptime, NULL);
	printf("start TIME: [%lu]: \n",
                    temptime.tv_sec*1000000 + temptime.tv_usec);
	lastId = tm.AddTimeout(&dl, &time, 10);
	gettimeofday(&temptime, NULL);
    printf("2 TIME: [%lu]: \n",
                    temptime.tv_sec*1000000 + temptime.tv_usec);
	gettimeofday(&time2, NULL);
	lastId = tm.AddTimeout(&dl, &time2, 1000);
	gettimeofday(&temptime, NULL);
    printf("3 TIME: [%lu]: \n",
                    temptime.tv_sec*1000000 + temptime.tv_usec);
    lastId = tm.ClearTimeout(tm.AddTimeout(&dl, NULL, 4000));
	gettimeofday(&temptime, NULL);
    printf("end TIME: [%lu]: \n",
                    temptime.tv_sec*1000000 + temptime.tv_usec);
    sleep(6);


    gettimeofday(&time, NULL);
	gettimeofday(&temptime, NULL);
    printf("start TIME: [%lu]: \n",
                    temptime.tv_sec*1000000 + temptime.tv_usec);
    lastId = tm.AddTimeout(&dl, &time, 2000);
	gettimeofday(&time2, NULL);
	gettimeofday(&temptime, NULL);
    printf("2 TIME: [%lu]: \n",
                    temptime.tv_sec*1000000 + temptime.tv_usec);
	lastId = tm.AddTimeout(&dl, &time2, 500);
	gettimeofday(&temptime, NULL);
    printf("3 TIME: [%lu]: \n",
                    temptime.tv_sec*1000000 + temptime.tv_usec);
    lastId = tm.ClearTimeout(lastId);
	gettimeofday(&temptime, NULL);
    printf("end TIME: [%lu]: \n",
                    temptime.tv_sec*1000000 + temptime.tv_usec);
	sleep(6);


	gettimeofday(&temptime, NULL);
	gettimeofday(&time, NULL);
	printf("start TIME: [%lu]: \n",
                    temptime.tv_sec*1000000 + temptime.tv_usec);
	lastId = tm.AddTimeout(&dl, &time, 4000);
	gettimeofday(&temptime, NULL);
	gettimeofday(&time2, NULL);
	printf("2 TIME: [%lu]: \n",
                    temptime.tv_sec*1000000 + temptime.tv_usec);
	lastId = tm.AddTimeout(&dl, &time2, 5000);
	gettimeofday(&temptime, NULL);
	gettimeofday(&time3, NULL);
	printf("3 TIME: [%lu]: \n",
                    temptime.tv_sec*1000000 + temptime.tv_usec);
	lastId = tm.AddTimeout(&dl, &time3, 50);
	gettimeofday(&temptime, NULL);
	gettimeofday(&time4, NULL);
	printf("4 TIME: [%lu]: \n",
                    temptime.tv_sec*1000000 + temptime.tv_usec);
	lastId = tm.AddTimeout(&dl, &time4, 100);
	gettimeofday(&temptime, NULL);
	printf("5 TIME: [%lu]: \n",
                    temptime.tv_sec*1000000 + temptime.tv_usec);
    unsigned int i = tm.AddTimeout(&dl, NULL, 3000);
    gettimeofday(&temptime, NULL);
	printf("6 TIME: [%lu]: \n",
                    temptime.tv_sec*1000000 + temptime.tv_usec);
	tm.ClearTimeout(i);
	gettimeofday(&temptime, NULL);
	printf("end TIME: [%lu]: \n",
                    temptime.tv_sec*1000000 + temptime.tv_usec);
    sleep(7);


    gettimeofday(&temptime, NULL);
    printf("start TIME: [%lu]: \n",
                    temptime.tv_sec*1000000 + temptime.tv_usec);
    tm.ClearTimeout(i);
    gettimeofday(&temptime, NULL);
    printf("start TIME: [%lu]: \n",
                    temptime.tv_sec*1000000 + temptime.tv_usec);

    lastId = tm.StopManager();
}
