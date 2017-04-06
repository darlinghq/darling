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
	// regular open
	void* handle = dlopen("test.bundle", RTLD_LAZY);
	if ( handle == NULL ) {
		FAIL("dlopen(\"test.bundle\", RTLD_LAZY) failed");
		exit(1);
	}
	
	// regular close
	int result = dlclose(handle);
	if ( result != 0 ) {
		FAIL("dlclose() failed");
		exit(1);
	}
	
	// now try to close again (expect to fail)
	result = dlclose(handle);
	if ( result != -1 ) {
		FAIL("dlclose() on released handle should have returned -1, but returned %d", result);
		exit(1);
	}
	
	// now try to close a bad handle value (expect to fail)
	result = dlclose((void*)0x12345);
	if ( result != -1 ) {
		FAIL("dlclose() on bogus handle should have returned -1, but returned %d", result);
		exit(1);
	}
	  
	PASS("dlclose-basic");
	return EXIT_SUCCESS;
}
