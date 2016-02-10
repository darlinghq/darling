/*	This module implements the C standard math functions nanf and nan.

	$Revision: 1.3 $, $Date: 2005/06/23 18:26:55 $
*/


/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */


/*	This module implements:

		float nanf(const char *tagp);
		double nan(const char *tagp);

	as defined in the C standard.  The specific NaN returned is
	implementation-defined, and we define it:

		The significand's "quiet" bit (its most significant bit) is set.

		If tagp points to a numeral, the remaining bits of the significand are
		set to the numeral's value (modulo the width available).  The numeral
		may be an octal numeral beginning with "0", a decimal numeral not
		beginning with "0", or a hexadecimal numeral beginning with "0x" or
		"0X".  Hexadecimal digits may be in uppercase or lowercase.  The string
		may not contain a sign or any characters other than the digits and a
		"0x" or "0X" prefix, and it may not contain any characters not forming
		part of a numeral, even as a trailing portion.  The value is intended
		to be the same as would be provided by strtoumax with zero for the
		base.

		Otherwise, the remaining bits are set to zero.  This may change in the
		future.

	The result is intended to match GCC's __builtin_nanf and __builtin_nan.

	Additional information is in nan.h.
*/


#include "nan.h"


// Here is nanf, as defined in the C standard and above.
float nanf(const char *tagp)
{
	// Parse tagp, initialize result, and move our significand into it.
	Float result =
		{
			.s.sign			=  0,
			.s.exponent		= ~0,
			.s.quiet		=  1,
			.s.significand	= ConstructSignificand(tagp)
		};

	return result.f;
}


// Here is nan, as defined in the C standard and above.
double nan(const char *tagp)
{
	// Parse tagp, initialize result, and move our significand into it.
	Double result =
		{
			.s.sign			=  0,
			.s.exponent		= ~0,
			.s.quiet		=  1,
			.s.significand	= ConstructSignificand(tagp)
		};

	return result.d;
}
