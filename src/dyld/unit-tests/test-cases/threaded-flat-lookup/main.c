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
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


void* h1 = NULL;
void* h2 = NULL;

static void* work(void* ignore)
{
	for (int i=0; i < 10000; ++i) {
		h1 = dlopen("libfoo1.dylib", 0);
		if ( h1 == NULL ) {
			FAIL("dlopen failed: %s", dlerror());
			exit(0);
		}
		dlclose(h2);	
		h2 = dlopen("libfoo2.dylib", 0);
		if ( h2 == NULL ) {
			FAIL("dlopen failed: %s", dlerror());
			exit(0);
		}
		dlclose(h1);	
	}
	
	//fprintf(stderr, "done with foos\n");
	return NULL;
}


int main()
{
	h2 = dlopen("libfoo2.dylib", 0);
	if ( h2 == NULL ) {
		FAIL("dlopen failed: %s", dlerror());
		exit(0);
	}

	// other thread dlopens and closes libfoo.dylib 500 times
	pthread_t other;
	if ( pthread_create(&other, NULL, work, NULL) != 0 ) {
		FAIL("pthread_create failed");
		exit(0);
	}
		
	// this thread looks up a symbol 10,000 times
	for (int i=0; i < 15000; ++i) {
		void* handle = dlopen("libclient.dylib", 0);
		if ( handle == NULL ) {
			FAIL("dlopen failed: %s", dlerror());
			exit(0);
		}
		typedef void (*proc_t)();
		proc_t proc = dlsym(handle, "doit");
		if ( proc == NULL ) {
			FAIL("dlsym failed: %s", dlerror());
			exit(0);
		}
		(*proc)();
		dlclose(handle);	
	}
	//fprintf(stderr, "done with libclient\n");

	void* result;
	pthread_join(other, &result);
	
	PASS("threaded-flat-lookup");
	return 0;
}
