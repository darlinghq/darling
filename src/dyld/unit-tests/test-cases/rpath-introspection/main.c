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
#include <stdio.h>
#include <stdlib.h>
#include <mach-o/dyld.h>
#include <dlfcn.h>
#include <string.h>

#include "test.h"

extern void foo();

int main()
{
	foo();
	
	// make sure foo's path does not have .. in it
	int count = _dyld_image_count();
	bool found = false;
	for(int i=0; i < count; ++i) {
		const char* name = _dyld_get_image_name(i);
		if ( strstr(name, "libfoo.dylib") == 0 ) {
			found = true;
			if ( strstr(name, "..") != NULL ) {
				FAIL("rpath-introspection: _dyld_get_image_name(%d) returned %s", i, name);
				return EXIT_SUCCESS;
			}
		}
	}
	if ( !found ) {
		FAIL("rpath-introspection: _dyld_get_image_name() never returned libfoo.dylib");
		return EXIT_SUCCESS;
	}
	
	// make sure dladdr path does not have .. in it
	Dl_info info;
	if ( dladdr(&foo, &info) == 0 ) {
		FAIL("rpath-introspection: dladdr() failed");
		return EXIT_SUCCESS;
	}
	if ( strstr(info.dli_fname, "..") != NULL ) {
		FAIL("rpath-introspection: dladdr() returned path with .. in it");
		return EXIT_SUCCESS;
	}
	
	PASS("rpath-introspection");
	return EXIT_SUCCESS;
}
