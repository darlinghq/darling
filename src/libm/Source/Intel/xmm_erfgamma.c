/*
 * Copyright (c) 2002 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*******************************************************************************
*                                                                              *
*      File gamma.c,                                                           *
*      Functions gamma(x),                                                     *
*      Implementation of the gamma function for the PowerPC.                   *
*                                                                              *
*      Copyright c 1993 Apple Computer, Inc.  All rights reserved.             *
*                                                                              *
*      Written by Ali Sazegari, started on January 1993,                       *
*		Ported by Ian Ollmann to i386, October 2005                            *
*                                                                              *
*      based on FORTRAN routines in SpecFun by J. W. Cody and L. Stoltz of     *
*      Applied Mathematics Division of Argonne National Laboratory,            *
*      Argonne, IL 60439.                                                      *
*                                                                              *
*      W A R N I N G:  This routine expects a 64 bit double model.             *
*                                                                              *
*      January  29  1993: First C implementation for PowerPC.                  *
*      April    26  1993: fixed a few bugs in the interval [xbig,inf].         *
*      July     14  1993: added #pragma fenv_access. This function is now      *
*                         using the the string oriented ÔnanÕ.  replaced       *
*                         feholdexcept by _feprocentry to guard rounding.      *
*      July     29  1993: corrected the string nan.                            *
*      October  07  1993: removed the raising of the overflow flag for arg= °. *
*      September19  1994: changed all environemental funtions to __setflm,     *
*                         changed HUGE_VAL to Huge.d for perfrormance.         *
*      January  11  1995: a humilating error corrected.  in the interval       *
*                         [12,178], the nonexistant array element c[7] is      *
*                         addressed.  it should be c[6].                       *
*                         a short error analysis reveals that in double        *
*                         precision referencing c[7] instead of c[6] has no    *
*                         impact on the accuracy of the result, provided that  *
*                         the compiler assigns 0.0 to c[7], which the ibm xlc  *
*                         does.  this explains why the double precision        *
*                         gamma function passed accuracy tests.  the relative  *
*                         error in extended is at most 5.56E-17 and occurs     *
*                         for x=12.0.  the calculation is no longer affected   *
*                         for arguments x³19.                                  *
*                                                                              *
********************************************************************************
*                                                                              *
*                              G  A  M  M  A  (  X  )                          *
*                                                                              *
*      The gamma function is an increasing function over [2,inf].  For large   *
*      enough x, it behaves like [ e^( x ln ( x/ e ) ] / sqrt(x/pi).  It may   *
*      be more appropriate to work with the logorithm of Gamma.                *
*                                                                              *
*      This routine calculates the gamma function for a real argument x.       *
*      Computation is based on an algorithm outlined in reference 1 below.     *
*      The program uses rational functions that approximate the gamma          *
*      function to at least 20 significant decimal digits.  Coefficients       *
*      for the approximation over the interval (1,2) are unpublished.          *
*      Those for the approximation for x >= 12 are from reference 2 below.     *
*                                                                              *
********************************************************************************
*                                                                              *
*      Explanation of machine-dependent constants:                             *
*                                                                              *
*      xbig     - the largest argument for which gamma(x) is representable     *
*                 in the machine, i.e., the solution to the equation           *
*                 gamma ( xbig ) = 2 ** MaxExp, where MaxExp is the maximum    *
*                 power of 2 before infinity;                                  *
*      xinf     - the largest machine representable floating-point number      *
*                 before infinity, approximately 2 ** MaxExp;                  *
*      eps      - the smallest positive floating-point number such that        *
*                 1.0 + eps > 1.0;                                             *
*      MinimumX - the smallest positive floating-point number such that        *
*                 1/MinimumX is machine representable.                         *
*                                                                              *
*      Approximate values for the macintosh and the powerpc are:               *
*                                                                              *
*                                base       MaxExp        xbig                 *
*                                                                              *
*      Macintosh     extended     2         16382        1755.36               *
*      PowerPC        double      2          1023        171.624               *
*                                                                              *
*                                 xinf         eps        MinimumX             *
*                                                                              *
*      Macintosh     extended   1.19x+4932   5.42x-20    8.40x-4933            *
*      PowerPC        double    1.79d+308    2.22d-16    2.23d-308             *
*                                                                              *
********************************************************************************
*                                                                              *
*      The program returns a quiet NaN for singularities and infinity when     *
*      overflow occurs.  The computation is believed to be free of undeserved  *
*      underflow and overflow.                                                 *
*                                                                              *
*      References:                                                             *
*                                                                              *
*      [1] "An Overview of Software Development for Special Functions",        *
*          W. J. Cody, Lecture Notes in Mathematics, 506, Numerical Analysis   *
*          Dundee, 1975, G. A. Watson (ed.), Springer Verlag, Berlin, 1976.    *
*                                                                              *
*      [2] Computer Approximations, Hart, Et. Al., Wiley and sons, New York    *
*          1968.                                                               *
*                                                                              *
*******************************************************************************/

#include      "math.h"
#include	  "float.h"
#include      "fenv.h"
#include      "fenv_private.h"

/*******************************************************************************
*            Functions needed for the computation.                             *
*******************************************************************************/

/*     the following fp.h functions are used:                                 */
/*     exp, log, sin, __fpclassifyd and nan;                                  */
/*     the following environmental functions are used:                        */
/*     __setflm.                                                              */

#include "xmmLibm_prefix.h"

/*******************************************************************************
*     Coefficients for P in gamma approximation over [1,2] in decreasing order.*
*******************************************************************************/

static const double p[8] = { -1.71618513886549492533811e+0,
                        2.47656508055759199108314e+1,
                       -3.79804256470945635097577e+2,
                        6.29331155312818442661052e+2,
                        8.66966202790413211295064e+2,
                       -3.14512729688483675254357e+4,
                       -3.61444134186911729807069e+4,
                        6.64561438202405440627855e+4 };

/*******************************************************************************
*     Coefficients for Q in gamma approximation over [1,2] in decreasing order.*
*******************************************************************************/

