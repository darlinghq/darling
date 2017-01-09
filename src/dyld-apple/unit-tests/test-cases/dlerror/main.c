/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


///
/// This tests that the dlerror message is kept per thread
///

static void* work(void* arg)
{
	const char* str = (char*)arg;
	for(int i=0; i < 1000; ++i) {
		//fprintf(stderr, "dlopen(%s)\n", str);
		void* handle = dlopen(str, RTLD_LAZY);
		if ( handle != NULL ) {
			FAIL("dlopen(%s) unexpectedly succeeded", str);
			exit(0);
		}
		char* msg = dlerror();
		//fprintf(stderr, "dlopen(%s) => %s\n", str, msg);
		if ( (msg == NULL) || (strstr(msg, str) == NULL) ) {
			FAIL("dlerror() did not contain library name that could not be loaded", str);
			exit(0);
		}
		
	
	}
	return 0;
}



int main()
{
	dlsym(RTLD_DEFAULT, "foobar");
	//fprintf(stderr, "%s\n", dlerror());

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
	
	PASS("dlerror-thread-test");
	return 0;
}
