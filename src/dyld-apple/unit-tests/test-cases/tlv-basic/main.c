/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
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

__attribute__((weak))
void foobar() {
}

__thread int a;
__thread int b = 5;
__thread static int c;
__thread static int d = 5;


static void* work(void* arg)
{
  uintptr_t offset = (uintptr_t)arg;
	//fprintf(stderr, "self=%p, &a=%p\n", pthread_self(), get_a());
  void* result = malloc(10);
	if ( a != 0 ) {
		FAIL("tlv-basic: a non-zero on slow-path");
		exit(0);
	}
	if ( b != 5 ) {
		FAIL("tlv-basic: b not five");
		exit(0);
	}
  
	if ( c != 0 ) {
		FAIL("tlv-basic: c non-zero");
		exit(0);
	}
  
	if ( d != 5 ) {
		FAIL("tlv-basic: gd not five");
		exit(0);
	}
  
  for(int i=0; i < 10000; ++i) {
    a = 3 + offset + i;
    b = 7 + offset + i;
    c = 11 + offset + i;
    d = 13 + offset + i;
    foobar();
    if ( a != 3 + offset + i ) {
      FAIL("tlv-basic: a not three");
      exit(0);
    }
    if ( b != 7 + offset + i ) {
      FAIL("tlv-basic: b not seven");
      exit(0);
    }
    if ( c != 11 + offset + i ) {
      FAIL("tlv-basic: c not eleven");
      exit(0);
    }
    if ( d != 13 + offset + i ) {
      FAIL("tlv-basic: d not thirteen");
      exit(0);
    }
  }

	return result;
}

int main()
{
	pthread_t worker1;
	if ( pthread_create(&worker1, NULL, work, (void*)1) != 0 ) {
		FAIL("pthread_create failed");
		exit(0);
	}

	pthread_t worker2;
	if ( pthread_create(&worker2, NULL, work, (void*)10) != 0 ) {
		FAIL("pthread_create failed");
		exit(0);
	}

	pthread_t worker3;
	if ( pthread_create(&worker3, NULL, work, (void*)100) != 0 ) {
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

	work(NULL);

	PASS("tlv-basic");
	return EXIT_SUCCESS;
}