static const double q[8] = { -3.08402300119738975254353e+1,
                        3.15350626979604161529144e+2,
                       -1.01515636749021914166146e+3,
                       -3.10777167157231109440444e+3,
                        2.25381184209801510330112e+4,
                        4.75584627752788110767815e+3,
                       -1.34659959864969306392456e+5,
                       -1.15132259675553483497211e+5 };
                         
/*******************************************************************************
*     Coefficients for Stirling's series  for ln(Gamma) over [12, INF].        *
*******************************************************************************/

static const long double c[7] = {	0xa.aaaaaaaaaaaaaabp-7,
	-0xb.60b60b60b60b60bp-12,
	0xd.00d00d00d00d00dp-14,
	-0x9.c09c09c09c09c0ap-14,
	0xd.ca8f158c7f91ab8p-14,
	-0xf.b5586ccc9e3e41p-13,
0xd.20d20d20d20d20dp-11 };

static const double LogSqrt2pi = 0.9189385332046727417803297e+0;
static const double pi         = 3.1415926535897932384626434e+0;
static const double xbig       = 171.624e+0;
static const double MinimumX   = 2.23e-308;
static const double eps        = 2.22e-16;

#define      GAMMA_NAN      "42"
#define      SET_INVALID    0x01000000


static inline double _tgamma ( double x )   ALWAYS_INLINE;
static inline double _tgamma ( double x )
{
      register int n, parity, i;
      register double y, y1, result, fact, fraction, z, numerator, denominator, ysquared, sum; 
    
	
/*******************************************************************************
*     The next switch will decipher what sort of argument we have. If argument *
*     is SNaN then a QNaN has to be returned and the invalid flag signaled.    * 
*******************************************************************************/

	if( x != x )
		return x + x;		//silence NaN
	
	if( x == 0.0 )
		return LogSqrt2pi / x;
	
	if( __builtin_fabs(x) == __builtin_inf() )
	{
		if( x < 0 )
		{
			SET_INVALID_FLAG();
			return __builtin_nan( GAMMA_NAN );
		}
	
		return x;
	}

      
	parity = 0;
	fact = 1.0;
	n = 0;
	y = x;

/*******************************************************************************
*     The argument is negative.                                                *
*******************************************************************************/

	if ( y <= 0.0 )
	{
		y = - x;
		if ( y < MinimumX )
			return 1.0 / x;

		y1 = trunc ( y );
		fraction = y - y1;
		if ( fraction != 0.0 )                   /* is it an integer?   */
	    {                                   /* is it odd or even?  */
			if ( y1 != trunc ( y1 * 0.5 ) * 2.0 ) 
				parity = 1;
			fact = - pi / sin ( pi * fraction );
			y += 1.0;
		}
        else
        {
			SET_INVALID_FLAG();
			return __builtin_nan( GAMMA_NAN );
		}
	}

/*******************************************************************************
*     The argument is positive.                                                *
*******************************************************************************/

	if ( y < eps )                         /* argument is less than epsilon. */
	{
		result = 1.0 / y;
	}
	else if ( y < 12.0 )                 /* argument x is eps < x < 12.0.  */
	{
		y1 = y;
		if ( y < 1.0 )                 /* x is in (eps, 1.0).            */
		{
			z = y;
			y += 1.0;
		}
		else                           /* x is in [1.0,12.0].            */
		{
			n = ( int ) y - 1;
			y -= ( double ) n;
			z = y - 1.0;
		}
		numerator = 0.0;
		denominator = 1.0;
		for ( i = 0; i < 8; i++ )
		{
			numerator = ( numerator + p[i] ) * z;
			denominator = denominator * z + q[i];
		}
		result = numerator / denominator + 1.0;
		if ( y1 < y )
			result /= y1;
		else if ( y1 > y )
		{
			for ( i = 0; i < n; i++ )
			{
				result *= y;
				y += 1.0;
			}
		}
	}
	else
	{
		if ( x <= xbig )
		{
			ysquared = y * y;
			sum = c[6];
			for ( i=5; i >= 0; i-- )
			sum = sum / ysquared + c[i];
			sum = sum / y - y + LogSqrt2pi;
			sum += ( y - 0.5 ) * log ( y );
			result = exp ( sum );
		}
		else
			return x * 0x1.0p1023;			//set overflow, return inf
	}
	
	if ( parity ) 
		result = - result;
	if ( fact != 1.0 ) 
		result = fact / result;

	return result;
}

double tgamma ( double x )
{
	return _tgamma( x );
}

double gamma ( double x )   //legacy -- required for various calculators in the OS and 3rd party
{
    return _tgamma( x );
}

float tgammaf( float x )
{
	return (float) _tgamma( x );
}

/*******************************************************************************
*     Coefficients for P in gamma approximation over [1,2] in decreasing order.*
*******************************************************************************/

static const long double pL[8] = { -1.71618513886549492533811e+0L,
									2.47656508055759199108314e+1L,
								   -3.79804256470945635097577e+2L,
									6.29331155312818442661052e+2L,
									8.66966202790413211295064e+2L,
								   -3.14512729688483675254357e+4L,
								   -3.61444134186911729807069e+4L,
									6.64561438202405440627855e+4L };

/*******************************************************************************
*     Coefficients for Q in gamma approximation over [1,2] in decreasing order.*
*******************************************************************************/

static const long double qL[8] = { -3.08402300119738975254353e+1L,
									3.15350626979604161529144e+2L,
								   -1.01515636749021914166146e+3L,
								   -3.10777167157231109440444e+3L,
									2.25381184209801510330112e+4L,
									4.75584627752788110767815e+3L,
								   -1.34659959864969306392456e+5L,
								   -1.15132259675553483497211e+5L };
                         
/*******************************************************************************
*     Coefficients for Stirling's Approximation to ln(Gamma) on [12,inf]       *
*******************************************************************************/

static const long double cL[7] = {	0xa.aaaaaaaaaaaaaabp-7L,
																	 -0xb.60b60b60b60b60bp-12L,
																		0xd.00d00d00d00d00dp-14L,
																	 -0x9.c09c09c09c09c0ap-14L,
																		0xd.ca8f158c7f91ab8p-14L,
																	 -0xf.b5586ccc9e3e41p-13L,
																		0xd.20d20d20d20d20dp-11L };

