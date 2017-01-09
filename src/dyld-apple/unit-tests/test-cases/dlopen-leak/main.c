/*
 * Copyright (c) 2007-2013 Apple Inc. All rights reserved.
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
#include <stdlib.h>

  
#include "test.h"


int main()
{
	for (int i=0; i < 100; ++i) {
		void* handle = dlopen("libfoo.dylib", RTLD_LAZY);
		if ( handle != NULL ) 
			dlclose(handle);
		dlopen("libnotthere.dylib", RTLD_LAZY);
	}
	

	// execute leaks command on myself
	char cmd[512];
	sprintf(cmd, "sudo leaks %u > /dev/null\n", getpid());
	int result = system(cmd);
	if ( result == EXIT_SUCCESS )
		PASS("dlopen-leak");
	else
		FAIL("dlopen-leak");

	return EXIT_SUCCESS;
}
