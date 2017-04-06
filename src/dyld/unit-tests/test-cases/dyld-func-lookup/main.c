/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
#include <stdbool.h>
#include <Availability.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


// _dyld_func_lookup is only available in 10.5 and earlier
#if defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && (__MAC_OS_X_VERSION_MIN_REQUIRED <= __MAC_10_5)
extern bool check_dyld_func_lookup();

typedef bool (*proc)(void);

static void trySO(const char* path)
{
	void* handle = dlopen(path, RTLD_LAZY);
	if ( handle == NULL ) {
		FAIL("dlopen(\"%s\") failed with: %s", path, dlerror());
		exit(0);
	}
	
	proc sym = (proc)dlsym(handle, "check_dyld_func_lookup");
	if ( sym == NULL ) {
		FAIL("dlsym(handle, \"foo\") failed");
		exit(0);
	}
	
	if ( sym == &check_dyld_func_lookup )
		FAIL("found same check_dyld_func_lookup as in main executable");
	
	if ( ! (*sym)() )
		FAIL("check_dyld_func_lookup failed for %s", path);
	
	dlclose(handle);
}
#endif


int main()
{
// _dyld_func_lookup is only available in 10.5 and earlier
#if defined(__MAC_OS_X_VERSION_MIN_REQUIRED) && (__MAC_OS_X_VERSION_MIN_REQUIRED <= __MAC_10_5)
	if ( ! check_dyld_func_lookup() )
		FAIL("check_dyld_func_lookup failed for main executable");

	trySO("test.bundle");
	trySO("test.dylib");
#endif

	PASS("dyld-func-lookup");
	return EXIT_SUCCESS;
}
