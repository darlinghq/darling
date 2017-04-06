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
#include <stdlib.h> // EXIT_SUCCESS
#include <stdio.h> 
#include <dlfcn.h> 
#include <mach-o/dyld.h>
#include <mach-o/dyld_images.h>

#include "test.h"

extern struct mach_header __dso_handle;


struct dyld_all_image_infos* getImageInfosFromKernel()
{
	task_dyld_info_data_t task_dyld_info;
	mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
    
    if ( task_info(mach_task_self(), TASK_DYLD_INFO, (task_info_t)&task_dyld_info, &count) ) {
		FAIL("all_image_infos: task_info() failed");
		exit(0);
	}
	return (struct dyld_all_image_infos*)(uintptr_t)task_dyld_info.all_image_info_addr;
}


int
main()
{
	struct dyld_all_image_infos* info = getImageInfosFromKernel();
	if ( info->version < 10 ) {
		FAIL("image-count: dyld_all_image_infos is < 10");
		exit(0);
	}

	if ( info->infoArrayCount !=  info->initialImageCount ) {
		FAIL("image-count: dyld_all_image_infos.infoArrayCount != dyld_all_image_infos.initialImageCount");
		exit(0);
	}

	void* h = dlopen("libfoo.dylib", RTLD_LAZY);
	if ( h == NULL ) {
		FAIL("image-count: dyld_all_image_infos is < 10");
		exit(0);
	}

	if ( info->infoArrayCount != (info->initialImageCount+1) ) {
		FAIL("image-count: infoArrayCount did not increment when libfoo.dylib was loaded");
		exit(0);
	}

	dlclose(h);
	
	if ( info->infoArrayCount !=  info->initialImageCount ) {
		FAIL("image-count: infoArrayCount did not decrement when libfoo.dylib was unloaded");
		exit(0);
	}

	PASS("image-count");
	return EXIT_SUCCESS;
}


