/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
#include <stdbool.h>  // fprintf(), NULL
#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <stdbool.h>
#include <string.h>
#include <mach-o/dyld.h>  
#include <mach-o/dyld_priv.h>  
#include <dlfcn.h>  

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


int main(int argc, const char* argv[])
{
// NSObjectFile* APIs are only available on Mac OS X - not iPhone OS
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	// load foo.bundle with old API
	NSObjectFileImage ofi;
	if ( NSCreateObjectFileImageFromFile("foo.bundle", &ofi) != NSObjectFileImageSuccess ) {
		FAIL("bundle-dont-gc: NSCreateObjectFileImageFromFile failed");
		return 1;
	}
	
	NSModule mod = NSLinkModule(ofi, "foo.bundle", NSLINKMODULE_OPTION_NONE);
	if ( mod == NULL ) {
		FAIL("bundle-dont-gc: NSLinkModule failed");
		return 1;
	}
	
	NSSymbol sym = NSLookupSymbolInModule(mod, "_foo");
	if ( sym == NULL ) {
		FAIL("bundle-dont-gc: NSLookupSymbolInModule failed");
		return 1;
	}

	void* fooAddr = NSAddressOfSymbol(sym);
	if ( fooAddr == NULL ) {
		FAIL("bundle-dont-gc: NSAddressOfSymbol failed");
		return 1;
	}

	if ( !NSDestroyObjectFileImage(ofi) ) {
		FAIL("bundle-dont-gc: NSDestroyObjectFileImage failed");
		return 1;
	}

	// open and close bar.bundle with new API, which causes a gc of images
	void* h1 = dlopen("bar.bundle", RTLD_LAZY);
	if ( h1 == NULL ) {
		FAIL("bundle-dont-gc: can't dlopen bar.bundle: %s", dlerror());
		return EXIT_SUCCESS;
	}
	dlclose(h1);
	
	// make sure foo is still loaded
	Dl_info info;
	if ( dladdr(fooAddr, &info) == 0 ) {
		FAIL("bundle-dont-gc: dladdr() failed");
		exit(0);
	}
	
	// unload foo.bundle with old API
	if ( !NSUnLinkModule(mod, NSUNLINKMODULE_OPTION_NONE) ) {
		FAIL("NSUnLinkModule failed");
		return 1;
	}
#endif	

	PASS("bundle-dont-gc");
	return EXIT_SUCCESS;
}
