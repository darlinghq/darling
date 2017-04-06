/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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




int main()
{
	Dl_info info;

	// load bar
	void* handleBar = dlopen("libbar.dylib", RTLD_LAZY);
	if ( handleBar == NULL ) {
		FAIL("dlclose-dylib-ref-count: dlopen(\"libbar.dylib\", RTLD_LAZY) failed with dlerror()=%s", dlerror());
		exit(0);
	}
	
	// load foo
	void* handleFoo = dlopen("libfoo.dylib", RTLD_LAZY);
	if ( handleFoo == NULL ) {
		FAIL("dlclose-dylib-ref-count: dlopen(\"libfoo.dylib\", RTLD_LAZY) failed with dlerror()=%s", dlerror());
		exit(0);
	}
	
	void* sym_base = dlsym(handleBar, "base");
	if ( sym_base == NULL ) {
		FAIL("dlclose-dylib-ref-count: dlsym(handleBar, \"base\") failed");
		exit(0);
	}
	
	void* sym_foo = dlsym(handleFoo, "foo");
	if ( sym_foo == NULL ) {
		FAIL("dlclose-dylib-ref-count: dlsym(handleBar, \"base\") failed");
		exit(0);
	}

	void* sym_bar = dlsym(handleBar, "bar");
	if ( sym_bar == NULL ) {
		FAIL("dlclose-dylib-ref-count: dlsym(handleBar, \"base\") failed");
		exit(0);
	}
	
	if ( dlclose(handleBar) != 0 ) {
		FAIL("dlclose-dylib-ref-count: dlclose(handleBar) != 0, dlerrr()=%s", dlerror());
		exit(0);
	}
	
	// sym_base should still be accessible via dladdr() because of external reference from libfoo.dylib
	if ( dladdr(sym_base, &info) == 0 ) {
		FAIL("dlclose-dylib-ref-count: dladdr(sym_base) == 0, but should have succeeded");
		exit(0);
	}
	
	// sym_foo should still be accessible via dladdr() because libfoo was dlopen'ed
	if ( dladdr(sym_foo, &info) == 0 ) {
		FAIL("dlclose-dylib-ref-count: dladdr(sym_foo) == 0, but should have succeeded");
		exit(0);
	}
	
	// sym_bar should not be accessible via dladdr() because libbar was dlclose'ed
	if ( dladdr(sym_bar, &info) != 0 ) {
		FAIL("dlclose-dylib-ref-count: dladdr(sym_bar) != 0, but should have failed");
		exit(0);
	}
	
	if ( dlclose(handleFoo) != 0 ) {
		FAIL("dlclose-dylib-ref-count: dlclose(handleBar) != 0, dlerrr()=%s", dlerror());
		exit(0);
	}
	
	// sym_base should no longer be accessible via dladdr() because libfoo and libbar both closed
	if ( dladdr(sym_base, &info) != 0 ) {
		FAIL("dlclose-dylib-ref-count: dladdr(base) != 0, but should have failed");
		exit(0);
	}
	
	// sym_foo should still be accessible via dladdr() because libfoo was dlclose'ed
	if ( dladdr(sym_foo, &info) != 0 ) {
		FAIL("dlclose-dylib-ref-count: dladdr(sym_foo) != 0, but should have failed");
		exit(0);
	}
	
	PASS("dlclose-dylib-ref-count");
	return EXIT_SUCCESS;
}
