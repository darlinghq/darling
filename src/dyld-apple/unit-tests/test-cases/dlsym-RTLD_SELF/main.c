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


///
/// This process has five foo functions.  They are in:
///		main, test.bundle, test.dylib, foo1.dylib, foo2.dylib
///
/// Both test.bundle and test.dylib call dlsym(RTLD_SELF, "foo");
/// They should find the ones in their own linkage unit and
/// call FAIL() otherwise.
/// We also check that this works in the main executable.
///


int foo()
{
	return 0;
}

typedef void (*TestProc)(void);

static void trySO(const char* pathToLoad)
{
	void* handle = dlopen(pathToLoad, RTLD_LAZY);
	if ( handle == NULL ) {
		FAIL("dlopen(\"%s\") failed", pathToLoad);
		exit(0);
	}
	
	TestProc sym = (TestProc)dlsym(handle, "test");
	if ( sym == NULL ) {
		FAIL("dlsym(handle, \"test\") failed");
		exit(0);
	}
	
	(*sym)();	
}



int main()
{
	trySO("test.bundle");
	trySO("test.dylib");

 	if ( dlsym(RTLD_SELF, "foo") != &foo ) {
		FAIL("dlsym(RTLD_SELF, \"foo\") returned wrong value");
	}
	
	PASS("dlsym-RTLD_SELF bundle and dylib");
	return EXIT_SUCCESS;
}