static const long double LogSqrt2piL = 0.9189385332046727417803297e+0L;		//Ln( sqrt( 2*pi))
static const long double piL         = 3.1415926535897932384626434e+0L;		//pi
static const long double xbigL       = 0xd.b718c066b352e21p+7L;						//cutoff for overflow condition = 1755.54...  
static const long double MinimumXL   = 1.0022L * LDBL_MIN;					//
static const long double epsL        = 0.9998L * LDBL_EPSILON;


long double tgammal( long double x )
{
      register int n, parity, i;
      register long double y, y1, result, fact, fraction, z, numerator, denominator, ysquared, sum; 
    
	
/*******************************************************************************
*     The next switch will decipher what sort of argument we have. If argument *
*     is SNaN then a QNaN has to be returned and the invalid flag signaled.    * 
*******************************************************************************/

	if( x != x )
		return x + x;		//silence NaN
	
	if( x == 0.0 )
		return LogSqrt2piL / x;
	
	if( __builtin_fabsl(x) == __builtin_infl() )
	{
		if( x < 0.0L )
		{
			SET_INVALID_FLAG();
			return __builtin_nanl( GAMMA_NAN ); 
		}
	
		return x;
	}

      
	parity = 0;
	fact = 1.0L;
	n = 0;
	y = x;

/*******************************************************************************
*     The argument is negative.                                                *
*******************************************************************************/

	if ( y <= 0.0L )
	{
		y = - x;
		if ( y < MinimumXL )
			return 1.0L / x;

		y1 = truncl( y );
		fraction = y - y1;
		if ( fraction != 0.0L )             /* is it an integer?   */
	    {                                   /* is it odd or even?  */
			if ( y1 != truncl ( y1 * 0.5L ) * 2.0L ) 
				parity = 1;
			fact = - piL / sinl ( piL * fraction );
			y += 1.0L;
		}
        else
        {
			volatile long double err = __builtin_nanl( GAMMA_NAN );
			return err + (int) err;
		}
	}

/*******************************************************************************
*     The argument is positive.                                                *
*******************************************************************************/

	if ( y < epsL )                         /* argument is less than epsilon. */
	{
		result = 1.0L / y;
	}
	else if ( y < 12.0L )                 /* argument x is eps < x < 12.0.  */
	{
		y1 = y;
		if ( y < 1.0L )                 /* x is in (eps, 1.0).            */
		{
			z = y;
			y += 1.0L;
		}
		else                           /* x is in [1.0,12.0].            */
		{
			n = ( int ) y - 1;
			y -= ( long double ) n;
			z = y - 1.0L;
		}
		numerator = 0.0L;
		denominator = 1.0L;
		for ( i = 0; i < 8; i++ )
		{
			numerator = ( numerator + pL[i] ) * z;
			denominator = denominator * z + qL[i];
		}
		result = numerator / denominator + 1.0L;
		if ( y1 < y )
			result /= y1;
		else if ( y1 > y )
		{
			for ( i = 0; i < n; i++ )
			{
				result *= y;
				y += 1.0L;
			}
		}
	}
	else
	{
		if ( x <= xbigL )
		{
			ysquared = y * y;
			sum = cL[6];
			for ( i = 5; i >= 0; i-- )
				sum = sum / ysquared + cL[i];
			sum = sum / y - y + LogSqrt2piL;
			sum += ( y - 0.5L ) * logl( y );
			result = expl ( sum );
		}
		else
			return x * 0x1.0p16383L;			//set overflow, return inf
	}
	
	if ( parity ) 
		result = - result;
	if ( fact != 1.0L ) 
		result = fact / result;

	return result;
}

#pragma mark -

/*******************************************************************************
*     Coefficients for P1 in lgamma approximation over [0.5,1.5) in decreasing *
*     order.                                                                   *
*******************************************************************************/

static const double d1 = -5.772156649015328605195174e-1;

static const double p1[8] = { 4.945235359296727046734888e+0,
                        2.018112620856775083915565e+2,
                        2.290838373831346393026739e+3,
                        1.131967205903380828685045e+4,
                        2.855724635671635335736389e+4,
                        3.848496228443793359990269e+4,
                        2.637748787624195437963534e+4,
                        7.225813979700288197698961e+3 };
                       
/*******************************************************************************
*     Coefficients for Q1 in gamma approximation over [0.5,1.5) in decreasing  *
*     order.                                                                   *
*******************************************************************************/

static const double q1[8] = { 6.748212550303777196073036e+1,
                        1.113332393857199323513008e+3,
                        7.738757056935398733233834e+3,
                        2.763987074403340708898585e+4,
                        5.499310206226157329794414e+4,
                        6.161122180066002127833352e+4,
                        3.635127591501940507276287e+4,
                        8.785536302431013170870835e+3 };
                        
/*******************************************************************************
*     Coefficients for P2 in lgamma approximation over [1.5,4) in decreasing   *
*     order.                                                                   *
*******************************************************************************/

static const double d2 = 4.227843350984671393993777e-1;

static const double p2[8] = { 4.974607845568932035012064e+0,
                        5.424138599891070494101986e+2,
                        1.550693864978364947665077e+4,
                        1.847932904445632425417223e+5,
                        1.088204769468828767498470e+6,
                        3.338152967987029735917223e+6,
                        5.106661678927352456275255e+6,
                        3.074109054850539556250927e+6 };

/*******************************************************************************
*     Coefficients for Q2 in gamma approximation over [1.5,4) in decreasing    *
*     order.                                                                   *
*******************************************************************************/

static const double q2[8] = { 1.830328399370592604055942e+2,
                        7.765049321445005871323047e+3,
                        1.331903827966074194402448e+5,
                        1.136705821321969608938755e+6,
                        5.267964117437946917577538e+6,
                        1.346701454311101692290052e+7,
                        1.782736530353274213975932e+7,
                        9.533095591844353613395747e+6 };

