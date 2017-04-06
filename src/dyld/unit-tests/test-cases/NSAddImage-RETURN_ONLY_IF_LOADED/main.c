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
#include <stdio.h>
#include <stdlib.h>
#include <mach-o/dyld.h>
#include <Availability.h>

#include "test.h"

/// rdar://problem/3748251 

int main()
{
// NSAddImage is only available on Mac OS X - not iPhone OS
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	// test that NSAddImage() does not crash if image is not loaded
	const struct mach_header * mh = NSAddImage("/System/Library/Frameworks/Cocoa.framework/Cocoa", NSADDIMAGE_OPTION_RETURN_ONLY_IF_LOADED);
	if ( mh != NULL )
		FAIL("NSAddImage-RETURN_ONLY_IF_LOADED");
	else
#endif
		PASS("NSAddImage-RETURN_ONLY_IF_LOADED");
	return EXIT_SUCCESS;
}
