/*
 * Copyright (c) 2005 Apple Computer p1, Inc. All rights reserved.
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
#include <string.h>
#include "foo.h"

			
#if __i386__
__attribute__((regparm(3)))
#endif
bool dointtest(int p1, int p2, int p3, int p4, int p5) 
{
	if ( p1 != 123)
		return false;
	if ( p2 != 456)
		return false;
	if ( p3 != 789)
		return false;
	if ( p4 != 4444)
		return false;
	if ( p5 != 55555)
		return false;
	return true;
}

#if __ppc__ || __ppc64__
bool dofloattest(double p1, double p2, double p3, double p4, double p5, double p6, double p7,
				 double p8, double p9, double p10, double p11, double p12, double p13)
{
	if ( p1 != 1.0 )
		return false;
	if ( p2 != 2.0 )
		return false;
	if ( p3 != 3.0 )
		return false;
	if ( p4 != 4.0 )
		return false;
	if ( p5 != 5.0 )
		return false;
	if ( p6 != 6.0 )
		return false;
	if ( p7 != 7.0 )
		return false;
	if ( p8 != 8.0 )
		return false;
	if ( p9 != 9.0 )
		return false;
	if ( p10 != 10.0)
		return false;
	if ( p11 != 11.0)
		return false;
	if ( p12 != 12.0)
		return false;
	if ( p13 != 13.0)
		return false;
	return true;
}
#endif



#if __i386__ || __x86_64__ || __ppc__ || __ppc64__

static bool comparevFloat(vFloat p1, vFloat p2)
{
	return (memcmp(&p1, &p2, 16) == 0);
}

bool dovectortest(vFloat p1, vFloat p2, vFloat p3, vFloat p4, vFloat p5)
{
	vFloat r1 = { 1.1, 1.2, 1.3, 1.4 };
	vFloat r2 = { 2.1, 2.2, 2.3, 2.4 };
	vFloat r3 = { 3.1, 3.2, 3.3, 3.4 };
	vFloat r4 = { 4.1, 4.2, 4.3, 4.4 };
	vFloat r5 = { 5.1, 5.2, 5.3, 5.4 };

	if ( !comparevFloat(p1, r1) )
		return false;
	if ( !comparevFloat(p2, r2) )
		return false;
	if ( !comparevFloat(p3, r3) )
		return false;
	if ( !comparevFloat(p4, r4) )
		return false;
	if ( !comparevFloat(p5, r5) )
		return false;
	return true;
}

#endif




