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
//	ArcHyperbolicDD.c
//	
//	Double-double Function Library
//	Copyright: © 1995-96 by Apple Computer, Inc., all rights reserved
//	
//	long double acoshl(long double x);
//	long double asinhl(long double x);
//	long double atanhl(long double x);
//

#include "math.h"
#include "fp_private.h"
#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)
#include "DD.h"


static const DblDbl kLn2DD = {{0x3fe62e42, 0xfefa39ef,
							  0x3c7abc9e, 0x3b39803f}};
static const Float kInfF   = {0x7f800000};

long double fabsl(long double x);
long double logl(long double x);
long double sqrtl(long double x);
long double log1pl(long double x);
long double copysignl(long double x, long double y);


long double acoshl(long double x)
{
    DblDbl xDD;
    long double y;
    uint32_t xhi;
	double fpenv;
    
    xDD.f = x;
    xhi   = xDD.i[0];

	// return x if x is a NaN or x is +INF
    if ((xDD.d[0] != xDD.d[0]) || (xDD.d[0] == kInfF.f))
		return x;
	
    FEGETENVD(fpenv);
    FESETENVD(0.0);
	
	// calculate ArcCosh for x >= 1.0
    if (x >= 1.0L) {
		if (xhi > 0x5fefffffu) {		// huge case (x >= 2.0^512)
			y = logl(x) + kLn2DD.f;
			FESETENVD(fpenv);
			return y;
		}
		else if (xhi > 0x3ff40000u) {	// intermediate case (x > 1.25)
			y = logl(2.0L*x - 1.0L/(x + sqrtl(x*x -1.0L)));
			FESETENVD(fpenv);
			return y;
		}
		else {							// small case (x <= 1.25)
			y = x - 1.0L;
			y = y + sqrtl(y * (2.0L + y));
			y = log1pl(y);
			FESETENVD(fpenv);
			return y;
		}
    }
    
	xDD.i[0] = 0x7ff80000u;		// return value is NaN
	xDD.i[1] = 0u;
	xDD.i[2] = 0x7ff80000u;
	xDD.i[3] = 0u;
    FESETENVD(fpenv);
	return (xDD.f);					// return NaN
}


long double asinhl(long double x)
{
    DblDbl xDD;
    long double absx, y;
    uint32_t xhi;
	double fpenv;
    
    xDD.f = x;
    absx  = fabsl(x);
    xhi   = xDD.i[0] & 0x7fffffffu;
    
    if (xhi >= 0x7ff00000u)		// NaN or INF is returned
		return x;
	
    if (xhi < 0x29900000u) 		// absx < 2.0^(-358)
		return x;
    
    FEGETENVD(fpenv);
    FESETENVD(0.0);
	
	// x is normal; calculate ArcSinh(x) based on magnitude of |x|
    if (xhi <= 0x3ff55555u) {		// absx <= 4.0/3.0
		y = 1.0L / absx;
		y = log1pl(absx + absx/(y + sqrtl(1.0L + y*y)));
    }
    else if (xhi < 0x5ff00000u)	// absx < 2.0^512
		y = logl(2.0L*absx + 1.0/(absx + sqrtl(1.0L + absx*absx)));
    else 							// absx >= 2.0^512
    	y = logl(absx) + kLn2DD.f;

	y = copysignl(y,x);
    FESETENVD(fpenv);
    return y;						// fix sign of result
}

	
long double atanhl(long double x)
{
    DblDbl xDD;
    long double absx, y;
    uint32_t xhi;
	double fpenv;
    
    xDD.f = x;
    absx  = fabsl(x);			// absx = |x|
    xhi   = xDD.i[0] & 0x7fffffffu;
    
    if (xDD.d[0] != xDD.d[0])	// NaN is returned
		return x;
	
    if (xhi < 0x29900000u) {	// absx < 2.0^(-358)
		return x;
    }
    
    FEGETENVD(fpenv);
    FESETENVD(0.0);
	
    if (absx <= 1.0L) {			// valid argument
		y = 0.5L * log1pl(2.0L*absx / (1.0L - absx));
		y = copysignl(y,x);
        FESETENVD(fpenv);
		return y;
    }
    else {						// invalid argument
		y = log1pl(-absx);
        FESETENVD(fpenv);
    	return y;
	}
}
#endif
