/*
 *  remainder.c
 *  xmmLibm
 *
 *  Created by Ian Ollmann on 8/29/05.
 *  Copyright 2005 Apple Computer. All rights reserved.
 *
 *  These functions are based on the algorithms from MathLib V3,
 *  Job Okada.
 */

#include "xmmLibm_prefix.h"
#include "math.h"
#include "fenv.h"

#define REM_NAN "9"

static const double infinityD = __builtin_inf();
static const double minNormD = 0x1.0p-1022;
static const double hugeHalvedD = 0x1.0p1023;
static const double largeD = 0x1.0p1022;
static const double oneD = 1.0;
static const xSInt64 denormBiasD = { 1022LL, 0LL }; 

static const float infinityF = __builtin_inff();
static const float minNormF = 0x1.0p-126f;
static const xSInt32 denormBiasF = { 126, 0,0,0 }; 
static const float oneF = 1.0f;
static const float hugeHalvedF = 0x1.0p127f;

long double __remquol( long double x, long double y, int *quo );

#if defined( BUILDING_FOR_CARBONCORE_LEGACY )

double remquo( double x, double y, int *quo )
{
	return __remquol( x, y, quo );
}

#else

double remainder( double x, double y )
{
	int quo;
	return __remquol( x, y, &quo );
}

float remquof( float x, float y, int *quo )
{
	return __remquol( x, y, quo );
}

float remainderf( float x, float y )
{
	int quo;
	return __remquol( x, y, &quo );
}

#endif /* CARBONCORE_LEGACY */
