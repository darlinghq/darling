/*
 * Copyright (c) 2007-2008 Apple Inc. All rights reserved.
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
#include <string.h>
#include <mach-o/dyld.h>  
#include <mach-o/dyld_priv.h>  
#include <dlfcn.h>  
  
#include "test.h"

static bool doCheck = false;

static const char* batchMappedHandler(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[])
{
	if ( doCheck ) 
		return "can't load";
	return NULL;
}


int main()
{
	// tell dyld we want to know when images are mapped
	dyld_register_image_state_change_handler(dyld_image_state_mapped, false, batchMappedHandler);
	doCheck = true;

	for (int i=0; i < 10; ++i) {
		dlopen_preflight("libfoo.dylib");
	}

	// execute leaks command on myself
	char cmd[512];
	sprintf(cmd, "sudo leaks %u > /dev/null\n", getpid());
	int result = system(cmd);
	if ( result == EXIT_SUCCESS )
		PASS("dlopen_preflight-leak-image-deny-single");
	else
		FAIL("dlopen_preflight-leak-image-deny-single");
	 	
	return EXIT_SUCCESS;
}
