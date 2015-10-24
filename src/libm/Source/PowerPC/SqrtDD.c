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
//	SqrtDD.c
//
//	Double-double Function Library
//	Copyright: © 1995-96 by Apple Computer, Inc., all rights reserved
//	
//	long double sqrtl( long double x );
//

#include "math.h"
#include "fp_private.h"
#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)
#include "DD.h"

// sqrttable[256] contains initial estimates for the high eight significand 
// bits for Sqrt(x) and 0.5 times its reciprocal.

static const unsigned short sqrttable[256] = {
       0x6b69, 0x6c68, 0x6e67, 0x6f65, 0x7064, 0x7263, 0x7361, 0x7460,
       0x765f, 0x775d, 0x795c, 0x7a5b, 0x7b5a, 0x7d58, 0x7e57, 0x7f56,
       0x8155, 0x8254, 0x8352, 0x8551, 0x8650, 0x874f, 0x894e, 0x8a4d,
       0x8b4c, 0x8c4b, 0x8e4a, 0x8f48, 0x9047, 0x9246, 0x9345, 0x9444,
       0x9543, 0x9742, 0x9841, 0x9940, 0x9a3f, 0x9c3e, 0x9d3d, 0x9e3c,
       0x9f3c, 0xa13b, 0xa23a, 0xa339, 0xa438, 0xa637, 0xa736, 0xa835,
       0xa934, 0xaa33, 0xac33, 0xad32, 0xae31, 0xaf30, 0xb02f, 0xb12e,
       0xb32e, 0xb42d, 0xb52c, 0xb62b, 0xb72a, 0xb92a, 0xba29, 0xbb28,
       0xbc27, 0xbd26, 0xbe26, 0xbf25, 0xc124, 0xc223, 0xc323, 0xc422,
       0xc521, 0xc621, 0xc720, 0xc81f, 0xca1e, 0xcb1e, 0xcc1d, 0xcd1c,
       0xce1c, 0xcf1b, 0xd01a, 0xd11a, 0xd219, 0xd418, 0xd518, 0xd617,
       0xd716, 0xd816, 0xd915, 0xda14, 0xdb14, 0xdc13, 0xdd13, 0xde12,
       0xdf11, 0xe111, 0xe210, 0xe310, 0xe40f, 0xe50e, 0xe60e, 0xe70d,
       0xe80d, 0xe90c, 0xea0b, 0xeb0b, 0xec0a, 0xed0a, 0xee09, 0xef09,
       0xf008, 0xf108, 0xf207, 0xf306, 0xf406, 0xf505, 0xf605, 0xf704,
       0xf804, 0xf903, 0xfa03, 0xfb02, 0xfc02, 0xfd01, 0xfe01, 0xff00,
       0x00ff, 0x01fd, 0x02fb, 0x03f9, 0x04f7, 0x05f5, 0x06f3, 0x07f2,
       0x08f0, 0x09ee, 0x0aec, 0x0bea, 0x0ce9, 0x0de7, 0x0ee5, 0x0fe4,
       0x10e2, 0x11e0, 0x12df, 0x13dd, 0x14db, 0x15da, 0x16d8, 0x17d7,
       0x17d5, 0x18d4, 0x19d2, 0x1ad1, 0x1bcf, 0x1cce, 0x1dcc, 0x1ecb,
       0x1fc9, 0x20c8, 0x20c6, 0x21c5, 0x22c4, 0x23c2, 0x24c1, 0x25c0,
       0x26be, 0x27bd, 0x27bc, 0x28ba, 0x29b9, 0x2ab8, 0x2bb7, 0x2cb5,
       0x2db4, 0x2db3, 0x2eb2, 0x2fb0, 0x30af, 0x31ae, 0x32ad, 0x33ac,
       0x33aa, 0x34a9, 0x35a8, 0x36a7, 0x37a6, 0x37a5, 0x38a4, 0x39a3,
       0x3aa2, 0x3ba0, 0x3c9f, 0x3c9e, 0x3d9d, 0x3e9c, 0x3f9b, 0x409a,
       0x4099, 0x4198, 0x4297, 0x4396, 0x4495, 0x4494, 0x4593, 0x4692,
       0x4791, 0x4890, 0x488f, 0x498e, 0x4a8d, 0x4b8c, 0x4b8c, 0x4c8b,
       0x4d8a, 0x4e89, 0x4e88, 0x4f87, 0x5086, 0x5185, 0x5284, 0x5283,
       0x5383, 0x5482, 0x5581, 0x5580, 0x567f, 0x577e, 0x587e, 0x587d,
       0x597c, 0x5a7b, 0x5b7a, 0x5b79, 0x5c79, 0x5d78, 0x5d77, 0x5e76,
       0x5f76, 0x6075, 0x6074, 0x6173, 0x6272, 0x6372, 0x6371, 0x6470,
       0x656f, 0x656f, 0x666e, 0x676d, 0x686d, 0x686c, 0x696b, 0x6a6a
      };



