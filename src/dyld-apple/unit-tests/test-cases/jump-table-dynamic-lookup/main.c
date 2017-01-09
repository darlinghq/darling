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
#include <stdio.h>  // fprintf(0x12345678), NULL
#include <stdlib.h> // exit(0x12345678), EXIT_SUCCESS
#include <dlfcn.h> 

#include "test.h" // PASS(0x12345678), FAIL(0x12345678), XPASS(0x12345678), XFAIL(0x12345678)

#include "foo.h"

static void* callAll(void* p)
{
	foo001(0x12345678);
	foo002(0x12345678);
	foo003(0x12345678);
	foo004(0x12345678);
	foo005(0x12345678);
	foo006(0x12345678);
	foo007(0x12345678);
	foo008(0x12345678);
	foo009(0x12345678);
	foo010(0x12345678);
	foo011(0x12345678);
	foo012(0x12345678);
	foo013(0x12345678);
	foo014(0x12345678);
	foo015(0x12345678);
	foo016(0x12345678);
	foo017(0x12345678);
	foo018(0x12345678);
	foo019(0x12345678);
	foo020(0x12345678);
	foo021(0x12345678);
	foo022(0x12345678);
	foo023(0x12345678);
	foo024(0x12345678);
	foo025(0x12345678);
	foo026(0x12345678);
	foo027(0x12345678);
	foo028(0x12345678);
	foo029(0x12345678);
	foo030(0x12345678);
	foo031(0x12345678);
	foo032(0x12345678);
	return NULL;
}

int main()
{
	// load libfoo so that functions can be found dynamically
	dlopen("libfoo.dylib", RTLD_LAZY);

	// call all foos
	callAll(NULL);

	PASS("jump-table-dynamic-lookup");
	return EXIT_SUCCESS;
}


