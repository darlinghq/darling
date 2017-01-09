/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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

#include "test.h" // PASS(), FAIL()

#include "foo.h"


int* pdata5 = &data5;
int* pdata6 = &data6;


int main(int argc, const char* argv[])
{
	if ( argc > 1 ) {
		// this weak case where the even symbol names are missing at runtime
		func1();
		func3();
		data1 = data3;
		if ( (&func2 == NULL) && (&func4 == NULL) && (&data2 == NULL) && (&data4 == NULL) && (pdata6 == NULL) )
			PASS("weak_import");
		else
			FAIL("weak_import");
	}
	else {
		// this is the regular case where all symbols exist at runtime
		func1();
		func2();
		func3();
		func4();
		data1 = data2 + data3 + data4;
		PASS("weak_import");
	}
	return 0;
}

