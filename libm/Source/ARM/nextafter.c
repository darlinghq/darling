/*
 *  nextafter.c
 *  cLibm
 *
 *  Created by Ian Ollmann on 6/14/07.
 *  Copyright 2007 Apple Inc. All rights reserved.
 *
 */
 
#include <math.h>
#include <stdint.h>

#ifdef ARMLIBM_SET_FLAGS
#include <fenv.h>
#include "required_arithmetic.h"
#pragma STDC FENV_ACCESS ON

double nextafter( double x, double y )
{
	union{ double d; uint64_t u;} ux = { x };
	uint64_t	step = 1;
	
	if( y != y || x != x)
		return x + y;

	if( y <= x ) // a subtraction here would risk Inf-Inf
	{
		if( y == x )
			return y;	//make sure sign of 0 is correct

		step = -1LL;
	}

	//correct for the sign
	int64_t signMask = (int64_t) ux.u >> 63;
	step = (step ^ signMask) - signMask;

	uint64_t absux = ux.u & 0x7fffffffffffffffULL;

	if(  absux - 0x0010000000000001ULL >= 0x7fefffffffffffffULL - 0x0010000000000001ULL )
	{ //0, Inf, max finite, min normal, denorm		
		//Nan is handled above and won't occur here
		//Inf can just fall through. We are only here if y!=x.

		if( absux == 0ULL )				// zero
		{
			ux.d = y;
			required_multiply_double( 0x1.0p-1000, 0x1.0p-1000 );
			ux.u = (ux.u & 0x8000000000000000ULL) + 1U;
			return ux.d;
		}
		else if( absux == 0x7fefffffffffffffULL ) // max finite
		{
			ux.u += step;			

			//if infinity is the result, set some flags
			if( 0 == (ux.u & 2ULL) )
			{
				required_add_double( x, 1.0 );		//set inexact
				required_multiply_double( x, x );	//set overflow
			}
			
			return ux.d;
		}

		ux.u += step;			
		if( 0ULL == (ux.u & 0x7ff0000000000000ULL))
			required_multiply_double( 0x1.0p-1000, 0x1.0p-1000 );
			
		return ux.d;
	}

	ux.u += step;
	
	return ux.d;
}

#else

double nextafter( double x, double y )
{
	union{ double d; uint64_t u;} ux = { x };
	uint64_t	step = 1;
	
	if( y != y || x != x)
		return x + y;
	
	if( y <= x ) // a subtraction here would risk Inf-Inf
	{
		if( y == x )
			return y;	//make sure sign of 0 is correct
		
		step = -1LL;
	}
	
	//correct for the sign
	int64_t signMask = (int64_t) ux.u >> 63;
	step = (step ^ signMask) - signMask;
	
	uint64_t absux = ux.u & 0x7fffffffffffffffULL;
		
	if( absux == 0ULL )				// zero
	{
		ux.d = y;
		ux.u = (ux.u & 0x8000000000000000ULL) + 1U;
		return ux.d;
	}
	
	ux.u += step;
	return ux.d;
}

#endif // ARMLIBM_SET_FLAGS
