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
#include <stdbool.h>

#include "base.h"
#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

static bool wasProblem = false;

static const char*	coal1Where = NULL;
static int*			coal1Addr = NULL;
static int			checkInCountCoal1 = 0;

void baseVerifyCoal1(const char* where, int* addr)
{
	//fprintf(stderr, "baseVerifyCoal1(%s, %p)\n", where, addr);
	++checkInCountCoal1;
	if ( coal1Where == NULL ) {
		coal1Where = where;
		coal1Addr = addr;
	}
	else {
		if ( addr != coal1Addr ) {
			fprintf(stderr, "coal1 resolved to different locations.  %p %s and %p %s\n", 
				coal1Addr, coal1Where, addr, where);
			wasProblem = true;
		}	
	}
}


static const char*	coal2Where = NULL;
static int*			coal2Addr = NULL;
static int			checkInCountCoal2 = 0;

void baseVerifyCoal2(const char* where, int* addr)
{
	//fprintf(stderr, "baseVerifyCoal2(%s, %p)\n", where, addr);
	++checkInCountCoal2;
	if ( coal2Where == NULL ) {
		coal2Where = where;
		coal2Addr = addr;
	}
	else {
		if ( addr != coal2Addr ) {
			fprintf(stderr, "coal2 resolved to different locations.  %p %s and %p %s\n", 
				coal2Addr, coal2Where, addr, where);
			wasProblem = true;
		}	
	}
}



void baseCheck()
{
	if ( wasProblem || (checkInCountCoal1 != 3) || (checkInCountCoal2 != 2) )
		FAIL("weak-coal");
	else
		PASS("weak-coal");
}

