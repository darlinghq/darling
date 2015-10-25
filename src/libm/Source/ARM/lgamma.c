
/*
 *	lgamma.c
 *
 *		by Ian Ollmann based heavily on the work of Ali Sazegari, Steve Peters and Jeff Kidder
 *
 *	Copyright (c) 2007, Apple Inc. All Rights Reserved. 
 */
 
#include <math.h>
#include <stdint.h>

#ifdef ARMLIBM_SET_FLAGS
#include "required_arithmetic.h"
#endif

static const double d1 = -5.772156649015328605195174e-1;

static const double p1[8] = { 4.945235359296727046734888e+0,
                        2.018112620856775083915565e+2,
                        2.290838373831346393026739e+3,
                        1.131967205903380828685045e+4,
                        2.855724635671635335736389e+4,
                        3.848496228443793359990269e+4,
                        2.637748787624195437963534e+4,
                        7.225813979700288197698961e+3 };
						
static const double q1[8] = { 6.748212550303777196073036e+1,
                        1.113332393857199323513008e+3,
                        7.738757056935398733233834e+3,
                        2.763987074403340708898585e+4,
                        5.499310206226157329794414e+4,
                        6.161122180066002127833352e+4,
                        3.635127591501940507276287e+4,
                        8.785536302431013170870835e+3 };

static const double d2 = 4.227843350984671393993777e-1;

static const double p2[8] = { 4.974607845568932035012064e+0,
                        5.424138599891070494101986e+2,
                        1.550693864978364947665077e+4,
                        1.847932904445632425417223e+5,
                        1.088204769468828767498470e+6,
                        3.338152967987029735917223e+6,
                        5.106661678927352456275255e+6,
                        3.074109054850539556250927e+6 };

static const double q2[8] = { 1.830328399370592604055942e+2,
                        7.765049321445005871323047e+3,
                        1.331903827966074194402448e+5,
                        1.136705821321969608938755e+6,
                        5.267964117437946917577538e+6,
                        1.346701454311101692290052e+7,
                        1.782736530353274213975932e+7,
                        9.533095591844353613395747e+6 };

static const double d4 = 1.791759469228055000094023e+0;

static const double p4[8] = { 1.474502166059939948905062e+04,
                        2.426813369486704502836312e+06,
                        1.214755574045093227939592e+08,
                        2.663432449630976949898078e+09,
                        2.940378956634553899906876e+10,
                        1.702665737765398868392998e+11,
                        4.926125793377430887588120e+11,
                        5.606251856223951465078242e+11 };

static const double q4[8] = { 2.690530175870899333379843e+03,
                        6.393885654300092398984238e+05,
                        4.135599930241388052042842e+07,
                        1.120872109616147941376570e+09,
                        1.488613728678813811542398e+10,
                        1.016803586272438228077304e+11,
                        3.417476345507377132798597e+11,
                        4.463158187419713286462081e+11 };
						
static const double cc[7] = { -1.910444077728e-03,
                        8.4171387781295e-04,
                       -5.952379913043012e-04,
                        7.93650793500350248e-04,
                       -2.777777777777681622553e-03,
                        8.333333333333333331554247e-02,
                        5.7083835261e-03 };


static const double Root4xbig  = 2.25e+76;
static const double pnt68      = 0.6796875e+0;
static const double xbigger       = 2.55e+305;
static const double pi         = 3.1415926535897932384626434e+0;
static const double eps        = 2.22e-16;
static const double LogSqrt2pi = 0.9189385332046727417803297e+0;

