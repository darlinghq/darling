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
#include <stdbool.h>  // fprintf(), NULL
#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <stdbool.h>
#include <string.h>
#include <mach-o/dyld.h>  
#include <mach-o/dyld_priv.h>  
#include <dlfcn.h>  

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


int main(int argc, const char* argv[])
{
	// open and close base
	void* h1 = dlopen("libbase.dylib", RTLD_LAZY);
	if ( h1 == NULL ) {
		FAIL("image-state-deny-dlclose: can't dlopen libbase.dylib: %s", dlerror());
		return EXIT_SUCCESS;
	}
	dlclose(h1);
	
	// if base was unloaded, then this dlopen will jump to neverland
	void* h2 = dlopen("libfoo.dylib", RTLD_LAZY);
	if ( h2 == NULL ) {
		FAIL("image-state-deny-dlclose: can't dlopen libfoo.dylib: %s", dlerror());
		return EXIT_SUCCESS;
	}
	
	void* h3 = dlopen("libbase.dylib", RTLD_LAZY);
	if ( h3 == NULL ) {
		FAIL("image-state-deny-dlclose: can't dlopen libbase.dylib: %s", dlerror());
		return EXIT_SUCCESS;
	}
	dlclose(h3);

	dlclose(h2);
		
		
	PASS("image-state-deny-dlclose");
		
	return EXIT_SUCCESS;
}
