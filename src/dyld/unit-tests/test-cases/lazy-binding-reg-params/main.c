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
#include <sys/sysctl.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


///
/// rdar://problem/4132378 support __attribute__((regparm()))
///
/// The stub binding helper for dyld needs to preserve registers
///

				
#include "foo.h"


static bool inttest()
{
	return dointtest(123,456,789,4444,55555);
}

static bool floattest()
{
#if __ppc__ || __ppc64__
	return dofloattest(1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0,10.0,11.0,12.0,13.0);
#elif __i386__ || __x86_64__ || __arm__
	return true;
#else
	#error unknown arch
#endif
}

#if __i386__ || __x86_64__ || __ppc__ || __ppc64__
static bool vectorSupport()
{
#if __ppc__
	uint32_t value;
	size_t size=sizeof(uint32_t);
	int err = sysctlbyname("hw.optional.altivec", &value, &size, NULL, 0);
	//fprintf(stderr, "sysctlbyname() = %d\n", err);
	return ( err == 0 );
#else
	return true;
#endif
}

static bool vectortest()
{
	vFloat p1 = { 1.1, 1.2, 1.3, 1.4 };
	vFloat p2 = { 2.1, 2.2, 2.3, 2.4 };
	vFloat p3 = { 3.1, 3.2, 3.3, 3.4 };
	vFloat p4 = { 4.1, 4.2, 4.3, 4.4 };
	vFloat p5 = { 5.1, 5.2, 5.3, 5.4 };
	return dovectortest(p1, p2, p3, p4, p5);
}
#endif

int main()
{
	if ( ! inttest() ) {
		FAIL("lazy-binding-reg-params int parameters");
		return EXIT_SUCCESS;
	}

	if ( ! floattest() ) {
		FAIL("lazy-binding-reg-params float parameters");
		return EXIT_SUCCESS;
	}

#if __i386__ || __x86_64__ || __ppc__ || __ppc64__
	if ( vectorSupport() && ! vectortest() ) {
		FAIL("lazy-binding-reg-params vector parameters");
		return EXIT_SUCCESS;
	}
#endif

	PASS("lazy-binding-reg-params");
	return EXIT_SUCCESS;
}