/*******************************************************************************
*     Coefficients for P4 in lgamma approximation over [4,12) in decreasing    *
*     order.                                                                   *
*******************************************************************************/

static const double d4 = 1.791759469228055000094023e+0;

static const double p4[8] = { 1.474502166059939948905062e+04,
                        2.426813369486704502836312e+06,
                        1.214755574045093227939592e+08,
                        2.663432449630976949898078e+09,
                        2.940378956634553899906876e+10,
                        1.702665737765398868392998e+11,
                        4.926125793377430887588120e+11,
                        5.606251856223951465078242e+11 };

/*******************************************************************************
*     Coefficients for Q4 in gamma approximation over [4,12) in decreasing     *
*     order.                                                                   *
*******************************************************************************/

static const double q4[8] = { 2.690530175870899333379843e+03,
                        6.393885654300092398984238e+05,
                        4.135599930241388052042842e+07,
                        1.120872109616147941376570e+09,
                        1.488613728678813811542398e+10,
                        1.016803586272438228077304e+11,
                        3.417476345507377132798597e+11,
                        4.463158187419713286462081e+11 };
                        
/*******************************************************************************
*     Coefficients for minimax approximation over [12, xbig].                  *
*******************************************************************************/

static const double cc[7] = { -1.910444077728e-03,
                        8.4171387781295e-04,
                       -5.952379913043012e-04,
                        7.93650793500350248e-04,
                       -2.777777777777681622553e-03,
                        8.333333333333333331554247e-02,
                        5.7083835261e-03 };

static const double xbigger       = 2.55e+305;
static const double Root4xbig  = 2.25e+76;
static const double pnt68      = 0.6796875e+0;

static const double twoTo52      = 0x1.0p+52; // 4503599627370496.0;

/*******************************************************************************
*            Value of special function NaN.                                    *
*******************************************************************************/

#define      SET_INVALID    0x01000000
#define      GAMMA_NAN      "42"

/* Note: The use of signgam is not thread safe */
/* The value of signgam is not meaningful if the result is NaN, but will be 1 */
int signgam; /* global return value by lgamma of the sign of gamma(x). */

static inline double lgammaApprox ( double x, int *psigngam );
static inline double lgammaApprox ( double x, int *psigngam )
{
      register int i;
      register double y, result, numerator, denominator, ysquared, 
                      corrector, xMinus1, xMinus2, xMinus4; 
      
	  *psigngam = 1; 
      
/*******************************************************************************
*     The next switch will decipher what sort of argument we have. If argument *
*     is SNaN then a QNaN has to be returned and the invalid flag signaled.    * 
*******************************************************************************/
	
	if( x != x )
		return x + x;
		
	if( x == 0.0 )
		return 1.0 / __builtin_fabs( x );			//set div/0 return inf

	if( __builtin_fabs( x ) == __builtin_inf() )
		return __builtin_fabs( x );
      
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
            
		if ( x <= -twoTo52 ) // big negative integer?
			return x / -0.0;
                
		y = - x;
		y1 = trunc ( y );
		fraction = y - y1; // excess over the boundary
            
		if ( fraction == 0.0 ) // negative integer?
			return x / -0.0;
		else {
			a = sin ( pi * fraction );
			if ( y1 == trunc ( y1 * 0.5 ) * 2.0 ) // 0, 2, 4, ...
				{
				*psigngam = -1; /* Gamma(x) < 0 */ 
				} // otherwise leave psigngam = 1
            }
			
		return log ( pi / __builtin_fabs ( a * x ) ) - lgammaApprox ( -x, &dummy );
	}
      
/*******************************************************************************
*     The argument is positive, if it is bigger than xbigger = 2.55e+305 then     *
*     overflow.                                                                *
*******************************************************************************/

	if ( x > xbigger )
		return x * 0x1.0p1023;	//return inf, set overflow

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
      
double lgamma ( double x ) //sets signgam as side effect
{
    return lgammaApprox ( x, &signgam );
}

double lgamma_r( double , int * );
double lgamma_r( double x , int *psigngam ) // threadsafe.
{
	return lgammaApprox(x, psigngam);
}

float lgammaf( float x ) //sets signgam as side effect
{
    return (float) lgammaApprox ( x, &signgam );
}

float lgammaf_r( float , int * );
float lgammaf_r( float x , int *psigngam ) // threadsafe.
{
	double lg = lgammaApprox((double)x, psigngam);
	return (float)lg;
}

/*******************************************************************************
*     Coefficients for P1 in lgamma approximation over [0.5,1.5) in decreasing *
*     order.                                                                   *
*******************************************************************************/

static const long double d1L = -5.772156649015328605195174e-1L;

static const long double p1L[8] = { 4.945235359296727046734888e+0L,
									2.018112620856775083915565e+2L,
									2.290838373831346393026739e+3L,
									1.131967205903380828685045e+4L,
									2.855724635671635335736389e+4L,
									3.848496228443793359990269e+4L,
									2.637748787624195437963534e+4L,
									7.225813979700288197698961e+3L };
                       
/*******************************************************************************
*     Coefficients for Q1 in gamma approximation over [0.5,1.5) in decreasing  *
*     order.                                                                   *
*******************************************************************************/

static const long double q1L[8] = { 6.748212550303777196073036e+1L,
									1.113332393857199323513008e+3L,
									7.738757056935398733233834e+3L,
									2.763987074403340708898585e+4L,
									5.499310206226157329794414e+4L,
									6.161122180066002127833352e+4L,
									3.635127591501940507276287e+4L,
									8.785536302431013170870835e+3L };
                        
/*******************************************************************************
*     Coefficients for P2 in lgamma approximation over [1.5,4) in decreasing   *
*     order.                                                                   *
*******************************************************************************/

static const long double d2L = 4.227843350984671393993777e-1L;

static const long double p2L[8] = { 4.974607845568932035012064e+0L,
									5.424138599891070494101986e+2L,
									1.550693864978364947665077e+4L,
									1.847932904445632425417223e+5L,
									1.088204769468828767498470e+6L,
									3.338152967987029735917223e+6L,
									5.106661678927352456275255e+6L,
									3.074109054850539556250927e+6L };

