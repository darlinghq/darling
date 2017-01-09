/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
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
#include <stdlib.h> // for atoi()
#include <mach-o/dyld.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()



extern int foo();

int main(int argc, const char* argv[])
{
	if ( argc > 2 ) {
		bool found = false;
		uint32_t count = _dyld_image_count();
		for(uint32_t i=0; i < count; ++i) {
			const char*  name = _dyld_get_image_name(i);
			if ( strstr(name, argv[2]) != NULL )
				found = true;
			//fprintf(stderr, "image[%d]=%s\n", i, name);
		}
		if ( !found ) {
			FAIL("DYLD_VERSIONED_FRAMEWORK_PATH-basic dylib has wrong path");
			return EXIT_SUCCESS;
		}
	}
	
	int expectedResult = atoi(argv[1]);
	int actualResult = foo();
	//fprintf(stderr, "foo() returned %d, expected %d\n", actualResult, expectedResult);
	if ( actualResult != expectedResult )
		FAIL("DYLD_VERSIONED_FRAMEWORK_PATH-basic using wrong dylib. foo() returned %d, expected %d", actualResult, expectedResult);
	else
		PASS("DYLD_VERSIONED_FRAMEWORK_PATH-basic");
		
	return EXIT_SUCCESS;
}

