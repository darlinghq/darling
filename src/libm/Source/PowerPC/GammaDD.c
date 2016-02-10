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
//	GammaDD.c
//
//	Double-double Function Library
//	Copyright: © 1996 by Apple Computer, Inc., all rights reserved
//	
//	long double gammal( long double x );
//	long double lgammal( long double x );
//

#include "math.h"
#include "fp_private.h"
#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)
#include "DD.h"

// Requires the following functions:
															
void _d3div(double a, double b, double c, double d, double e, double f,
			double *high, double *mid, double *low);
void _d3mul(double a, double b, double c, double d, double e, double f,
			double *high, double *mid, double *low);
void _d3add(double a, double b, double c, double d, double e, double f,
			double *high, double *mid, double *low);

long double _LogInnerLD(double, double, double, double *, int);
long double _ExpInnerLD(double alpha, double beta, double gamma, double *extra,
						int exptype);
long double rintl(long double x);
long double sinl(long double x);

typedef double DD[2];

//  Coefficients for asymptotic formula:
//  real*8 bern(1:2,1:11)
//  FORTRAN: bern(i,j)    C: bern[j][i-1]
//      DD *bern = (DD *)(bernData - 4);

static const uint32_t bernData[] __attribute__ ((aligned(8))) = {
	0x3FB55555, 0x55555555,   0x3C555555, 0x55555555,
	0xBF66C16C, 0x16C16C17,   0x3BFF49F4, 0x9F49F49F,
	0x3F4A01A0, 0x1A01A01A,   0x3B8A01A0, 0x1A01A01A,
	0xBF438138, 0x13813814,   0x3BEFB1FB, 0x1FB1FB20,
	0x3F4B951E, 0x2B18FF23,   0x3BE5C3A9, 0xCE01B952,
	0xBF5F6AB0, 0xD9993C7D,   0x3BFF8255, 0x3C999B0E,
	0x3F7A41A4, 0x1A41A41A,   0x3C106906, 0x90690690,
	0xBF9E4286, 0xCB0F5398,   0x3C21EFCD, 0xAB896745,
	0x3FC6FE96, 0x381E0680,   0xBC279E24, 0x05A71F88,
	0xBFF64767, 0x01181F3A,   0x3C724246, 0x319DA678,
	0x402ACE44, 0x322CE006,   0xBCC62C2B, 0x1BBCDD32
};

//  Coefficients of approximating polynomial for LnGamma(2+x)
//  real*8 zeta(1:2,2:53)
//  FORTRAN: zeta(i,j)    C: zeta[j][i-1]
//      DD *zeta = (DD *)(zetaData - 8);

