/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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
#include <string.h>
#include <stdbool.h>
#include <mach-o/dyld.h>  
#include <mach-o/dyld_priv.h>  
#include <dlfcn.h>  

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()



static enum { start, barDeps, bar, fooDeps, foo, mainDeps, main } step = start;


static const char* depInitHandler(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[])
{
	if ( infoCount != 1 ) {
		FAIL("image-state-dependents-initialized: depInitHandler infoCount != 1\n");
		exit(0);
	}

	//fprintf(stderr, "depInitHandler(%s), state=%d\n", info[0].imageFilePath, step);

	if ( (step == start) && (strstr(info[0].imageFilePath, "libbar.dylib") != NULL) )
		step = barDeps;
	else if ( (step == bar) && (strstr(info[0].imageFilePath, "libfoo.dylib") != NULL) )
		step = fooDeps;
	else if ( (step == foo) && (strstr(info[0].imageFilePath, "/main") != NULL) )
		step = mainDeps;
	else {
		fprintf(stderr, "out of order depInitHandler\n");
		exit(0);
	}
	

	return NULL;
}

static const char* initHandler(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[])
{
	if ( infoCount != 1 ) {
		FAIL("image-state-dependents-initialized: initHandler infoCount != 1\n");
		exit(0);
	}

	//fprintf(stderr, "initHandler(%s), state=%d\n", info[0].imageFilePath, step);
	bool isBar = (strstr(info[0].imageFilePath, "libbar.dylib") != NULL);
	bool isFoo = (strstr(info[0].imageFilePath, "libfoo.dylib") != NULL);
	bool isMain= (strstr(info[0].imageFilePath, "/main") != NULL);
	
	if ( isBar ) {
		if ( step == barDeps ) {
			step = bar;
		}
		else {
			fprintf(stderr, "out of order initHandler bar\n");
			exit(0);
		}
	}
	else if ( isFoo ) {
		if ( step == fooDeps ) {
			step = foo;
		}
		else {
			fprintf(stderr, "out of order initHandler foo\n");
			exit(0);
		}
	}
	else if ( isMain ) {
		if ( step == mainDeps ) {
			step = main;
		}
		else {
			fprintf(stderr, "out of order initHandler main\n");
			exit(0);
		}
	}


	return NULL;
}




void __attribute__((constructor)) setup_bar()
{
	// tell dyld we want to know when images are mapped
	dyld_register_image_state_change_handler(dyld_image_state_dependents_initialized, false, depInitHandler);
	dyld_register_image_state_change_handler(dyld_image_state_initialized, false, initHandler);
}



