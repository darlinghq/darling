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
//
//	AuxiliaryDD.c
//	
//	Double-double Function Library
//	Copyright: © 1995-96 by Apple Computer, Inc., all rights reserved
//	

#include "math.h"
#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)
#include "DD.h"
#include "fp_private.h"
#include "limits.h"

extern long double sqrtl(long double);

#define REM_NAN "9"
#define kSignMask 0x80000000u
#define kExpMask 0x7ff00000u

static const Double kTZ = {{0x0,0x1}};
static const Double kUP = {{0x0,0x2}};
static const Double kDN = {{0x0,0x3}};
static const float kTwoTo52 = 4503599627370496.0f;	// 0x1.0p+52

long double logbl( long double x )
{
	DblDbl xDD;
	double fpenv, sum;
	
	xDD.f = x;

        FEGETENVD(fpenv);
        FESETENVD(kTZ.f);                                                       // round toward zero
	sum = xDD.d[0] + xDD.d[1];						// sum head and tail
        FESETENVD(fpenv);
	
	return logb(sum);
}

int ilogbl( long double x )
{
	DblDbl xDD;
	double fpenv, sum;
	
	xDD.f = x;

        FEGETENVD(fpenv);
        FESETENVD(kTZ.f);                                                       // round toward zero
	sum = xDD.d[0] + xDD.d[1];						// sum head and tail
        FESETENVD(fpenv);
	
	return ilogb(sum);
}

long double scalbnl( long double x, int iscale )
{
	DblDbl xDD;
	double fpenv;
	double Z,z;
	uint32_t hibits;
	
        FEGETENVD(fpenv);
        FESETENVD(0.0);
	xDD.f = x;
	xDD.d[0] = scalbn(xDD.d[0],iscale);					// scale head
	hibits = (xDD.i[0] >> 20) & 0x7ffu;					// biased exp of scaled head
	if ((hibits != 0u) && (hibits != 0x7ffu)) {				// normal case
		z = scalbn(xDD.d[1],iscale);					//   scale tail
		FESETENVD(kTZ.f);                                               // round toward zero
		Z = xDD.d[0] + z;						//   get in canonical form
		xDD.d[1] = (xDD.d[0] - Z) + z;
		xDD.d[0] = Z;
                FESETENVD(fpenv);
		return (xDD.f);							//   return normal result
	}
	
	// head of result is infinite, zero, subnormal, or NaN
	Z = xDD.d[0];
	if ((Z != Z) || (Z == 0.0))						// NaN or zero result
		xDD.d[1] = Z;
	else if (hibits == 0x7ffu)						// infinite result
		xDD.d[1] = 0.0;
	else									// subnormal result
		xDD.d[1] = scalbn(xDD.d[1],iscale);
	
        FESETENVD(fpenv);
	return (xDD.f);								// return result
}

long double scalblnl( long double x, long int n )
{
    int m;
    
    // Clip n
    if (unlikely(n > 2097))
        m = 2098;
    else if (unlikely(n < -2098))
        m = -2099;
    else
        m = (int) n;

    return scalbnl(x, m);
}

// These work just as well for the LP64 ABI
long int lrintl ( long double x )
{
	long double t;
	long int result;
	double fenv;
	
	if (unlikely(x != x))
	{
		feraiseexcept(FE_INVALID);
		return LONG_MIN;
	}
	
	FEGETENVD(fenv);
	t = rintl ( x );
	FESETENVD(fenv);
	
	if ( t < (long double)LONG_MIN )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_MIN;
	}
	else if ( t > (long double)LONG_MAX )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_MAX;
	}
	else if (t != x)
	{
		feraiseexcept(FE_INEXACT);
		result = (long int) t;
	}
	else
	{
		result = (long int) t;
	}
	
	return result;
}

