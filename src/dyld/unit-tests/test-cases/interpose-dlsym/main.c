/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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
#include <stdlib.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


int main()
{
	void* handle = dlopen("/usr/lib/libSystem.B.dylib", RTLD_LAZY);
	if ( handle == NULL ) {
		FAIL("interpose-dlsym: dlopen() error: %s", dlerror());
		exit(0);
	}
	void* dlsym_free = dlsym(handle, "free");
	if ( dlsym_free == NULL ) {
		FAIL("interpose-dlsym: dlsym() error: %s", dlerror());
		exit(0);
	}
	
	if ( dlsym_free == &free )
		PASS("interpose-dlsym");
	else
		FAIL("interpose-dlsym: %p != %p", dlsym_free, &free);
	return EXIT_SUCCESS;
}
