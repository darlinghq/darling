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
#include <string.h>
#include <dlfcn.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()



int main(int argc, const char* argv[])
{
	// open first object which defines foo
	void* handle = dlopen(argv[1], RTLD_LOCAL);
	if ( handle == NULL ) {
		const char* msg = dlerror();
		// Panther dlopen() fails on RTLD_LOCAL of a dylib
		if ( strstr(msg, "RTLD_LOCAL") != NULL ) 
			XFAIL("dlopen(\"%s\", RTLD_LOCAL) failed: %s", argv[1], msg);
		else
			FAIL("dlopen(\"%s\", RTLD_LOCAL) failed: %s", argv[1], msg);
		return EXIT_SUCCESS;
	}

	// open second object which uses foo
	void* handle2 = dlopen(argv[2], RTLD_NOW);
	if ( handle2 != NULL ) {
		FAIL("dlopen(\"%s\") succeeded but foo should have been not found", argv[2]);
		return EXIT_SUCCESS;
	}
	const char* msg = dlerror();
	if ( strstr(msg, "foo") == NULL ) {
		FAIL("dlopen(\"%s\") correctly failed, but foo was not in error mesage: %s", argv[2], msg);
		return EXIT_SUCCESS;
	}
  
  
	PASS("dlopen-RTLD_LOCAL");
	return EXIT_SUCCESS;
}
