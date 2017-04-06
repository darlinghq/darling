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
#include <unistd.h>
#include <mach-o/dyld_priv.h>

#include <dlfcn.h>
#include <pthread.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


static void* work(void* libName)
{
	for (int i=0; i < 500; ++i) {
		void* handle = dlopen((char*)libName, 0);
		if ( handle == NULL ) {
			FAIL("dlopen failed: %s", dlerror());
			exit(0);
		}
		dlclose(handle);	
	}
	
	return NULL;
}

static const char* batchMappedHandler(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[])
{
	usleep(10000);
	return NULL;
}


int main()
{
	// tell dyld we want to know when images are mapped
	dyld_register_image_state_change_handler(dyld_image_state_initialized, false, batchMappedHandler);

	// other thread dlopens and closes libfoo.dylib 500 times
	pthread_t t1;
	if ( pthread_create(&t1, NULL, work, "libfoo1.dylib") != 0 ) {
		FAIL("pthread_create failed");
		exit(0);
	}
	
	pthread_t t2;
	if ( pthread_create(&t2, NULL, work, "libfoo2.dylib") != 0 ) {
		FAIL("pthread_create failed");
		exit(0);
	}
		
	pthread_t t3;
	if ( pthread_create(&t3, NULL, work, "libfoo3.dylib") != 0 ) {
		FAIL("pthread_create failed");
		exit(0);
	}

	pthread_t t4;
	if ( pthread_create(&t4, NULL, work, "libfoo4.dylib") != 0 ) {
		FAIL("pthread_create failed");
		exit(0);
	}

	void* result;
	pthread_join(t1, &result);
	pthread_join(t2, &result);
	pthread_join(t3, &result);
	pthread_join(t4, &result);
	
	PASS("concurrent-dlopen-initializers");
	return 0;
}
