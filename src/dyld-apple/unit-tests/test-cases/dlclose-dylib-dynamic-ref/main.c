/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

	// load foo
	void* handleFoo = dlopen("libfoo.dylib", RTLD_LAZY);
	if ( handleFoo == NULL ) {
		FAIL("dlclose-dylib-dynamic-ref: dlopen(\"libfoo.dylib\", RTLD_LAZY) failed with dlerror()=%s", dlerror());
		exit(0);
	}

	// load bar
	void* handleBar = dlopen("libbar.dylib", RTLD_LAZY);
	if ( handleBar == NULL ) {
		FAIL("dlclose-dylib-dynamic-ref: dlopen(\"libbar.dylib\", RTLD_LAZY) failed with dlerror()=%s", dlerror());
		exit(0);
	}
	
	// load baz
	void* handleBaz = dlopen("libbaz.dylib", RTLD_LAZY);
	if ( handleBaz == NULL ) {
		FAIL("dlclose-dylib-dynamic-ref: dlopen(\"libbaz.dylib\", RTLD_LAZY) failed with dlerror()=%s", dlerror());
		exit(0);
	}
	
	
	void* sym_foo = dlsym(handleFoo, "foo");
	if ( sym_foo == NULL ) {
		FAIL("dlclose-dylib-dynamic-ref: dlsym(handleFoo, \"foo\") failed");
		exit(0);
	}

	void* sym_bar = dlsym(handleBar, "bar");
	if ( sym_bar == NULL ) {
		FAIL("dlclose-dylib-dynamic-ref: dlsym(handleBar, \"bar\") failed");
		exit(0);
	}
	
	void* sym_baz = dlsym(handleBaz, "baz");
	if ( sym_baz == NULL ) {
		FAIL("dlclose-dylib-dynamic-ref: dlsym(handleBaz, \"baz\") failed");
		exit(0);
	}
	
	// since foo loaded first, bar and baz should have a dynamic reference to foo because all
	// the weak mydata symbols were coaleseced to the one in foo.
	
	
	if ( dlclose(handleFoo) != 0 ) {
		FAIL("dlclose-dylib-dynamic-ref: dlclose(handleBar) != 0, dlerrr()=%s", dlerror());
		exit(0);
	}
	
	// sym_foo should still be accessible via dladdr() because libbar and libbaz
	// have dynamic references to libfoo.
	if ( dladdr(sym_foo, &info) == 0 ) {
		FAIL("dlclose-dylib-dynamic-ref: dladdr(sym_base) == 0, but should have succeeded");
		exit(0);
	}
	
	
	if ( dlclose(handleBar) != 0 ) {
		FAIL("dlclose-dylib-dynamic-ref: dlclose(handleBar) != 0, dlerrr()=%s", dlerror());
		exit(0);
	}
	
	
	// sym_bar should not be accessible via dladdr() because libbar was dlclose'ed
	if ( dladdr(sym_bar, &info) != 0 ) {
		FAIL("dlclose-dylib-dynamic-ref: dladdr(sym_bar) != 0, but should have failed");
		exit(0);
	}
	
	// sym_foo should still be accessible via dladdr() because of external libbaz
	// has a dynamic references to libfoo.
	if ( dladdr(sym_foo, &info) == 0 ) {
		FAIL("dlclose-dylib-dynamic-ref: dladdr(sym_base) == 0, but should have succeeded");
		exit(0);
	}


	if ( dlclose(handleBaz) != 0 ) {
		FAIL("dlclose-dylib-dynamic-ref: dlclose(handleBar) != 0, dlerrr()=%s", dlerror());
		exit(0);
	}
	
	// sym_baz should not be accessible via dladdr() because libbar was dlclose'ed
	if ( dladdr(sym_baz, &info) != 0 ) {
		FAIL("dlclose-dylib-dynamic-ref: dladdr(sym_baz) != 0, but should have failed");
		exit(0);
	}

	
	// sym_foo should finally be inaccessible via dladdr() because all dynamic references to libfoo are gone
	if ( dladdr(sym_foo, &info) != 0 ) {
		FAIL("dlclose-dylib-dynamic-ref: dladdr(sym_foo) == 0, but should have succeeded");
		exit(0);
	}
	
	
	PASS("dlclose-dylib-dynamic-ref");
	return EXIT_SUCCESS;
}
