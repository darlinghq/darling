
/*
 *	erf.c
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007, Apple Inc. All Rights Reserved.
 */
 
#include <math.h>
#include <stdint.h>
#include "erf.h"

double erf( double x )
{
	union{ double d; uint64_t u;} u = { x };

	uint64_t	sign = u.u & 0x8000000000000000ULL;

	//Inf, NaN, 0
	if( (u.u & 0x7fffffffffffffffULL) - 1ULL >= 0x7ff0000000000000ULL - 1ULL )
	{
		if( 0.0 == x )
			return x;
	
		if( x != x )
			return x + x;
			
		u.u = 0x3ff0000000000000ULL | sign;
		return u.d;
	}
	
	u.d = ErrFunApprox( x, 1.0, 0 );
	u.u ^= sign;
	return u.d;
}
