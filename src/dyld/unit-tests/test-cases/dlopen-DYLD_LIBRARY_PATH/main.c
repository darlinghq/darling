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
#include <string.h>
#include <stdlib.h> // for getenv()

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()



typedef int (*fooproc)();

int main(int argc, const char* argv[])
{
	void* handle = dlopen(argv[1], RTLD_LAZY);
	if ( handle == NULL ) {
		FAIL("dlopen(\"%s\") failed", argv[1]);
		exit(0);
	}
	
	fooproc sym = (fooproc)dlsym(handle, "foo");
	if ( sym == NULL ) {
		FAIL("dlsym(handle, \"foo\") failed");
		exit(0);
	}
	
	int expectedResult = 0;
	if ( getenv("DYLD_LIBRARY_PATH") != NULL )
		expectedResult = 1;
		
	int actualResult = (*sym)();
	
	if ( actualResult != expectedResult )
		FAIL("dlopen-DYLD_LIBRARY_PATH using wrong dylib");
	else
		PASS("dlopen-DYLD_LIBRARY_PATH");
		
	return EXIT_SUCCESS;
}
