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
#include <stdlib.h> // EXIT_SUCCESS

#include "test.h"
#include "foo.h"

int main(int argc, const char *argv[])
{
	if (argc > 1) {
		// when arg is passed, libfoo is used and foo should be defined
		if ( &foo == NULL ) 
			FAIL("weak-symbol-flat foo not found");
		else if ( foo() != 10 )
			FAIL("weak-symbol-flat foo not found");
		else
			PASS("weak-symbol-flat when symbol found");
	}
	else {
		// when no args are passed, libfoo_missing is used and foo should be undefined
		if ( &foo != NULL ) 
			FAIL("weak-symbol-flat foo found");
		else
			PASS("weak-symbol-flat when symbol missing");
	}
	return EXIT_SUCCESS;
}
