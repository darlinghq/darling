/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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

#include "test.h" // PASS(), FAIL()

/// 
/// The point of this test is to load and unload a bundle and
/// verify that the address->image cache is properly invalidated.  rdar://problem/4212667
///

typedef void (*fooProc)();

// test.bundle
void doit()
{
// NSCreateObjectFileImageFromMemory is only available on Mac OS X - not iPhone OS
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	NSObjectFileImage ofi;
	if ( NSCreateObjectFileImageFromFile("test.bundle", &ofi) != NSObjectFileImageSuccess ) {
		FAIL("NSCreateObjectFileImageFromFile failed");
		exit(0);
	}

	NSModule mod = NSLinkModule(ofi,"test.bundle", NSLINKMODULE_OPTION_PRIVATE | NSLINKMODULE_OPTION_BINDNOW | NSLINKMODULE_OPTION_RETURN_ON_ERROR);
	if ( mod == NULL ) {
		FAIL("NSLinkModule failed");
		exit(0);
	}

	NSSymbol sym = NSLookupSymbolInModule(mod, "_foo");
	if ( sym == NULL ) {
		FAIL("NSLookupSymbolInModule failed");
		exit(0);
	}

	fooProc func = (fooProc)NSAddressOfSymbol(sym);
	(*func)();
	
	if ( !NSUnLinkModule(mod, NSUNLINKMODULE_OPTION_NONE) ) {
		FAIL("NSUnLinkModule failed");
		exit(0);
	}

	if ( !NSDestroyObjectFileImage(ofi) ) {
		FAIL("NSDestroyObjectFileImage failed");
		exit(0);
	}
#endif
}


static void myRemoveImage(const struct mach_header *mh, intptr_t vmaddr_slide)
{
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	// calling _dyld_get_image_header_containing_address() during the remove image hook 
	// could cause dyld to not flush the address->image cache
	_dyld_get_image_header_containing_address(mh);
#endif
}


int main()
{
	_dyld_register_func_for_remove_image(&myRemoveImage);

	doit();
	doit();
	doit();
	
	PASS("bundle-reload");
	return 0;
}