static const uint32_t zetaData[] __attribute__ ((aligned(8))) = {
	0x3FD4A34C, 0xC4A60FA6,   0x3C71873D, 0x8912200C,
	0xBFB13E00, 0x1A557607,   0x3C5FB68B, 0xE2F8821F,
	0x3F951322, 0xAC7D8483,   0x3C3AFC89, 0x088CB729,
	0xBF7E404F, 0xC218F5F2,   0x3C1E4A62, 0x7CF1EB34,
	0x3F67ADD6, 0xEADB6C30,   0xBBF5B782, 0x8C7FD7F4,
	0xBF538AC5, 0xC2BF8E08,   0x3BE8A4C1, 0xCFD9CEC8,
	0x3F40B36A, 0xF86396E9,   0xBBE0698D, 0x6C892967,
	0xBF2D3FD4, 0xC76D2FC8,   0x3BBC7C55, 0xCFCCBB83,
	0x3F1A127B, 0x0F17D65A,   0x3BA9D309, 0xAA700268,
	0xBF078DE5, 0xBD7C81EF,   0x3B7A2054, 0x1CDE47A6,
	0x3EF580DC, 0xEE66EB02,   0x3B826057, 0x4B258F72,
	0xBEE3CBC9, 0x63CE2243,   0x3B8EA56E, 0x6C7D5329,
	0x3ED2597A, 0x39F34AAC,   0xBB7BF911, 0x462A7D81,
	0xBEC11B2E, 0xB7679541,   0xBB4C76B0, 0xE65AC63A,
	0x3EB0064C, 0xDEB22F0F,   0x3B4D0156, 0xAFFDBC11,
	0xBE9E2600, 0xD93CFD2F,   0x3B3130AC, 0x39E5C106,
	0x3E8C76BB, 0xB3F07A4D,   0x3B2D9A2B, 0x77769B52,
	0xBE7AF5A6, 0xCBBF8A97,   0xBB195F22, 0x7E96D83E,
	0x3E699B93, 0xC2070B0F,   0x3B003271, 0x64736428,
	0xBE5862C7, 0x34DF3EAC,   0xBAFB3280, 0x2BEC0DA0,
	0x3E47469D, 0xACCFADCD,   0xBAE369D3, 0x88CEBAA9,
	0xBE36434A, 0x8447AEAD,   0xBA8AF72E, 0xDF876FCD,
	0x3E2555A8, 0x77FFD2C3,   0xBAC87506, 0x5F26A43B,
	0xBE147B16, 0x79258D0E,   0xBAB04F36, 0xE0E854E4,
	0x3E03B15D, 0x2B2FC10C,   0xBA9D79F6, 0xFEEEB28B,
	0xBDF2F69A, 0x9FABE3E0,   0x3A9A162A, 0xB374C789,
	0x3DE24932, 0xA337434C,   0x3A806082, 0x9C24508F,
	0xBDD1A7C2, 0x6EC2523C,   0xBA74F4EB, 0xDB4A04B5,
	0x3DC11116, 0xE693ED98,   0xBA6C7034, 0xD49E7FC7,
	0xBDB08424, 0xCBC543D8,   0xBA440EF8, 0x20DBC9EA,
	0x3DA00002, 0x6E3F644F,   0x3A43546A, 0x6054C889,
	0xBD8F07C5, 0x14FC9F0A,   0xB9F75B6B, 0xE545AC09,
	0x3D7E1E20, 0x26AAFCD8,   0xBA162A85, 0x86538620,
	0xBD6D41D5, 0x6E5EE2E2,   0x39F43894, 0xD27CED5E,
	0x3D5C71C7, 0xF6F10E37,   0xB9F01074, 0x764D33F2,
	0xBD4BACF9, 0xA27BC89B,   0x39D4A5A2, 0x15E0508E,
	0x3D3AF287, 0x18A10D6E,   0x39C40D7F, 0x1B842CC3,
	0xBD2A41A4, 0x5603E5B6,   0x39C62BE9, 0xCF212D90,
	0x3D199999, 0xC0716EE9,   0xB9B39E10, 0xF90435CB,
	0xBD08F9C1, 0xA8DF9D78,   0x3989DA56, 0xD4471922,
	0x3CF86186, 0x28D28905,   0xB989D7D4, 0xEE5A8893,
	0xBCE7D05F, 0x4C31C560,   0xB9871BBA, 0x0B7CC339,
	0x3CD745D1, 0x7B56BA4A,   0x3979D38B, 0xC00D6F02,
	0xBCC6C16C, 0x1B4D6456,   0xB96AED17, 0x2E5C90F2,
	0x3CB642C8, 0x5C023D9D,   0xB95DE052, 0x190D755D,
	0xBCA5C988, 0x2D825E9D,   0x3949723F, 0x1BF240B7,
	0x3C955555, 0x5698A866,   0x392CF5C8, 0x64970970,
	0xBC84E5E0, 0xA8022BC9,   0x39128B9D, 0xC88F5BE2,
	0x3C747AE1, 0x4838081F,   0xB91DF461, 0x306465C0,
	0xBC641414, 0x146E3E31,   0xB8FE4773, 0xEA1309D6,
	0x3C53B13B, 0x13EC2F3A,   0x38C41C5B, 0x07A32CA9,
	0xBC43521C, 0xFB520859,   0xB8E225B1, 0x0AA3CE51
};

static const double zero     = 0.0;
static const Double pi       = {{0x400921FB, 0x54442D18}};
static const Double pib      = {{0x3CA1A626, 0x33145C07}};
static const Double pic      = {{0xB92F1976, 0xB7ED8FBC}};
static const Double infinity = {{0x7ff00000, 0x00000000}};
static const Double scaleup  = {{0x4ff00000, 0x00000000}};
static const Double tiny     = {{0x0E000000, 0x00000000}};
static const Double sclog    = {{0x40662E42, 0xFEFA39EF}};
static const Double sclog2   = {{0x3CFABC9E, 0x3B39803F}};
static const Double sclog3   = {{0x3987B57A, 0x079A1934}};