static inline double lgammaApprox ( double x, int *psigngam );
static inline double lgammaApprox ( double x, int *psigngam )
{
	union{ double d; uint64_t u; }u = {x};

	// deal with 0, NaN, Inf
	if( (u.u & 0x7fffffffffffffffULL) -1ULL >= 0x7ff0000000000000ULL -1ULL )
	{
		// 0
		if( x == 0.0 ) {
#ifdef ARMLIBM_SET_FLAGS
			return required_divide_double( 1.0, __builtin_fabs(x) );		//set div/0 return inf
#else
			return __builtin_inf();
#endif
		}

		//NaN
		if( x != x )
			return x + x;
			
		//Inf
		return __builtin_fabs( x );
	}

      register int i;
      register double y, result, numerator, denominator, ysquared, 
                      corrector, xMinus1, xMinus2, xMinus4; 
      
	  *psigngam = 1; 
      
      
/*******************************************************************************
 *      For negative x, since (G is gamma function)
 *		-x*G(-x)*G(x) = pi/sin(pi*x),
 * 	we have
 * 		G(x) = pi/(sin(pi*x)*(-x)*G(-x))
 *	since G(-x) is positive, sign(G(x)) = sign(sin(pi*x)) for x<0
 *	Hence, for x<0, signgam = sign(sin(pi*x)) and
 *		lgamma(x) = log(|Gamma(x)|)
 *			  = log(pi/(|x*sin(pi*x)|)) - lgamma(-x);
 *******************************************************************************/

	if ( x < 0.0 )
	{
		int dummy = 1; 	
		register double a, y1, fraction;
            
		if ( x <= -0x1.0p52 ) // big negative integer?
			return x / -0.0;
                
		y = - x;
		y1 = y;

		// y1 = trunc ( y );
		int64_t ll = 0;
		if( y < 0x1.0p63 )
		{
			ll = (int64_t) y;
			y1 = (double) ll;
		}
		fraction = y - y1; // excess over the boundary
            
		if ( fraction == 0.0 ) { // negative integer?
#ifdef ARMLIBM_SET_FLAGS
			return required_divide_double( x, -0.0 );
#else
			return __builtin_copysign( __builtin_inf(), -x);
#endif
		}
		
		else 
		{
			a = sin ( pi * fraction );
			if ( 0ULL == (ll & 1ULL) ) // 0, 2, 4, ...
				*psigngam = -1; /* Gamma(x) < 0 */ 
		}
			
		return log ( pi / __builtin_fabs ( a * x ) ) - lgammaApprox ( -x, &dummy );
	}
      
/*******************************************************************************
*     The argument is positive, if it is bigger than xbigger = 2.55e+305 then     *
*     overflow.                                                                *
*******************************************************************************/

	if ( x > xbigger ) {
#ifdef ARMLIBM_SET_FLAGS
		return required_multiply_double( x, 0x1.0p1023 );	//return inf, set overflow
#else
		return __builtin_inf();
#endif
	}

	y = x;

/*******************************************************************************
*     x <= eps then return the approximation log(x).                           *
*******************************************************************************/

	if ( y <= eps )
		return ( - log ( y ) );

/*******************************************************************************
*     x is in (eps,1.5] then use d1, p1 and q1 coefficients.                   *
*******************************************************************************/

	else if ( y <= 1.5 )
	{
		if ( y < pnt68 )
		{
			corrector = - log ( y );
			xMinus1 = y;
		}
		else
	    {
			corrector = 0.0;
			xMinus1 = ( y - 0.5 ) - 0.5;
		}
		
		if ( ( y <= 0.5 ) || ( y >= pnt68 ) )
		{
			if( 0.0 == xMinus1 )	//early out for exact zero so we get inexact flag right
				return corrector;

			denominator = 1.0;
			numerator = 0.0;
			for ( i = 0; i < 8; i++ )
			{
				numerator = numerator * xMinus1 + p1[i];
				denominator = denominator * xMinus1 + q1[i];
			}
			result = corrector + ( xMinus1 * ( d1 + xMinus1 * ( numerator / denominator ) ) );
		}
		else
		{
			xMinus2 = ( y - 0.5 ) - 0.5;
			denominator = 1.0;
			numerator = 0.0;
			for ( i = 0; i < 8; i++ )
			{
				numerator = numerator * xMinus2 + p2[i];
				denominator = denominator * xMinus2 + q2[i];
			}
			result = corrector + ( xMinus2 * ( d2 + xMinus2 * ( numerator / denominator ) ) );
		}
	}

/*******************************************************************************
*     x is in (1.5,4.0] then use d2, p2 and q2 coefficients.                   *
*******************************************************************************/

	else if ( y <= 4.0 )
	{
		xMinus2 = y - 2.0;
		if( 0.0 == xMinus2 )
			return 0.0;

		denominator = 1.0;
		numerator = 0.0;
		for ( i = 0; i < 8; i++ )
		{
			numerator = numerator * xMinus2 + p2[i];
			denominator = denominator * xMinus2 + q2[i];
		}
		result = xMinus2 * ( d2 + xMinus2 * ( numerator / denominator ) );
	}
            
/*******************************************************************************
*     x is in (4.0,12.0] then use d4, p4 and q4 coefficients.                  *
*******************************************************************************/

	else if ( y <= 12.0 )
	{
		xMinus4 = y - 4.0;
		denominator = - 1.0;
		numerator = 0.0;
		for ( i = 0; i < 8; i++ )
		{
			numerator = numerator * xMinus4 + p4[i];
			denominator = denominator * xMinus4 + q4[i];
		}
		result = d4 + xMinus4 * ( numerator / denominator );
	}
	else  /* ( y >= 12.0 ) */
	{
		result = 0.0;
		if ( y <= Root4xbig )
		{
			result = cc[6];
			ysquared = y * y;
			for ( i = 0; i < 6; i++ )
				result = result / ysquared + cc[i];
		}
		result /= y;
		corrector = log ( y );
		result += LogSqrt2pi - 0.5 * corrector;
		result += y * ( corrector - 1.0 );
	}
      
	x = rint ( x ); // INEXACT set as a side effect for non integer x  
	return result;
}

double lgamma( double x )
{
    return lgammaApprox ( x, &signgam );
}

double lgamma_r( double, int *);
double lgamma_r( double x, int *psigngam ) {
	return lgammaApprox(x, psigngam);
}
