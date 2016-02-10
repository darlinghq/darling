/*
 *  fmod.c
 *  cLibm
 *
 *     Written by Jon Okada, started on December 7th, 1992.                      
 *     Modified by Paul Finlayson (PAF) for MathLib v2.                          
 *     Modified by A. Sazegari (ali) for MathLib v3.                             
 *     Modified and ported by Robert A. Murley (ram) for Mac OS X.               
 *	   Modified for cLibm, fixed a few edge cases, rewrote local_ funcs by Ian Ollmann.
 *     Modified for armLibm, removed code to set flags as this is a soft-float fallback only.
 *
 *  Copyright 2007 Apple Inc. All rights reserved.
 *
 */
 
#if defined(__SOFTFP__)

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

 
double fmod( double x, double y )
{
	union{ double d; uint64_t u;}ux = {x};
	union{ double d; uint64_t u;}uy = {y};
	
	uint64_t absx = ux.u & ~0x8000000000000000ULL;
	uint64_t absy = uy.u & ~0x8000000000000000ULL;

	if( absx-1ULL >= 0x7fefffffffffffffULL || absy-1ULL >= 0x7fefffffffffffffULL )
	{
		double fabsx = __builtin_fabs(x);
		double fabsy = __builtin_fabs(y);
		
		// deal with NaN
		if( x != x || y != y )
			return x + y;

		//x = Inf or y = 0, return Invalid per IEEE-754
		if( fabsx == __builtin_inf() || 0.0 == y )
		{
			return __builtin_nan("");
		}

		//handle trivial case
		if( fabsy == __builtin_inf() || 0.0 == x )
		{
			return x;
		}
	}
 
	if( absy >= absx )
	{
		if( absy == absx )
		{
			ux.u ^= absx;
			return ux.d;
		}
		
		return x;
	}
 
	int32_t expx = absx >> 52;
	int32_t expy = absy >> 52;
	int64_t sx = absx & 0x000fffffffffffff;
	int64_t sy = absy & 0x000fffffffffffff;

	if( 0 == expx )
	{
		uint32_t shift = __builtin_clzll( absx ) - (64-53);
		sx <<= shift;
		expx = 1 - shift;
	}

	if( 0 == expy )
	{
		uint32_t shift = __builtin_clzll( absy ) - (64-53);
		sy <<= shift;
		expy = 1 - shift;
	}
	sx |= 0x0010000000000000ULL;
	sy |= 0x0010000000000000ULL;


	int32_t idiff = expx - expy;
	int32_t shift = 0;
	int64_t mask;
	
	do
	{
		sx <<= shift;
		idiff += ~shift;
		sx -= sy;
		mask = sx >> 63;
		sx += sx;
		sx += sy & mask;
		shift = __builtin_clzll( sx ) - (64-53);
	}
	while( idiff >= shift && sx != 0LL );

	if( idiff < 0 )
	{
		sx += sy & mask;
		sx >>= 1;
		idiff = 0;
	}
	
	sx <<= idiff;
	
	if( 0 == sx )
	{
		ux.u &= 0x8000000000000000;
		return ux.d;
	}
	
	shift = __builtin_clzll( sx ) - (64-53);
	sx <<= shift;
	expy -= shift;
	sx &= 0x000fffffffffffffULL;
	sx |= ux.u & 0x8000000000000000ULL;
	if( expy > 0 )
	{
		ux.u = sx | ((int64_t) expy << 52);
		return ux.d;
	}
	
	expy += 1022;
	ux.u = sx | ((int64_t) expy << 52);
	return ux.d * 0x1.0p-1022;

}

#endif