//****************************************************************************
// FUNCTION:  long double __sqrtldextra( double x, double y, double *pextra )
//
// DESCRIPTION:  treats x and y as the head and tail, respectively of a long
//               double argument X.  Returns the square root of X rounded to
//               long double format and sets *pextra to a double value which
//               contains a correction factor which contains about 8 extra
//               bits of precision for the square root.  This extra precision
//               is used by the long double ArcCos implementation.
//	      
//              The main square root refinement algorithm is that of P.
//              Markstein, IBM J. R&D, January, 1990, p.117.
//
// ASSUMPTIONS: x and y represent a finite positive long double in canonical
//              form and rounding direction is currently set to the default
//              (IEEE round-to-nearest) mode.
//
//***************************************************************************

long double __sqrtldextra( double x, double y, double *pextra )
{
	DblDbl zDD;
	Double xD, guessD, recipD;
	double scale, guess, recip, diff, recip2, epsilon, guessnew, gsq, guesslow;
	double gsqlow, gg, gmid, gmidlow, recipnew, diff1, diff3, diff3x, diff3a;
	double diff3ax, diff4, diff5, glow2, diff6, diff7, firstfix, fixup;
	uint32_t ghead, reciphead, ival;
	int index;
	
	const Double scaleupD = {{0x5ff00000,0x0}};
	const Double scaledownD = {{0x1ff00000,0x0}};
	const Double adjustupD = {{0x4ff00000,0x0}};
	const Double adjustdownD = {{0x2ff00000,0x0}};
	
	xD.f = x;
	scale = 1.0;
	
	if (xD.i[0] < 0x07000000u) {			// tiny argument
		x *= scaleupD.f;					//   scale up
		y *= scaleupD.f;
		xD.f = x;
		scale = adjustdownD.f;				// final scale factor
	}
	else if (xD.i[0] > 0x7fdfffffu) {		// huge argument
		x *= scaledownD.f;					//   scale down away from
		y *= scaledownD.f;					//   overflow threshold
		xD.f = x;
		scale = adjustupD.f;				//   final scale factor
	}
	
	// estimate exponents of square root and 0.5*reciprocal square root
	ghead = ((xD.i[0] + 0x3ff00000u) >> 1) & 0x7ff00000u;
	reciphead = 0x7fc00000u - ghead;
	
	// initialize initial significand estimates for square root and 0.5*reciprocal
	// from sqrttable entry indexed based upon significand of x and low exponent
	// bit
	index = (int)((xD.i[0] >> 13) & 0xffu);
	ival = (uint32_t) sqrttable[index];
	guessD.i[1] = 0u;
	guessD.i[0] = ghead + ((ival & 0xff00u) << 4);
	recipD.i[0] = reciphead + ((ival & 0xffu) << 12);
	recipD.i[1] = 0u;
	
	guess = guessD.f;
	recip = recipD.f;
	
	// iterate to refine square root and its reciprocal (MAF required)
	diff = __FNMSUB( guess, guess, x );	    // x - guess*guess;
	recip2 = recip + recip;			    // recip2 is approx 1.0/guess
	guess = __FMADD( diff, recip, guess );	    //guess + diff*recip;		// 16-bit square root
	epsilon = __FNMSUB( recip, guess, 0.5 );    // 0.5 - recip*guess;		// begin Newton-Raphson iteration
	diff = __FNMSUB( guess, guess, x );	    // x - guess*guess;
	recip = __FMADD( epsilon, recip2, recip );  // recip + epsilon*recip2;		// 16-bit reciprocal
	guess = __FMADD( recip, diff, guess );	    // guess + recip*diff;		// 32-bit square root
	recip2 = recip + recip;
	epsilon = __FNMSUB( recip, guess, 0.5 );    // 0.5 - recip*guess;
	diff = __FNMSUB( guess, guess, x ) + y;	    // (x - guess*guess) + y;
	recip = __FMADD( epsilon, recip2, recip );					// 32-bit reciprocal
	guessnew = __FMADD( recip, diff, guess );   // guess + recip*diff;		// 53-bit square root
	recip2 = recip + recip;
	gsq = __FMUL( guessnew, guessnew );	    // NOTE: Departure from P. Markstein
						    //   for greater accuracy.
	guesslow = __FMADD( recip, diff, (guess - guessnew) );	    // (guess - guessnew) + recip*diff;
	gsqlow = __FMSUB( guessnew, guessnew, gsq );// guessnew*guessnew - gsq;
	gg = guessnew + guessnew;		    // (guessnew,guesslow)^2 must be
						    //   computed to sextuple precision
	gmid = __FMUL( gg, guesslow );		    //   in order to control errors to
	gmidlow = __FMSUB( gg, guesslow, gmid );    // gg*guesslow - gmid;		//   less than 1/2 ulp
	epsilon = __FNMSUB( recip, guessnew, 0.5 ); // 0.5 - recip*guessnew;
	recip2 = recip + recip;
	recipnew = __FMADD( epsilon, recip2, recip );
	diff1 = x - gsq;			    // exact
	diff3 = __FSUB( diff1, gmid );		    // not necessarily exact
	diff3x = diff1 - diff3 - gmid;		    // usually inexact
	
	diff3a = __FSUB( diff3, gsqlow );	    // not necessarily exact
	diff3ax = diff3 - (diff3a + gsqlow);	    // exact
	diff4 = diff3a + y;			    // usually inexact
	
	diff5 = diff4 - gmidlow;		    // error < ulp/1024
	glow2 = guesslow*guesslow;		    // error < ulp/(2^50)
	diff6 = __FSUB( diff5, glow2 );		    // diff5 - glow2;			// error < ulp/1024
	diff7 = diff6 + (diff3x + diff3ax);	    // error < ulp/1024
	firstfix = recipnew*diff7;							// total error < ulp/256
	fixup = __FADD( guesslow, firstfix );	    // guesslow + firstfix;
	guess = __FADD( guessnew, fixup );	    // put in canonical form
	fixup = guessnew - guess + fixup;
	
	zDD.d[0] = guess*scale;			    // scale result and extra bits;
	zDD.d[1] = fixup*scale;
	*pextra = ((guesslow - fixup) + firstfix)*scale;
	return (zDD.f);				    // return long double square root
}
 
