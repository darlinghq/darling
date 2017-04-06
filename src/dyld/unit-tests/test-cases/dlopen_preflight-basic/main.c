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

//
// By returning a string, we prevent that image from loading.
// We just prevent any image with "bar" in its name from loading.
//

static const char* batchMappedHandler(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[])
{
	//for (uint32_t i=0; i < infoCount; ++i) {
	//	fprintf(stderr, "batch mapped: %d/%d %s\n", i, infoCount, info[i].imageFilePath);
	//}
	for (uint32_t i=0; i < infoCount; ++i) {
		if ( strstr(info[i].imageFilePath, "bar") != NULL )
			return "cannot load bar";
	}
	return NULL;
}

static const char* singleMappedHandler(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[])
{
	//fprintf(stderr, "single mapped: %s\n", info[0].imageFilePath);
	if ( strstr(info[0].imageFilePath, "bar") != NULL )
		return "can't load bar";
	return NULL;
}


int main(int argc, const char* argv[])
{
	// tell dyld we want to know when images are mapped
	if ( argc > 1 )
		dyld_register_image_state_change_handler(dyld_image_state_dependents_mapped, true, batchMappedHandler);
	else
		dyld_register_image_state_change_handler(dyld_image_state_mapped, false, singleMappedHandler);
	
	if ( dlopen_preflight("foo.bundle") ) {
		FAIL("dlopen_preflight-basic foo.bundle should not be loadable");
		exit(0);
	}
	//fprintf(stderr, "foo.bundle, dlerror: %s\n", dlerror());

	if ( dlopen_preflight("bar.bundle") ) {
		FAIL("dlopen_preflight-basic bar.bundle should not be loadable");
		exit(0);
	}
	//fprintf(stderr, "bar.bundle, dlerror: %s\n", dlerror());

	if ( dlopen_preflight("bogus.bundle") ) {
		FAIL("dlopen_preflight-basic bogus.bundle should not be loadable");
		exit(0);
	}
	//fprintf(stderr, "bogus.bundle, dlerror: %s\n", dlerror());

	if ( ! dlopen_preflight("/usr/lib/libSystem.B.dylib") ) {
		FAIL("dlopen_preflight-basic libSystem should be loadable, but got: %s", dlerror());
		exit(0);
	}

	
	PASS("dlopen_preflight-basic");
		
	return EXIT_SUCCESS;
}
