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
#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <dlfcn.h>
#include <pthread.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

#include "base.h"


///
/// This test verifies that dlopen() releases the dyld lock 
/// while initializers are run. 
/// 
///


static void* work(void* arg)
{
	waitForState(1);
	dlopen("libbar.dylib", RTLD_LAZY);		// dladdr() will hang until dyld lock is released
	setState(2);				// the initializer in libfoo.dylib is block waiting for state 2
	return NULL;
}


int main()
{
	pthread_t otherThread;
	if ( pthread_create(&otherThread, NULL, work, NULL) != 0 ) {
		FAIL("dlopen-dyld-locking: pthread_create failed");
		exit(0);
	}


	void* handle = dlopen("libfoo.dylib", RTLD_LAZY);
	if ( handle == NULL ) {
		FAIL("dlopen-dyld-locking: dlopen(\"libfoo.dylib\") failed: %s", dlerror());
		exit(0);
	}

	PASS("dlopen-dyld-locking");
	return EXIT_SUCCESS;
}


