/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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
#include <mach-o/dyld_priv.h>
#include <mach/mach.h>
#include <Availability.h>
#include <mach/shared_region.h>

#include "test.h"
#include "dyld_cache_format.h"



int main()
{
    uuid_t curUuid;
    _dyld_get_shared_cache_uuid(curUuid);

    int __block imageCount = 0;
    int result = dyld_shared_cache_iterate_text(curUuid, ^(const dyld_shared_cache_dylib_text_info* info) {
        ++imageCount;
        //printf("  cur: 0x%09llX -> 0x%09llX  off=0x%0llX %s\n", info->loadAddressUnslid, info->loadAddressUnslid +  info->textSegmentSize, info->textSegmentOffset, info->path);
    });
	if ( result != 0 ) {
		FAIL("dyld_shared_cache_iterate_text: dyld_shared_cache_iterate_text() failed");
		exit(0);
	}

 	if ( imageCount < 500 ) {
		FAIL("dyld_shared_cache_iterate_text: dyld_shared_cache_iterate_text() iterated less than 500 dylibs (%d)", imageCount);
		exit(0);
	}
#if 0
   //uuid_t fixedUUID = {0x3E, 0xDE, 0x37, 0x05, 0x81, 0x68, 0x33, 0xEF, 0xBF, 0x97, 0xC0, 0xF6, 0xD2, 0x4D, 0x93, 0xEC};
    uuid_t fixedUUID = {0xD4, 0x3B, 0x31, 0x2B, 0xA5, 0xA7, 0x3C, 0x55, 0x90, 0xA0, 0x9A, 0x37, 0x60, 0x7D, 0x70, 0xAF};
    result = dyld_shared_cache_iterate_text(fixedUUID, ^(const dyld_shared_cache_dylib_text_info* info) {
        printf("  my: 0x%09llX -> 0x%09llX  %s\n", info->loadAddressUnslid, info->loadAddressUnslid +  info->textSegmentSize, info->path);
    });
#endif

	PASS("dyld_shared_cache_iterate_text");
	return EXIT_SUCCESS;
}