/*******************************************************************************
*     Coefficients for Q2 in gamma approximation over [1.5,4) in decreasing    *
*     order.                                                                   *
*******************************************************************************/

static const long double q2L[8] = {	1.830328399370592604055942e+2L,
									7.765049321445005871323047e+3L,
									1.331903827966074194402448e+5L,
									1.136705821321969608938755e+6L,
									5.267964117437946917577538e+6L,
									1.346701454311101692290052e+7L,
									1.782736530353274213975932e+7L,
									9.533095591844353613395747e+6L };

/*******************************************************************************
*     Coefficients for P4 in lgamma approximation over [4,12) in decreasing    *
*     order.                                                                   *
*******************************************************************************/

static const long double d4L = 1.791759469228055000094023e+0L;

static const long double p4L[8] = { 1.474502166059939948905062e+04L,
									2.426813369486704502836312e+06L,
									1.214755574045093227939592e+08L,
									2.663432449630976949898078e+09L,
									2.940378956634553899906876e+10L,
									1.702665737765398868392998e+11L,
									4.926125793377430887588120e+11L,
									5.606251856223951465078242e+11L };

/*******************************************************************************
*     Coefficients for Q4 in gamma approximation over [4,12) in decreasing     *
*     order.                                                                   *
*******************************************************************************/

static const long double q4L[8] = { 2.690530175870899333379843e+03L,
									6.393885654300092398984238e+05L,
									4.135599930241388052042842e+07L,
									1.120872109616147941376570e+09L,
									1.488613728678813811542398e+10L,
									1.016803586272438228077304e+11L,
									3.417476345507377132798597e+11L,
									4.463158187419713286462081e+11L };
                        
/*******************************************************************************
*     Coefficients for minimax approximation over [12, xbig].                  *
*******************************************************************************/

static const long double ccL[7] = { -1.910444077728e-03L,
									8.4171387781295e-04L,
								   -5.952379913043012e-04L,
									7.93650793500350248e-04L,
								   -2.777777777777681622553e-03L,
									8.333333333333333331554247e-02L,
									5.7083835261e-03L };

static const long double xbiggerL		= 2.55e+305L;		// ????
static const long double Root4xbigL		= 2.25e+76L;		// ????  //pow( xbiggerL, 0.25 )
static const long double pnt68L			= 0.6796875e+0L;	

static const long double twoTo63		= 0x1.0p+63L; // 4503599627370496.0;


static inline long double lgammaApproxL ( long double x, int *psigngam );
static inline long double lgammaApproxL ( long double x, int *psigngam )
{
      register int i;
      register long double y, result, numerator, denominator, ysquared, 
                      corrector, xMinus1, xMinus2, xMinus4; 
      
	  *psigngam = 1; 
      
/*******************************************************************************
*     The next switch will decipher what sort of argument we have. If argument *
*     is SNaN then a QNaN has to be returned and the invalid flag signaled.    * 
*******************************************************************************/
	
	if( x != x )
		return x + x;
		
	if( x == 0.0L )
		return 1.0L / __builtin_fabsl( x );			//set div/0 return inf

	if( __builtin_fabsl( x ) == __builtin_infl() )
		return __builtin_fabsl( x );
      
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

	if ( x < 0.0L )
	{
		int dummy = 1; 	
		register long double a, y1, fraction;
            
		if ( x <= -twoTo63 ) // big negative integer?
			return x / -0.0L;
                
		y = - x;
		y1 = truncl( y );
		fraction = y - y1; // excess over the boundary
            
		if ( fraction == 0.0L ) // negative integer?
			return x / -0.0L;
		else {
			a = sinl ( pi * fraction );
			if ( y1 == truncl ( y1 * 0.5 ) * 2.0 ) // 0, 2, 4, ...
				{
				*psigngam = -1; /* Gamma(x) < 0 */ 
				} // otherwise leave psigngam = 1
            }
			
		return logl ( pi / __builtin_fabsl ( a * x ) ) - lgammaApproxL ( -x, &dummy );
	}
      
/*******************************************************************************
*     The argument is positive, if it is bigger than xbigger = 2.55e+305 then     *
*     overflow.                                                                *
*******************************************************************************/

	if ( x > xbiggerL )
		return x * 0x1.0p16383L;	//return inf, set overflow

	y = x;

/*******************************************************************************
*     x <= eps then return the approximation log(x).                           *
*******************************************************************************/

	if ( y <= epsL )
		return ( - logl ( y ) );

/*******************************************************************************
*     x is in (eps,1.5] then use d1, p1 and q1 coefficients.                   *
*******************************************************************************/

	else if ( y <= 1.5L )
	{
		if ( y < pnt68L )
		{
			corrector = - logl ( y );
			xMinus1 = y;
		}
		else
	    {
			corrector = 0.0L;
			xMinus1 = ( y - 0.5L ) - 0.5L;
		}
		
		if ( ( y <= 0.5 ) || ( y >= pnt68L ) )
		{
			denominator = 1.0L;
			numerator = 0.0L;
			for ( i = 0; i < 8; i++ )
			{
				numerator = numerator * xMinus1 + p1L[i];
				denominator = denominator * xMinus1 + q1L[i];
			}
			result = corrector + ( xMinus1 * ( d1L + xMinus1 * ( numerator / denominator ) ) );
		}
		else
		{
			xMinus2 = ( y - 0.5L ) - 0.5L;
			denominator = 1.0L;
			numerator = 0.0L;
			for ( i = 0; i < 8; i++ )
			{
				numerator = numerator * xMinus2 + p2L[i];
				denominator = denominator * xMinus2 + q2L[i];
			}
			result = corrector + ( xMinus2 * ( d2L + xMinus2 * ( numerator / denominator ) ) );
		}
	}

/*******************************************************************************
*     x is in (1.5,4.0] then use d2, p2 and q2 coefficients.                   *
*******************************************************************************/

	else if ( y <= 4.0L )
	{
		xMinus2 = y - 2.0L;
		denominator = 1.0L;
		numerator = 0.0L;
		for ( i = 0; i < 8; i++ )
		{
			numerator = numerator * xMinus2 + p2L[i];
			denominator = denominator * xMinus2 + q2L[i];
		}
		result = xMinus2 * ( d2L + xMinus2 * ( numerator / denominator ) );
	}
            
/*******************************************************************************
*     x is in (4.0,12.0] then use d4, p4 and q4 coefficients.                  *
*******************************************************************************/

	else if ( y <= 12.0L )
	{
		xMinus4 = y - 4.0L;
		denominator = - 1.0L;
		numerator = 0.0L;
		for ( i = 0; i < 8; i++ )
		{
			numerator = numerator * xMinus4 + p4L[i];
			denominator = denominator * xMinus4 + q4L[i];
		}
		result = d4L + xMinus4 * ( numerator / denominator );
	}
	else  /* ( y >= 12.0 ) */
	{
		result = 0.0L;
		if ( y <= Root4xbigL )
		{
			result = ccL[6];
			ysquared = y * y;
			for ( i = 0; i < 6; i++ )
				result = result / ysquared + ccL[i];
		}
		result /= y;
		corrector = logl( y );
		result += LogSqrt2piL - 0.5L * corrector;
		result += y * ( corrector - 1.0L );
	}
      
	x = rintl ( x ); // INEXACT set as a side effect for non integer x  
	return result;
}

