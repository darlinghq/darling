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

static bool doneRegistering = false;


//
// By returning a string, we prevent that image from loading.
// We just prevent any image with "bar" in its name from loading.
//

static const char* batchMappedHandler(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[])
{
	for (uint32_t i=0; i < infoCount; ++i) {
		//fprintf(stderr, "batch mapped: %d %s\n", i, info[i].imageFilePath);
		if ( strstr(info[i].imageFilePath, "bar") != NULL )
			return "can't load bar";
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



static const char* singleInitializedHandler(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[])
{
	//fprintf(stderr, "singleInitializedHandler(%s)\n", info[0].imageFilePath);
	if ( doneRegistering ) {
		FAIL("image-state-deny something loaded");
		exit(0);
	}
	return NULL;
}

//static const char* batchBoundHandler(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[])
//{
//	//for (uint32_t i=0; i < infoCount; ++i) 
//	//	fprintf(stderr, "bound: %u %s\n", i, info[i].imageFilePath);
//	return NULL;
//}

static void load(const char* name)
{
	void* handle = dlopen(name, RTLD_LAZY);
	if ( handle != NULL ) {
		FAIL("image-state-deny: dlopen(%s) should have failed", name);
		exit(0);
	}
}


int main(int argc, const char* argv[])
{
	// tell dyld we want to know when images successfully loaded
	dyld_register_image_state_change_handler(dyld_image_state_initialized, false, singleInitializedHandler);
	doneRegistering = true;
	
	// tell dyld we want to know when images successfully loaded
	//dyld_register_image_state_change_handler(dyld_image_state_bound, true, batchBoundHandler);

	// tell dyld we want to know when images are mapped
	if ( argc > 1 )
		dyld_register_image_state_change_handler(dyld_image_state_dependents_mapped, true, batchMappedHandler);
	else
		dyld_register_image_state_change_handler(dyld_image_state_mapped, false, singleMappedHandler);
	
	load("foo.bundle");

	load("bar.bundle");
	
	//dlopen("/usr/lib/libz.1.2.3.dylib", RTLD_LAZY);
	
	PASS("image-state-deny");
		
	return EXIT_SUCCESS;
}
