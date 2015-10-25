/*
 *  nextafterf.c
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

float nextafterf( float x, float y )
{
	union{ float f; uint32_t u;} ux = { x };
	uint32_t	step = 1;
	
	if( y != y || x != x)
		return x + y;

	if( y <= x ) // a subtraction here would risk Inf-Inf
	{
		if( y == x )
			return y;	//make sure sign of 0 is correct

		step = -1;
	}

	//correct for the sign
	int32_t signMask = (int32_t) ux.u >> 31;
	step = (step ^ signMask) - signMask;

	uint32_t absux = ux.u & 0x7fffffffU;

	if(  absux - 0x00800001U >= 0x7f7fffffU - 0x00800001U )
	{ //0, Inf, max finite, min normal, denorm		
		//Nan is handled above and won't occur here
		//Inf can just fall through. We are only here if y!=x.

		if( absux == 0 )				// zero
		{
			ux.f = y;
			required_multiply_float( 0x1.0p-120f, 0x1.0p-120f );
			ux.u = (ux.u & 0x80000000U) + 1U;
			return ux.f;
		}
		else if( absux == 0x7f7fffffU ) // max finite
		{
			ux.u += step;			

			//if infinity is the result, set some flags
			if( 0 == (ux.u & 2) )
			{
				required_add_float( x, 1.0f );		//set inexact
				required_multiply_float( x, x );	//set overflow
			}
			
			return ux.f;
		}

		ux.u += step;			
		if( 0 == (ux.u & 0x7f800000))
			required_multiply_float( 0x1.0p-120f, 0x1.0p-120f );
			
		return ux.f;
	}

	ux.u += step;
	
	return ux.f;
}

#else

float nextafterf( float x, float y )
{
	union{ float f; uint32_t u;} ux = { x };
	uint32_t	step = 1;
	
	if( y != y || x != x)
		return x + y;
	
	if( y <= x ) // a subtraction here would risk Inf-Inf
	{
		if( y == x )
			return y;	//make sure sign of 0 is correct
		
		step = -1;
	}
	
	//correct for the sign
	int32_t signMask = (int32_t) ux.u >> 31;
	step = (step ^ signMask) - signMask;
	
	uint32_t absux = ux.u & 0x7fffffffU;
	
	if( absux == 0 )				// zero
	{
		ux.f = y;
		ux.u = (ux.u & 0x80000000U) + 1U;
		return ux.f;
	}
	
	ux.u += step;
	return ux.f;
}

#endif // ARMLIBM_SET_FLAGS
