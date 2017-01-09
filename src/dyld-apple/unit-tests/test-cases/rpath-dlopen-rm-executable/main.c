/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
#include <dlfcn.h>
#include <unistd.h>
#include <mach-o/dyld.h>

#include "test.h"


int main()
{
	char buf[2048];
	uint32_t bufSize = sizeof(buf);
	if ( _NSGetExecutablePath(buf, &bufSize) ) {
		FAIL("rpath-dlopen-rm-exectuable: _NSGetExecutablePath()");
		return EXIT_SUCCESS;
	}
	
	unlink(buf);
	
	void* handle = dlopen("libz.dylib", RTLD_LAZY);
	if ( handle == NULL ) {
		FAIL("rpath-dlopen-rm-exectuable: %s", dlerror());
		return EXIT_SUCCESS;
	}
		
	PASS("rpath-dlopen-rm-exectuable");
	return EXIT_SUCCESS;
}