long double hypotl( long double x, long double y )
{
	DblDbl xDD, yDD;
	double fpenv;
	long double ldtemp;
	uint32_t expx, expy;
	const Float kINF = {0x7f800000};
	
        FEGETENVD(fpenv);
        FESETENVD(0.0);

	xDD.f = x;
	yDD.f = y;
	
	// calculate absolute values of x and y
	if (xDD.i[0] >= kSignMask) {
			xDD.i[0] ^= kSignMask;
			xDD.i[2] ^= kSignMask;
	}
	if (yDD.i[0] >= kSignMask) {
			yDD.i[0] ^= kSignMask;
			yDD.i[2] ^= kSignMask;
	}
	
	expx = (xDD.i[0] >> 20) & 0x7ffu;
	expy = (yDD.i[0] >> 20) & 0x7ffu;
	
	// NaN or INF arg(s)
	if ((expx == 0x7ffu) || (expy == 0x7ffu)) {
		if (xDD.d[0] == kINF.f)	 {					// propagate INF
                        FESETENVD(fpenv);
			return (xDD.f);
		}
		else if (yDD.d[0] == kINF.f) {
                        FESETENVD(fpenv);
			return (yDD.f);
		}
		else {
			ldtemp = x + y;
                        FESETENVD(fpenv);
			return ldtemp;						// propagate NaN
		}
	}
	
	// finite arguments at this point
	if (yDD.f > xDD.f) {							// order arguments
		ldtemp = yDD.f;
		yDD.f = xDD.f;
		xDD.f = ldtemp;
	}
	if (yDD.d[0] == 0.0) {							// zero argument(s)
                FESETENVD(fpenv);
		return (xDD.f);
	}
	
	// usual case
	ldtemp = yDD.f / xDD.f;							// (avoids UNDERFLOW)
	ldtemp *= ldtemp;
	ldtemp = xDD.f*sqrtl(1.0L + ldtemp);
	
        FESETENVD(fpenv);
	return (ldtemp);
}


long double truncl( long double x )
{
	DblDbl ldu;
	double xd, result, fpenv;
	
        FEGETENVD(fpenv);
        FESETENVD(0.0);

	ldu.f = x;
	
	if ((ldu.i[0] & kExpMask) == kExpMask) {				// NaN or INF
                FESETENVD(fpenv);
		return (x);
	}
	
	if (fabs(ldu.d[1]) >= kTwoTo52) {					// large integral x
                FESETENVD(fpenv);
		return (x);
	}
	
	// binary point is within or to the left of x's bits
        FESETENVD(kTZ.f);							// round toward zero
	xd = ldu.d[0] + ldu.d[1];						// sum head and tail
	
	if (fabs(xd) < kTwoTo52) {						// binary point in head
		ldu.d[1] = 0.0;							// result tail must be zero
		if (xd >= 0.0)							// round head to integral
			result = (xd + kTwoTo52) - kTwoTo52;
		else
			result = (xd - kTwoTo52) + kTwoTo52;
		if (result == 0.0) {						// preserve sign of zero
			if (ldu.i[0] < kSignMask)
				result = +0.0;
			else {
				result = -0.0;
				ldu.d[1] = -0.0;
			}
		}
		ldu.d[0] = result;
	        FESETENVD(fpenv);						// restore caller's rounding
		return (ldu.f);							// deliver result
	}
	
	// binary point in tail or between head and tail
	if (xd > 0.0)								// round tail to integral
        {
		FESETENVD(kDN.f);
        }
	else
        {
		FESETENVD(kUP.f);
        }
	if (ldu.d[1] >= 0.0)
		result = (ldu.d[1] + kTwoTo52) - kTwoTo52;
	else
		result = (ldu.d[1] - kTwoTo52) + kTwoTo52;
	
	// make result "canonical" in to-nearest rounding (preserves value)
	FESETENVD(0.0);

	xd = ldu.d[0] + result;
	ldu.d[1] = (ldu.d[0] - xd) + result;
	ldu.d[0] = xd;
	
        FESETENVD(fpenv);
	return (ldu.f);								// deliver result
}


long double floorl( long double x )
{
	DblDbl ldu;
	double xd, result, fpenv;
	
        FEGETENVD(fpenv);
        FESETENVD(0.0);

	ldu.f = x;
	
	if ((ldu.i[0] & kExpMask) == kExpMask) {				// NaN or INF
	        FESETENVD(fpenv);
		return (x);
	}
	
	if (fabs(ldu.d[1]) >= kTwoTo52) {					// large integral x
	        FESETENVD(fpenv);
		return (x);
	}
	
	// binary point is within or to the left of x's bits
	FESETENVD(kDN.f);							// round downward
	
	if (fabs(ldu.d[0]) < kTwoTo52) {					// binary point in head
		xd = ldu.d[0] + ldu.d[1];					// sum head and tail of x
		ldu.d[1] = 0.0;							// result tail is zero
		if (ldu.d[0] >= 0.0)						// round to integral
			result = (xd + kTwoTo52) - kTwoTo52;
		else
			result = (xd - kTwoTo52) + kTwoTo52;
		if (result == 0.0) {						// preserve sign of zero
			if (ldu.i[0] < kSignMask)
				result = 0.0;
			else {
				result = -0.0;
				ldu.d[1] = result;
			}
		}
		ldu.d[0] = result;
	        FESETENVD(fpenv);						// restore caller's rounding
		return (ldu.f);							// deliver result
	}
	
	// binary point in tail or between head and tail;
	if (ldu.d[1] >= 0.0)							// round to integral
		result = (ldu.d[1] + kTwoTo52) - kTwoTo52;
	else
		result = (ldu.d[1] - kTwoTo52) + kTwoTo52;
	
	// make result "canonical" in to-nearest rounding (preserves value)
	FESETENVD(0.0);

	xd = ldu.d[0] + result;
	ldu.d[1] = (ldu.d[0] - xd) + result;
	ldu.d[0] = xd;
	
        FESETENVD(fpenv);
	return (ldu.f);								// deliver result
}


