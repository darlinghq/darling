
/*
 *	lrint.c
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007, Apple Inc.  All Rights Reserved.
 *
 *	Implementation of C99 lrint.
 *
 *		Note that this function contains a bug that will cause values in the  
 *		ranges: {LONG_MIN-1 < x < LONG_MIN} and { LONG_MAX-1 < x < LONG_MAX }  to 
 *		possibly return both inexact and invalid flags, depending on current 
 *		rounding mode.  This problem only occurs for 32-bit longs. Under LP64, 
 *		there are no double precision floating point values in the indicated ranges.
 *
 *		We couldn't think of a way to get this right in C without extensive use
 *		of fenv functionality, which would slow down the function by many-fold.
 *		A processor-specific version can use the hardware rounding mode sensitive 
 *		float to int convert instruction (e.g. fctiw or cvtss2si) here to get the 
 *		right answer. Please see the Intel lrint in the regular Libm for sample code.
 */
 
#include <math.h>

#if defined(__SOFTFP__)

long lrint( double x )
{
	double rounded = round( x );
	long result = (long)rounded;
	
	if (__builtin_fabs(result - x) != 0.5)
		return result;
	else {
		// Exact halfway case
		if (result & 1L) {
			// If the trailing bit is set, we rounded the wrong way
			long step = (result >> 30) | 1L;// x < 0 ? -1 : 1
			return result - step;			// x < 0 ? result + 1 : result - 1
		}
		else {
			return result;
		}
	}
}

long lround( double x )
{
	return (long)round(x);
}

#else
						
#include "d2i.h"

long lrint( double x )
{
	return lrint_private( x );
}

#endif
