/*
 *  xmm_misc.c
 *  xmmLibm
 *
 *  Created by Ian Ollmann on 8/18/05.
 *  Copyright 2005 Apple Computer. All rights reserved.
 *
 */

#include "xmmLibm_prefix.h"
#include "math.h"

#if defined(BUILDING_FOR_CARBONCORE_LEGACY)
 
double __inf ( void )
{
      return __builtin_inf();
}

#else /* BUILDING_FOR_CARBONCORE_LEGACY */

/******************************************************************************
*     No other functions are called by these routines outside of fpmacros.c.  *
******************************************************************************/

int __math_errhandling ( void )
{
    return (MATH_ERREXCEPT); // return the bitmask indicating the error discipline(s) in use.
}

int __isfinited( double x )
{
    return __inline_isfinited(x);
}

int __isfinitef( float x )
{
    return __inline_isfinitef(x);
}

int __isfinite( long double x )
{
    return __inline_isfinite(x);
}

//legacy
int finite( double x )
{
	return __isfinited(  x );
}

int __isinfd( double x )
{
    return __inline_isinfd(x);
}

int __isinff( float x )
{
    return __inline_isinff(x);
}

int __isinf( long double x )
{
    return __inline_isinf(x);
}

int __isnand( double x )
{
    return __inline_isnand(x);
}

int __isnanf( float x )
{
    return __inline_isnanf(x);
}

int __isnan( long double x )
{
    return __inline_isnan(x);
}

int __isnormald( double x )
{
    return __inline_isnormald( x );
}

int __isnormalf( float x )
{
    return __inline_isnormalf( x );
}

int __isnormal( long double x )
{
    return __inline_isnormal( x );
}

int __signbitd( double x )
{
	return __inline_signbitd(x);
}

int __signbitf( float x )
{
	return __inline_signbitf(x);
}

int __signbitl( long double x )
{
	return __inline_signbit(x);
}

int __fpclassifyd( double x )
{
    x = __builtin_fabs(x);
    if( EXPECT_FALSE( x == 0.0 ) )
        return FP_ZERO;
        
    if( EXPECT_FALSE( x < 0x1.0p-1022 ) )
        return FP_SUBNORMAL;
    
    if( EXPECT_TRUE( x < __builtin_inf() ) )
        return FP_NORMAL;
        
    if( EXPECT_TRUE( x == __builtin_inf() ) )
        return FP_INFINITE;

    return FP_NAN;
}

int __fpclassifyf( float x )
{
    x = __builtin_fabsf(x);
    if( EXPECT_FALSE( x == 0.0f ) )
        return FP_ZERO;
        
    if( EXPECT_FALSE( x < 0x1.0p-126f ) )
        return FP_SUBNORMAL;
    
    if( EXPECT_TRUE( x < __builtin_inff() ) )
        return FP_NORMAL;
        
    if( EXPECT_TRUE( x == __builtin_inff() ) )
        return FP_INFINITE;

    return FP_NAN;
}

int __fpclassify( long double x )
{
    x = __builtin_fabsl(x);
    if( EXPECT_FALSE( x == 0.0L ) )
        return FP_ZERO;
        
    if( EXPECT_FALSE( x < 0x1.0p-16382L ) )
        return FP_SUBNORMAL;
    
    if( EXPECT_TRUE( x < __builtin_infl() ) )
        return FP_NORMAL;
        
    if( EXPECT_TRUE( x == __builtin_infl() ) )
        return FP_INFINITE;

    return FP_NAN;
}

float __inff( void )
{
    return __builtin_inff();
}

long double __infl( void )
{
    return __builtin_infl();
}

float __nan( void )
{
    return __builtin_nanf("");
}




#endif /* BUILDING_FOR_CARBONCORE_LEGACY */

