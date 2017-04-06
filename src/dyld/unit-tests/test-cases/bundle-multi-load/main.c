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
#include <mach-o/dyld.h>
#include <Availability.h>

#include "test.h" // PASS(), FAIL()

///
/// The point of this test is to load the same bundle file multiple times and
/// verify each time it is linked is a new instantiations (new globals, etc)
///

int main()
{
// NSCreateObjectFileImageFromMemory is only available on Mac OS X - not iPhone OS
#if __MAC_OS_X_VERSION_MIN_REQUIRED
	NSObjectFileImage ofi;
	if ( NSCreateObjectFileImageFromFile("test.bundle", &ofi) != NSObjectFileImageSuccess ) {
		FAIL("NSCreateObjectFileImageFromFile failed");
		return 0;
	}

	NSModule mod = NSLinkModule(ofi, "test.bundle", NSLINKMODULE_OPTION_NONE);
	if ( mod == NULL ) {
		FAIL("NSLinkModule failed");
		return 0;
	}

	NSSymbol sym = NSLookupSymbolInModule(mod, "_foo");
	if ( sym == NULL ) {
		FAIL("NSLookupSymbolInModule failed");
		return 0;
	}

	void* func = NSAddressOfSymbol(sym);
	//fprintf(stderr, "1st address of foo() = %p in module %p in OFI %p\n", func, mod, ofi);


	NSObjectFileImage ofi2;
	if ( NSCreateObjectFileImageFromFile("test.bundle", &ofi2) != NSObjectFileImageSuccess ) {
		FAIL("2nd NSCreateObjectFileImageFromFile failed");
		return 0;
	}

	NSModule mod2 = NSLinkModule(ofi2, "test2.bundle", NSLINKMODULE_OPTION_NONE);
	if ( mod2 == NULL ) {
		FAIL("2nd NSLookupSymbolInModule failed");
		return 0;
	}
	if ( mod == mod2 ) {
		FAIL("2nd NSLinkModule return same function address as first\n");
		return 0;
	}

	NSSymbol sym2 = NSLookupSymbolInModule(mod2, "_foo");
	if ( sym2 == NULL ) {
		FAIL("2nd NSLookupSymbolInModule failed\n");
		return 0;
	}

	void* func2 = NSAddressOfSymbol(sym2);
	//fprintf(stderr, "2nd address of foo() = %p in module %p in OFI %p\n", func2, mod2, ofi2);
	if ( func == func2 ) {
		FAIL("2nd NSAddressOfSymbol return same function address as 1st\n");
		return 0;
	}
	
	
	NSObjectFileImage ofi3;
	if ( NSCreateObjectFileImageFromFile("test.bundle", &ofi3) != NSObjectFileImageSuccess ) {
		FAIL("3rd NSCreateObjectFileImageFromFile failed");
		return 0;
	}
	NSModule mod3 = NSLinkModule(ofi3, "test3.bundle", NSLINKMODULE_OPTION_NONE);
	if ( mod3 == NULL ) {
		FAIL("3rd NSLinkModule failed\n");
		return 0;
	}
	if ( mod3 == mod ) {
		FAIL("3rd NSLinkModule return same function address as 1st\n");
		return 0;
	}
	if ( mod3 == mod2 ) {
		FAIL("3rd NSLinkModule return same function address as 2nd\n");
		return 0;
	}

   NSSymbol sym3 = NSLookupSymbolInModule(mod3, "_foo");
	if ( sym3 == NULL ) {
		FAIL("3rd NSLookupSymbolInModule failed\n");
		return 0;
	}
	void* func3 = NSAddressOfSymbol(sym3);
	//fprintf(stderr, "3rd address of foo() = %p in module %p in OFI %p\n", func3, mod3, ofi3);
	if ( func3 == func ) {
		FAIL("3rd NSAddressOfSymbol return same function address as 1st\n");
		return 0;
	}
	if ( func3 == func2 ) {
		FAIL("3rd NSAddressOfSymbol return same function address as 2nd\n");
		return 0;
	}
		
	if ( !NSUnLinkModule(mod, NSUNLINKMODULE_OPTION_NONE) ) {
		FAIL("NSUnLinkModule failed");
		return 0;
	}

	if ( !NSUnLinkModule(mod3, NSUNLINKMODULE_OPTION_NONE) ) {
		FAIL("3rd NSUnLinkModule failed");
		return 0;
	}
	
	// note, we are calling NSDestroyObjectFileImage() before NSUnLinkModule()
	if ( !NSDestroyObjectFileImage(ofi2) ) {
		FAIL("2nd NSDestroyObjectFileImage failed");
		return 0;
	}
	if ( !NSUnLinkModule(mod2, NSUNLINKMODULE_OPTION_NONE) ) {
		FAIL("2nd NSUnLinkModule failed");
		return 0;
	}
	
	if ( !NSDestroyObjectFileImage(ofi) ) {
		FAIL("1st NSDestroyObjectFileImage failed");
		return 0;
	}
	if ( !NSDestroyObjectFileImage(ofi3) ) {
		FAIL("3rd NSDestroyObjectFileImage failed");
		return 0;
	}
#endif	
	PASS("bundle-multi-load");
	return 0;
}

