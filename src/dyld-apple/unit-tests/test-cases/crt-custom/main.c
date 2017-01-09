/*
 * Copyright (c) 2007-2009 Apple Inc. All rights reserved.
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
#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <dlfcn.h>
#include <Availability.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

// the value of flag is altered by mystart
int flag = 0;


#if __LP64__
	// for 64-bit binaries initializers are always called before entry point
	#define ENTRY_BEFORE_INIT 0
#else
	#if __MAC_OS_X_VERSION_MIN_REQUIRED
		// for pre 10.5, 32-bit binaries, entry point is called which then calls initializers
		#if __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_5
			#define ENTRY_BEFORE_INIT 1
		#else
			#define ENTRY_BEFORE_INIT 0
		#endif
	#else
		// for iPhoneOS, initializers are always called before entry point
		#define ENTRY_BEFORE_INIT 0
	#endif
#endif


void __attribute__((constructor)) myinit()
{
#if ENTRY_BEFORE_INIT
	if ( flag != 2 ) {
		FAIL("crt-custom entry point not called before initializer");
		exit(0);
	}
#endif
	flag = 1;
}


int main() 
{ 
#if ENTRY_BEFORE_INIT
	if ( flag != 1 ) {
		FAIL("crt-custom initializer not called");
		exit(0);
	}
#else
	if ( flag != 2 ) {
		FAIL("crt-custom entry not called");
		exit(0);
	}
#endif

	PASS("crt-custom");

	return 0; 
}
