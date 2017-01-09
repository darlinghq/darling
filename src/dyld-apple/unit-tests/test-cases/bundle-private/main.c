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
#include <stdbool.h>
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
	
	NSModule modPriv = NSLinkModule(ofi, "test.bundle-private", NSLINKMODULE_OPTION_PRIVATE);
	if ( modPriv == NULL ) {
		FAIL("NSLinkModule failed");
		return 1;
	}
	
	NSSymbol symPriv = NSLookupSymbolInModule(modPriv, "_findme");
	if ( symPriv == NULL ) {
		FAIL("NSLookupSymbolInModule failed");
		return 1;
	}

	if ( NSIsSymbolNameDefined("_findme") ) {
		FAIL("NSIsSymbolNameDefined (incorrectly) found symbol in private bundle");
		return 1;
	}

	NSModule modPublic = NSLinkModule(ofi, "test.bundle-public", NSLINKMODULE_OPTION_NONE);
	if ( modPublic == NULL ) {
		FAIL("NSLinkModule failed");
		return 1;
	}
	
	NSSymbol symPublic = NSLookupSymbolInModule(modPublic, "_findme");
	if ( symPublic == NULL ) {
		FAIL("NSLookupSymbolInModule failed");
		return 1;
	}

	if ( !NSIsSymbolNameDefined("_findme") ) {
		FAIL("NSIsSymbolNameDefined did not found symbol in public bundle");
		return 1;
	}

	if ( !NSUnLinkModule(modPriv, NSUNLINKMODULE_OPTION_NONE) ) {
		FAIL("NSUnLinkModule failed");
		return 1;
	}

	if ( !NSUnLinkModule(modPublic, NSUNLINKMODULE_OPTION_NONE) ) {
		FAIL("NSUnLinkModule failed");
		return 1;
	}

	if ( NSIsSymbolNameDefined("_findme") ) {
		FAIL("NSIsSymbolNameDefined found unlinked symbol in public bundle");
		return 1;
	}

	if ( !NSDestroyObjectFileImage(ofi) ) {
		FAIL("NSDestroyObjectFileImage failed");
		return 1;
	}
#endif
	PASS("bundle-private");
	return 0;
}