static const Double bump     = {{0x3FED67F1, 0xC864BEB5}};	// 0.5 ln (2 Pi)
static const Double bump2    = {{0xBC865B5A, 0x1B7FF5DF}};
static const Double bump3    = {{0xB91B7F70, 0xC13DC168}};

static const Double ec       = {{0x3FDB0EE6, 0x072093CE}};	// 1 - Euler's constant
static const Double ec2      = {{0x3C56CB90, 0x701FBFAB}};
static const Double ec3      = {{0x38F34A95, 0xE3133C51}};

static const Double piln     = {{0x3FF250D0, 0x48E7A1BD}};	// log(pi)
static const Double pilnb    = {{0x3C67ABF2, 0xAD8D5088}};
static const Double pilnc    = {{0xB8E6CCF4, 0x32447B52}};

static const Double zeta32   = {{0xB914C685, 0x28DDC956}};	// Extra word of precision
															// for (zeta(2,1),zeta(2,2))

static long double GammaCommon( double dhead, double dtail, int igamma )
{
	int signgam, ireflect, increase, i;
	
	double int1, int2, arg, arg2, arg3, argt, arg2t, anew;
	double factor, factor1, factor2, factor3, f1, f2, f3;
	double prod1, prod2, pextra, pextra1, pextra2, sum1, sum2, sextra;
	double sum21, sum22, sextra2, sum31, sum32, sextra3;
	double diff1, diff2, dextra, sarg1, sarg2, saextra;
	double denom1, denom2, sum, suml, sumt, recip1, recip2, recipsq1, recipsq2;
	double extra, extra2, extra3, extrax, extray, extralg, gmextra;
	double temp, eexp, diff, z, z2, y, y2, y3, asize, c;
	
	DblDbl gamdd, tempdd, intdd, xhintdd, sinargdd;
	
	DD *bern = (DD *)(bernData - 4);
	DD *zeta = (DD *)(zetaData - 8);

	signgam = 1;

	if (dhead <= 0) {					// special for neg arguments
		tempdd.d[0] = dhead;
		tempdd.d[1] = dtail;
		intdd.f = rintl(tempdd.f);		// get integer part of argument
		int1 = intdd.d[0];
		int2 = intdd.d[1];
		
		if (isinf(dhead) || (dhead - int1 + dtail - int2) == 0) {
			
			//*********************************************************************
			//    Argument is a non-positive integer.  Gamma is not defined.      *
			//    Return a NaN. 		                                          *
			//*********************************************************************
			gamdd.d[0] = INFINITY; // per IEC, [was: zero/zero;]
			gamdd.d[1] = zero;
			return gamdd.f;
		}
		if (dhead > -20.0) {
			ireflect = 0;				// reflection formula not used for
										// modest sized neg numbers
		}
		else {
			ireflect = 1;				// signal use of reflection formula
			dhead = -dhead;				// change sign of argument
			dtail = -dtail;
		}
	}
	else								// else, positive arguments --
		ireflect = 0;					// signal reflection formula not used
	
	if (isinf(dhead) || (dhead + dtail) != (dhead + dtail)) {
		gamdd.d[0] = dhead + dtail;
		gamdd.d[1] = zero;
		return gamdd.f;
	}
	
	arg  = dhead;						// working copy of argument
	arg2 = dtail;
	arg3 = 0.0;
	factor1 = 1.0;
	factor2 = 0.0;
	factor3 = 0.0;
	
	if (arg > 18.0) {					// use asymptotic formula
		while (arg < 31.0) {
			_d3mul( factor1, factor2, factor3, arg, arg2, arg3,
						&factor1, &factor2, &factor3 );
			arg = arg + 1.0;
		}								// argument in range for asympt. formula
		while (arg < 35.0) {
			_d3mul( factor1, factor2, factor3, arg, arg2, arg3,
						&factor1, &factor2, &factor3 );
			argt = __FADD( arg, 1.0 );
			arg2t = __FADD( (arg - argt + 1.0), arg2 );
			arg3 = ((arg - argt + 1.0) - arg2t + arg2) + arg3;
			arg = argt;
			arg2 = arg2t;
		}
		tempdd.f = _LogInnerLD(arg, arg2, 0.0, &f3, 1);		// ln x
		f3 = f3 + arg3/arg;
		f1 = tempdd.d[0];
		f2 = tempdd.d[1];

		_d3mul(f1, f2, f3, (arg - 0.5), arg2, arg3,
					&prod1, &prod2, &c);					// (x-.5)ln x
		_d3add(prod1, prod2, c, -arg, -arg2, -arg3,
					&sum1, &sum2, &sextra);					// (x-.5)ln x-x
		_d3add(sum1, sum2, sextra, bump.f, bump2.f, bump3.f,
					&sum21, &sum22, &sextra2);
		
		//**************************************************************
		//   sum21, sum22, sextra2 represent (x-.5)ln x-x+.5 ln(2 Pi)  *
		//**************************************************************
		
		_d3div(1.0, 0.0, 0.0, arg, arg2, arg3, &recip1, &recip2, &extra);
		
		// argument for asymptotic formula
		_d3mul(recip1, recip2, extra, recip1, recip2, extra,
					&recipsq1, &recipsq2, &extra2);
		
		sum  = bern[11][0];
		suml = bern[11][1];
		for (i = 10; i >= 1; --i) {
			temp = __FMADD( sum, recipsq1, bern[i][0] ); // bern[i][0] + sum*recipsq1;
			/* suml = bern[i][0] - temp + sum*recipsq1 +
			         (bern[i][1] + sum*recipsq2 + suml*recipsq1); */
			suml = __FMADD( sum, recipsq1, bern[i][0] - temp ) +
					__FMADD( suml, recipsq1, __FMADD( sum, recipsq2, bern[i][1] ) );
			sum = temp;
		}								// finish summation of asymptotic series

		_d3mul(sum, suml, 0.0, recip1, recip2, extra, &prod1, &prod2, &extra3);
		_d3add(sum21, sum22, sextra2, prod1, prod2, extra3, &sum31, &sum32, &sextra3);
		
		//******************************************************************************
		//   At this point, log(gamma*factor) is computed as (sum31, sum32, sextra3).  *
		//******************************************************************************
		
		if ((igamma == 1) && (ireflect != 1))  {
			// Gamma entry point (without use of reflection formula)?
			tempdd.f = _ExpInnerLD(sum31, sum32, sextra3, &eexp, 4);
			if (factor1 == 1.0) {
				gamdd.f = tempdd.f;
				gmextra = eexp;
			}
			else {
				_d3div(tempdd.d[0], tempdd.d[1], eexp, factor1, factor2, factor3,
							&(gamdd.d[0]), &(gamdd.d[1]), &gmextra);
			}
		}
		else {							// Computing log(gamma(x))
			factor = factor1;
			if (factor == 1.0) {
				gamdd.d[0] = sum31;
				gamdd.d[1] = sum32;
				gmextra = sextra3;
			}
			else {
				tempdd.f = _LogInnerLD(factor1, factor2, 0.0, &extrax, 1);
				// computing log gamma.
				extray = -(extrax + factor3/factor);
				_d3add(sum31, sum32, sextra3, -tempdd.d[0], -tempdd.d[1], extray,
							&(gamdd.d[0]), &(gamdd.d[1]), &gmextra);
			}
		}
	}
	else {								// use formula for interval [0.5,1.5]
		arg = dhead;
		arg2 = dtail;					// working copy of argument
		arg3 = 0.0;
		increase = 0;					// signal-> no scaling for result
		if (arg < 1.5) {				// scale up argument by recursion formula
			increase = -1;				// signal-> result to be divided by factor
			factor1 = arg;
			factor2 = arg2;
			factor3 = 0.0;				// factor is the old argument
			while (arg < 1.5) {
				_d3add(arg, arg2, arg3, 1.0, 0.0, 0.0, &arg, &arg2, &arg3);
				if (arg < 1.5) {
					_d3mul(factor1, factor2, factor3, arg, arg2, arg3,
								&factor1, &factor2, &factor3);
				}
			}
			if (factor1 < 0.0) {
				signgam = -1;			// special case of negative arguments
			}
		}
		else if (arg > 2.5) {
			increase = +1;				// signal-> result must be mult by factor
			factor1 = 1.0;
			factor2 = 0.0;
			factor3 = 0.0;
			while (arg > 2.5) {						// reduce argument by 1
				arg = arg - 1.0;					// there may be room for bits to
				anew = __FADD( arg, arg2 );			// shift from low order word to
				arg2 = arg - anew + arg2;			// high order word
				arg = anew;
				_d3mul(factor1, factor2, factor3, arg, arg2, 0.0,
								&factor1, &factor2, &factor3);
			}
			arg3 = 0.0;
		}
		diff = __FSUB( arg, 2.0 );					// series in terms of
		z = __FADD( (arg - (diff + 2.0)), arg2 );	// (diff,z,x2)=arg-2
		z2 = (arg - (diff + 2.0)) - z + arg2 + arg3;
		y = __FADD( diff, z );
		y2 = (diff - y + z) + z2;
		y3 = (diff - y + z) - y2 + z2;
		sum  = zeta[53][0];
		suml = zeta[53][1];
		for (i = 52; i >= 40; --i) {
			sum = __FMADD( sum, y, zeta[i][0] ); // zeta[i][0] + sum*y;
		}
		sumt = __FMADD( sum, y, zeta[39][0] ); // zeta[39][0] + sum*y;
		suml = __FMADD( sum, y, zeta[39][0] - sumt) + __FMADD( sum, y2, zeta[39][1] ); // zeta[39][0] - sumt + sum*y + (zeta[39][1] + sum*y2);
		sum = sumt;
		for (i = 38; i >= 3; --i) {
			temp = __FMADD( sum, y, zeta[i][0] ); // zeta[i][0] + sum*y;
			// suml = (zeta[i][0] - temp + sum*y) + zeta[i][1] + (sum*y2 + suml*y);
			suml = __FMADD( sum, y, zeta[i][0] - temp ) +  zeta[i][1] + __FMADD( sum, y2, suml*y ); 
			sum = temp;
		}
		_d3mul(sum, suml, 0.0, y, y2, y3, &prod1, &prod2, &pextra2);
		_d3add(prod1, prod2, pextra2, zeta[2][0], zeta[2][1], zeta32.f,
					&sum1, &sum2, &sextra);
		_d3mul(sum1, sum2, sextra, y, y2, y3,
					&prod1, &prod2, &pextra1);  			// multiply sum by z
		_d3add(prod1, prod2, pextra1, ec.f, ec2.f, ec3.f,
					&sum1, &sum2, &sextra);	 				// add linear term
		_d3mul(sum1, sum2, sextra, y, y2, y3,
					&prod1, &prod2, &pextra);				// final mult. by z.
		
		if (igamma == 1) {			 // a Gamma entry
			tempdd.f = _ExpInnerLD(prod1, prod2, pextra, &eexp, 4);
			if (increase == 1) {
				_d3mul(tempdd.d[0], tempdd.d[1], eexp, factor1, factor2, factor3,
								&(gamdd.d[0]), &(gamdd.d[1]), &gmextra);
			}
			else if (increase == -1) {
				_d3div(tempdd.d[0], tempdd.d[1], eexp, factor1, factor2, factor3,
								&(gamdd.d[0]), &(gamdd.d[1]), &gmextra);
			}
			else {
				gamdd.f = tempdd.f;
				gmextra = eexp;
			}
		}
		else {				// entry was for log gamma
			if (increase == 0) {
				gamdd.d[0] = prod1;
				gamdd.d[1] = prod2;
				gmextra = pextra;
			}
			else {
				if (signgam < 0) {
					factor1 = -factor1;
					factor2 = -factor2;
					factor3 = -factor3;
				}
				factor = factor1;
				tempdd.f = _LogInnerLD(factor1, factor2, 0.0, &extra, 1);
				extra = extra + factor3/factor;
				if (increase == -1) {				// change sign of log
					tempdd.d[0] = -tempdd.d[0];
					tempdd.d[1] = -tempdd.d[1];
					extra = -extra;
				}
				_d3add(prod1, prod2, pextra, tempdd.d[0], tempdd.d[1], extra,
							&(gamdd.d[0]), &(gamdd.d[1]), &gmextra);
			}
		}
	}
	
	if (gamdd.d[0] != gamdd.d[0]) {
		gamdd.d[0] = infinity.f;
		gamdd.d[1] = zero;
		gmextra = 0.0;
	}
	
	if (ireflect == 1) {							// original argument less than 0.0
		arg = -dhead;								// recover original argument
		arg2 = -dtail;
													// reduce argument for computation
		_d3add(arg, arg2, 0.0, -int1, -int2, 0.0, &diff1, &diff2, &dextra);
		asize = fabs(diff1);

		if (asize < tiny.f) {  				 		// For arguments very close
			diff1  *= scaleup.f;					// to an integer, rescale,
			diff2  *= scaleup.f;            		// so that sin can be computed
			dextra *= scaleup.f;					// to a full 107+ bits
		}											// of sin (Pi x)

		_d3mul(diff1, diff2, dextra, pi.f, pib.f, pic.f, &sarg1, &sarg2, &saextra);
		xhintdd.f = 2.0*rintl(0.5*intdd.f);
		
		tempdd.d[0] = sarg1;
		tempdd.d[1] = sarg2;
		sinargdd.f = sinl(tempdd.f);				// sin of argument

		if ((xhintdd.d[0] - intdd.d[0] + xhintdd.d[1] - intdd.d[1]) != 0.0) {
			sinargdd.f = -sinargdd.f;
		}
			
		if (sinargdd.d[0] < 0.0) {
			sinargdd.f = -sinargdd.f;				// force sin(pi x) to have plus sign
			signgam = -signgam;						// show gamma(x) has negative sign
		}

		if (fabs(gamdd.d[0]) == infinity.f){		// result will underflow
			if(igamma == 1) {
				gamdd.d[0] = zero;
				gamdd.d[1] = zero;
			}
			else {
				gamdd.d[0] = -infinity.f;			//  gamma underflows, so
				gamdd.d[1] = zero;					//  lgamma overflows to -INF
			}
			return gamdd.f;
		}
		_d3mul(dhead, dtail, 0.0, sinargdd.d[0], sinargdd.d[1], 0.0,
					&prod1, &prod2, &pextra);		//  x sin(pi x)
		
		tempdd.f = _LogInnerLD(prod1, prod2, 0.0, &extralg, 1);	// log (x sin(pi x))
		extralg = extralg + pextra/prod1;
		_d3add(tempdd.d[0], tempdd.d[1], extralg, gamdd.d[0], gamdd.d[1], gmextra,
					&denom1, &denom2, &dextra);
		_d3add(piln.f, pilnb.f, pilnc.f, -denom1, -denom2, -dextra,
					&(gamdd.d[0]), &(gamdd.d[1]), &gmextra);
		
		if (asize < tiny.f) {
			// Compensate for scaling of argument to sin(pi x)
			_d3add(gamdd.d[0], gamdd.d[1], gmextra, sclog.f, sclog2.f, sclog3.f,
						&(gamdd.d[0]), &(gamdd.d[1]), &gmextra);
		}
		if (igamma == 1) {    // we really want gamma itself ?
			tempdd.f = _ExpInnerLD(gamdd.d[0], gamdd.d[1], gmextra, &eexp, 4);
			
			if (signgam == 1) {
				gamdd.f = tempdd.f;
			}
			else {
				gamdd.f = -tempdd.f;
			}
		}
	}
	return gamdd.f;
}	

long double tgammal( long double x )
{
	double head, fenv;
	DblDbl t;
	
	if (x == 0.0L)
	    return copysignl( INFINITY, x );
	
        FEGETENVD(fenv);
        FESETENVD(0.0);
	
	t.f = x;
	
	t.f = GammaCommon(t.d[0], t.d[1], 1);
	
	head   = __FADD( t.d[0], t.d[1] );
	t.d[1] = (t.d[0] - head) + t.d[1];	//  cannonize tail
	t.d[0] = head;						//  cannonize head
	
        FESETENVD(fenv);
	return t.f;
}

long double lgammal( long double x )
{
	double head, fenv;
	DblDbl t;
	
        FEGETENVD(fenv);
        FESETENVD(0.0);
	
	t.f = x;
	
	t.f = GammaCommon(t.d[0], t.d[1], 0);
	
	head   = __FADD( t.d[0], t.d[1] );
	t.d[1] = (t.d[0] - head) + t.d[1];	//  cannonize tail
	t.d[0] = head;						//  cannonize head
	
        FESETENVD(fenv);
	return t.f;
}
#endif
