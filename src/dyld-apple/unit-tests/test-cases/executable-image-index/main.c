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
#include <stdint.h>
#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <string.h>
#include <mach-o/ldsyms.h>
#include <mach-o/dyld.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

// many apps rely on _dyld_get_image_header(0) being the main executable
// DYLD_INSERT_LIBRARIES can change the order

extern const struct mach_header __dso_handle;

int main()
{
	if ( _dyld_get_image_header(0) == &__dso_handle )
		PASS("executable-image-index");
	else
		FAIL("executable-image-index: index(0) is %s", _dyld_get_image_name(0));
		
	return EXIT_SUCCESS;
}
