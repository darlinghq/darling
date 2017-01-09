/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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
#include <mach-o/dyld_priv.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

extern uint32_t _cpu_capabilities;

extern const char* foo();

typedef const char* (*BarProc)(void);

const char* myStr = "myStr";

int myInt;

int main()
{
    if ( !_dyld_is_memory_immutable(myStr, 6) ) {
		FAIL("_dyld_is_memory_immutable() returned false for string in main executable");
        return EXIT_SUCCESS;
    }

    if ( _dyld_is_memory_immutable(strdup("hello"), 6) ) {
		FAIL("_dyld_is_memory_immutable() returned true for result from strdup()");
        return EXIT_SUCCESS;
    }

    if ( _dyld_is_memory_immutable(&myInt, 4) ) {
		FAIL("_dyld_is_memory_immutable() returned true for global variabe in main executable");
        return EXIT_SUCCESS;
    }

    if ( !_dyld_is_memory_immutable(foo(), 4) ) {
		FAIL("_dyld_is_memory_immutable() returned false for string in statically linked dylib");
        return EXIT_SUCCESS;
    }

    if ( !_dyld_is_memory_immutable(&strcpy, 4) ) {
		FAIL("_dyld_is_memory_immutable() returned false for function in dyld shared cache");
        return EXIT_SUCCESS;
    }

    if ( _dyld_is_memory_immutable(&_cpu_capabilities, 4) ) {
		FAIL("_dyld_is_memory_immutable() returned true for global variabe in shared cache");
        return EXIT_SUCCESS;
    }

    void* handle = dlopen("libbar.dylib", 0);
    if ( handle == NULL ) {
		FAIL("dlopen(libbar.dylib) failed");
        return EXIT_SUCCESS;
    }

    BarProc proc = dlsym(handle, "bar");
    if ( proc == NULL ) {
		FAIL("dlsym(bar) failed");
        return EXIT_SUCCESS;
    }
    const char* barStr = (*proc)();
    if ( _dyld_is_memory_immutable(barStr, 4) ) {
		FAIL("_dyld_is_memory_immutable() returned true for string in unloadable dylib");
        return EXIT_SUCCESS;
    }


	PASS("_dyld_is_memory_immutable");
	return 0;
}