long double lgammal ( long double x ) //sets signgam as side effect
{
    return lgammaApproxL ( x, &signgam );
}

long double lgammal_r ( long double, int * );
long double lgammal_r ( long double x, int *psigngam )
{
	return lgammaApproxL ( x, psigngam );
}

#pragma mark -

/*******************************************************************************
*     Coefficients for approximation to erf in [ -0.46875, 0.46875] in         *
*     decreasing order.                                                        *
*******************************************************************************/

static const double a[5] = { 3.16112374387056560e+0,
                       1.13864154151050156e+2,
                       3.77485237685302021e+2,
                       3.20937758913846947e+3,
                       1.85777706184603153e-1 };
                       
static const double b[4] = { 2.36012909523441209e+1,
                       2.44024637934444173e+2,
                       1.28261652607737228e+3,
                       2.84423683343917062e+3 };
     
/*******************************************************************************
*     Coefficients for approximation to erfc in [-4.0,-0.46875)U(0.46875,4.0]  *
*     in decreasing order.                                                     *
*******************************************************************************/

static const double ccc[9] = { 5.64188496988670089e-1,
                       8.88314979438837594e+0,
                       6.61191906371416295e+1,
                       2.98635138197400131e+2,
                       8.81952221241769090e+2,
                       1.71204761263407058e+3,
                       2.05107837782607147e+3,
                       1.23033935479799725e+3,
                       2.15311535474403846e-8 };

static const double d[8] = { 1.57449261107098347e+1,
                       1.17693950891312499e+2,
                       5.37181101862009858e+2,
                       1.62138957456669019e+3,
                       3.29079923573345963e+3,
                       4.36261909014324716e+3,
                       3.43936767414372164e+3,
                       1.23033935480374942e+3 };
                       
/*******************************************************************************
*    Coefficients for approximation to  erfc in [-inf,-4.0)U(4.0,inf] in       *
*    decreasing order.                                                         *
*******************************************************************************/

static const double pp[6] = { 3.05326634961232344e-1,
                       3.60344899949804439e-1,
                       1.25781726111229246e-1,
                       1.60837851487422766e-2,
                       6.58749161529837803e-4,
                       1.63153871373020978e-2 };

static const double qq[5] = { 2.56852019228982242e+0,
                       1.87295284992346047e+0,
                       5.27905102951428412e-1,
                       6.05183413124413191e-2,
                       2.33520497626869185e-3 };

static const double InvSqrtPI = 5.6418958354775628695e-1;
static const double xxbig      = 27.2e+0;		
static const double Maximum   = 2.53e+307;
static const double _HUGE      = 6.71e+7;

static inline double ErrFunApprox ( double arg, double result, int which ) ALWAYS_INLINE;

/*******************************************************************************
*              E   R   R   O   R      F   U   N   C   T   I   O   N            *
*******************************************************************************/

double erf ( double x )
{
	register int which = 0;
	register double result = 0.0;
      
/*******************************************************************************
*     The next switch will decipher what sort of argument we have. If argument *
*     is SNaN then a QNaN has to be returned and the invalid flag signaled.    * 
*******************************************************************************/

	if( x != x )
		return x + x;

	if( x == 0.0 )
		return x;

	if( __builtin_fabs(x) == __builtin_inf() )
		return x > 0.0 ? 1.0 : -1.0;

      result = 1.0;
      result = ErrFunApprox ( x, result, which );

/*******************************************************************************
*      Take care of the negative argument.                                     *
*******************************************************************************/

	return x < 0 ? -result : result;
}

float erff( float x )
{
	register int which = 0;
	register float result = 0.0f;
      
/*******************************************************************************
*     The next switch will decipher what sort of argument we have. If argument *
*     is SNaN then a QNaN has to be returned and the invalid flag signaled.    * 
*******************************************************************************/

	if( x != x )
		return x + x;

	if( x == 0.0f )
		return x;

	if( __builtin_fabsf(x) == __builtin_inff() )
		return x > 0.0f ? 1.0f : -1.0f;

      result = 1.0f;
      result = (float) ErrFunApprox ( x, result, which );

/*******************************************************************************
*      Take care of the negative argument.                                     *
*******************************************************************************/

	return x < 0 ? -result : result;
}

/*******************************************************************************
*        C O M P L E M E N T A R Y    E R R O R    F U N C T I O N             *
*******************************************************************************/

