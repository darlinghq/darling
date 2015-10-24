/*
 *	tgamma.c
 *
 *		by Ian Ollmann, drawing heavily from work by Steve Peters, Jeff Kidder, and Ali Sazegari.
 *
 *	Copyright (c) 2007 Apple Inc. All Rights Resterved. 
 */
 
#include <math.h>
#include <stdint.h>

#ifdef ARMLIBM_SET_FLAGS
#include "required_arithmetic.h"
#endif

static const double LogSqrt2pi = 0.9189385332046727417803297e+0;
static const double pi         = 3.1415926535897932384626434e+0;
static const double eps        = 0x1.0p-52;
static const double xbig       = 171.624e+0;


static const double p[8] = { -1.71618513886549492533811e+0,
                        2.47656508055759199108314e+1,
                       -3.79804256470945635097577e+2,
                        6.29331155312818442661052e+2,
                        8.66966202790413211295064e+2,
                       -3.14512729688483675254357e+4,
                       -3.61444134186911729807069e+4,
                        6.64561438202405440627855e+4 };

static const double q[8] = { -3.08402300119738975254353e+1,
                        3.15350626979604161529144e+2,
                       -1.01515636749021914166146e+3,
                       -3.10777167157231109440444e+3,
                        2.25381184209801510330112e+4,
                        4.75584627752788110767815e+3,
                       -1.34659959864969306392456e+5,
                       -1.15132259675553483497211e+5 };

static const double c[7] = {	0xa.aaaaaaaaaaaaaabp-7,
								-0xb.60b60b60b60b60bp-12,
								0xd.00d00d00d00d00dp-14,
								-0x9.c09c09c09c09c0ap-14,
								0xd.ca8f158c7f91ab8p-14,
								-0xf.b5586ccc9e3e41p-13,
								0xd.20d20d20d20d20dp-11 };

double tgamma( double x )
{
	union{ double d; uint64_t u; }u = {x};

	// deal with 0, NaN, Inf
	if( (u.u & 0x7fffffffffffffffULL) -1ULL >= 0x7ff0000000000000ULL -1ULL )
	{
		// 0
		if( x == 0.0 ) {
#ifdef ARMLIBM_SET_FLAGS
			return required_divide_double( 1.0, x );
#else
			return __builtin_copysign(__builtin_inf(), x);
#endif
		}

		//NaN
		if( x != x )
			return x + x;
			
		//Inf
		if( x < 0.0 ) {
#ifdef ARMLIBM_SET_FLAGS
			return required_multiply_double( x, 0.0 );		// -Inf: set invalid, return NaN
#else
			return __builtin_nan("");
#endif
		}
		
		return x;	// Inf
	}

	int n = 0;
	int parity = 0;
	double y = x;
	double fact = 1.0;

	// negative x
	if( x <= 0.0 )
	{
		y = -x;
		if( y < 0x1.0p-1022 )
			return 1.0 / x;

		// y1 = trunc ( y );  //fast trunc, no edge cases
		double iy = y;
		int64_t ll = 0;
		if( y < 0x1.0p54 )
		{
			ll = y;
			iy = ll; 
		}
	
		double fraction = y - iy;
		
		if( fraction != 0.0 )
		{
			parity = (int) (ll & 1);
			fact = -pi / sin( pi * fraction );
			y += 1.0;
		}
		else {
#ifdef ARMLIBM_SET_FLAGS
			return required_add_double( -__builtin_inf(), __builtin_inf() );	// set invalid, return NaN
#else
			return __builtin_nan("");
#endif
		}
	}

	double result;
	int i;

	if( y < eps )
	{
		result = 1.0 / y;
	}
	else if( y < 12.0 )
	{
		double y1 = y;
		double z;
		if( y < 1.0)
		{
			z = y;
			y += 1.0;
		}
		else
		{	
			n = (int)y - 1;
			y -= (double) n;
			z = y - 1.0;
		}
		
		double numerator = 0.0;
		double denomenator = 1.0;
		for( i = 0; i < 8; i++ )
		{
			numerator = (numerator + p[i]) * z;
			denomenator = denomenator * z + q[i];
		}
		result = numerator / denomenator + 1.0;
		if( y1 < y )
			result /= y1;
		else if( y1 > y )
		{
			for( i = 0; i < n; i++ )
			{
				result *= y;
				y += 1.0;
			}
		}
	}
	else
	{ // y large
		if( x < xbig )
		{
			double yy = y * y;
			double sum = c[6];
			for( i = 5; i >= 0; i-- )
				sum = sum / yy + c[i];
			sum = sum / y - y + LogSqrt2pi;
			sum += (y - 0.5) * log( y );
			result = exp( sum );
		}
		else
			return x * 0x1.0p1023;		//set overflow, return inf
	}
	
	if( parity )
		result = -result;
	
	if( fact != 1.0 )
		result = fact / result;
		
	return result;
}
