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
/// This tests that dlsym() will search indirect libraries
///  rdar://problem/4047391
///



int main(int argc, const char* argv[])
{
	const char* path = argv[1];
	const char* otherPath = "foo3.dylib";

	void* handle = dlopen(path, RTLD_LAZY);
	if ( handle == NULL ) {
		FAIL("dlsym-indirect dlopen(\"%s\") failed", path);
		exit(0);
	}
	
	void* handle3 = dlopen(otherPath, RTLD_LAZY);
	if ( handle3 == NULL ) {
		FAIL("dlsym-indirect dlopen(\"%s\") failed", otherPath);
		exit(0);
	}
	
	void* foo = dlsym(handle, "foo");
	if ( foo == NULL ) {
		FAIL("dlsym-indirect dlsym(handle, \"foo\") failed");
		exit(0);
	}
 
	void* foo1 = dlsym(handle, "foo1");
	if ( foo1 == NULL ) {
		FAIL("dlsym-indirect dlsym(handle, \"foo1\") failed");
		//exit(0);
	}
 
	void* foo2 = dlsym(handle, "foo2");
	if ( foo2 == NULL ) {
		FAIL("dlsym-indirect dlsym(handle, \"foo2\") failed");
		//exit(0);
	}
 
	void* foo3 = dlsym(handle, "foo3");
	if ( foo3 != NULL ) {
		FAIL("dlsym-indirect dlsym(handle, \"foo3\") should have failed");
		//exit(0);
	}
 
	PASS("dlsym-indirect %s", path);
	return EXIT_SUCCESS;
}
