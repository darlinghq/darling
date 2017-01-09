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
#include <mach-o/dyld.h>
#include <Availability.h>

#include "test.h" // PASS(), FAIL()


///
/// The point of this test case is to "load" a bundle once, but link it multiple times.
/// Each link creats a new instantiation of the bundle (e.g. new base address and new globals).
///
///

typedef void (*setter)(int);
typedef int (*getter)(void);

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

	NSSymbol sym = NSLookupSymbolInModule(mod, "_setValue");
	if ( sym == NULL ) {
		FAIL("NSLookupSymbolInModule failed");
		return 0;
	}

	setter func = NSAddressOfSymbol(sym);
	(*func)(1);
	//fprintf(stderr, "address of foo() = %p in bundle first load %p\n", func, mod);


	NSModule mod2 = NSLinkModule(ofi, "test2.bundle", NSLINKMODULE_OPTION_NONE);
	if ( mod2 == NULL ) {
		NSLinkEditErrors c; int errorNumber; const char* fileName; const char* errorString;
		NSLinkEditError(&c, &errorNumber, &fileName, &errorString);
		FAIL("2nd NSLinkModule failed: %s", errorString);
		return 0;
	}
	if ( mod == mod2 ) {
		FAIL("2nd NSLinkModule return same function address as first");
		return 0;
	}

	NSSymbol sym2getter = NSLookupSymbolInModule(mod2, "_getValue");
	if ( sym2getter == NULL ) {
		FAIL("2nd NSLookupSymbolInModule failed");
		return 0;
	}
	getter func2getter = NSAddressOfSymbol(sym2getter);
	if ( (*func2getter)() != 0 ) {
		FAIL("_getValue() on second link returned non-zero");
		return 0;
	}

	NSSymbol sym2 = NSLookupSymbolInModule(mod2, "_setValue");
	if ( sym2 == NULL ) {
		FAIL("2nd NSLookupSymbolInModule failed");
		return 0;
	}
	setter func2 = NSAddressOfSymbol(sym2);
	(*func2)(2);
	
	//fprintf(stderr, "address of foo() = %p in bundle second load %p\n", func2, mod2);
	if ( func == func2 ) {
		FAIL("2nd NSAddressOfSymbol return same function address as 1st");
		return 0;
	}
	
	
	NSModule mod3 = NSLinkModule(ofi, "test3.bundle", NSLINKMODULE_OPTION_NONE);
	if ( mod3 == NULL ) {
		FAIL("3rd NSLinkModule failed");
		return 0;
	}
	if ( mod3 == mod ) {
		FAIL("3rd NSLinkModule return same function address as 1st");
		return 0;
	}
	if ( mod3 == mod2 ) {
		FAIL("3rd NSLinkModule return same function address as 2nd");
		return 0;
	}

   NSSymbol sym3 = NSLookupSymbolInModule(mod3, "_setValue");
	if ( sym3 == NULL ) {
		FAIL("3rd NSLookupSymbolInModule failed");
		return 0;
	}
	setter func3 = NSAddressOfSymbol(sym3);
	(*func3)(3);
	//fprintf(stderr, "address of foo() = %p in bundle third load %p\n", func3, mod3);
	if ( func3 == func ) {
		FAIL("3rd NSAddressOfSymbol return same function address as 1st");
		return 0;
	}
	if ( func3 == func2 ) {
		FAIL("3rd NSAddressOfSymbol return same function address as 2nd");
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
	
	if ( !NSUnLinkModule(mod2, NSUNLINKMODULE_OPTION_NONE) ) {
		FAIL("2nd NSUnLinkModule failed");
		return 0;
	}
	
	// now link again after unlinking everything
	NSModule mod4 = NSLinkModule(ofi, "test4.bundle", NSLINKMODULE_OPTION_NONE);
	if ( mod4 == NULL ) {
		FAIL("4th NSLinkModule failed");
		return 0;
	}

	// check that this is really a new copy by verifying the getValue() returns zero
	NSSymbol sym4getter = NSLookupSymbolInModule(mod4, "_getValue");
	if ( sym4getter == NULL ) {
		FAIL("4th NSLookupSymbolInModule failed");
		return 0;
	}
	getter func4getter = NSAddressOfSymbol(sym4getter);
	if ( (*func4getter)() != 0 ) {
		FAIL("_getValue() on fourth link returned non-zero");
		return 0;
	}
	
	if ( !NSUnLinkModule(mod4, NSUNLINKMODULE_OPTION_NONE) ) {
		FAIL("4th NSUnLinkModule failed");
		return 0;
	}
	
	
	if ( !NSDestroyObjectFileImage(ofi) ) {
		FAIL("NSDestroyObjectFileImage failed");
		return 0;
	}
#endif

	PASS("bundle-multi-link");
	return 0;
}

