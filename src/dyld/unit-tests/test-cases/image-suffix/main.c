/*
 * Copyright (c) 2005-2009 Apple Inc. All rights reserved.
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
#include <stdlib.h>
#include <mach-o/dyld.h>
#include <dlfcn.h>

#include "test.h"

int main(int argc, const char* argv[])
{
	if ( argc < 2 ) {
		FAIL("too few arguments to main()");
		return EXIT_SUCCESS;
	}

	uint32_t imageCount = _dyld_image_count();
	dlopen(argv[1], 0);
	if ( imageCount != _dyld_image_count() ) {
		FAIL("image count changed");
		return EXIT_SUCCESS;
	}
	
	PASS("images loaded properly");
	return EXIT_SUCCESS;
}