double erfc ( double x )
{
	register int which = 1;
	register double result = 0.0;
      
      
/*******************************************************************************
*     The next switch will decipher what sort of argument we have. If argument *
*     is SNaN then a QNaN has to be returned and the invalid flag signaled.    * 
*******************************************************************************/

	if( x != x )
		return x + x;			//silence NaNs
		
	if( x == 0.0 )
		return 1.0;
	
	if( __builtin_fabs(x) == __builtin_inf() )
		return x > 0.0 ? 0.0 : 2.0;

	
	result = 0.0;
	result = ErrFunApprox ( x, result, which );

/*******************************************************************************
*      Take care of the negative argument.                                     *
*******************************************************************************/

	if ( x < 0.0 )
		result = 2.0 - result;
      
	return ( result );
}


float erfcf ( float x )
{
	register int which = 1;
	register float result = 0.0f;
      
      
/*******************************************************************************
*     The next switch will decipher what sort of argument we have. If argument *
*     is SNaN then a QNaN has to be returned and the invalid flag signaled.    * 
*******************************************************************************/

	if( x != x )
		return x + x;			//silence NaNs
		
	if( x == 0.0f )
		return 1.0f;
	
	if( __builtin_fabsf(x) == __builtin_inff() )
		return x > 0.0f ? 0.0f : 2.0f;

	
	result = 0.0f;
	result = (float) ErrFunApprox ( x, result, which );

/*******************************************************************************
*      Take care of the negative argument.                                     *
*******************************************************************************/

	if ( x < 0.0f )
		result = 2.0f - result;
      
	return ( result );
}


/*******************************************************************************
*            C  O  R  E    A  P  P  R  O  X  I  M  A  T  I  O  N               *
*******************************************************************************/

static inline double ErrFunApprox ( double arg, double result, int which )
{
	register int i;
	register double x, y, ysquared, numerator, denominator, del; 

	x = arg;
	y = __builtin_fabs ( x );

/*******************************************************************************
*      Evaluate  erfc  for |x| <= 0.46875.                                     *
*******************************************************************************/

	if ( y <= 0.46875e+0 )
	{
		ysquared = 0.0;
		if ( y > 1.11e-16 )
			ysquared = y * y;
		numerator = a[4] * ysquared;
		denominator = ysquared;
		for ( i = 0; i < 3; i++ )
		{
			numerator = ( numerator + a[i] ) * ysquared;
			denominator = ( denominator + b[i] ) * ysquared;
		}
		result = y * ( numerator + a[3] ) / ( denominator + b[3] );
		if ( which )
			result = 1.0 - result;
		return result;
	}

/*******************************************************************************
*      Evaluate  erfc  for 0.46875 < |x| <= 4.0                                *
*******************************************************************************/
      
	else if ( y <= 4.0 )
	{
		numerator = ccc[8] * y;
		denominator = y;
		for ( i = 0; i < 7; i++ )
		{
			numerator = ( numerator + ccc[i] ) * y;
			denominator = ( denominator + d[i] ) * y;
		}
		result = ( numerator + ccc[7] ) / ( denominator + d[7] );
		ysquared = trunc ( y * 16.0 ) / 16.0;
		del = ( y - ysquared ) * ( y + ysquared );
		result = exp ( - ysquared * ysquared ) * exp ( - del ) * result;
	}

/*******************************************************************************
*      Evaluate  erfc  for |x| > 4.0                                           *
*******************************************************************************/
      
	else
	{
		if ( y >= xxbig )
		{
			if ( ( which != 2 ) || ( y >= Maximum ) )
			{
				if ( which == 1 )
				{
					double oldresult = result;
					result *= 0x1.0000000000001p-1022;
					result *= 0x1.0000000000001p-1022;
					result *= 0x1.0000000000001p-1022;			//set underflow
					result += oldresult;
				}

				return result;
			}
			if ( y >= _HUGE )
			{
				result = InvSqrtPI / y;
				return result;
			}
		}
		ysquared = 1.0 / ( y * y );
		numerator = pp[5] * ysquared;
		denominator = ysquared;
		for ( i = 0; i < 4; i++ )
		{
			numerator = ( numerator + pp[i] ) * ysquared;
			denominator = ( denominator + qq[i] ) * ysquared;
		}
		result = ysquared * ( numerator + pp[4] ) / ( denominator + qq[4] );
		result = ( InvSqrtPI - result ) / y;
		ysquared = trunc ( y * 16.0 ) / 16.0;
		del = ( y - ysquared ) * ( y + ysquared );
		result = exp ( - ysquared * ysquared ) * exp ( - del ) * result;
	}
      
/*******************************************************************************
*     if the calling function is erf instead of erfc, take care of the		 *
*     underserved underflow.  otherwise, the computation will produce the	 *
*	exception for erfc.                                                      *
*******************************************************************************/

		
	return ( which ) ? result : ( 0.5 - result ) + 0.5;
}


/*******************************************************************************
*     Coefficients for approximation to erf in [ -0.46875, 0.46875] in         *
*     decreasing order.                                                        *
*******************************************************************************/

static const long double aL[5] = { 3.16112374387056560e+0L,
								   1.13864154151050156e+2L,
								   3.77485237685302021e+2L,
								   3.20937758913846947e+3L,
								   1.85777706184603153e-1L };
                       
static const long double bL[4] = {  2.36012909523441209e+1L,
								   2.44024637934444173e+2L,
								   1.28261652607737228e+3L,
								   2.84423683343917062e+3L };
				 
/*******************************************************************************
*     Coefficients for approximation to erfc in [-4.0,-0.46875)U(0.46875,4.0]  *
*     in decreasing order.                                                     *
*******************************************************************************/

static const long double cccL[9] = {   5.64188496988670089e-1L,
									   8.88314979438837594e+0L,
									   6.61191906371416295e+1L,
									   2.98635138197400131e+2L,
									   8.81952221241769090e+2L,
									   1.71204761263407058e+3L,
									   2.05107837782607147e+3L,
									   1.23033935479799725e+3L,
									   2.15311535474403846e-8L };

static const long double dL[8] = { 1.57449261107098347e+1L,
								   1.17693950891312499e+2L,
								   5.37181101862009858e+2L,
								   1.62138957456669019e+3L,
								   3.29079923573345963e+3L,
								   4.36261909014324716e+3L,
								   3.43936767414372164e+3L,
								   1.23033935480374942e+3L };
                       
