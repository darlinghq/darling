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

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


int foo()
{
	return 42;
}


int main()
{
	void* handle = (void*)0x12345;	// bogus value
	
	// expect dlsym() to return NULL
	void* sym = dlsym(handle, "foo");
	if ( sym != NULL ) {
		FAIL("dlsym(handle, \"foo\") should not have succeeded");
		exit(1);
	}
	// expect dlerro() to mention "handle"
	if ( strstr(dlerror(), "handle") == NULL ) {
		FAIL("dlerror() after dlsym(handle, \"foo\") does not mention \"handle\"");
		exit(1);
	}
	
	PASS("dlsym-error");
	return EXIT_SUCCESS;
}
