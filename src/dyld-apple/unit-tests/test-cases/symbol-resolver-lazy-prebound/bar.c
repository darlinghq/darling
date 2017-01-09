/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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

#include <stdlib.h>
#include <stdbool.h>
#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

static bool hasBeenInited = false;

__attribute__((constructor))
void myInit() 
{
	hasBeenInited = true;
}


int barGood()
{
	return 1;
}

int barBad()
{
	return 0;
}


// This bar is a "resolver" function that return the actual address of "bar"
void* bar()
{
	__asm__(".symbol_resolver _bar");  // magic until we have compiler support
	// <rdar://problem/12629331> Resolver function run before initializers
	if ( hasBeenInited )
		return &barGood;
	else
		return &barBad;
}

