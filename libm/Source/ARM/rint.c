
/*
 *	rint.c
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007, Apple Inc. All rights Reserved. 
 *
 *	C99 implementation of rint.
 */
 
#include <math.h>
#include <stdint.h>

double rint( double x )
{
	union{ double d; uint64_t u;}u = {x};
	uint64_t absux = u.u & 0x7fffffffffffffffULL;
	
	//special case code for large int, Inf, NaN, 0
	if( absux - 1LL >= 0x4330000000000000ULL - 1LL )
		return x;

	u.u = (u.u & 0x8000000000000000ULL) | 0x4330000000000000ULL;	//copysign( 0x1.0p23f, x )

	x += u.d;
	x -= u.d;

	return x;
}
