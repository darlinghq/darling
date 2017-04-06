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
#include <mach-o/dyld_images.h>
#include <mach-o/dyld_priv.h>  
#include <dlfcn.h>  

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


//
// By returning a string, we prevent that image from loading.
// We just prevent any image with "bar" in its name from loading.
//


static const char* singleMappedHandler(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[])
{
	//fprintf(stderr, "single mapped: %s\n", info[0].imageFilePath);
	if ( strstr(info[0].imageFilePath, "bar") != NULL )
		return "can't load bar";
	return NULL;
}


static void load(const char* name)
{
	void* handle = dlopen(name, RTLD_LAZY);
	if ( handle != NULL ) {
		FAIL("image-state-deny-all_image_infos: dlopen(%s) should have failed", name);
		exit(0);
	}
}


int main(int argc, const char* argv[])
{
	// tell dyld we want to know when images are mapped
	dyld_register_image_state_change_handler(dyld_image_state_mapped, false, singleMappedHandler);
	
    const struct dyld_all_image_infos* infos = _dyld_get_all_image_infos();
    const uint32_t initialCount = infos->infoArrayCount;
    
	load("bar.bundle");

    if ( infos->infoArrayCount != initialCount ){
		FAIL("image-state-deny-all_image_infos: infoArrayCount should not have changed");
		exit(0);
	}

    //for (int i=0; i < infos->infoArrayCount; ++i)
    //    printf("mh=%p, path=%s\n", infos->infoArray[i].imageLoadAddress, infos->infoArray[i].imageFilePath); 
	
	PASS("image-state-deny-all_image_infos");
		
	return EXIT_SUCCESS;
}
