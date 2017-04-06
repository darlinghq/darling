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
#include <stdbool.h>  // fprintf(), NULL

bool floattest(double p1, float p2, double p3, float p4, double p5, float p6,
				double p7, float p8, double p9, float p10, double p11, float p12,
				double p13, float p14)
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
	if ( p10 != 10.0 )
		return false;
	if ( p11 != 11.0 )
		return false;
	if ( p12 != 12.0 )
		return false;
	if ( p13 != 13.0 )
		return false;
	if ( p14 != 14.0 )
		return false;
	return true;
}
				
bool inttest(long long p1, long long p2, long long p3, long long p4, long long p5)
{
	if ( p1 != 0x100000002)
		return false;
	if ( p2 != 0x300000004)
		return false;
	if ( p3 != 0x500000006)
		return false;
	if ( p4 != 0x700000008)
		return false;
	if ( p5 != 0x90000000A)
		return false;
	return true;
}


