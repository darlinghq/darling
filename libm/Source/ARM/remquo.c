/*
 *  remquo.c
 *  cLibm
 *
 *     Written by Jon Okada, started on December 7th, 1992.                      
 *     Modified by Paul Finlayson (PAF) for MathLib v2.                          
 *     Modified by A. Sazegari (ali) for MathLib v3.                             
 *     Modified and ported by Robert A. Murley (ram) for Mac OS X.               
 *	   Modified for cLibm, fixed a few edge cases, rewrote local_ funcs by Ian Ollmann.
 *
 *  Copyright 2007 Apple Inc. All rights reserved.
 *
 */
 
 #include <math.h>
 #include <float.h>
 #include <stdint.h>
 
static inline int local_ilogb( double x ) __attribute__ ((always_inline));
static inline int local_ilogb( double x )
{
	union{ double d; uint64_t u;}u = {x};
	
	u.u &= 0x7fffffffffffffffULL;
	int32_t exp = (int32_t) (u.u >> 52);
	
	if( __builtin_expect( (uint32_t) exp - 1U >= 2046, 0 ) )
	{ // +-denorm, the other possibilities: +0 +-inf, NaN are screend out by caller
		u.u |= 0x3ff0000000000000ULL;
		u.d -= 1.0;
		exp = (int32_t) (u.u >> 52);
		
		return exp - (1023+1022);
	}

	return exp - 1023;
}

static inline  double local_scalbn( double x, int n );
static inline  double local_scalbn( double x, int n )
{
	union{ uint64_t u; double d;} u;
	
	uint32_t absn = n >> (8*sizeof(n)-1);
	absn = (n ^ absn) - absn;
	
	if( absn > 1022 )
	{
		// step = copysign( 1022, n )
		int signMask = n >> (8 * sizeof( int ) - 1);
		int step = (1022 ^ signMask) - signMask;		
	
		u.u = ( (int64_t) step + 1023ULL) << 52;
		
		if( n < 0 )
		{
			do
			{
				x *= u.d;
				n -= step;
			}while( n < -1022 );
		}
		else
		{
			do
			{
				x *= u.d;
				n -= step;
			}while( n > 1022 );
		}
	}
  
	//create 2**n in double
	u.u = ( (int64_t) n + 1023ULL) << 52;
 
	//scale by appropriate power of 2
	x *= u.d;
 
	//return result
	return x;
}

#ifdef ARMLIBM_SET_FLAGS
#include "required_arithmetic.h"

double remquo( double x, double y, int *quo )
{
 
	*quo = 0;					// Init quotient result

	// deal with NaN
	if( x != x || y != y )
		return x + y;

	//x = Inf or y = 0, return Invalid per IEEE-754
	double fabsx = __builtin_fabs(x);	
	if( fabsx == __builtin_inf() || 0.0 == y )
	{
		return required_add_double( __builtin_inf(), -__builtin_inf() );	//set invalid
	}

	//handle trivial case
	double fabsy = __builtin_fabs(y);
	if( fabsy == __builtin_inf() || 0.0 == x )
	{
		required_add_double( fabsx, 0.0 );			//signal underflow (that is, no flag set, but exception occurs if unmasked) if x is denorm
		return x;
	}
 
	//we have to work
	int32_t iquo = 0;
	int32_t iscx = local_ilogb(fabsx);
	int32_t iscy = local_ilogb(fabsy);
	int32_t idiff = iscx - iscy;
	double x1, y1, z;
	x1 = fabsx;
	y1 = fabsy;
	if( idiff >= 0 )
	{
		int32_t i;
		
		if( idiff )
		{
			y1 = local_scalbn( y1, -iscy );
			x1 = local_scalbn( x1, -iscx );
			for( i = idiff; i != 0; i-- )
			{
				if( x1 >= y1 )
				{
					x1 -= y1;
					iquo += 1;
				}
				iquo += iquo;
				x1 += x1;
			}
			x1 = local_scalbn( x1, iscy );
		}
		if( x1 >= fabsy )
		{
			x1 -= fabsy;
			iquo += 1;
		}
	}
	
	if( x1 < 0x1.0p1022 )
	{
		z = x1 + x1;
		if( (z > fabsy) || ((z == fabsy) & ((iquo & 1) != 0 )))
		{
			x1 -= fabsy;
			iquo += 1;
		}
	}
	else
	{
		// x1 is only this large if y is even bigger, so we can safely divide y by 2
		double halfy = 0.5*fabsy;
		if( (x1 > halfy) || ((x1 == halfy) & ((iquo & 1) != 0 )))
		{
			x1 -= fabsy;
			iquo += 1;
		}
	}
 
 
	if( x < 0 )
		x1 = -x1;
	iquo &= 0x7f;
	
	union{ double d[2]; uint64_t u[2];} u = { { x, y } };
	int32_t sign = (int32_t) ( (u.u[0] ^ u.u[1]) >> 32 );
	sign = sign >> 31;
	*quo = (iquo ^ sign) - sign;
	
	return x1;
}

#else

double remquo( double x, double y, int *quo )
{
	
	*quo = 0;					// Init quotient result
	
	// deal with NaN
	if( x != x || y != y )
		return x + y;
	
	//x = Inf or y = 0, return Invalid per IEEE-754
	double fabsx = __builtin_fabs(x);	
	if( fabsx == __builtin_inf() || 0.0 == y )
	{
		return __builtin_nan("");
	}
	
	//handle trivial case
	double fabsy = __builtin_fabs(y);
	if( fabsy == __builtin_inf() || 0.0 == x )
	{
		return x;
	}
	
	//we have to work
	int32_t iquo = 0;
	int32_t iscx = local_ilogb(fabsx);
	int32_t iscy = local_ilogb(fabsy);
	int32_t idiff = iscx - iscy;
	double x1, y1, z;
	x1 = fabsx;
	y1 = fabsy;
	if( idiff >= 0 )
	{
		int32_t i;
		
		if( idiff )
		{
			y1 = local_scalbn( y1, -iscy );
			x1 = local_scalbn( x1, -iscx );
			for( i = idiff; i != 0; i-- )
			{
				if( x1 >= y1 )
				{
					x1 -= y1;
					iquo += 1;
				}
				iquo += iquo;
				x1 += x1;
			}
			x1 = local_scalbn( x1, iscy );
		}
		if( x1 >= fabsy )
		{
			x1 -= fabsy;
			iquo += 1;
		}
	}
	
	if( x1 < 0x1.0p1022 )
	{
		z = x1 + x1;
		if( (z > fabsy) || ((z == fabsy) & ((iquo & 1) != 0 )))
		{
			x1 -= fabsy;
			iquo += 1;
		}
	}
	else
	{
		// x1 is only this large if y is even bigger, so we can safely divide y by 2
		double halfy = 0.5*fabsy;
		if( (x1 > halfy) || ((x1 == halfy) & ((iquo & 1) != 0 )))
		{
			x1 -= fabsy;
			iquo += 1;
		}
	}
	
	if( x < 0 )
		x1 = -x1;
	iquo &= 0x7f;
	
	union{ double d[2]; uint64_t u[2];} u = { { x, y } };
	int32_t sign = (int32_t) ( (u.u[0] ^ u.u[1]) >> 32 );
	sign = sign >> 31;
	*quo = (iquo ^ sign) - sign;
	
	return x1;
}


#endif // ARMLIBM_SET_FLAGS
