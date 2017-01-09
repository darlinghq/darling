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
#include <stdbool.h>  
#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <dlfcn.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

extern bool floattest(double p1, float p2, double p3, float p4, double p5, float p6,
					double p7, float p8, double p9, float p10, double p11, float p12,
					double p13, float p14);
				
extern bool inttest(long long p1, long long p2, long long p3, long long p4, long long p5);



int main()
{
	if ( ! floattest(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0) ) {
		FAIL("lazy-pointer-binding float parameters");
		return EXIT_SUCCESS;
	}
	
	if ( ! inttest(0x100000002, 0x300000004, 0x500000006, 0x700000008, 0x90000000A) ) {
		FAIL("lazy-pointer-binding int parameters");
		return EXIT_SUCCESS;
	}
	
	PASS("lazy-pointer-binding");
	return EXIT_SUCCESS;
}
