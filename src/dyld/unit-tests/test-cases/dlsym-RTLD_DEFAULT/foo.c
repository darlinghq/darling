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
#include <string.h>
#include <dlfcn.h>
#include <stdlib.h> // exit(), EXIT_SUCCESS

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

char* strdup(const char* str)
{
	return "from foo";
}

typedef char* (*strdupProc)(const char* str);


void myinit() __attribute__((constructor));
void myinit()
{
	strdupProc sym = (strdupProc)dlsym(RTLD_DEFAULT, "strdup");
	if ( sym == NULL ) {
		FAIL("dlsym(RTLD_DEFAULT, \"strdup\") failed");
		exit(0);
	}
	
	const char* result = (*sym)("hello");
	if ( strcmp(result, "from main") != 0 ) {
		FAIL("dlsym(RTLD_DEFAULT, \"strdup\") returned wrong strdup: %s", result);
		exit(0);
	}

}
