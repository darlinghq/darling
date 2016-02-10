
/*
 *	erfc.c
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007, Apple Inc. All Rights Reserved.
 */
 
#include <math.h>
#include <stdint.h>
#include "erf.h"

double erfc( double x )
{
	union{ double d; uint64_t u;} u = { x };


	//Inf, NaN, 0
	if( (u.u & 0x7fffffffffffffffULL) - 1ULL >= 0x7ff0000000000000ULL - 1ULL )
	{
		if( 0.0 == x )
			return 1.0;
	
		if( x != x )
			return x + x;
			
		u.u = 0x3ff0000000000000ULL | (u.u & 0x8000000000000000ULL);
		return 1.0 - u.d;
	}
	
	double result = ErrFunApprox( x, 0.0, 1 );
	
	if( x < 0.0 )
		return 2.0 - result;

	return result;
}
