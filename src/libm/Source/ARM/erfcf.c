
/*
 *	erfcf.c
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007, Apple Inc. All Rights Reserved.
 */
 
#include <math.h>
#include <stdint.h>
#include "erf.h"

float erfcf( float x )
{
	union{ float d; uint32_t u;} u = { x };


	//Inf, NaN, 0
	if( (u.u & 0x7fffffffU) - 1U >= 0x7f800000U - 1U )
	{
		if( 0.0f == x )
			return 1.0f;
	
		if( x != x )
			return x + x;
			
		u.u = 0x3f800000U | (u.u & 0x80000000U);
		return 1.0f - u.d;
	}
	
	float result = (float) ErrFunApprox( (double) x, 0.0, 1 );
	
	if( x < 0.0f )
		return 2.0f - result;

	return result;
}
