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
//	PowerDD.c
//	
//	Double-double Function Library
//	Copyright: © 1995-96 by Apple Computer, Inc., all rights reserved
//	
//	long double powl( long double base, long double exponent );
//

#include "math.h"
#include "fp_private.h"
#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)
#include "DD.h"

extern long double rintl(long double);


// Floating-point constants

static const Double kInfinityu = {{0x7ff00000,0x0}};

long double _ExpInnerLD(double, double, double, double *, int);
long double _LogInnerLD(double, double, double, double *, int);
static long double PowerInnerLD(double, double, double, double);

long double powl( long double base, long double exponent )
{
	DblDbl u, v;
	double basehi, baselo, exphi, explo;
	int isExpOddInt;
	uint32_t expExp;
	double fpenv;
	long double ldTemp;

	u.f = base;
	basehi = u.d[0];
	baselo = u.d[1];
	v.f = exponent;
	exphi = v.d[0];
	explo = v.d[1];
	
	if (((v.i[0] & 0x000fffffu) | v.i[1] | (v.i[2] & 0x7fffffffu) | v.i[3]) == 0) {
		expExp = v.i[0] & 0xfff00000u;
		if (expExp == 0x40000000) return base*base;     	// if exponent ==  2.0
		else if (exponent == 0.0) return 1.0;           	// if exponent ==  0.0
		else if (expExp == 0x3ff00000) return base;     	// if exponent ==  1.0
		else if (expExp == 0xbff00000) return 1.0/base; 	// if exponent == -1.0
	}
	
        FEGETENVD(fpenv);
        FESETENVD(0.0);
	
	if ((v.i[0] & 0x7ff00000u) < 0x7ff00000u) {			// exponent is finite
		if ((u.i[0] & 0x7ff00000u) < 0x7ff00000u) {   	// base is finite
			if (basehi > 0.0) { 							// base positive
				ldTemp = PowerInnerLD(basehi, baselo, exphi, explo);
                                FESETENVD(fpenv);
				return ldTemp;
			}
			if (basehi < 0.0) { 							// base negative
				if (rintl(exponent) != exponent) {			// exponent is non-integer
					u.d[0] = NAN;
					u.d[1] = 0.0;
                                        FESETENVD(fpenv);
					return u.f;
				}
				u.f = PowerInnerLD(-basehi, -baselo, exphi, explo);
				if (rintl(0.5*exponent) != 0.5*exponent) 	// exponent is odd
					u.f = -u.f;
                                FESETENVD(fpenv);
				return u.f;
			}
			else {											// base is 0.0:
				isExpOddInt = ((rintl(exponent) == exponent) ?
								(rintl(0.5*exponent) != 0.5*exponent) : 0);
				if (exphi > 0.0) {
					ldTemp = (isExpOddInt) ? base : 0.0;
                                        FESETENVD(fpenv);
					return ldTemp;
				}
				else { //   exponent < 0.0
					u.d[0] = (isExpOddInt) ? 1.0/basehi : 1.0/fabs(basehi);
					u.d[1] = 0.0;
                                        FESETENVD(fpenv);
					return u.f;
				}
			}
		}																				// Exponent is finite, Base is not.
		else if (basehi != basehi)							// Base is NaN, return NaN
			return base;		
		else																		// base is +-Inf
		{
			if (basehi > 0.0)											// base is +inf
			{
				if (exphi > 0.0)										// pow(+inf, >0)
					return (long double) INFINITY;
				else																// pow(+inf, <0)
					return 0.0L;
			}
			else																	// base is -inf
			{
				// If exponent is an integer, and exponent/2 is not, then exponent is an odd integer.
				isExpOddInt = ((rintl(exponent) == exponent) && (rintl(0.5*exponent) != 0.5*exponent));
				if (exphi > 0.0) {
					// previously signs were incorrect in this expression; we should be returning
					// -Inf when the exponent is an odd integer, +Inf otherwise.
					ldTemp = (isExpOddInt) ? (long double) -INFINITY : (long double) INFINITY;
                                        FESETENVD(fpenv);
					return ldTemp;
				}
				else { //   exponent < 0.0
					// Return -0 if exponent is an odd integer, +0 otherwise.
					u.d[0] = (isExpOddInt) ? -0.0 : 0.0;
					u.d[1] = 0.0;
                                        FESETENVD(fpenv);
					return u.f;
				}
			}
		}		
	}
	else if (exphi != exphi)	// Exponent is a NaN
	{
		if (base == 1.0L)
			return base;
		else
			return base + exponent;
	}
	else										// exponent is +-Inf
	{
		if (basehi != basehi)
			return base;
	
		ldTemp = fabsl(base);
		if ( (exphi > 0.0 && ldTemp > 1.0L) || (exphi < 0.0 && ldTemp < 1.0L) )
			return (long double) INFINITY;
		else if ( (exphi > 0.0 && ldTemp < 1.0L) || (exphi < 0.0 && ldTemp > 1.0L) )
			return 0.0L;
		else
			return 1.0L;
	}
}

