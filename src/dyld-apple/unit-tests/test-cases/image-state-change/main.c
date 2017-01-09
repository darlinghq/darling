/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
#include <mach-o/dyld.h>  
#include <mach-o/dyld_priv.h>  
#include <dlfcn.h>  

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()



static int singleMappedCount = 0;
static int batchMappedCount = 0;
static int singleUnMappedCount = 0;
static int batchBoundCount = 0;
static int singleDepsInitedCount = 0;
static int singleBoundCount = 0;

static const char* batchMappedHandler(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[])
{
	for (uint32_t i=0; i < infoCount; ++i) 
		printf("batchMappedHandler(): %u/%u -> %s\n", i, infoCount, info[i].imageFilePath);
	if ( state != dyld_image_state_dependents_mapped ) {
		FAIL("image-state-change: batchMappedHandler passed state %d", state);
		exit(0);
	}
	batchMappedCount += infoCount;
	return NULL;
}

static const char* batchBoundHandler(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[])
{
	for (uint32_t i=0; i < infoCount; ++i) 
		printf("batchBoundHandler(): %u/%u -> %s\n", i, infoCount, info[i].imageFilePath);
	if ( state != dyld_image_state_bound ) {
		FAIL("image-state-change: batchBoundHandler passed state %d", state);
		exit(0);
	}
	batchBoundCount += infoCount;
	return NULL;
}

static const char* singleMappedHandler(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[])
{
	printf("singleMappedHandler(%s)\n", info[0].imageFilePath);
	if ( state != dyld_image_state_mapped ) {
		FAIL("image-state-change: singleMappedHandler passed state %d", state);
		exit(0);
	}
	if ( infoCount != 1 ) {
		FAIL("image-state-change: singleMappedHandler given %d images", infoCount);
		exit(0);
	}
	++singleMappedCount;
	return NULL;
}


static const char* singleBoundHandler(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[])
{
	printf("singleBoundHandler(%s)\n", info[0].imageFilePath);
	if ( state != dyld_image_state_bound ) {
		FAIL("image-state-change: singleBoundHandler passed state %d", state);
		exit(0);
	}
	if ( infoCount != 1 ) {
		FAIL("image-state-change: singleBoundHandler given %d images", infoCount);
		exit(0);
	}
	++singleBoundCount;
	return NULL;
}



static const char* singleDepsInitedHandler(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[])
{
	printf("singleDepsInitedHandler(%s)\n", info[0].imageFilePath);
	if ( state != dyld_image_state_dependents_initialized ) {
		FAIL("image-state-change: singleDepsInitedHandler passed state %d", state);
		exit(0);
	}
	if ( infoCount != 1 ) {
		FAIL("image-state-change: singleDepsInitedHandler given %d images", infoCount);
		exit(0);
	}
	++singleDepsInitedCount;
	return NULL;
}



static const char* singleUnmappedHandler(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[])
{
	printf("singleUnmappedHandler(%s)\n", info[0].imageFilePath);
	if ( state != dyld_image_state_terminated ) {
		FAIL("image-state-change: singleUnmappedHandler passed state %d", state);
		exit(0);
	}
	if ( infoCount != 1 ) {
		FAIL("image-state-change: singleUnmappedHandler given %d images", infoCount);
		exit(0);
	}
	++singleUnMappedCount;
	return NULL;
}

static void loadAndUnLoad()
{
	void* handle = dlopen("foo.bundle", RTLD_LAZY);
	if ( handle == NULL ) {
		FAIL("image-state-change: dlopen(foo.bundle) failed: %s", dlerror());
		exit(0);
	}
		
	int result = dlclose(handle);
	if ( result != 0 ) {
		FAIL("image-state-change: dlclose(handle) returned %d, %s", result, dlerror());
		exit(0);
	}
}


int main(int argc, const char* argv[])
{
	// tell dyld we want to know when images are mapped
	dyld_register_image_state_change_handler(dyld_image_state_dependents_mapped, true, batchMappedHandler);
	dyld_register_image_state_change_handler(dyld_image_state_mapped, false, singleMappedHandler);
	dyld_register_image_state_change_handler(dyld_image_state_bound, true, batchBoundHandler);
 	dyld_register_image_state_change_handler(dyld_image_state_bound, false, singleBoundHandler);
	dyld_register_image_state_change_handler(dyld_image_state_dependents_initialized, false, singleDepsInitedHandler);
	dyld_register_image_state_change_handler(dyld_image_state_terminated, false, singleUnmappedHandler);

	// with batch mode we get notified of existing images, but not with single mode, so re-sync counts
	batchMappedCount=0;
	
	loadAndUnLoad();

	loadAndUnLoad();
	
	if ( (singleMappedCount == batchMappedCount) && (singleMappedCount == singleUnMappedCount) )
		PASS("image-state-change");
	else
		FAIL("image-state-change: batch count=%d, single count=%d", batchMappedCount, singleMappedCount);
		
	return EXIT_SUCCESS;
}
