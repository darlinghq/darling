/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
#include <mach-o/dyld.h>
#include <mach-o/dyld_priv.h>
#include <dlfcn.h>

#include "test.h"

extern void foo();

intptr_t findSlide(const struct mach_header* mh)
{
	int count = _dyld_image_count();
	for(int i=0; i < count; ++i) {
		if ( mh == _dyld_get_image_header(i) ) {
			return _dyld_get_image_vmaddr_slide(i);
		}
	}
	return -1;
}


int
main()
{
	// find mach_header for libfoo.dylib
	const struct mach_header* mh;
	Dl_info info;
	if ( dladdr(&foo, &info) ) {
		mh = info.dli_fbase;
	}
	else {
		FAIL("dladdr() could not find foo()");
		exit(0);
	}
	
	
	intptr_t slide1 = _dyld_get_image_slide(mh);
	intptr_t slide2 = findSlide(mh);
	if ( slide1 == slide2 )
		PASS("image-slide");
	else
		FAIL("image-slide: 0x%lX != 0x%lX", slide1, slide2);
	
	return EXIT_SUCCESS;
}


