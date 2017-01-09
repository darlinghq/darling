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
	// main alreadly links with foo.dylib
	// now dynamically link with bar.dylib
	void* handle = dlopen("./bar.dylib", RTLD_GLOBAL);
	if ( handle == NULL ) {
		const char* msg = dlerror();
		FAIL("dlopen(\"%s\", RTLD_GLOBAL) failed: %s", argv[1], msg);
		return EXIT_SUCCESS;
	}

	// verify we can find _foo
	void* fooSym = dlsym(RTLD_DEFAULT, "foo");
	if ( fooSym == NULL ) {
		const char* msg = dlerror();
		FAIL("dlsym(RTLD_DEFAULT, \"foo\") failed: %s", msg);
		return EXIT_SUCCESS;
	}
	
	// verify we can find _bar
	void* barSym = dlsym(RTLD_DEFAULT, "bar");
	if ( barSym == NULL ) {
		const char* msg = dlerror();
		FAIL("dlsym(RTLD_DEFAULT, \"bar\") failed: %s", msg);
		return EXIT_SUCCESS;
	}

	// open foo privately (since it was already opened global, RTLD_LOCAL should have no effect
	void* handleFoo = dlopen("./foo.dylib", RTLD_LOCAL);
	if ( handleFoo == NULL ) {
		const char* msg = dlerror();
		FAIL("dlopen(\"%s\", RTLD_LOCAL) failed: %s", "./foo.dylib", msg);
		return EXIT_SUCCESS;
	}

	// open foo privately (since it was already opened global, RTLD_LOCAL should have no effect
	void* handleBar = dlopen("./bar.dylib", RTLD_LOCAL);
	if ( handleBar == NULL ) {
		const char* msg = dlerror();
		FAIL("dlopen(\"%s\", RTLD_LOCAL) failed: %s", "./bar.dylib", msg);
		return EXIT_SUCCESS;
	}

	// verify we can still find _foo
	fooSym = dlsym(RTLD_DEFAULT, "foo");
	if ( fooSym == NULL ) {
		const char* msg = dlerror();
		FAIL("dlsym(RTLD_DEFAULT, \"foo\") failed: %s", msg);
		return EXIT_SUCCESS;
	}
	
	// verify we can still find _bar
	barSym = dlsym(RTLD_DEFAULT, "bar");
	if ( barSym == NULL ) {
		const char* msg = dlerror();
		FAIL("dlsym(RTLD_DEFAULT, \"bar\") failed: %s", msg);
		return EXIT_SUCCESS;
	}
  
	PASS("dlopen-RTLD_LOCAL-ignore");
	return EXIT_SUCCESS;
}
