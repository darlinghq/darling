
/*
 *	llrint.c
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007, Apple Inc.  All Rights Reserved.
 *
 *	C99 implementation of llrint.
 */
 
#include <math.h>
#include <stdint.h>
#include <limits.h>

#warning *** untested -- we don't have tests for this

long long llrint( double x )
{
	union{ double d;		uint64_t u;} u = {x};
	
	uint64_t absx = u.u & 0x7fffffffffffffffULL;
	
	// handle x is zero, large, or NaN simply convert to long long and return
	if( absx >= 0x4330000000000000ULL )
	{
		long long result = (long long) x;	//set invalid if necessary

		//Deal with overflow cases
		if( x < (double) LONG_LONG_MIN )
			return LONG_LONG_MIN;

		// Note: float representation of LONG_LONG_MAX likely inexact, 
		//		  which is why we do >= here
		if( x >= -((double) LONG_LONG_MIN) )
			return LONG_LONG_MAX;
	
		return result;
	}

	// copysign( 0x1.0p52, x )
	u.u = (u.u & 0x8000000000000000ULL) | 0x4330000000000000ULL;
	
	//round according to current rounding mode
	x += u.d;
	x -= u.d;

	return (long long) x;
}
