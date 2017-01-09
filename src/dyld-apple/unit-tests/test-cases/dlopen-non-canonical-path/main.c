/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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
#include <stdlib.h> // EXIT_SUCCESS
#include <stdio.h> 
#include <dlfcn.h>
#include <Availability.h>

#include "test.h"

static void tryPath(const char* path)
{
	if ( dlopen_preflight(path) ) {
		void* handle = dlopen(path, RTLD_LAZY);
		if ( handle == NULL ) {
			FAIL("dlopen-non-canonical-path: dlopen(%s)", path);
			exit(0);
		}
	}
	else {
		FAIL("dlopen-non-canonical-path: dlopen_preflight(%s)", path);
		exit(0);
	}
}

//
//  <rdar://problem/7017050> dlopen() not opening frameworks with non-canonical paths
//

int main()
{
	tryPath("//usr/lib/libSystem.B.dylib");
	tryPath("/usr/bin/../lib/libSystem.B.dylib");
	tryPath("/usr/lib/./libSystem.B.dylib");
	tryPath("/usr/lib//libSystem.B.dylib");

	PASS("dlopen-non-canonical-path");
	return EXIT_SUCCESS;
}


