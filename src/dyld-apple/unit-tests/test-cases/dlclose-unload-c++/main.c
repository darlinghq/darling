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
#include <stdbool.h>
#include <dlfcn.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

///
/// This tests that if a C++ symbol (any weak symbol) is bound to an image
/// that is dynamically unloaed, the image is not unloaded until all its clients are
///

typedef void* (*proc)(void);

bool inImage(void* x)
{
  Dl_info info;
  return ( dladdr(x, &info) != 0 );
}


int main()
{
	void* handle1 = dlopen("libfoo.dylib", RTLD_LAZY);
	if ( handle1 == NULL ) {
		FAIL("dlclose-unload-c++: dlopen(\"libfoo.dylib\", RTLD_LAZY) failed with dlerror()=%s", dlerror());
		exit(0);
	}
	
	proc fooProc = (proc)dlsym(handle1, "foo");
	if ( fooProc == NULL ) {
		FAIL("dlclose-unload-c++: dlsym(handle1, \"foo\") failed");
		exit(0);
	}

	void* handle2 = dlopen("libbar.dylib", RTLD_LAZY);
	if ( handle2 == NULL ) {
		FAIL("dlclose-unload-c++: dlopen(\"libfoo.dylib\", RTLD_LAZY) failed with dlerror()=%s", dlerror());
		exit(0);
	}

	proc barProc = (proc)dlsym(handle2, "bar");
	if ( barProc == NULL ) {
		FAIL("dlclose-unload-c++: dlsym(handle2, \"bar\") failed");
		exit(0);
	}

	// verify that uniquing is happening
	void* fooResult = (*fooProc)();
	void* barResult = (*barProc)();
	if ( fooResult != barResult ) {
		FAIL("dlclose-unload-c++: foo() and bar() returned different values");
		exit(0);
	}
	
	// close libfoo, even though libbar is using libfoo
	dlclose(handle1);
	
	// error if libfoo was unloaded
	if ( !inImage(fooProc) ) {
		FAIL("dlclose-unload-c++: libfoo should not have been unloaded");
		exit(0);
	}
	
	// close libbar which should release libfoo
	dlclose(handle2);
	
	// error if libfoo was not unloaded
	if ( inImage(fooProc) ) {
		FAIL("dlclose-unload-c++: libfoo should have been unloaded");
		exit(0);
	}
	
	PASS("dlclose-unload-c++");
	return EXIT_SUCCESS;
}
