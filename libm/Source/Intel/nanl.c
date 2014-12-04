/*	This module implements the C standard math function nanl.

	$Revision: 1.4 $, $Date: 2006/02/01 18:36:35 $
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

		long double nanl(const char *tagp);

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

	The result is intended to match GCC's __builtin_nanl.

	A previous implementation did not set the "quiet" bit if the hexadecimal
	numeral did not indicate it and did set the significand to one if it were
	zero.  The former does not conform to the C standard; a quiet NaN must be
	returned.  The latter is unneeded.  It was needed only to avoid returning
	infinity (all significand bits are zero) instead of a NaN (significand is
	not zero).

	Additional information is in nan.h.
*/


#include "nan.h"


/*	Here is nanl, as defined in the C standard and above.

	Note that the declaration of nanl in math.h includes some GCC mechanisms to
	control the name of nanl in the object file, with the result that what we
	call nanl here appears as _nanl$LDBL128 on PowerPC compiled with
	-mlong-double-128 (the default on PowerPC), or _nanl otherwise.
*/
long double nanl(const char *tagp)
{
	/*	Parse tagp, initialize result, and move our significand into it.

		Setting .ld to zero initializes the entire long double.  This
		accomplishes the result of setting the second double to zero, if
		present, without requiring conditional code.  (It is not necessary to
		set the second double to zero; it may have any value in a NaN.)

		We could eliminate the conditional code for .s.integer by merging it
		with the quiet and/or exponent bits in the definition of LongDouble
		above, but that might be more confusing than the conditional code.
	*/
	LongDouble result =
		{
			.ld				=  0,
			.s.sign			=  0,
			.s.exponent		= ~0,
			#if defined( __i386__ ) || defined( __x86_64__ )
				.s.integer	=  1,	// Set integer bit on IA-32.
			#endif
			.s.quiet		=  1,
			.s.significand	= ConstructSignificand(tagp)
		};

	return result.ld;
}