/*******************************************************************************
*    Coefficients for approximation to  erfc in [-inf,-4.0)U(4.0,inf] in       *
*    decreasing order.                                                         *
*******************************************************************************/

static const long double ppL[6] = { 3.05326634961232344e-1L,
								   3.60344899949804439e-1L,
								   1.25781726111229246e-1L,
								   1.60837851487422766e-2L,
								   6.58749161529837803e-4L,
								   1.63153871373020978e-2L };

static const long double qqL[5] = { 2.56852019228982242e+0L,
								   1.87295284992346047e+0L,
								   5.27905102951428412e-1L,
								   6.05183413124413191e-2L,
								   2.33520497626869185e-3L };

static const long double InvSqrtPIL = 5.6418958354775628695e-1L;
static const long double xxbigL      = 108.7;						// a bit larger than sqrt( ln( LDBL_MAX) )
static const long double MaximumL   = ( 2.53e+307L / DBL_MAX ) * LDBL_MAX;
static const long double _HUGEL      = 6.71e+7L;			// This appears to not be used because which is always 0 or 1

static inline long double ErrFunApproxL ( long double arg, long double result, int which ) ALWAYS_INLINE;

static inline long double ErrFunApproxL ( long double arg, long double result, int which ) 
{
	register int i;
	register long double x, y, ysquared, numerator, denominator, del; 

	x = arg;
	y = __builtin_fabsl ( x );

/*******************************************************************************
*      Evaluate  erfc  for |x| <= 0.46875.                                     *
*******************************************************************************/

	if ( y <= 0.46875e+0L )
	{
		ysquared = 0.0L;
		if ( y > LDBL_EPSILON / 2.0L )
			ysquared = y * y;
		numerator = aL[4] * ysquared;
		denominator = ysquared;
		for ( i = 0; i < 3; i++ )
		{
			numerator = ( numerator + aL[i] ) * ysquared;
			denominator = ( denominator + bL[i] ) * ysquared;
		}
		result = y * ( numerator + aL[3] ) / ( denominator + bL[3] );
		if ( which )
			result = 1.0L - result;
		return result;
	}

/*******************************************************************************
*      Evaluate  erfc  for 0.46875 < |x| <= 4.0                                *
*******************************************************************************/
      
	else if ( y <= 4.0 )
	{
		numerator = cccL[8] * y;
		denominator = y;
		for ( i = 0; i < 7; i++ )
		{
			numerator = ( numerator + cccL[i] ) * y;
			denominator = ( denominator + dL[i] ) * y;
		}
		result = ( numerator + cccL[7] ) / ( denominator + dL[7] );
		ysquared = trunc ( y * 16.0L ) / 16.0L;
		del = ( y - ysquared ) * ( y + ysquared );
		result = expl ( - ysquared * ysquared ) * expl ( - del ) * result;
	}

/*******************************************************************************
*      Evaluate  erfc  for |x| > 4.0                                           *
*******************************************************************************/
      
	else
	{
		if ( y >= xxbigL )
		{
			if ( ( which != 2 ) || ( y >= MaximumL ) )
			{
				if ( which == 1 )
				{
					long double oldresult = result;
					result *= 0x1.0000000000001p-16382L;
					result *= 0x1.0000000000001p-16382L;
					result *= 0x1.0000000000001p-16382L;			//set underflow
					result += oldresult;
				}

				return result;
			}
			if ( y >= _HUGEL )
			{
				result = InvSqrtPIL / y;
				return result;
			}
		}
		ysquared = 1.0L / ( y * y );
		numerator = ppL[5] * ysquared;
		denominator = ysquared;
		for ( i = 0; i < 4; i++ )
		{
			numerator = ( numerator + ppL[i] ) * ysquared;
			denominator = ( denominator + qqL[i] ) * ysquared;
		}
		result = ysquared * ( numerator + ppL[4] ) / ( denominator + qqL[4] );
		result = ( InvSqrtPIL - result ) / y;
		ysquared = trunc ( y * 16.0L ) / 16.0L;
		del = ( y - ysquared ) * ( y + ysquared );
		result = expl ( - ysquared * ysquared ) * expl ( - del ) * result;
	}
      
/*******************************************************************************
*     if the calling function is erf instead of erfc, take care of the		 *
*     underserved underflow.  otherwise, the computation will produce the	 *
*	exception for erfc.                                                      *
*******************************************************************************/

		
	return ( which ) ? result : ( 0.5L - result ) + 0.5L;
}

long double erfl ( long double x )
{
	register int which = 0;
	register long double result = 0.0L;
      
/*******************************************************************************
*     The next switch will decipher what sort of argument we have. If argument *
*     is SNaN then a QNaN has to be returned and the invalid flag signaled.    * 
*******************************************************************************/

	if( x != x )
		return x + x;

	if( x == 0.0L )
		return x;

	if( __builtin_fabsl(x) == __builtin_infl() )
		return x > 0.0L ? 1.0L : -1.0L;

      result = 1.0L;
      result = ErrFunApproxL ( x, result, which );

/*******************************************************************************
*      Take care of the negative argument.                                     *
*******************************************************************************/

	return x < 0 ? -result : result;
}


long double erfcl ( long double x )
{
	register int which = 1;
	register long double result = 0.0L;
      
      
/*******************************************************************************
*     The next switch will decipher what sort of argument we have. If argument *
*     is SNaN then a QNaN has to be returned and the invalid flag signaled.    * 
*******************************************************************************/

	if( x != x )
		return x + x;			//silence NaNs
		
	if( x == 0.0L )
		return 1.0L;
	
	if( __builtin_fabsl(x) == __builtin_infl() )
		return x > 0.0L ? 0.0L : 2.0L;

	
	result = 0.0L;
	result = ErrFunApproxL ( x, result, which );

/*******************************************************************************
*      Take care of the negative argument.                                     *
*******************************************************************************/

	if ( x < 0.0L )
		result = 2.0L - result;
      
	return ( result );
}

