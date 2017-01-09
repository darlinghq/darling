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

static uintptr_t libSystemSlide()
{
	Dl_info info;
	if ( dladdr(&malloc, &info) == 0 ) {
		FAIL("all_image_infos-cache-slide: dladdr(&malloc, xx) failed");
		exit(0);
	}
	
	const struct mach_header* mallocMh = (struct mach_header*)info.dli_fbase;
	if ( (mallocMh->flags & 0x80000000) == 0 ) {
		FAIL("all_image_infos-cache-slide: image containing _malloc not in shared cache");
		exit(0);
	}
	
	int count = _dyld_image_count();
	for(int i=0; i < count; ++i) {
		if ( mallocMh == _dyld_get_image_header(i) ) {
			return _dyld_get_image_vmaddr_slide(i);
		}
	}

	FAIL("all_image_infos-cache-slide: slide of image containing _malloc not found");
	exit(0);
}


int main(int argc, const char* argv[])
{
	task_dyld_info_data_t task_dyld_info;
	mach_msg_type_number_t count = TASK_DYLD_INFO_COUNT;
    
    if ( task_info(mach_task_self(), TASK_DYLD_INFO, (task_info_t)&task_dyld_info, &count) ) {
		FAIL("all_image_infos-cache-slide: task_info() failed");
		return 0;
	}
	struct dyld_all_image_infos* infos = (struct dyld_all_image_infos*)(uintptr_t)task_dyld_info.all_image_info_addr;
	
	if ( infos->version < 12 ) {
		FAIL("all_image_infos-cache-slide: version < 12");
		return 0;
	}

	if ( libSystemSlide() != infos->sharedCacheSlide ) {
		FAIL("all_image_infos-cache-slide: dyld infos sharedCacheSlide (0x%08X) does not match computed slide (0x%08X)",
				infos->sharedCacheSlide, libSystemSlide());
		return 0;
	}

	PASS("all_image_infos-cache-slide");
	return EXIT_SUCCESS;
}
