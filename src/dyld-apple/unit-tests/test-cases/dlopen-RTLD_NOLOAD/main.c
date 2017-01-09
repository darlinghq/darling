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


///
/// This tests that RTLD_NOLOAD binds finds existing images
///


int main()
{
	// main links against libfoo.dylib so it should already be loaded
	void* handle = dlopen("libfoo.dylib", RTLD_NOLOAD);
	if ( handle == NULL ) {
		const char* msg = dlerror();
		// Panther dlcompat does not check existing loaded images (only those opened with dlopen)
		if ( strstr(msg, "RTLD_NOLOAD") != NULL )
			XFAIL("dlopen(libfoo.dylib, RTLD_NOLOAD) failed but it should have worked: %s", msg);
		else
			FAIL("dlopen(libfoo.dylib, RTLD_NOLOAD) failed but it should have worked: %s", msg);
		return 0;
	}
	void* sym = dlsym(handle, "foo");
	if ( sym == NULL ) {
		const char* msg = dlerror();
		FAIL("dlsym(handle, \"foo\") failed but it should have worked: %s", msg);
		return 0;
	}
	
	// libfobbulate.dylib does not exist, so load should return NULL
	void* handle2 = dlopen("libfobbulate.dylib", RTLD_NOLOAD);
	if ( handle2 != NULL ) {
		FAIL("dlopen(libfobbulate.dylib, RTLD_NOLOAD) succeeded but it should have failed");
		return 0;
	}

	
	
	PASS("dlopen-RTLD_NOLOAD");
	return 0;
}
