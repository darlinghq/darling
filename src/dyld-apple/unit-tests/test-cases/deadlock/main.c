/*
 * Copyright (c) 2005-2009 Apple Inc. All rights reserved.
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
#include <stdio.h>
#include <stdlib.h>
#include <mach-o/dyld.h>
#include <pthread.h>
#include <unistd.h>
#include <Availability.h>

#include "test.h"

/// rdar://problem/3811777

//  barrier		thread 1						thread 2
//				add image
//	1	
//												acquire sMyLock
//	2
//				in callback acquire sMyLock		call lazy pointer
//				release sMyLock					release sMyLock

extern void foo();

#if __MAC_OS_X_VERSION_MIN_REQUIRED

static volatile int		sBarrier = 0;
static pthread_mutex_t	sBarrierMutex;
static pthread_cond_t	sBarrierFree;

static void blockUntilBarrier(int n)
{
	pthread_mutex_lock(&sBarrierMutex);
	while ( sBarrier < n )	
		pthread_cond_wait(&sBarrierFree, &sBarrierMutex);
	pthread_mutex_unlock(&sBarrierMutex);
}

static void advanceToBarrier(int n)
{
	pthread_mutex_lock(&sBarrierMutex);
	sBarrier = n;	
	pthread_cond_broadcast(&sBarrierFree);
	pthread_mutex_unlock(&sBarrierMutex);
}





static pthread_mutex_t	sMyLock;

static void* thread2(void* arg)
{
	// thread 2
	blockUntilBarrier(1);
	pthread_mutex_lock(&sMyLock);
	advanceToBarrier(2);
	foo();
	pthread_mutex_unlock(&sMyLock);
	return NULL;
}



static void myImageHandler(const struct mach_header *mh, intptr_t vmaddr_slide)
{
	// thread 1
	if ( NSLookupSymbolInImage(mh, "_bar", 0) != NULL ) {
		advanceToBarrier(1);
		blockUntilBarrier(2);
		pthread_mutex_lock(&sMyLock);
		pthread_mutex_unlock(&sMyLock);
	}
}
#endif

int main()
{
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	pthread_mutex_init(&sBarrierMutex, NULL);
	pthread_cond_init(&sBarrierFree, NULL);
	pthread_mutex_init(&sMyLock, NULL);

	// self-terminate this process if it locks up for two seconds
	alarm(2);
	
	advanceToBarrier(0);

	pthread_t pthread2;
	if ( pthread_create(&pthread2, NULL, thread2, NULL) != 0 ) {
		FAIL("pthread_create failed");
		exit(0);
	}

	// thread 1
	_dyld_register_func_for_add_image(&myImageHandler);
	NSAddImage("bar.dylib", 0);
#endif
	
	PASS("deadlock");
	return 0;
}