//***************************************************************************
//
//    FUNCTION:  static long double PowerInnerLD(double alpha, double beta,
//               double beta, double gamma);
//
//    DESCRIPTION:  Computes the base to the exponent power where: alpha/beta
//                  are the head/tail of base, and gamma/delta are head/tail
//                  of exponent . This routine is called internally by the 
//                  long double Power function family. It assumes that
//                  base is finite and > 0, exponent is finite.
//
//***************************************************************************/

static long double PowerInnerLD(double alpha, double beta, double gamma, double delta)
{
	DblDbl u;
	double extra, ahi, amid, amid2, alow, amid3;
	double amid4, ahi1, amid5, amid6, ahi2, amid7, amid8;
	
	u.f = _LogInnerLD(alpha, beta, 0.0, &extra, 1);
	ahi = __FMUL( u.d[0], gamma );
	amid = __FMSUB( u.d[0], gamma, ahi );		// u.d[0] * gamma - ahi;
	amid2 = __FMUL( u.d[1], gamma );
	alow = __FMADD( gamma, extra, __FMSUB( u.d[1], gamma, amid2 ) ); // u.d[1] * gamma - amid2 + gamma * extra;
	amid3 = __FMUL( u.d[0], delta );
	alow = __FMADD( delta, u.d[1], __FMSUB( u.d[0], delta, amid3) + alow ); // u.d[0] * delta - amid3 + alow + delta * u.d[1];
	amid4 = __FADD( amid, amid2 );
	ahi1 = __FADD( ahi, amid4 );
	amid5 = ahi - ahi1 + amid4;
	if (fabs(amid) > fabs(amid2))
		alow = amid - amid4 + amid2 + alow;
	else
		alow = amid2 - amid4 + amid + alow;
	amid6 = __FADD( amid3, amid5 );
	ahi2 = __FADD( ahi1, amid6 );
	if (fabs(amid3) > fabs(amid5))
		alow = amid3 - amid6 + amid5 + alow;
	else
		alow = amid5 - amid6 + amid3 + alow;
	amid7 = ahi1 - ahi2 + amid6;
	amid8 = __FADD( amid7, alow );
	alow = amid7 - amid8 + alow;
	if (fabs(ahi) == kInfinityu.f) {
		if (ahi > 0.0) {
			u.d[0] = kInfinityu.f;
			u.d[1] = 0.0;
		}
		else {  
			u.d[0] = 0.0;
			u.d[1] = 0.0;
		}
	}
	else {
		u.f = _ExpInnerLD(ahi2, amid8, alow, &extra, 3);
		if ((u.i[0] & 0x7ff00000) == 0x7ff00000)
			u.d[1] = 0.0;
	}
	return u.f;
}
#endif