long double ceill( long double x )
{
	DblDbl ldu;
	double xd, result, fpenv;
	
        FEGETENVD(fpenv);
        FESETENVD(0.0);
	
	ldu.f = x;
	
	if ((ldu.i[0] & kExpMask) == kExpMask) {				// NaN or INF
	        FESETENVD(fpenv);
		return (x);
	}
	
	if (fabs(ldu.d[1]) >= kTwoTo52) {					// large integral x
	        FESETENVD(fpenv);
		return (x);
	}
	
	// binary point is within or to the left of x's bits
	FESETENVD(kUP.f);							// round upward
	
	if (fabs(ldu.d[0]) < kTwoTo52) {					// binary point in head
		xd = ldu.d[0] + ldu.d[1];					// sum head and tail of x
		ldu.d[1] = 0.0;							// result tail is zero
		if (ldu.d[0] >= 0.0)						// round to integral
			result = (xd + kTwoTo52) - kTwoTo52;
		else
			result = (xd - kTwoTo52) + kTwoTo52;
		if (result == 0.0) {						// preserve sign of zero
			if (ldu.i[0] < kSignMask)
				result = 0.0;
			else {
				result = -0.0;
				ldu.d[1] = result;
			}
		}
		ldu.d[0] = result;
	        FESETENVD(fpenv);
										// restore caller's rounding
		return (ldu.f);							// deliver result
	}
	
	// binary point in tail or between head and tail;
	if (ldu.d[1] >= 0.0)							// round to integral
		result = (ldu.d[1] + kTwoTo52) - kTwoTo52;
	else
		result = (ldu.d[1] - kTwoTo52) + kTwoTo52;
	
	// make result "canonical" in to-nearest rounding (preserves value)
	FESETENVD(0.0);

	xd = ldu.d[0] + result;
	ldu.d[1] = (ldu.d[0] - xd) + result;
	ldu.d[0] = xd;

        FESETENVD(fpenv);							// restore caller's rounding
	return (ldu.f);								// deliver result
}


long double rintl( long double x )
{
	DblDbl ldu;
	double fpenv;
        Double fenv;
	double result,xh,xt;
	uint32_t rnddir;
	
        FEGETENVD(fpenv);
        FESETENVD(0.0);
        fenv.f = fpenv;
	rnddir = fenv.i[1] & FE_ALL_RND;					// isolate round mode
	
	if (rnddir == FE_TONEAREST) {						// default rounding
		ldu.f = x;
		
		if ((ldu.i[0] & kExpMask) == kExpMask) {
                        FESETENVD(fpenv);
			return (x);
		}
		
		if (fabs(ldu.d[1]) >= kTwoTo52) {
                        FESETENVD(fpenv);
			return (x);
		}
		
		// binary point is within or to the left of x's bits; assume x is in
		//   canonical form for default rounding
		xh = ldu.d[0];							// put x in canonical form
		xt = ldu.d[1];
		
		if (fabs(xh) < kTwoTo52) {					// binary point in head
			ldu.d[1] = 0.0;						//   result tail is zero
			if (xh >= 0.0)						//   Rint(head)
				result = (xh + kTwoTo52) - kTwoTo52;
			else
				result = (xh - kTwoTo52) + kTwoTo52;
			
			// Fix up only false halfway cases
			if ((fabs(result - xh) == 0.5) && (xt != 0.0))
				result = (xt > 0.0) ? (xh + 0.5) : (xh - 0.5);
			
			if (result == 0.0) {					// preserve sign of zero
				if (ldu.i[0] < kSignMask) {
					result = 0.0;
				}
				else {
					result = -0.0;
					ldu.d[1] = result;
				}
			}
			ldu.d[0] = result;
                        FESETENVD(fpenv);
			return (ldu.f);						// deliver result
		}								// [binary point in head]
		
		// binary point in tail or between head and tail
		if (xt >= 0.0)							// round to integral
			result = (xt + kTwoTo52) - kTwoTo52;
		else
			result = (xt - kTwoTo52) + kTwoTo52;
		
		ldu.d[0] = xh + result;						// make canonical
		ldu.d[1] = xh - ldu.d[0] + result;
                FESETENVD(fpenv);
		return (ldu.f);							// deliver result
	}									// [default rounding]
	
	// non-default rounding
        FESETENVD(fpenv);
	
	if (rnddir == FE_TOWARDZERO)						// round toward zero
		return (truncl(x));
	else if (rnddir == FE_UPWARD)						// round upward
		return (ceill(x));
	else	/* rnddir == FE_DOWNWARD */					// round downward
		return (floorl(x));
}

