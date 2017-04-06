/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
#include <string.h> // strstr()
#include <dlfcn.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()



int main()
{
	// test error message of a dylib that does not exist
	void* handle = dlopen("libdoesnotexist.dylib", RTLD_LAZY);
	if ( handle != NULL ) {
		FAIL("dlopen-error: dlopen(libdoesnotexist.dylib, RTLD_LAZY) should have failed");
		return 0;
	}
	if ( strstr(dlerror(), "image not found") == NULL ) {
		FAIL("dlopen-error: expected 'image not found' in dlerror() string");
		return 0;
	}
	
	// test error message of a dylib that is not readabble
	handle = dlopen("libnoread.dylib", RTLD_LAZY);
	if ( handle != NULL ) {
		FAIL("dlopen-error: dlopen(libnoread.dylib, RTLD_LAZY) should have failed");
		return 0;
	}
	if ( strstr(dlerror(), "failed with errno=13") == NULL ) {
		FAIL("dlopen-error: expected 'failed with errno=13' in dlerror() string");
		return 0;
	}
	

	PASS("dlopen-error");
	return EXIT_SUCCESS;
}
