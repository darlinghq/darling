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
#include <dlfcn.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

typedef int (*fooproc)(void);

int main()
{
	// dlopen of regular executable should fail
	void* handle = dlopen("./foo.exe", RTLD_LAZY);
	if ( handle != NULL ) {
		FAIL("dlopen-executable: dlopen(\"./foo.exe\") did not fail");
		return EXIT_SUCCESS;
	}
	
	// dlopen of pie should work
	handle = dlopen("./foo.pie", RTLD_LAZY);
	if ( handle == NULL ) {
		FAIL("dlopen-executable: dlopen(\"./foo.pie\") failed with: %s", dlerror());
		return EXIT_SUCCESS;
	}
	
	fooproc pfoo = (fooproc)dlsym(handle, "foo");
	if ( pfoo == NULL ) {
		FAIL("dlopen-executable: dlsym(handle, \"foo\") failed");
		return EXIT_SUCCESS;
	}
	
	if ( (*pfoo)() != 10 ) {
		FAIL("dlopen-executable: foo() != 10");
		return EXIT_SUCCESS;
	}
	
	int result = dlclose(handle);
	if ( result != 0 ) {
		FAIL("dlopen-executable: dlclose(handle) returned %d", result);
		return EXIT_SUCCESS;
	}
  
	PASS("dlopen-executable");
	return EXIT_SUCCESS;
}
