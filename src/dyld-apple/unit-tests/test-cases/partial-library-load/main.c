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
#include <string.h>
#include <mach-o/dyld.h>

#include "test.h" // PASS(), FAIL()


int main()
{
// NSCreateObjectFileImageFromMemory is only available on Mac OS X - not iPhone OS
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	// load bundle which indirectly loads libfoo and libbar
	NSObjectFileImage ofi;
	if ( NSCreateObjectFileImageFromFile("test.bundle", &ofi) != NSObjectFileImageSuccess ) {
		FAIL("NSCreateObjectFileImageFromFile failed");
		return 1;
	}
	
	// link bundle, this will fail because bar2 is missing from libbar
	NSModule mod = NSLinkModule(ofi, "test.bundle", NSLINKMODULE_OPTION_RETURN_ON_ERROR);
	if ( mod != NULL ) {
		FAIL("NSLinkModule succeeded but should have failed");
		return 1;
	}

	// load libfoo, this should fail because it can't be loaded
	const struct mach_header* mh = NSAddImage("libfoo.dylib", NSADDIMAGE_OPTION_RETURN_ON_ERROR);
	if ( mh != NULL ) {
		return 1;
	}
#endif
#if 0	
	// find foo
	NSSymbol sym = NSLookupSymbolInImage(mh, "_foo", NSLOOKUPSYMBOLINIMAGE_OPTION_BIND);
	if ( sym == NULL ) {
		FAIL("NSLookupSymbolInImage failed");
		return 1;
	}

	// if foo() was only partially bound, this will crash
	int (*fooPtr)() = NSAddressOfSymbol(sym);
	(*fooPtr)();
#endif	
	return 0;
}