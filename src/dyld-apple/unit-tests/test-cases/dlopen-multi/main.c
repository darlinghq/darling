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
#include <dlfcn.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

int main()
{
	void* handle1 = dlopen("test.bundle", RTLD_LAZY);
	void* handle2 = dlopen("test.bundle", RTLD_LAZY);
	void* handle3 = dlopen("test.dylib", RTLD_LAZY);
	void* handle4 = dlopen("test.dylib", RTLD_LAZY);	
	if ((NULL == handle1)||(NULL == handle2)) {
		FAIL("dlopen(\"test.bundle\") failed");
	}
	if ((NULL == handle3)||(NULL == handle4)) {
		FAIL("dlopen(\"test.dylib\") failed");
	}
	if (handle1 != handle2) {
		FAIL("dlopen handle1 and handle2 are not equal %p != %p",handle1,handle2);
	}
	if (handle3 != handle4) {
		FAIL("dlopen handle3 and handle4 are not equal %p != %p",handle3,handle4);
	}
	if (dlclose(handle4)) {
		XFAIL("Could not close handle4");
	}
	if (dlclose(handle2)) {
		FAIL("Could not close handle2");
	}
	void* sym = dlsym(handle1, "foo");
	if ( sym == NULL ) {
		FAIL("dlsym(handle1, \"foo\") failed");
		exit(0);
	}
	sym = dlsym(handle3, "foo");
	if ( sym == NULL ) {
		FAIL("dlsym(handle3, \"foo\") failed");
		exit(0);
	}
	if (dlclose(handle1)) {
		XFAIL("Could not close handle1");
	}
	if (dlclose(handle3)) {
		XFAIL("Could not close handle3");
	}
	
	PASS("dlopen-multi");
	return EXIT_SUCCESS;
}
