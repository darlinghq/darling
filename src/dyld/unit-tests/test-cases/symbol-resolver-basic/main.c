/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
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

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

extern int foo();
extern int fooPlusOne();


int main()
{
	if ( getenv("TEN") != NULL ) {
		if ( foo() != 10 )
			FAIL("symbol-resolver-basic: foo() != 10");
		else if ( fooPlusOne() != 11 )
			FAIL("symbol-resolver-basic: fooPlusOne() != 11");
		else
			PASS("symbol-resolver-basic");
	}
	else {
		if ( foo() != 0 )
			FAIL("symbol-resolver-basic: foo() != 0");
		else if ( fooPlusOne() != 1 )
			FAIL("symbol-resolver-basic: fooPlusOne() != 1");
		else
			PASS("symbol-resolver-basic");
	}
  
	return EXIT_SUCCESS;
}
