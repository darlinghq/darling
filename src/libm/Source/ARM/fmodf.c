/*
 *  fmodf.c
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

#ifdef ARMLIBM_SET_FLAGS
 #include "required_arithmetic.h"
#endif

static inline int local_ilogbf( float x ) __attribute__ ((always_inline));
static inline int local_ilogbf( float x )
{
	union{ float f; uint32_t u;}u = {x};
	
	u.u &= 0x7fffffff;
	int32_t exp = u.u >> 23;
	
	if( __builtin_expect( (uint32_t) exp - 1U >= 254, 0 ) )
	{ // +-denorm, other possibilities: +-0, +-inf, NaN are screened out by caller
	
		u.u |= 0x3f800000U;
		u.f -= 1.0f;
		exp = u.u >> 23;
		
		return exp - (127+126);
	}

	return exp - 127;
}

static inline float local_scalbnf( float x, int n ) __attribute__ ((always_inline));
static inline float local_scalbnf( float x, int n )
{
	//create 2**n in double
	union{ uint64_t u; double d;} u = {  ( (int64_t) n + 1023) << 52 };
	double d = x;
 
	//scale by appropriate power of 2
	d *= u.d;
 
	//return result
	return (float) d;
}


float fmodf( float x, float y )
{
	union{ float f; uint32_t u; }ux = {x};
	union{ float f; uint32_t u; }uy = {y};

	uint32_t absx = ux.u & 0x7fffffffU;
	uint32_t absy = uy.u & 0x7fffffffU;
	if( absx - 1U >= 0x7f7fffffU || absy - 1U >= 0x7f7fffffU )
	{
		float fabsx = __builtin_fabsf(x);	
		float fabsy = __builtin_fabsf(y);

		// deal with NaN
		if( x != x || y != y )
			return x + y;

		//x = Inf or y = 0, return Invalid per IEEE-754
		if( fabsx == __builtin_inff() || 0.0f == y )
		{
#ifdef ARMLIBM_SET_FLAGS
			return required_add_float( __builtin_inff(), -__builtin_inff() );	//set invalid
#else
			return __builtin_nan("");
#endif
		}

		//handle trivial case
		if( fabsy == __builtin_inff() || 0.0f == x )
		{
#ifdef ARMLIBM_SET_FLAGS
			required_add_float( fabsx, 0.0f );	// signal underflow (that is, no flag set,
												// but exception occurs if unmasked) if x is denorm
#endif
			return x;
		}
	}
 
	if( absy >= absx )
	{
		if( absy == absx )
		{
			ux.u &= 0x80000000;
			return ux.f;
		}
		return x;
	}
 
	//extract exponent, mantissa
	int32_t expx = absx >> 23;
	int32_t expy = absy >> 23;
	int32_t sx = absx & 0x007fffff;			// x significand
	int32_t sy = absy & 0x007fffff;			// y significand
	if( 0 == expx )		//denormal x
	{
		//calculate shift to move leading bit to exponents field
		uint32_t shift = __builtin_clzl( absx ) - (8*sizeof(long) - 24);
		sx <<= shift;	//do the shift
		expx = 1-shift;	//adjust the biased exponent accordingly, -1 here for the implicit shift to make implicit denorm leading bit explicit
	}
	if( 0 == expy )		//denormal y
	{
		//calculate shift to move leading bit to exponents field
		uint32_t shift = __builtin_clzl( absy ) - (8*sizeof(long) - 24);
		sy <<= shift;	//do the shift
		expy = 1-shift;	//adjust the biased exponent accordingly, -1 here for the implicit shift to make implicit denorm leading bit explicit
	}
	//make implicit bits explicit
	sx |= 0x00800000;
	sy |= 0x00800000;
	
		
	// The idea here is to iterate until the exponent of x is the same as y
	// Calculate the number of bits we can safely shave off before we reach parity
	int32_t idiff = expx - expy;	
	int32_t shift = 0;
	int32_t mask;	

	//since idiff is always >= 0 here, it is safe to enter
	//We always shift by shift+1 here, so in the first iteration, the worst that can happen
	do
	{
		// move the leading bit of x to the 23rd bit
		sx <<= shift;

		//Keep track that we did that
		idiff += ~shift;			// idiff -= shift + 1, +1 for the shift below
	
		//The two values both have the 23rd bit set as the leading bit  (24 bit unsigned number)
		//subtract one from the other. This gives us a signed 23 bit number in the range { -0x00ffffff ... 0x00ffffff }
		sx -= sy;		

		//record the sign
		mask = sx >> 31;

		//shift x left 1 to restore a 24 bit quantity
		sx += sx;		//this is potentially 1 shift too far, which we patch up later

		//if negative, we add back sy to restore to postiveness. This is the same as x - y + 0.5y = - 0.5y 
		// instead of x-y. We've effectively hoisted the subtract that would have appeared in the next loop
		// iteration here, and saved a test+branch in exchange for a shift and and. (The add happens either way.)
		sx += sy & mask;	

		//figure out how far we need to shift sx to get the leading bit into the 23rd position
		shift = __builtin_clzl( sx ) - (8*sizeof(long) - 24);
	}
	while( idiff >= shift && sx != 0);
	
	//We may have gone too far 
	if( idiff < 0 )	
	{
		//If so, rewind a little.
		// if sx - sy succeeded, it was the right thing to do, the only thing we did wrong was the shift
		// if sx - sy yielded a negative number, then we shouldn't have done that either
		sx += sy & mask;
		sx >>= 1;
//debug code to make sure we didn't do something dumb here
		idiff = 0;
	}

	//We may have some bits left over to shift in. 
	sx <<= idiff;
	
//convert back to float
	if( 0 == sx )
	{
		ux.u &= 0x80000000;
		return ux.f;
	}
			
	//figure out how far we need to shift in order to move leading bit into exponent field
	shift = __builtin_clzl( sx ) - (8*sizeof(long) - 24);
	sx <<= shift;				// move leading bit into exponent field
	expy -= shift;				// account for the shift in the exponent
	sx &= 0x007fffff;			// remove leading bit
	sx |= ux.u & 0x80000000;	//or in sign
	if( expy > 0 )
	{
		ux.u = sx | (expy << 23);
		return ux.f;
	}
	
	//denormal
	expy += 126;
	ux.u = sx | (expy << 23);
	return ux.f * 0x1.0p-126f;
}

//old slower floating point based one.
/*{
	union{ float f; uint32_t u; }ux = {x};
	union{ float f; uint32_t u; }uy = {y};

	float fabsx = __builtin_fabsf(x);	
	float fabsy = __builtin_fabsf(y);
	if( (ux.u & 0x7fffffffU) - 1U >= 0x7f7fffffU || (uy.u & 0x7fffffffU) - 1U >= 0x7f7fffffU )
	{
		// deal with NaN
		if( x != x || y != y )
			return x + y;

		//x = Inf or y = 0, return Invalid per IEEE-754
		if( fabsx == __builtin_inff() || 0.0f == y )
		{
			return required_add_float( __builtin_inff(), -__builtin_inff() );	//set invalid
		}

		//handle trivial case
		if( fabsy == __builtin_inff() || 0.0f == x )
		{
			required_add_float( fabsx, 0.0f );			//signal underflow (that is, no flag set, but exception occurs if unmasked) if x is denorm
			return x;
		}
	}
 
	if( fabsy > fabsx )
		return x;
 
	//we have to work
	int32_t iscx = local_ilogbf(fabsx);
	int32_t iscy = local_ilogbf(fabsy);
	int32_t idiff = iscx - iscy;
	float x1, y1;
	x1 = fabsx;
	y1 = fabsy;
	int32_t i;
		
	if( idiff )
	{
		y1 = local_scalbnf( y1, -iscy );
		x1 = local_scalbnf( x1, -iscx );
		for( i = idiff; i != 0; i-- )
		{
			if( x1 >= y1 )
				x1 -= y1;
			x1 += x1;
		}
		x1 = local_scalbnf( x1, iscy );
	}
	if( x1 >= fabsy )
		x1 -= fabsy;
	 
	if( x < 0 )
		x1 = -x1;
	
	return x1;
}
*/