long long int llrintl ( long double x )
{
	long double t;
	long long int result;
	double fenv;
	
	if (unlikely(x != x))
	{
		feraiseexcept(FE_INVALID);
		return LONG_LONG_MIN;
	}
	
	FEGETENVD(fenv);
	t = rintl ( x );
	FESETENVD(fenv);
	
	if ( t < (long double)LONG_LONG_MIN )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_LONG_MIN;
	}
	else if ( t > (long double)LONG_LONG_MAX )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_LONG_MAX;
	}
	else if (t != x)
	{
		feraiseexcept(FE_INEXACT);
		result = (long long int) t;
	}
	else
	{
		result = (long long int) t;
	}
	
	return result;
}

long double nearbyintl( long double x )
{
	return (rintl(x));
}


long double roundl( long double x )
{
	DblDbl ldu;
	double xh, xt, xd, result, fpenv;
	
        FEGETENVD(fpenv);
        FESETENVD(0.0);

	ldu.f = x;
	xh = ldu.d[0];
	xt = ldu.d[1];
	
	if ((ldu.i[0] & kExpMask) == kExpMask) {				// NaN or INF
	        FESETENVD(fpenv);
		return (x);
	}
	
	if (fabs(xt) >= kTwoTo52) {						// large integral x
	        FESETENVD(fpenv);
		return (x);
	}
	
	// binary point is within or to the left of x's bits
        FESETENVD(kTZ.f);							// round toward zero
	
	if (fabs(xh) < kTwoTo52) {						// binary point in head
		ldu.d[1] = 0.0;							// result tail is zero
		if (xh >= 0.0) {
			xd = (xt + 0.5) + xh;
			result = (xd + kTwoTo52) - kTwoTo52;
		}
		else {
			xd = (xt - 0.5) + xh;
			result = (xd - kTwoTo52) + kTwoTo52;
		}
		
		if (result == 0.0) {						// preserve sign of zero
			if (ldu.i[0] < kSignMask)
				result = 0.0;
			else {
				result = -0.0;
				ldu.d[1] = result;
			}
		}
		
		ldu.d[0] = result;
	        FESETENVD(fpenv);						// restore caller's env
		return (ldu.f);							// deliver result
	}									// [binary point in head]
	
	// binary point in tail or between head and tail
	if (xh > 0.0) {
		xd = xt + 0.5;
		FESETENVD(kDN.f);
	}
	else {
		xd = xt - 0.5;
		FESETENVD(kUP.f);
	}
	
	if (xd >= 0.0)
		result = (xd + kTwoTo52) - kTwoTo52;
	else
		result = (xd - kTwoTo52) + kTwoTo52;
	
	// make result "canonical" in to-nearest rounding (preserves value)
        FESETENVD(0.0);
	xd = ldu.d[0] + result;
	ldu.d[1] = (ldu.d[0] - xd) + result;
	ldu.d[0] = xd;
        FESETENVD(fpenv);
	return (ldu.f);								// deliver result
}

long long int llroundl ( long double x )
{       
	long double t;
	long long int result;
	double fenv;
	
	if (unlikely(x != x))
	{
		feraiseexcept(FE_INVALID);
		return LONG_LONG_MAX;
	}
	
	FEGETENVD(fenv);
	t = roundl ( x );
	FESETENVD(fenv);
	
	if ( t < (long double)LONG_LONG_MIN )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_LONG_MIN;
	}
	else if ( t > (long double)LONG_LONG_MAX )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_LONG_MAX;
	}
	else if (t != x)
	{
		feraiseexcept(FE_INEXACT);
		result = (long long int) t;
	}
	else
	{
		result = (long long int) t;
	}
	
	return result;
}

long double fdiml(long double x, long double y)
{
	double fenv;
	long double result;
	
	FEGETENVD(fenv);
	FESETENVD(0.0);
	
	if (unlikely((x != x) || (y != y)))
	{
	    FESETENVD(fenv);
	    return ( x + y );
	}
	else if (x > y)
		result = (x - y);
	else
		result = 0.0L;
		
	FESETENVD(fenv);
	return result;
}

