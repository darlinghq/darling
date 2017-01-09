/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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
#include <stdbool.h>

#include "test.h"


#if LAZY_HANDLER
// If a linkage unit defines dyld_lazy_dylib_proxy()
// then when binding lazy symbols, if one 
// cannot be bound, it is bound to this function
// instead.
int dyld_lazy_dylib_proxy()
{
	return 7;
}
#endif


extern int foo();	// should exist
extern int bar();	// should not exist

int main()
{
	// sanity check that foo was found
	if ( foo() != 1 ) {
		FAIL("lazy-dylib-missing-symbol: foo not found");
		return 0;
	}
	
#if LAZY_HANDLER
	// sanity check that bar was not found
	// lazy loading return 7 when function cannot be found
	if ( bar() != 7 ) {
		FAIL("lazy-dylib-missing-symbol: dyld_lazy_dylib_proxy() not used for bar");
		return 0;
	}
#else
	// sanity check that bar was not found
	// lazy loading return 0 when function cannot be found
	if ( bar() != 0 ) {
		FAIL("lazy-dylib-missing-symbol: bar found");
		return 0;
	}
#endif
	PASS("lazy-dylib-missing-symbol");
	return 0;
}

