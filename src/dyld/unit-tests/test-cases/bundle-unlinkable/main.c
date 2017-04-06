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
#include <string.h>
#include <mach-o/dyld.h>
#include <Availability.h>

#include "test.h" // PASS(), FAIL()

typedef bool (*CheckFunc)();

int main()
{
// NSCreateObjectFileImageFromMemory is only available on Mac OS X - not iPhone OS
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	NSObjectFileImage ofi;
	if ( NSCreateObjectFileImageFromFile("test.bundle", &ofi) != NSObjectFileImageSuccess ) {
		FAIL("NSCreateObjectFileImageFromFile failed");
		return 1;
	}
	
	// make sure not-yet-linked-ofi is not visible through _dyld_get_image_name
	int count = _dyld_image_count();
	for(int i=0; i < count; ++i) {
	const char* name = _dyld_get_image_name(i);
		if ( strcmp(name, "test.bundle") == 0 ) {
			FAIL("unlinked test.bundle found via _dyld_get_image_name()");
			return 1;
		}
	}

	NSModule mod = NSLinkModule(ofi, "test.bundle", NSLINKMODULE_OPTION_RETURN_ON_ERROR);
	if ( mod != NULL ) {
		FAIL("NSLinkModule succeeded but should have failed");
		return 1;
	}
	
	// make sure link-failed-ofi is not visible through _dyld_get_image_name
	count = _dyld_image_count();
	for(int i=0; i < count; ++i) {
		const char* name = _dyld_get_image_name(i);
		if ( strcmp(name, "test.bundle") == 0 ) {
			FAIL("failed linked test.bundle found via _dyld_get_image_name()");
			return 1;
		}
	}
#endif
	PASS("bundle-unlinkable");
	return 0;
}