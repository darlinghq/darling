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
/// Both test.bundle and test.dylib call dlsym(RTLD_NEXT, "foo");
/// They should find the ones in foo1.dylib and foo2.dylib respectively.
/// We test this be looking up those symbols explictly.
///


int foo()
{
	return 0;
}

typedef void* (*TestProc)(void);

static void trySO(const char* pathToLoad, const char* indirectLibrary)
{
	void* indirectHandle = dlopen(indirectLibrary, RTLD_LAZY);
	if ( indirectHandle == NULL ) {
		FAIL("dlopen(\"%s\") failed", indirectLibrary);
		exit(0);
	}
	
	void* indirectFoo = (TestProc)dlsym(indirectHandle, "foo");
	if ( indirectFoo == NULL ) {
		FAIL("dlsym(handle, \"test\") failed");
		exit(0);
	}

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
	
	void* targetFoo = (*sym)();
	
	//printf("targetFoo = %p, indirectFoo = %p\n", targetFoo, indirectFoo);
	 
	if ( targetFoo != indirectFoo ) {
		FAIL("dlsym-RTLD_NEXT wrong foo found");
		exit(0);
	}
	
}



int main()
{
	trySO("test.bundle", "foo1.dylib");
	trySO("test.dylib", "foo2.dylib");
  
	PASS("dlsym-RTLD_NEXT bundle and dylib");
	return EXIT_SUCCESS;
}