long double fmaxl( long double x, long double y )
{
	DblDbl xDD,yDD;
	double fpenv;
	
        FEGETENVD(fpenv);
        FESETENVD(0.0);

	xDD.f = x;
	yDD.f = y;
	
	// filter out NaN arguments
	if (yDD.d[0] != yDD.d[0]) {
	        FESETENVD(fpenv);
		return (x);
	}
	
	if (xDD.d[0] != xDD.d[0]) {
	        FESETENVD(fpenv);
		return (y);
	}
	
	if (y > x) {
	        FESETENVD(fpenv);
		return (y);
	}
	else {
	        FESETENVD(fpenv);
		return (x);
	}
}


long double fminl( long double x, long double y )
{
	DblDbl xDD,yDD;
	double fpenv;
	
        FEGETENVD(fpenv);
        FESETENVD(0.0);
	
	xDD.f = x;
	yDD.f = y;
	
	// filter out NaN arguments
	if (yDD.d[0] != yDD.d[0]) {
	        FESETENVD(fpenv);
		return (x);
	}
	
	if (xDD.d[0] != xDD.d[0]) {
	        FESETENVD(fpenv);
		return (y);
	}
	
	if (y < x) {
	        FESETENVD(fpenv);
		return (y);
	}
	else {
	        FESETENVD(fpenv);
		return (x);
	}
}

long double fmal( long double x, long double y, long double z )
{
	DblDbl xDD, yDD, zDD;
	double th, tm, tl, u;
	
	xDD.f = x;
	yDD.f = y;
	zDD.f = z;
	
	_d3mul( xDD.d[0], xDD.d[1], 0.0, yDD.d[0], yDD.d[1], 0.0, &th, &tm, &tl );
	_d3add( th, tm, tl, zDD.d[0], zDD.d[1], 0.0, &xDD.d[0], &xDD.d[1], &u );
	
	return xDD.f;
}

long double fabsl( long double x )
{
	DblDbl xDD;
	
	xDD.f = x;
	
	if (xDD.i[0] < kSignMask)	// positive x
		return (x);
	
	else {				// negative x
		xDD.i[0] ^= kSignMask;
		xDD.i[2] ^= kSignMask;
		return (xDD.f);
	}
}


long double copysignl( long double x, long double y )
{
	DblDbl xDD,yDD;
	
	xDD.f = x;
	yDD.f = y;
	
	if ((xDD.i[0] & kSignMask) == (yDD.i[0] & kSignMask))
		return (x);
	
	else {	// signs different
		xDD.i[0] ^= kSignMask;
		xDD.i[2] ^= kSignMask;
		return (xDD.f);
	}
}


long rinttoll( long double x )
{
	DblDbl ldu;
	uint32_t expx, lowbit;
	long temp;
	double fpenv;
	
	ldu.f = x;
	expx = (ldu.i[0] >> 20) & 0x7ffu;	// biased exp
	lowbit = ldu.i[1] & 1u;			// low head sig bit
	
	// if x is roughly in range of long format and tail component is nonzero,
	// adjust its head component to contain proper value and sticky information
	// for conversion to long.
	
	if ((expx < 0x41fu) && (expx > 0x3fdu)) {	// 2^32 > |x| > (0.5 - 1 ulp)
		if ((lowbit == 0) && (ldu.d[1] != 0.0)) {
			uint32_t signh = (ldu.i[0] >> 31) & 1u;	// sign of head
			uint32_t signt = (ldu.i[2] >> 31) & 1u;	// sign of tail
			if (signh == signt)			// same sign
				ldu.i[1] |= 1u;
			else {
				ldu.i[1] -= 1u;		// unlike signs
				if (ldu.i[1] == 0xffffffffu)
				ldu.i[0] -= 1u;
			}
		}
	}
        FEGETENVD(fpenv);
        FESETENVD(0.0);
	temp = rinttol(ldu.d[0]);
        FESETENVD(fpenv);

//	return (rinttol(ldu.d[0]));
	return (temp);
}
	    
// These work just as well for the LP64 ABI
long int lroundl( long double x )
{
	long double t;
	long int result;
	double fenv;
	
	if (unlikely(x != x))
	{
		feraiseexcept(FE_INVALID);
		return LONG_MAX;
	}
	
	FEGETENVD(fenv);
	t = roundl ( x );
	FESETENVD(fenv);
	
	if ( t < (long double)LONG_MIN )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_MIN;
	}
	else if ( t > (long double)LONG_MAX )
	{
		feraiseexcept(FE_INVALID);
		result = LONG_MAX;
	}
	else if (t != x)
	{
		feraiseexcept(FE_INEXACT);
		result = (long int) t;
	}
	else
	{
		result = (long int) t;
	}
	
    return result;
}
	    
