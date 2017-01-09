/*
 * Copyright (c) 2005-2009 Apple Inc. All rights reserved.
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
#include <mach-o/dyld.h>
#include <Availability.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


typedef void* (*fooProc)();


static void notify(const struct mach_header *mh, intptr_t vmaddr_slide) 
{
// NSLookupSymbolInImage is only available on Mac OS X - not iPhone OS
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	//fprintf(stderr, "mh=%p\n", mh);
	NSLookupSymbolInImage(mh, "_bar", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND_FULLY | NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR);
#endif
}


int main()
{
	_dyld_register_func_for_add_image(&notify);

	void* handle = dlopen("libfoo.dylib", RTLD_LAZY);
	if ( handle == NULL ) {
		FAIL("dlopen(\"%s\") failed with: %s", "libfoo.dylib", dlerror());
		exit(0);
	}
	
	fooProc fooPtr = (fooProc)dlsym(handle, "foo");
	if ( fooPtr == NULL ) {
		FAIL("dlsym(handle, \"foo\") failed");
		exit(0);
	}
	
	void* foosMalloc = (*fooPtr)();
	//fprintf(stderr, "foo says &malloc=%p\n", foosMalloc);
	//fprintf(stderr, "&malloc=%p\n", &malloc);
	
	dlclose(handle);
  
	if ( foosMalloc == &malloc )
		PASS("dlopen-notify-bind");
	else
		FAIL("dlopen-notify-bind libfoo.dylib double bound");
	return EXIT_SUCCESS;
}
