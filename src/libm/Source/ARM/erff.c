
/*
 *	erff.c
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007, Apple Inc. All Rights Reserved.
 */
 
#include <math.h>
#include <stdint.h>
#include "erf.h"

float erff( float x )
{
	union{ float d; uint32_t u;} u = { x };

	uint32_t	sign = u.u & 0x80000000U;

	//Inf, NaN, 0
	if( (u.u & 0x7fffffffU) - 1U >= 0x7f800000U - 1U )
	{
		if( 0.0f == x )
			return x;
	
		if( x != x )
			return x + x;
			
		u.u = 0x3f800000U | sign;
		return u.d;
	}
	
	u.d = (float) ErrFunApprox( (double) x, 1.0, 0 );
	u.u ^= sign;
	return u.d;
}