long double ldexpl(long double x, int n)
{
    int m;
    
    // Clip n
    if (unlikely(n > 2097))
        m = 2098;
    else if (unlikely(n < -2098))
        m = -2099;
    else
        m = n;

    return scalbnl(x, m);
}

long double frexpl(long double x, int *exponent)
{
	DblDbl ldu;
	double xHead;
	double fenv;
	long double result;
	
	ldu.f = x;
	xHead = ldu.d[0];
	
        FEGETENVD(fenv);
        FESETENVD(0.0);
	
	switch (__fpclassifyd(xHead))
	{
		//case FP_SNAN:
		//case FP_QNAN:
		case FP_NAN:
		case FP_INFINITE:
			FESETENVD(fenv);
			return x;
		default:
			*exponent = (x == 0.0) ? 0 : (int)(1.0 + logbl(x));
			result = scalbnl(x, - (*exponent));
			FESETENVD(fenv);
			return result;
	}
}
//
//
//relop relationl(long double x, long double y)
//{
//	double fenv;
//	relop result;
//	
//	fenv = __setflm(0.0);
//	
//	if (( x != x ) || ( y != y ))
//		result = UNORDERED;
//	else if (x == y)
//		result = EQUALTO;
//	else if (x < y)
//		result = LESSTHAN;
//	else
//		result = GREATERTHAN;
//		
//	__setflm(fenv);
//	return result;
//}

long double fmodl(long double xx, long double yy)
{
    int			  iclx,icly;                           /* classify results of x,y */
    int32_t		  iscx,iscy,idiff;                     /* logb values and difference */
    int			  i;                                   /* loop variable */
    long double		  absy,x1,y1,z;                        /* local floating-point variables */
    long double		  rslt;
    fenv_t		  OldEnv;
    hexdouble		  OldEnvironment;
    int			  newexc;
    DblDbl		  xDD, yDD;
    
    xDD.f = xx;
    yDD.f = yy;
    
    FEGETENVD( OldEnvironment.d );
    FESETENVD( 0.0 );
    OldEnv = OldEnvironment.i.lo;
    
    iclx = __fpclassifyd(xDD.d[0]);
    icly = __fpclassifyd(yDD.d[0]);
    if (likely((iclx & icly) >= FP_NORMAL))    {      /* x,y both nonzero finite case */
         x1 = fabsl(xx);                              /* work with absolute values */
         absy = fabsl(yy);
         if (absy > x1) {
              rslt = xx;                               /* trivial case */
	      goto ret;
	 }
         else {                                       /* nontrivial case requires reduction */
              iscx = (int32_t) logbl(x1);             /* get binary exponents of |x| and |y| */
              iscy = (int32_t) logbl(absy);
              idiff = iscx - iscy;                    /* exponent difference */
              if (idiff != 0) {                       /* exponent of x1 > exponent of y1 */
                   y1 = scalbnl(absy,-iscy);          /* scale |y| to unit binade */
                   x1 = scalbnl(x1,-iscx);            /* ditto for |x| */
                   for (i = idiff; i != 0; i--) {     /* begin remainder loop */
                        if ((z = x1 - y1) >= 0) {     /* nonzero remainder step result */
                            x1 = z;                   /*   update remainder (x1) */
                        }
                        x1 += x1;                     /* shift (by doubling) remainder */
                   }                                  /* end of remainder loop */
                   x1 = scalbnl(x1,iscy);               /* scale result to binade of |y| */
              }                                       /* remainder exponent >= exponent of y */
              if (x1 >= absy) {                       /* last step to obtain modulus */
                   x1 -= absy;
              }    
         }                                            /* x1 is |result| */
         if (xx < 0.0)
              x1 = -x1;                               /* modulus if x is negative */
         rslt = x1;
         goto ret;
    }                                                 /* end of x,y both nonzero finite case */
    else if ((iclx <= FP_QNAN) || (icly <= FP_QNAN)) {
         rslt = xx+yy;                                  /* at least one NaN operand */
         goto ret;
      }
    else if ((iclx == FP_INFINITE)||(icly == FP_ZERO)) {    /* invalid result */
         rslt = nanl(REM_NAN);
            OldEnvironment.i.lo |= SET_INVALID;
            FESETENVD ( OldEnvironment.d );
         goto ret;
    }
    else                                              /* trivial cases (finite MOD infinite   */
         rslt = xx;                                    /*  or  zero REM nonzero) with *quo = 0 */
  ret:
    FEGETENVD (OldEnvironment.d );
    newexc = OldEnvironment.i.lo & FE_ALL_EXCEPT;
    OldEnvironment.i.lo = OldEnv;
    if ((newexc & FE_INVALID) != 0)
          OldEnvironment.i.lo |= SET_INVALID;
    OldEnvironment.i.lo |=  newexc & ( FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW );
    FESETENVD (OldEnvironment.d );
    return rslt;
}

