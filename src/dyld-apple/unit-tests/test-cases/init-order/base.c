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

static int state = 1;
bool badOrder = false;

void setState(int nextState)
{
	if ( nextState == state )
		++state;
	else
		badOrder = true;

}


void baseCheck()
{
	if ( badOrder ) {
		switch ( state ) {
			case 1:
				FAIL("init-order -init not run first");
				break;
			case 2:
				FAIL("init-order myInit1 not run second");
				break;
			case 3:
				FAIL("init-order myInit2 not run third");
				break;
			case 4:
				FAIL("init-order myInit3 not run fourth");
				break;
		}
	}
	else
		PASS("init-order");
}

