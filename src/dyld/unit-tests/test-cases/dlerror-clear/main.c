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
#include <pthread.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


///
/// This tests that the dlerror message is cleared when dlerror is called
///

int main()
{
	// try to non-existent library
	void* handle1 = dlopen("frobulite", RTLD_LAZY);
	if ( handle1 != NULL ) {
		FAIL("dlerror-clear: dlopen(\"frobulite\", RTLD_LAZY) succeeded but should have failed");
		exit(0);
	}
	
	// verify there is an error message
	const char* msg = dlerror();
	if ( msg == NULL ) {
		FAIL("dlerror-clear: dlerror() returned NULL but should have returned an error message");
		exit(0);
	}
	
	// verify error message was cleared
	const char* msg2 = dlerror();
	if ( msg2 != NULL ) {
		FAIL("dlerror-clear: dlerror() returned message but should have returned NULL");
		exit(0);
	}
	
	PASS("dlerror-clear");
	return 0;
}
