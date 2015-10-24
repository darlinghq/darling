
/*
 *	erf.h
 *
 *		by Ian Ollmann, based on original work by Ali Sazegari, Steve Peters and Jeff Kidder
 *
 *	Copyright (c) 2007 Apple Inc. All Rights Reserved.
 *
 */
 
#include <math.h>

static const double xxbig      = 27.2e+0;		
static const double Maximum   = 2.53e+307;
static const double _HUGE      = 6.71e+7;
static const double InvSqrtPI = 5.6418958354775628695e-1;

static const double a[5] = { 3.16112374387056560e+0,
                       1.13864154151050156e+2,
                       3.77485237685302021e+2,
                       3.20937758913846947e+3,
                       1.85777706184603153e-1 };
                       
static const double b[4] = { 2.36012909523441209e+1,
                       2.44024637934444173e+2,
                       1.28261652607737228e+3,
                       2.84423683343917062e+3 };

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


static inline double ErrFunApprox ( double arg, double result, int which ) __attribute__ ((always_inline));
static inline double ErrFunApprox ( double arg, double result, int which )
{
	register int i;
	register double x, y, ysquared, numerator, denominator, del; 

	x = arg;
	y = __builtin_fabs( x );

/*******************************************************************************
*      Evaluate  erfc  for |x| <= 0.46875.                                     *
*******************************************************************************/

	if ( y <= 0.46875e+0 )
	{
		if ( y > 1.11e-16 )
		{
			ysquared = y * y;
			numerator = a[4] * ysquared;
			denominator = ysquared;
			for ( i = 0; i < 3; i++ )
			{
				numerator = ( numerator + a[i] ) * ysquared;
				denominator = ( denominator + b[i] ) * ysquared;
			}
			result = y * ( numerator + a[3] ) / ( denominator + b[3] );
		}
		else
			result = y * a[3] / b[3];
			
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
