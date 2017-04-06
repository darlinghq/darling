/*
 * Copyright (c) 2010-2011 Apple Inc. All rights reserved.
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
#include <pthread.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

__thread int a = 0;		// initially 0
__thread int b = 5;		// initially 5

// simulate C++ initializer on thread local variables
void myinit()
{
	a = 11;
	b = 42;
}

static void* work(void* arg)
{
	//fprintf(stderr, "self=%p, &a=%p\n", pthread_self(), get_a());
	if ( a != 11 ) {
		FAIL("tlv-initializer: get_a() not initialized to 11");
		exit(0);
	}
	if ( b != 42 ) {
		FAIL("tlv-initializer: get_b() not initialized to 42");
		exit(0);
	}
	return NULL;
}

int main()
{
	pthread_t worker1;
	if ( pthread_create(&worker1, NULL, work, NULL) != 0 ) {
		FAIL("pthread_create failed");
		exit(0);
	}

	pthread_t worker2;
	if ( pthread_create(&worker2, NULL, work, NULL) != 0 ) {
		FAIL("pthread_create failed");
		exit(0);
	}

	void* result;
	//fprintf(stderr, "waiting for worker 1\n");
	pthread_join(worker1, &result);
	//fprintf(stderr, "waiting for worker 2\n");
	pthread_join(worker2, &result);

	work(NULL);

	PASS("tlv-initializer");
	return EXIT_SUCCESS;
}
