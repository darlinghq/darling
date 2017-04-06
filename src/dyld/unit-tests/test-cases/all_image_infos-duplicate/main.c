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
#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <mach-o/dyld_images.h>
#include <mach/mach.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

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


int main(int argc, const char* argv[])
{
	void* handle = dlopen("libfoo.dylib", RTLD_LAZY);
	if ( handle == NULL ) {
		FAIL("dlopen(\"%s\") failed with: %s", "libfoo.dylib", dlerror());
		exit(0);
	}
	
	void* handle2 = dlopen("libfoodup.dylib", RTLD_LAZY);
	if ( handle2 == NULL ) {
		FAIL("dlopen(\"%s\") failed with: %s", "libfoodup.dylib", dlerror());
		exit(0);
	}
	
 	struct dyld_all_image_infos* infos = getImageInfosFromKernel();
	for( int i=0; i < infos->infoArrayCount; ++i) {
		//fprintf(stderr, "[%d] %s\n", i , infos->infoArray[i].imageFilePath);
		if ( strlen(infos->infoArray[i].imageFilePath) < 5 ) {
			FAIL("all_image_infos-duplicates: bad entry for address 0x%08X", infos->infoArray[i].imageLoadAddress);
			return EXIT_SUCCESS;
		}
	}

	PASS("all_image_infos-duplicates");
	return EXIT_SUCCESS;
}
