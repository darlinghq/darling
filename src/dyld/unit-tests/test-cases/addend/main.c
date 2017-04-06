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
#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <dlfcn.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


extern const char a;
extern const char b;
extern const char c;
extern const char d;
extern const char e;


const char* pc   = &c;
const char* pd_2 = &d - 2;
const char* pb2  = &b + 2;

const char* pd_1234567890 = &d - 1234567890;
const char* pd1234567890  = &d + 1234567890;

#if __LP64__
const char* pd_12345678901234 = &d - 12345678901234;
const char* pd12345678901234  = &d + 12345678901234;
#endif

int main()
{
	if (*pc != 12 ) {
		FAIL("addend: *pc != 12");
		return 0;
	}
	
	if (*pd_2 != 11 ) {
		FAIL("addend: *pd_2 != 11");
		return 0;
	}
	
	if (*pb2 != 13 ) {
		FAIL("addend: *pb2 != 13");
		return 0;
	}
	
	if (pd_1234567890[1234567890] != 13 ) {
		FAIL("addend: pd_1234567890[1234567890] != 13");
		return 0;
	}
	
	if (pd1234567890[-1234567890] != 13 ) {
		FAIL("addend: pd1234567890[-1234567890] != 13");
		return 0;
	}
	
#if __LP64__
	if (pd_12345678901234[12345678901234] != 13 ) {
		FAIL("addend: pd_12345678901234[12345678901234] != 13");
		return 0;
	}
	
	if (pd12345678901234[-12345678901234] != 13 ) {
		FAIL("addend: pd12345678901234[-12345678901234] != 13");
		return 0;
	}
#endif	
	
	PASS("addend");
	return 0;
}
