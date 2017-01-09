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
#include <stdio.h>
#include <stdbool.h>
#include <dlfcn.h> 
#include <mach-o/dyld.h>

#include "test.h" // PASS(), FAIL()

typedef bool (*CheckFunc)();

int main()
{
// NSCreateObjectFileImageFromMemory is only available on Mac OS X - not iPhone OS
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	const char* path = "test.bundle";

	NSObjectFileImage ofi;
	if ( NSCreateObjectFileImageFromFile(path, &ofi) != NSObjectFileImageSuccess ) {
		FAIL("NSCreateObjectFileImageFromFile failed");
		return 0;
	}
	
	NSModule mod = NSLinkModule(ofi, path, NSLINKMODULE_OPTION_NONE);
	if ( mod == NULL ) {
		FAIL("NSLinkModule failed");
		return 0;
	}
	
	NSSymbol sym = NSLookupSymbolInModule(mod, "_checkdata");
	if ( sym == NULL ) {
		FAIL("NSLookupSymbolInModule failed");
		return 0;
	}

	CheckFunc func = NSAddressOfSymbol(sym);
	if ( !func() ) {
		FAIL("NSAddressOfSymbol failed");
		return 0;
	}

	Dl_info info;
	if ( dladdr(func, &info) == 0 ) {
		FAIL("dladdr(func, &info) failed");
		return 0;
	}
	
	if ( info.dli_fname == path ) {
		FAIL("NSLinkModule() did not make a copy of the path");
		return 0;
	}

	if ( !NSUnLinkModule(mod, NSUNLINKMODULE_OPTION_NONE) ) {
		FAIL("NSUnLinkModule failed");
		return 0;
	}

	if ( !NSDestroyObjectFileImage(ofi) ) {
		FAIL("NSDestroyObjectFileImage failed");
		return 0;
	}
#endif
	PASS("bundle-name-ownership");
	return 0;
}