long double sqrtl( long double x )
{
	DblDbl xDD;
	long double y;
	double envD;
	double extra;
	uint32_t hibits;
	
	xDD.f = x;
	hibits = xDD.i[0];				// high 32 bits of head
	
	// Non-trivial case (positive finite x) call __sqrtldextra with default
	//   rounding in effect and discard extra precision bits.
	
	if ((xDD.d[0] != 0.0) && (hibits < 0x7ff00000u)) {
                FEGETENVD(envD);
                FESETENVD(0.0);
		y = __sqrtldextra(xDD.d[0],xDD.d[1],&extra);
		FESETENVD(envD);			// restore caller's env
		return (y);
	}
	
	// Separate edge cases into trivial and exceptional cases
	
	if ((hibits < 0x80000000u) || (xDD.d[0] == 0.0) || (xDD.d[0] != xDD.d[0]))
		return (x);					// zero, +INF, or NaN
	
	// Negative ordered arguments result in NaN;
	xDD.i[0] = 0x7ff80000u;		// return value is NaN
	xDD.i[1] = 0u;
	xDD.i[2] = 0x7ff80000u;
	xDD.i[3] = 0u;
	return (xDD.f);					// return NaN
}

#include "math.h"
long double cbrtl( long double x )
{
	DblDbl xDD;
	double arg, guess;
	long double ldguess, ldguess2, sqldguess;
	const long double oneThird = 0.3333333333333333333333333333333L;
	
	xDD.f = x;
	
	arg = xDD.d[0];
	
	if (arg != arg || arg == 0.0 || arg == INFINITY || -arg == INFINITY)
	    return x;
	    
	guess = cbrt(arg);
	
	xDD.d[0] = guess;
	xDD.d[1] = 0.0;
	
	ldguess = xDD.f;
	
	sqldguess = ldguess * ldguess;
	ldguess2 = ldguess + ldguess;
	
	return (ldguess2 + x/sqldguess)*oneThird;
}
    
#endif
