/*
 * Copyright (c) 2008-2009 Apple Inc. All rights reserved.
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
#include <mach-o/dyld.h>
#include <mach-o/dyld_images.h>
#include <mach/mach.h>
#include <Availability.h>

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
	struct dyld_all_image_infos* infos = getImageInfosFromKernel();
	if ( infos->version < 9 ) {
		FAIL("all_image_infos: dyld_all_image_infos is not version >= 9, is  %d", infos->version);
		exit(0);
	}

	if ( infos->infoArrayCount < 2 ) {
		FAIL("all_image_infos: dyld_all_image_infos.infoArrayCount is  < 2");
		exit(0);
	}

	//for( int i=0; i < infos->infoArrayCount; ++i) {
	//	fprintf(stderr, "infos->infoArray[%d].imageLoadAddress=%p %s\n", i, infos->infoArray[i].imageLoadAddress, infos->infoArray[i].imageFilePath);
	//}
	
	if ( infos->infoArray[0].imageLoadAddress != &__dso_handle ) {
		FAIL("all_image_infos: dyld_all_image_infos.infoArray for main executable is wrong, infos->infoArray[0].imageLoadAddress=0x%08lX vs 0x%08X", 
					infos->infoArray[0].imageLoadAddress, &__dso_handle);
		exit(0);
	}

#if __IPHONE_OS_VERSION_MIN_REQUIRED
	for (const struct dyld_image_info* p = infos->infoArray; p < &infos->infoArray[infos->infoArrayCount]; ++p) {
		//fprintf(stderr, "addr=0x%p, mTime=0x%lX, path=%s\n", p->imageLoadAddress, p->imageFileModDate, p->imageFilePath);
		if ( (strncmp(p->imageFilePath, "/usr/lib", 8) == 0) && (p->imageFileModDate != 0) ) {
			FAIL("all_image_infos, mTime not zero for cache image %s", p->imageFilePath);
			exit(0);
		}
	}
#endif

	PASS("all_image_infos");
	return EXIT_SUCCESS;
}