#warning remquol: cannot gaurantee exact result!
static const hexdouble Huge		= HEXDOUBLE(0x7ff00000, 0x00000000);
static const hexdouble HugeHalved	= HEXDOUBLE(0x7fe00000, 0x00000000);
long double remquol ( long double x, long double y, int *quo)
{
      int		iclx,icly;			  /* classify results of x,y */
      int32_t		iquo;                             /* low 32 bits of integral quotient */
      int32_t		iscx, iscy, idiff;                /* logb values and difference */
      int		i;                                /* loop variable */
      long double       absy,x1,y1,z;                     /* local floating-point variables */
      long double       rslt;
      fenv_t        OldEnv;
      hexdouble     OldEnvironment;
      int           newexc;

      FEGETENVD ( OldEnvironment.d );
      FESETENVD ( 0.0 );
      __NOOP;
      __NOOP;

      OldEnv = OldEnvironment.i.lo;
      
      *quo = 0;                                       /* initialize quotient result */
      iclx = fpclassify(x);
      icly = fpclassify(y);
      if (likely((iclx & icly) >= FP_NORMAL))    {    /* x,y both nonzero finite case */
         x1 = fabsl(x);                              /* work with absolute values */
         absy = fabsl(y);
         iquo = 0;                                    /* zero local quotient */
         iscx = (int32_t) logbl(x1);                  /* get binary exponents */
         iscy = (int32_t) logbl(absy);
         idiff = iscx - iscy;                         /* exponent difference */
         if (idiff >= 0) {                            /* exponent of x1 >= exponent of y1 */
              if (idiff != 0) {                       /* exponent of x1 > exponent of y1 */
                   y1 = scalbnl(absy,-iscy);         /* scale |y| to unit binade */
                   x1 = scalbnl(x1,-iscx);           /* ditto for |x| */
                   for (i = idiff; i != 0; i--) {     /* begin remainder loop */
                        if ((z = x1 - y1) >= 0) {     /* nonzero remainder step result */
                            x1 = z;                   /* update remainder (x1) */
                            iquo += 1;                /* increment quotient */
                        }
                        iquo += iquo;                 /* shift quotient left one bit */
                        x1 += x1;                     /* shift (double) remainder */
                   }                                  /* end of remainder loop */
                   x1 = scalbnl(x1,iscy);            /* scale remainder to binade of |y| */
              }                                       /* remainder has exponent <= exponent of y */
              if (x1 >= absy) {                       /* last remainder step */
                   x1 -= absy;
                   iquo +=1;
              }                                       /* end of last remainder step */
         }                                            /* remainder (x1) has smaller exponent than y */
         if (likely( x1 < HugeHalved.d ))
            z = scalbnl( x1, 1 ); // z = x1 + x1;                              /* double remainder, without overflow */
         else
            z = Huge.d;
         if ((z > absy) || ((z == absy) && ((iquo & 1) != 0))) {
              x1 -= absy;                             /* final remainder correction */
              iquo += 1;
         }
         if (x < 0.0)
              x1 = -x1;                               /* remainder if x is negative */
         iquo &= 0x0000007f;                          /* retain low 7 bits of integer quotient */
         if ((signbit(x) ^ signbit(y)) != 0)    /* take care of sign of quotient */
              iquo = -iquo;
         *quo = iquo;                                 /* deliver quotient result */
         rslt = x1;
         goto ret;
    }                                                 /* end of x,y both nonzero finite case */
    else if ((iclx <= FP_QNAN) || (icly <= FP_QNAN)) {
         rslt = x+y;                                  /* at least one NaN operand */
         goto ret;
    }
    else if ((iclx == FP_INFINITE)||(icly == FP_ZERO)) {    /* invalid result */
         rslt = nan("9");
            OldEnvironment.i.lo |= SET_INVALID;
            FESETENVD_GRP( OldEnvironment.d );
         goto ret;
    }
    else                                              /* trivial cases (finite REM infinite   */
         rslt = x;                                    /*  or  zero REM nonzero) with *quo = 0 */
  ret:
      FEGETENVD_GRP( OldEnvironment.d );
      newexc = OldEnvironment.i.lo & FE_ALL_EXCEPT;
      OldEnvironment.i.lo = OldEnv;
      if ((newexc & FE_INVALID) != 0)
            OldEnvironment.i.lo |= SET_INVALID;
      OldEnvironment.i.lo |=  newexc & ( FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW );
      FESETENVD_GRP( OldEnvironment.d );
      return rslt;
}

