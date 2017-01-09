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

const char* sBlockName = NULL;

static const char* singleMappedHandler(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[])
{
	//fprintf(stderr, "single mapped: %s\n", info[0].imageFilePath);
	if ( strstr(info[0].imageFilePath, sBlockName) != NULL )
		return "can't load blocked name";
	return NULL;
}


int main(int argc, const char* argv[])
{
	// tell dyld we want to know when images are mapped
  dyld_register_image_state_change_handler(dyld_image_state_mapped, false, singleMappedHandler);

  sBlockName = "libbar1";
	void* handle = dlopen("libfoo.dylib", RTLD_LAZY);
	if ( handle != NULL ) {
    FAIL("image-state-deny-leak: dlopen(libfoo.dylib) should have failed");
    return EXIT_SUCCESS;
	}

  sBlockName = "libbar2";
	handle = dlopen("libfoo.dylib", RTLD_LAZY);
	if ( handle == NULL ) {
    FAIL("image-state-deny-leak: dlopen(libfoo.dylib) should have succeeded");
    return EXIT_SUCCESS;
	}

  // execute leaks command on myself
	char cmd[512];
	sprintf(cmd, "sudo leaks %u > /dev/null \n", getpid());
	int result = system(cmd);
	if ( result == EXIT_SUCCESS )
		PASS("image-state-deny-leak");
	else
		FAIL("image-state-deny-leak");


		
	return EXIT_SUCCESS;
}
