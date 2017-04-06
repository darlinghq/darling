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
#include <dlfcn.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

int main()
{
	// open same bundle three times
	void* handle1 = dlopen("test.bundle", RTLD_LAZY);
	if ( handle1 == NULL ) {
		FAIL("dlclose-bundle-unload: dlopen(\"test.bundle\", RTLD_LAZY) failed with dlerror()=%s", dlerror());
		exit(0);
	}
	
	void* handle2 = dlopen("test.bundle", RTLD_LAZY);
	if ( handle2 == NULL ) {
		FAIL("dlclose-bundle-unload: dlopen(\"test.bundle\", RTLD_LAZY) failed with dlerror()=%s", dlerror());
		exit(0);
	}

	void* handle3 = dlopen("test.bundle", RTLD_LAZY);
	if ( handle3 == NULL ) {
		FAIL("dlclose-bundle-unload: dlopen(\"test.bundle\", RTLD_LAZY) failed with dlerror()=%s", dlerror());
		exit(0);
	}

	// get symbol
	void* sym = dlsym(handle1, "foo");
	if ( sym == NULL ) {
		FAIL("dlclose-bundle-unload: dlsym(handle1, \"foo\") failed");
		exit(0);
	}

	// close same bundle three times
	if ( dlclose(handle3) != 0 ) {
		FAIL("dlclose-bundle-unload: dlclose(handle3) != 0, dlerrr()=%s", dlerror());
		exit(0);
	}

	if ( dlclose(handle2) != 0 ) {
		FAIL("dlclose-bundle-unload: dlclose(handle2) != 0, dlerrr()=%s", dlerror());
		exit(0);
	}

	if ( dlclose(handle1) != 0 ) {
		FAIL("dlclose-bundle-unload: dlclose(handle1) != 0, dlerrr()=%s", dlerror());
		exit(0);
	}


	// extra close should fail
	if ( dlclose(handle1) == 0 ) {
		FAIL("dlclose-bundle-unload: dlclose(handle4) == 0, but should have failed");
		exit(0);
	}


	
	PASS("dlclose-bundle-unload");
	return EXIT_SUCCESS;
}
