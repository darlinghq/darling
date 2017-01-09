/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/time.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

static pthread_mutex_t	sBarrierMutex;
static pthread_cond_t	sBarrierFree;

static volatile int sValue = 0;

static void __attribute__((constructor)) myinit()
{
	pthread_mutex_init(&sBarrierMutex, NULL);
	pthread_cond_init(&sBarrierFree, NULL);
}

void waitForState(int value)
{
	//fprintf(stderr, "waitForState(%d), currently %d\n", value, sValue);
	pthread_mutex_lock(&sBarrierMutex);
	while ( sValue < value ) {
		struct timeval        tvNow;
		struct timespec       tsTimeout;		
		gettimeofday(&tvNow, NULL);
		TIMEVAL_TO_TIMESPEC(&tvNow, &tsTimeout);
		tsTimeout.tv_sec += 2;	  // fail if block for 2 seconds
		if ( pthread_cond_timedwait(&sBarrierFree, &sBarrierMutex, &tsTimeout) == ETIMEDOUT ) {
			FAIL("dlopen-dyld-locking: lock timed out");
			exit(0);
		}
	}
	pthread_mutex_unlock(&sBarrierMutex);
}


void setState(int value)
{
	pthread_mutex_lock(&sBarrierMutex);
	//fprintf(stderr, "setState(%d)\n", value);
	sValue = value;	
	pthread_cond_broadcast(&sBarrierFree);
	pthread_mutex_unlock(&sBarrierMutex);
}


