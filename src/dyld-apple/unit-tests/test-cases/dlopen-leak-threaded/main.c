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
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

  
#include "test.h"


///
/// Test that the dyld copy of the unwinder keeps each thread's
/// exception object seperate.
///

static void* work(void* arg)
{
	// will fail and cause exception to be thrown inside dyld
	dlopen((char*)arg, RTLD_LAZY);
	return NULL;
}


int main()
{
	pthread_t worker1;
	if ( pthread_create(&worker1, NULL, work, "/frazzle/bar") != 0 ) {
		FAIL("pthread_create failed");
		exit(0);
	}

	pthread_t worker2;
	if ( pthread_create(&worker2, NULL, work, "/frazzle/foo") != 0 ) {
		FAIL("pthread_create failed");
		exit(0);
	}
	
	pthread_t worker3;
	if ( pthread_create(&worker3, NULL, work, "/frazzle/dazzle") != 0 ) {
		FAIL("pthread_create failed");
		exit(0);
	}

	
	void* result;
	//fprintf(stderr, "waiting for worker 1\n");
	pthread_join(worker1, &result);
	//fprintf(stderr, "waiting for worker 2\n");
	pthread_join(worker2, &result);
	//fprintf(stderr, "waiting for worker 3\n");
	pthread_join(worker3, &result);

	// execute leaks command on myself
	char cmd[512];
	sprintf(cmd, "sudo leaks %u > /dev/null\n", getpid());
	int status = system(cmd);
	if ( status == EXIT_SUCCESS )
		PASS("dlopen-leak-threaded");
	else
		FAIL("dlopen-leak-threaded");
	 	
	return EXIT_SUCCESS;
}


