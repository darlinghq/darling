/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

// this strong A should not override weak
// As in loaded code because of RTLD_LOCAL
int A[] = { 10, 11, 12, 13 };

typedef int* (*getproc)(void);

int main(int argc, const char* argv[])
{
	// open first object 
	void* fooHandle = dlopen("libfoo.dylib", RTLD_LOCAL);
	if ( fooHandle == NULL ) {
		FAIL("dlopen-RTLD_LOCAL-weak: dlopen(\"libfoo.dylib\", RTLD_LOCAL) failed: %s", dlerror());
		return EXIT_SUCCESS;
	}

	// open second object 
	void* barHandle = dlopen("libbar.dylib", RTLD_LOCAL);
	if ( barHandle == NULL ) {
		FAIL("dlopen-RTLD_LOCAL-weak: dlopen(\"libbar.dylib\", RTLD_LOCAL) failed: %s", dlerror());
		return EXIT_SUCCESS;
	}
	
	// get functions
	getproc fooproc = (getproc)dlsym(fooHandle, "getA");
	if ( fooproc == NULL ) {
		FAIL("dlopen-RTLD_LOCAL-weak: dlsym(getA) failed: %s", dlerror());
		return EXIT_SUCCESS;
	}
	getproc barproc = (getproc)dlsym(barHandle, "getA");
	if ( barproc == NULL ) {
		FAIL("dlopen-RTLD_LOCAL-weak: dlsym(getA) failed: %s", dlerror());
		return EXIT_SUCCESS;
	}
	
	// get values
	int* fooA = (*fooproc)();
	int* barA = (*barproc)();
	
	if ( fooA == A )
		FAIL("dlopen-RTLD_LOCAL-weak: fooA == A");
	else if ( barA == A )
		FAIL("dlopen-RTLD_LOCAL-weak: barA == A");
	else if ( fooA == barA )
		FAIL("dlopen-RTLD_LOCAL-weak: fooA == barA");
	else
		PASS("dlopen-RTLD_LOCAL-weak");
	return EXIT_SUCCESS;
}