long double remainderl(long double x, long double y)
{
    int quo;
    return (remquol(x, y, &quo));
}

long double modfl ( long double x, long double *iptr )
{
	DblDbl ldu;
	double xh, xt, frach, fract, inth, intt, fpenv;
	long double l;
	
        FEGETENVD(fpenv);
        FESETENVD(0.0);
	
	ldu.f = x;
	xh = ldu.d[0];
	xt = ldu.d[1];
	
	frach = modf( xh, &inth );
	fract = modf( xt, &intt );
	
	*iptr = (long double)(inth + intt);
	l = ((long double)frach) + ((long double)fract);
	if (l >= 1.0L)
	{
	    l = l - 1.0L;
	    *iptr = *iptr + 1.0L;
	}
	

	FESETENVD(fpenv);
	return l;
}

long double nextafterl(long double x, long double y)
{
    static const hexdouble EPSILON = HEXDOUBLE(0x00000000, 0x00000001);
    static const hexdouble PosBigHead  = HEXDOUBLE(0x7fefffff, 0xffffffff);
    static const hexdouble PosBigTail  = HEXDOUBLE(0x7c9fffff, 0xffffffff);
    
    DblDbl ldu;
    double fpenv;
    
    if (unlikely( ( x != x ) || ( y != y ) ))      // one of the arguments is a NaN
	return x + y;
    else if ( x == y )
    {
	if ( ( x ==  0.0L ) && ( y == 0.0L )) // (*0, -0)-> -0, (*0, +0)-> +0 i.e. "y"
	    return y;
	else
	    return x;
    }
    else if (unlikely( isinf( x ) )) // N.B. y is not equal to (infinite) x hence is finite
    {
	ldu.d[0] = PosBigHead.d;
	ldu.d[1] = PosBigTail.d;
	 
	return copysignl( ldu.f, x );
    } 
    else if (x == 0.0L)
    {
	ldu.d[0] = EPSILON.d;
	ldu.d[1] = 0.0L;
	 
	return copysignl( ldu.f, y );
    }
    else if ( ( ( x < 0.0 ) && ( x < y ) ) || ( ( x > 0.0 ) && ( x > y ) ) )
    {
        FEGETENVD(fpenv);
        FESETENVD(kTZ.f);
	x = (x < 0.0L) ? x + (long double)EPSILON.d : x - (long double)EPSILON.d;
        FESETENVD(fpenv);
	return x;
    }
    else if ( ( x < 0.0 ) && ( x > y ) )
    {
        FEGETENVD(fpenv);
        FESETENVD(kDN.f);
	x = x - (long double)EPSILON.d;
        FESETENVD(fpenv);
	return x;
    }
    else // ( ( x > 0.0 ) && ( x < y ) )
    {
        FEGETENVD(fpenv);
        FESETENVD(kUP.f);
	x = x + (long double)EPSILON.d;
        FESETENVD(fpenv);
	return x;
    }
}

float nexttowardf(float x, long double y)
{
	DblDbl ldu;
	double yh, yt, yd, fpenv;
	
	if ((long double)x == y)
	    return (float)y; // 7.12.11.4 requires this behavior
	
        FEGETENVD(fpenv);
        FESETENVD(0.0);

	ldu.f = y;
	yh = ldu.d[0];
	yt = ldu.d[1];
	
	yd = yh + yt;
	FESETENVD(fpenv);
	
	return nextafterf(x, (float)yd);
}

double nexttoward(double x, long double y)
{
	DblDbl ldu;
	double yh, yt, yd, fpenv;
	
	if ((long double)x == y)
	    return (double)y; // 7.12.11.4 requires this behavior

        FEGETENVD(fpenv);
        FESETENVD(0.0);

	ldu.f = y;
	yh = ldu.d[0];
	yt = ldu.d[1];
	
	yd = yh + yt;
	FESETENVD(fpenv);
	
	return nextafter(x, y);
}

long double nexttowardl(long double x, long double y)
{
	if (x == y)
	    return y; // 7.12.11.4 requires this behavior
	else
	    return nextafterl( x, y );
}
#endif

