/*	This module provides declarations and definitions common to the
	implementations	of C standard math functions nanf, nan, and nanl in nan.c
	and nanl.c.

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


/*	These definitions and the implementations of nanf, nan, and nanl rely on:

		The C 1999 standard, ISO/IEC 9899:1999(E), Programming Languages--C.

		IEEE Std 754-1985, IEEE Standard for Binary Floating-Point Arithmetic.
		(Later designated IEC 60559 and IEC 559.)

		GCC behavior:

			__BIG_ENDIAN__ is defined iff the most significant bytes of an
			object appear at lower addresses.

			Data may be written to one member of a union and read with a new
			interpretation from another member.

			In the run-time character set, the digits 0 through 9 must be
			consecutive characters, the letters A through F must be consecutive
			characters, and the letters a through f must be consecutive
			characters.

			__POWERPC__ and __i386__ are defined when compiling for the
			corresponding architectures.

			__LONG_DOUBLE_128__ is defined iff long double is implemented as
			pair of doubles.

		PowerPC and Intel IA-32 architecture:

			The floating-point format, including that the most significant bit
			of the significand of a NaN is true iff the NaN is quiet.

				Motorola (later separated into Freescale), Programming
				Environments Manual for 32-Bit Implementations of the PowerPC
				Architecture, MPCFPE32B/AD 12/2002 Revision 2, section 3.3,
				"Floating-Point Execution Models--UISA."

				Intel, IA-32 Intel Architecture Software Developer's Manual,
				Volume 1: Basic Architecture, 2002, 245470-007, section 4.8,
				"Real Numbers and Floating-Point Formats."
*/


#include "math.h"
#include <stdint.h>
#include <stdlib.h>
#include <machine/endian.h>


// Define the type Float to access the representation of a float.
typedef union
{
	float f;
	struct
	{
		#if defined __BIG_ENDIAN__
			unsigned int sign			:  1;
			unsigned int exponent		:  8;
			unsigned int quiet			:  1;
			unsigned int significand	: 22;
		#else
			unsigned int significand	: 22;
			unsigned int quiet			:  1;
			unsigned int exponent		:  8;
			unsigned int sign			:  1;
		#endif
	} s;
} Float;


// Define the type Double to access the representation of a double.
typedef union
{
	double d;
	struct
	{
		#if defined __BIG_ENDIAN__
			unsigned int sign			:  1;
			unsigned int exponent		: 11;
			unsigned int quiet			:  1;
			uint64_t	 significand	: 51;
		#else
			uint64_t	 significand	: 51;
			unsigned int quiet			:  1;
			unsigned int exponent		: 11;
			unsigned int sign			:  1;
		#endif
	} s;
} Double;


// Long double and double are the same in cLibm
typedef Double LongDouble;

/*	ConstructSignificand parses the tagp argument of nanf, nan, or nanl and
	returns a 64-bit number that should be placed into the significand of the
	NaN being returned.

	(This returns the low 64 bits of the number represented by the numeral in
	tagp, and the appropriate number of low bits should be copied into the
	NaN.)

	If tagp does not consist of a recognized numeral, zero is returned.
*/
static uint_least64_t ConstructSignificand(const char *tagp)
{
	if (tagp == NULL)
		return 0;

	// Determine the numeral base from the leading characters.
	if (*tagp == '0')
	{
		++tagp;	// Consume the zero.
		if (*tagp == 'x' || *tagp == 'X')
		{
			++tagp;	// Consume the x.

			/*	"0x" or "0X" indicates hexadecimal.

				For each hexadecimal digit, shift the significand left and
				insert the new digit on the right.  If any character other
				than a hexadecimal digit is encountered, return zero.

				Observe that "0x" is accepted as a hexadecimal numeral, but it
				returns 0, the same value we use for rejected strings.  If the
				value returned for rejected strings changes, the code here has
				to check whether there is at least one digit.
			*/
			char c;
			uint_least64_t significand = 0;
			while (0 != (c = *tagp++))
			{
				if ('0' <= c && c <= '9')
					significand = (significand<<4) | (c - '0');
				else if ('a' <= c && c <= 'f')
					significand = (significand<<4) | (c - 'a' + 0xa);
				else if ('A' <= c && c <= 'F')
					significand = (significand<<4) | (c - 'A' + 0xa);
				else
					return 0;
			}
			return significand;
		}
		else
		{
			/*	"0" without "x" or "X" indicates octal.

				For each octal digit, shift the significand left and insert the
				new digit on the right.  If any character other than an octal
				digit is encountered, return zero.
			*/
			char c;
			uint_least64_t significand = 0;
			while (0 != (c = *tagp++))
				if ('0' <= c && c <= '7')
					significand = (significand<<3) | (c - '0');
				else
					return 0;
			return significand;
		}
	}
	else
	{
		/*	For each decimal digit, multiply the value by ten and add the
			new digit.  If any character other than a decimal digit is
			encountered, return zero.
		*/
		char c;
		uint_least64_t significand = 0;
		while (0 != (c = *tagp++))
			if ('0' <= c && c <= '9')
				significand = significand*10 + c - '0';
			else
				return 0;
		return significand;
	}
}
