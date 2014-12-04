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
//	ArcSinCosDD.c
//	
//	Double-double Function Library
//	Copyright: © 1995-96 by Apple Computer, Inc., all rights reserved
//	
//	long double asinl( long double x );
//	long double acosl( long double x );    
//

#include "math.h"
#include "fp_private.h"
#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)
#include "DD.h"

long double __sqrtldextra( double x, double y, double *pextra);

// Floating-point constants
static const double kPiBy2 = 1.570796326794896619231322;		// 0x1.921FB54442D18p0   
static const double kPiBy2Tail1  = 6.123233995736766e-17;		// 0x1.1A62633145C07p-54 
static const double kPiBy2Tail2 = -1.4973849048591698e-33;		// 0x1.F1976B7ED8FBCp-110  

static const double kPiBy4 = 7.85398163397448279e-01;			// 0x1.921FB54442D18p-1   
static const double kPiBy4Tail1  = 3.061616997868383018e-17;	// 0x1.1A62633145C07p-55  
static const double kPiBy4Tail2 = -7.486924524295849165e-34;	// 0x1.F1976B7ED8FBCp-111  

static const double k3PiBy4 = 2.356194490192344837;				// 0x1.2D97C7F3321D2p1   
static const double k3PiBy4Tail1  = 9.184850993605148438e-17;	// 0x1.A79394C9E8A0Ap-54  
static const double k3PiBy4Tail2 = 3.916898464750400322e-33;	// 0x1.456737B06EA1Ap-108  

static const double kPi = 3.141592653589793116;					// 0x1.921FB54442D18p1   
static const double kPiTail1  = 1.224646799147353207e-16;		// 0x1.1A62633145C07p-53  
static const double kPiTail2 = -2.994769809718339666e-33;		// 0x1.F1976B7ED8FBCp-109 

static const double kSqrt3By2 = 0.866025403684439;				// 0x1.bb67ae84a8e3ep-1   

static const int	kNterms = 23; 

static const Float  kNaNu = {0x7fc00000};

static const uint32_t asinCoeff[] __attribute__ ((aligned(8))) =  {
	0x3FF00000,		0x00000000,        0x00000000,		0x00000000,
	0x3FC55555,		0x55555555,        0x3C655555,		0x55555555,
	0x3FB33333,		0x33333333,        0x3C499999,		0x9999999A,
	0x3FA6DB6D,		0xB6DB6DB7,        0xBC324924,		0x92492492,
	0x3F9F1C71,		0xC71C71C7,        0x3C1C71C7,		0x1C71C71C,
	0x3F96E8BA,		0x2E8BA2E9,        0xBC31745D,		0x1745D174,
	0x3F91C4EC,		0x4EC4EC4F,        0xBC2D89D8,		0x9D89D89E,
	0x3F8C9999,		0x9999999A,        0xBC299999,		0x9999999A,
	0x3F87A878,		0x78787878,        0x3C2E1E1E,		0x1E1E1E1E,
	0x3F83FDE5,		0x0D79435E,        0x3C2435E5,		0x0D79435E,
	0x3F812EF3,		0xCF3CF3CF,        0x3C1E79E7,		0x9E79E79E,
	0x3F7DF3BD,		0x37A6F4DF,        0xBC190B21,		0x642C8591,
	0x3F7A6863,		0xD70A3D71,        0xBC170A3D,		0x70A3D70A,
	0x3F7782DD,		0xA12F684C,        0xBC02F684,		0xBDA12F68,
	0x3F751BA3,		0x08D3DCB1,        0xBC1CB08D,		0x3DCB08D4,
	0x3F731683,		0xBDEF7BDF,        0xBBE08421,		0x08421084,
	0x3F715EE9,		0xD45D1746,        0xBC0745D1,		0x745D1746,
	0x3F6FCAF8,		0xFB6DB6DB,        0x3C0B6DB6,		0xDB6DB6DB,
	0x3F6D3D2A,		0x8E0DD67D,        0xBC0D67C8,		0xA60DD67D,
	0x3F6B026F,		0x57B13B14,        0xBC03B13B,		0x13B13B14,
	0x3F690CB7,		0x7F60C7CE,        0x3BD8F9C1,		0x8F9C18FA,
	0x3F6750DE,		0x64D7D05F,        0x3C005F41,		0x7D05F418,
	0x3F65C5F5,		0x6EFAAAAB,        0xBC055555,		0x55555555,
	0x3F6464C0,		0x950F7D47,        0xBBF882B9,		0x31057262,
	0x3F632755,		0x86C5F2F0,        0x3C04E5E0,		0xA72F0539,
	0x3F6208D3,		0x570AE5A6,        0xBC069696,		0x96969697,
	0x3F61052B,		0xC5FA960A,        0xBC05BC60,		0x9A90E7D9,
	0x3F6018F9,		0x63C229BF,        0xBBF4F209,		0x4F2094F2,
	0x3F5E82BE,		0x60D9127E,        0xBBEF7047,		0xDC11F704,
	0x3F5CF7DE,		0xA5B6E830,        0xBBF15B1E,		0x5F75270D,
	0x3F5B8D2E,		0x5667CE6C,        0x3BD0C971,		0x4FBCDA3B,
	0x3F5A3F1E,		0xF82137EE,        0x3BEC71C7,		0x1C71C71C,
	0x3F590A9F,		0x747DB95D,        0xBBE6A56A,		0x56A56A57,
	0x3F57ED07,		0x9ED4C037,        0xBBD03D22,		0x6357E16F,
	0x3F56E407,		0x90442038,        0x3BCA6F4D,		0xE9BD37A7,
	0x3F55ED9A,		0x0BD901B6,        0xBBF040E6,		0xC2B4481D,
	0x3F5507F9,		0x4C2470BD,        0x3BE56070,		0x381C0E07,
	0x3F543195,		0xBF54E5D7,        0xBBFB9E4B,		0x17E4B17E,
	0x3F53690E,		0x51F04536,        0x3BF9D974,		0x5D1745D1,
	0x3F52AD29,		0xFCD49D54,        0x3BF82FBF,		0x309B8B57,
	0x3F51FCD2,		0x5AE4A26E,        0x3BECA730,		0xFCD6E9E0,
	0x3F51570F,		0x16ECE10A,        0x3BFA9E4B,		0xF3A9A378,
	0x3F50BB02,		0x0BC1EAA0,        0x3BF8C4B0,		0x78787878,
	0x3F5027E3,		0xF7FCD8BF,        0x3BF67F63,		0x2C234F73,
	0x3F4F3A03,		0x591C2915,        0x3BE22B2B,		0xE05C0B81,
	0x3F4E3373,		0x43F5C1F5,        0x3BEEF13D,		0x589D89D9,
	0x3F4D3AF3,		0xC78CE2E4,        0x3BB8EEC6,		0x4A5294A5,
	0x3F4C4F7C,		0x88F08B5E,        0x3BD4D455,		0x26BCA1AF,
	0x3F4B701D,		0x9E1F038E,        0xBBE2BB23,		0xE09FAB8C,
	0x3F4A9BFC,		0xD93A26FD,        0x3BC437EC,		0xD445D174,
	0x3F49D253,		0x6C99619A,        0xBB9FD25B,		0x093F5DC8  
};

struct asinCoeffEntry { 
   double Head;				
   double Tail;
} asinCoeffEntry;
	
long double asinl( long double x )
{
	double arg, argl, argsq, argsq2;
	double temp,temp2, temp3; 
	double sum, sumt, suml;
	double xHead, xTail;
	double sq, sq2, sq3;
	double res, reslow, resmid, restiny, reshi, resbot, resinf;
	double h, h2, g2, g3, g4, p, q,  t; 
	double prod, prodlow;	
	double fenv;
	DblDbl ddx;
	
	int i;
	uint32_t hibits;
	struct asinCoeffEntry *asinPtr = (struct asinCoeffEntry *) asinCoeff;
	
        FEGETENVD(fenv);
        FESETENVD(0.0);
	
	ddx.f = x;
	
	xHead = ddx.d[0];
	xTail = ddx.d[1];
	
	hibits = ddx.i[0] & 0x7FFFFFFFu;		// highest 32 bits as uint32_t
	
	// This filters most of the invalid cases
	if (hibits > 0x3ff00000u) {			// |x| > 1.0 or NaN
		if (xHead != xHead) {				// x is a NaN
			FESETENVD(fenv);
			return x;
		}
		else {								// |x| > 1.0							
			ddx.d[0] = kNaNu.f;				// NaN 
			ddx.d[1] = ddx.d[0];
			FESETENVD(fenv);
			return (ddx.f);
		}
	}
	// For values of |x| very close to unity, xTail needs to be considered.
	if ((xHead > 0.0) && (xHead -1.0 + xTail > 0.0)) {	// x > 1 out of range
		ddx.d[0] = kNaNu.f;					// NaN 
		ddx.d[1] = ddx.d[0];
                FESETENVD(fenv);
		return (ddx.f);
	}
	if ((xHead < 0.0) && (xHead +1.0 + xTail < 0.0)) {	// x < -1 out of range
		ddx.d[0] = kNaNu.f;					// NaN 
		ddx.d[1] = ddx.d[0];
                FESETENVD(fenv);
		return (ddx.f);
	}
	
	// finite x, with |x| <= 1 
	
	//for tiny x, ArcSin x = x
	if (hibits < 0x3c800000u) {  // 2^(-55);	
                FESETENVD(fenv);
		return x;
	}
	
	
	// finite result but not tiny
	
	//Argument is valid, i.e., |x| <= 1    range reduce
	
	if (hibits <=  0x3fe00000u) {														// |xHead| <= 0.5

		temp = __FMUL( (2.0*xHead), xTail );
		argsq = __FMADD( xHead, xHead, temp );			// temp + xHead*xHead;
		/* argsq2 = xHead*xHead - argsq + temp + ((2.0*xHead)*xTail-temp); */
		argsq2 = __FMSUB( xHead, xHead, argsq ) + temp + __FMSUB( (2.0*xHead), xTail, temp );
		
		sum = asinPtr[50].Head;				// Use Taylor Series
		sum = __FMADD( sum, argsq, asinPtr[49].Head );	// asinPtr[49].Head + sum*argsq;
		sum = __FMADD( sum, argsq, asinPtr[48].Head );	// asinPtr[48].Head + sum*argsq;
		sum = __FMADD( sum, argsq, asinPtr[47].Head );	// asinPtr[47].Head + sum*argsq;
		sum = __FMADD( sum, argsq, asinPtr[46].Head );	// asinPtr[46].Head + sum*argsq;
		
		for (i = 45; i > kNterms+1; i--)												// First 25 terms in working precison
			sum = __FMADD( sum, argsq, asinPtr[i].Head );// asinPtr[i].Head + sum*argsq;        
		
		sumt = __FMADD( sum, argsq, asinPtr[kNterms+1].Head );// asinPtr[kNterms+1].Head + sum*argsq;
		suml = __FMADD( sum, argsq, asinPtr[kNterms+1].Head - sumt );// asinPtr[kNterms+1].Head - sumt + sum*argsq;
		sum = sumt;
		
		for (i = 1; i <= kNterms; i++) {		
			temp = __FMADD( sum, argsq, asinPtr[kNterms-i+1].Head ); // asinPtr[kNterms-i+1].Head + sum*argsq;
			/* suml = asinPtr[kNterms-i+1].Head - temp+sum*argsq +
				(asinPtr[kNterms-i+1].Tail + sum*argsq2 + suml*argsq); */
			suml =  __FMADD( sum, argsq, asinPtr[kNterms-i+1].Head - temp ) +
					__FMADD( suml, argsq, __FMADD( sum, argsq2, asinPtr[kNterms-i+1].Tail ) );
			sum = temp;
		}
		
		prodlow = __FMADD( suml, argsq, sum*argsq2 );	// suml*argsq + sum*argsq2;	// mult. by arg^2
		prod = __FMADD( sum, argsq, prodlow );			// sum*argsq + prodlow;
		prodlow = __FMSUB( sum, argsq, prod ) + prodlow;// sum*argsq - prod+prodlow;
		temp2 = __FMADD( prodlow, xHead, prod*xTail );  // prodlow*xHead + prod*xTail;	// mult. by arg
		temp = __FMADD( prod, xHead, temp2 );			// prod*xHead + temp2;
		temp2 = __FMSUB( prod, xHead, temp ) + temp2;	// prod*xHead - temp+temp2;
		
		res = __FADD( xHead, temp );					// except for argument
		reslow = (xHead - res + temp);					// exact
		resmid = __FADD( xTail, temp2 );
		restiny = xTail - resmid + temp2;
		p = __FADD( reslow, resmid );					// sum of middle terms
		q = reslow - p + resmid;						// possible residual
		reshi = __FADD( res, p );
		resbot = (res - reshi + p) + (q + restiny);
		
		ddx.d[0] = reshi;  
		ddx.d[1] = resbot;
                FESETENVD(fenv);
		return (ddx.f);
	} 
	else if (fabs(xHead) < kSqrt3By2) {	// Pi/6 < |result| < Pi/3
		
		// Use 1-2.*(xHead,xTail)^2 as arg.
		h = __FMUL( xHead, xHead );														// careful computation of
		h2 = __FMSUB( xHead, xHead, h );				// xHead*xHead - h;				// square of original argument
		g2 = __FMUL( (2.0*xHead), xTail );
		g3 = __FMADD( xTail, xTail, __FMSUB( (2.0*xHead), xTail, g2 ) ); // (2.0*xHead)*xTail - g2 + xTail*xTail;
		t = __FADD( h2 , g2 );						//sum of middle parts
		sq = __FADD( h, t );
		
		if (fabs(h2) > fabs(g2))		// More than 107 bits are needed,
			g4 = h2 - t + g2 + g3;			// because the square will be
		else								// subtracted from small constants,
			g4 = g2 - t + h2 + g3;			// causing leading bit cancellations,
											// which must be filled in from the
		sq2 = __FADD( (h - sq + t), g4 );	// right
		sq3 = (h - sq + t) - sq2 + g4;		// This captures extra low order bits
		temp = __FNMSUB( 2.0, sq, 1.0 );	// 1.0 - 2.0 * sq;
		temp2 = __FNMSUB( 2.0, sq2, __FNMSUB( 2.0, sq, 1.0 - temp ) ); // 1.0 - temp - 2.0*sq - 2.0*sq2;
		arg = __FADD( temp, temp2 );
		argl = __FNMSUB( 2.0, sq3, temp - arg + temp2 ); // temp - arg + temp2 - 2.0*sq3;
		temp = 2.0*arg*argl;
		argsq = __FMADD( arg, arg, temp );	// __FADD( arg*arg, temp );				// Square of new argument
		argsq2 = __FMSUB( arg, arg, argsq ) + temp; // arg*arg - argsq + temp;
											// Compute result as
											// Pi/4-0.5*asin(arg,argl)
											// Compute asin with
		sum = asinPtr[50].Head;				// Taylor Series
		
		for (i = 49; i > kNterms+1; i--)		// First 25 terms in working precison
			sum = __FMADD( sum, argsq, asinPtr[i].Head );// asinPtr[i].Head + sum*argsq;	// precison
			
		sumt = __FMADD( sum, argsq, asinPtr[kNterms+1].Head );// asinPtr[kNterms+1].Head + sum*argsq;
		suml = __FMADD( sum, argsq, asinPtr[kNterms+1].Head - sumt );// asinPtr[kNterms+1].Head - sumt + sum*argsq;
		sum = sumt;
		
		for (i = 1; i <= kNterms; i++) {	// remaining terms in quad precision	
			temp = __FMADD( sum, argsq, asinPtr[kNterms-i+1].Head ); // asinPtr[kNterms-i+1].Head + sum*argsq;
			/* suml = asinPtr[kNterms-i+1].Head - temp + sum*argsq +
				(asinPtr[kNterms-i+1].Tail + sum*argsq2 + suml*argsq); */
			suml = __FMADD( sum, argsq, asinPtr[kNterms-i+1].Head - temp ) +
					__FMADD( suml, argsq, __FMADD( sum, argsq2, asinPtr[kNterms-i+1].Tail ) );
			sum = temp;
		}
		prodlow = __FMADD( suml, argsq, sum*argsq2 );	// suml*argsq + sum*argsq2;	// mult. by arg^2
		prod = __FMADD( sum, argsq, prodlow );			// sum*argsq + prodlow;
		prodlow = __FMSUB( sum, argsq, prod ) + prodlow;// sum*argsq - prod+prodlow;
		temp2 = __FMADD( prodlow, arg, prod*argl );  // prodlow*arg + prod*argl;	// mult. by arg
		temp = __FMADD( prod, arg, temp2 );			// prod*arg + temp2;
		temp2 = __FMSUB( prod, arg, temp ) + temp2;	// prod*arg - temp+temp2;
		
		res = __FADD( arg, temp );					// add arg for asin(arg)
		reslow = (arg - res + temp);		// exact
		resmid = __FADD( argl, temp2 );
		restiny = argl - resmid + temp2;
		p = __FADD( reslow, resmid );				// sum of middle terms
		q = reslow - p + resmid;			// possible residual
		reshi = __FADD( res, p );
		resbot = __FADD( (res - reshi + p), (q + restiny) );	//get ready to subtract from pi/4
		resinf = (res - reshi + p) - resbot + (q + restiny);
		temp = kPiBy4;
		temp2 = kPiBy4Tail1;
		temp3 = kPiBy4Tail2;
		res = __FNMSUB( 0.5, reshi, temp ); // temp - 0.5*reshi;
		reslow = __FNMSUB( 0.5, reshi, temp - res ) ; // (temp - res - 0.5*reshi);
		resmid = __FNMSUB( 0.5, resbot, temp2 ); // temp2 - 0.5*resbot;
		if (fabs(temp2) > fabs(0.5*resbot))
			/* restiny = temp2 - resmid - 0.5*resbot + temp3 - 0.5*resinf; */
			restiny = __FNMSUB( 0.5, resbot, temp2 - resmid ) + __FNMSUB( 0.5, resinf, temp3 );
		else
			/* restiny = temp2 - (0.5*resbot + resmid) + temp3 - 0.5*resinf; */
			restiny = temp2 - __FMADD( 0.5, resbot, resmid ) + __FNMSUB( 0.5, resinf, temp3 );
		p = __FADD( reslow, resmid );
		q = reslow - p + resmid;
		reshi = __FADD( res, p );
		resbot = (res - reshi + p) + (q + restiny);
		
		if (xHead > 0.0) {
			ddx.d[0] = reshi;  
			ddx.d[1] = resbot;
		}
		else {
			ddx.d[0] = -reshi;  
			ddx.d[1] = -resbot;
		}
		
                FESETENVD(fenv);
		return (ddx.f);
	}
	else {	// "large arguments",  .866 < Abs(xHead,xTail) <= 1.0 
		
		if ((xHead   - 1.0 + xTail)   == 0.0) {	//for input of 1.0, return Pi/2
			ddx.d[0] = kPiBy2;
			ddx.d[1] = kPiBy2Tail1;
			FESETENVD(fenv);
			return ddx.f;
		}
		if ((xHead   + 1.0 - xTail)   == 0.0) {	//for input of -1.0, return -Pi/2
			ddx.d[0] = - kPiBy2;
			ddx.d[1] = - kPiBy2Tail1;
			FESETENVD(fenv);
			return ddx.f;
		}
		if (xHead < 0.0) {					// Use absolute value of input
			temp  = -xHead;					// to work only in the first
			temp2 = -xTail;					// quadrant
		}
		else { 								// For arg x > 0.5, use identity
			temp  = xHead;					// asin(x)=pi/2-2asin((1-x)/2)^.5)
			temp2 = xTail;
		}
		h = __FNMSUB( 0.5, temp, 0.5 ); // 0.5 - 0.5*temp;					// exact
		argsq = __FNMSUB( 0.5, temp2, h ); // h - 0.5*temp2;
		argsq2 = __FNMSUB( 0.5, temp2, h - argsq ); // __FSUB( h, argsq ) - 0.5*temp2;		//square of reduced arg, exact
		
		ddx.f = __sqrtldextra(argsq, argsq2 , &temp);	// The new argument
		
		sum = asinPtr[25].Head;							// Taylor Series
		
		for (i = 24; i > kNterms/2+1; i--)				// First 25 terms in working precison
			sum = __FMADD( sum, argsq, asinPtr[i].Head );// asinPtr[i].Head + sum*argsq;	// precison
			
		sumt = __FMADD( sum, argsq, asinPtr[kNterms/2+1].Head );// asinPtr[kNterms/2+1].Head + sum*argsq;
		suml = __FMADD( sum, argsq, asinPtr[kNterms/2+1].Head - sumt );// __FSUB( asinPtr[kNterms/2+1].Head, sumt ) + sum*argsq;
		sum = sumt;
		
		for (i = 1; i <= kNterms/2; i++) {	//remaining terms in quad	
			temp = __FMADD( sum, argsq, asinPtr[kNterms/2-i+1].Head ); // asinPtr[kNterms/2-i+1].Head + sum*argsq;
			/* suml = asinPtr[kNterms/2-i+1].Head - temp + sum*argsq +
				(asinPtr[kNterms/2-i+1].Tail + sum*argsq2 + suml*argsq); */
			suml = __FMADD( sum, argsq, asinPtr[kNterms/2-i+1].Head - temp ) +
					__FMADD( suml, argsq, __FMADD( sum, argsq2, asinPtr[kNterms/2-i+1].Tail ) );
			sum = temp;
		}
		
		arg = ddx.d[0];
		argl = ddx.d[1];
		
		prodlow = __FMADD( suml, argsq, sum*argsq2 );	// suml*argsq + sum*argsq2;	// mult. by arg^2
		prod = __FMADD( sum, argsq, prodlow );			// sum*argsq + prodlow;
		prodlow = __FMSUB( sum, argsq, prod ) + prodlow;// sum*argsq - prod+prodlow;
		temp2 = __FMADD( prodlow, arg, prod*argl );		// prodlow*arg + prod*argl;	// multiply by arg
		temp = __FMADD( prod, arg, temp2 );				// prod*arg + temp2;
		temp2 = __FMSUB( prod, arg, temp ) + temp2;		// prod*arg - temp + temp2;
		
		res = __FADD( arg, temp );					// add arg for asin(arg)
		reslow = (arg - res + temp);		// exact
		resmid = __FADD( argl, temp2 );
		restiny = argl - resmid + temp2;
		p = __FADD( reslow, resmid );				// sum of middle terms
		q = reslow - p + resmid;			// possible residual
		reshi = __FADD( res, p );
		resbot = __FADD( (res - reshi + p), (q + restiny) );	//get ready to subtract from pi/4
		resinf = (res - reshi + p) - resbot + (q + restiny);
		
		res = __FNMSUB( 2.0, reshi, kPiBy2 ); // __FSUB( kPiBy2, (2.0*reshi) );
		reslow = __FNMSUB( 2.0, reshi, kPiBy2 - res ); // (kPiBy2 - res - (2.0*reshi));
		resmid = __FNMSUB( 2.0, resbot, kPiBy2Tail1 ); // __FSUB( kPiBy2Tail1, (2.0*resbot) );
		
		if (kPiBy2Tail1 > fabs(2.0*resbot))
			/* restiny = kPiBy2Tail1 - resmid - (2.0*resbot) + (kPiBy2Tail2 - 2.0*resinf); */
			restiny = __FNMSUB( 2.0, resbot, kPiBy2Tail1 - resmid ) + __FNMSUB( 2.0, resinf, kPiBy2Tail2 );
		else
			/* restiny = kPiBy2Tail1 - (resmid + (2.0*resbot)) + (kPiBy2Tail2 - 2.0*resinf); */
			restiny = kPiBy2Tail1 - __FMADD( 2.0, resbot, resmid ) + __FNMSUB( 2.0, resinf, kPiBy2Tail2 );
		p = __FADD( reslow, resmid );
		q = reslow - p+resmid;
		reshi = __FADD( res, p );
		resbot = (res - reshi + p) + (q + restiny);
		if (xHead < 0) {
			ddx.d[0] = -reshi;  
			ddx.d[1] = -resbot;
		}
		else {
			ddx.d[0] = reshi;  
			ddx.d[1] = resbot;
		}
                FESETENVD(fenv);
		return (ddx.f);
	}
}
	
long double acosl( long double x )     
{
	double arg, argl, argsq, argsq2;
	double temp,temp2, temp3; 
	double sum, sumt, suml;
	double xHead, xTail;
	double sq, sq2, sq3;
	double sqrtextra;
	double res, reslow, resmid, restiny, reshi, resbot, resinf;
	double h, h2, g2, g3, g4, p, q,  t; 
	double prod, prodlow;	
	double fenv;
	DblDbl ddx;
	
	int i;
	uint32_t hibits;
	
	struct asinCoeffEntry *asinPtr = (struct asinCoeffEntry *) asinCoeff;
	
        FEGETENVD(fenv);
        FESETENVD(0.0);
	
	ddx.f = x;
	
	xHead = ddx.d[0];
	xTail = ddx.d[1];
	
	hibits = ddx.i[0] & 0x7FFFFFFFu;					// highest 32 bits as uint32_t
	
	// This filters most of the invalid cases (NaNs, inf and |x| > 1)
	
	if (hibits > 0x3ff00000u) {						// |x| > 1.0 or NaN
		if (xHead != xHead) {							// x is a NaN
                        FESETENVD(fenv);
			return x;
		}
		else {											// |x| > 1.0							
			ddx.d[0] = kNaNu.f;							// NaN 
			ddx.d[1] = ddx.d[0];
                        FESETENVD(fenv);
			return (ddx.f);
		}
	}
	
	// For values of |x| very close to unity, xTail needs to be considered.
	
	if ((xHead > 0.0) && (xHead -1.0 + xTail > 0.0)) {	// x > 1 out of range
		ddx.d[0] = kNaNu.f;								// NaN 
		ddx.d[1] = ddx.d[0];
                FESETENVD(fenv);
		return (ddx.f);
	}
	
	if ((xHead < 0.0) && (xHead +1.0 + xTail < 0.0)) {	// x < -1 out of range
		ddx.d[0] = kNaNu.f;								// NaN 
		ddx.d[1] = ddx.d[0];
                FESETENVD(fenv);
		return (ddx.f);
	}
	
	//finite result
	
	// finite tiny x 
	if (hibits < 0x3c800000u) {						// Tiny argument -- |x| < 2^-55
		//subnormal case underflow. Zero must be excluded. 
		res = kPiBy2;
		reslow = -xHead;  
		resmid = __FSUB( kPiBy2Tail1, xTail );  
		restiny = kPiBy2Tail1 - resmid - xTail + kPiBy2Tail2;
		p = __FADD( reslow, resmid );
		reshi = __FADD( res, p );
		if (fabs  (reslow) > fabs  (resmid))
			q = reslow - p + resmid;
		else
			q = resmid - p + reslow;
		resbot = (res-reshi+p) + (q+restiny);
		ddx.d[0] = __FADD( reshi, resbot ); 			
		ddx.d[1] = (reshi - ddx.d[0]) + resbot; 
                FESETENVD(fenv);
		return (ddx.f);
	}
	
	// Argument is valid, i.e., |x| <= 1    range reduce
	
	if (hibits <=  0x3fe00000u) {						// |xHead| <= 0.5
														// in power series for ASIN
		temp = __FMUL( (2.0*xHead), xTail );			// Compute acos(x)=pi/2-asin(x)
		argsq = __FMADD( xHead, xHead, temp );			// temp + xHead*xHead;
		/* argsq2 = xHead*xHead - argsq + temp + ((2.0*xHead)*xTail-temp); */
		argsq2 = __FMSUB( xHead, xHead, argsq ) + temp + __FMSUB( (2.0*xHead), xTail, temp );
		
		sum = asinPtr[50].Head;				// Use Taylor Series
		sum = __FMADD( sum, argsq, asinPtr[49].Head );	// asinPtr[49].Head + sum*argsq;
		sum = __FMADD( sum, argsq, asinPtr[48].Head );	// asinPtr[48].Head + sum*argsq;
		sum = __FMADD( sum, argsq, asinPtr[47].Head );	// asinPtr[47].Head + sum*argsq;
		sum = __FMADD( sum, argsq, asinPtr[46].Head );	// asinPtr[46].Head + sum*argsq;
		
		for (i = 45; i >= kNterms+2; i--)				// First 25 terms in working	
			sum = __FMADD( sum, argsq, asinPtr[i].Head );// asinPtr[i].Head + sum*argsq;        
		
		sumt = __FMADD( sum, argsq, asinPtr[kNterms+1].Head );// asinPtr[kNterms+1].Head + sum*argsq;
		suml = __FMADD( sum, argsq, asinPtr[kNterms+1].Head - sumt );// asinPtr[kNterms+1].Head - sumt + sum*argsq;
		sum = sumt;
		
		for (i = 1; i <= kNterms; i++) {		
			temp = __FMADD( sum, argsq, asinPtr[kNterms-i+1].Head ); // asinPtr[kNterms-i+1].Head + sum*argsq;
			/* suml = asinPtr[kNterms-i+1].Head - temp+sum*argsq +
				(asinPtr[kNterms-i+1].Tail + sum*argsq2 + suml*argsq); */
			suml = __FMADD( sum, argsq, asinPtr[kNterms-i+1].Head - temp ) +
					__FMADD( suml, argsq, __FMADD( sum, argsq2, asinPtr[kNterms-i+1].Tail ) );
			sum = temp;
		}
		
		prodlow = __FMADD( suml, argsq, sum*argsq2 );	// suml*argsq + sum*argsq2;	// mult. by arg^2
		prod = __FMADD( sum, argsq, prodlow );			// sum*argsq + prodlow;
		prodlow = __FMSUB( sum, argsq, prod ) + prodlow;// sum*argsq - prod+prodlow;
		temp2 = __FMADD( prodlow, xHead, prod*xTail );  // prodlow*xHead + prod*xTail;	// mult. by arg
		temp = __FMADD( prod, xHead, temp2 );			// prod*xHead + temp2;
		temp2 = __FMSUB( prod, xHead, temp ) + temp2;	// prod*xHead - temp+temp2;
		
		res = __FADD( xHead, temp );					// add argument
		reslow = (xHead - res + temp);
		resmid = __FADD( xTail, temp2 );
		restiny = xTail - resmid + temp2;
		p = __FADD( reslow, resmid );					// sum of middle terms
		q = reslow - p + resmid;						// possible residual
		reshi = __FADD( res, p );
		resbot = __FADD( (res - reshi + p),  (q + restiny) );
		resinf = (res - reshi + p) - resbot + (q + restiny);
		
		res = __FSUB( kPiBy2, reshi );							// and subtract from Pi/2
		reslow = kPiBy2 - res - reshi;
		resmid = __FSUB( kPiBy2Tail1, resbot );
		
		if (kPiBy2Tail1 > fabs (resbot)) 
			restiny = kPiBy2Tail1 - resmid - resbot + (kPiBy2Tail2 - resinf);
		else
			restiny = kPiBy2Tail1 - (resmid + resbot) + (kPiBy2Tail2 - resinf);
		p = __FADD( reslow, resmid );
		q = reslow - p + resmid;
		reshi = __FADD( res, p );
		resbot = (res - reshi + p) + (q + restiny);
		
		ddx.d[0] = __FADD( reshi, resbot ); 			
		ddx.d[1] = (reshi - ddx.d[0]) + resbot; 
		
                FESETENVD(fenv);
		return (ddx.f);
	}
	
	
	if (fabs (xHead) < kSqrt3By2) {   // Pi/6 < |result| < Pi/3
		//Use 2.*(xHead  ,xTail  )^2-1 as arg.
		h = __FMUL( xHead, xHead );														// careful computation of
		h2 = __FMSUB( xHead, xHead, h );				// xHead*xHead - h;				// square of original argument
		g2 = __FMUL( (2.0*xHead), xTail );
		g3 = __FMADD( xTail, xTail, __FMSUB( (2.0*xHead), xTail, g2 ) ); // (2.0*xHead)*xTail - g2 + xTail*xTail;
		t = __FADD( h2 , g2 );						//sum of middle parts
		sq = __FADD( h, t );
		
		if (fabs  (h2) > fabs  (g2))				// More than 107 bits are needed,
			g4 = h2 - t + g2 + g3;						// because the square will be
		else											// subtracted from small constants,
			g4 = g2 - t + h2 + g3;						// causing leading bit cancel-
														//  lations, which must be filled
		sq2 = __FADD( (h - sq + t), g4 );				// in from the right
		sq3 = (h - sq + t) - sq2 + g4;					// This captures extra l.o. bits
		temp = __FMSUB( 2.0, sq, 1.0 );	// 2.0*sq - 1.0;
		temp2 = __FMADD( 2.0, sq2, __FMSUB( 2.0, sq, 1.0 + temp ) ); // 2.0*sq - (1.0 + temp) + 2.0*sq2;
		arg = __FADD( temp, temp2 );
		argl = __FMADD( 2.0, sq3, temp - arg + temp2 ); // temp - arg + temp2 + 2.0*sq3;
		temp = 2.0*arg*argl;
		argsq = __FMADD( arg, arg, temp ); // arg*arg + temp;	// Square of new argument
		argsq2 = __FMSUB( arg, arg, argsq ) + temp; // arg*arg - argsq + temp;
														// Compute result as
														// Pi/4-0.5*asin(arg,argl)
														// Compute asin with
		sum = asinPtr[50].Head;							// Taylor Series
		
		for (i = 49;i >= kNterms+2;i--)					// First half(approx) terms in		
			sum = __FMADD( sum, argsq, asinPtr[i].Head );// asinPtr[i].Head + sum*argsq;	// precison
			
		sumt = __FMADD( sum, argsq, asinPtr[kNterms+1].Head );// asinPtr[kNterms+1].Head + sum*argsq;
		suml = __FMADD( sum, argsq, asinPtr[kNterms+1].Head - sumt );// asinPtr[kNterms+1].Head - sumt + sum*argsq;
		sum = sumt;
		
		for (i = 1;i <= kNterms;i++) {					// remaining terms in quad prec.		
			temp = __FMADD( sum, argsq, asinPtr[kNterms-i+1].Head ); // asinPtr[kNterms-i+1].Head + sum*argsq;
			/* suml = asinPtr[kNterms-i+1].Head - temp + sum*argsq +
				(asinPtr[kNterms-i+1].Tail + sum*argsq2 + suml*argsq); */
			suml = __FMADD( sum, argsq, asinPtr[kNterms-i+1].Head - temp ) +
					__FMADD( suml, argsq, __FMADD( sum, argsq2, asinPtr[kNterms-i+1].Tail ) );
			sum = temp;
		}
		
		prodlow = __FMADD( suml, argsq, sum*argsq2 );	// suml*argsq + sum*argsq2;	// mult. by arg^2
		prod = __FMADD( sum, argsq, prodlow );			// sum*argsq + prodlow;
		prodlow = __FMSUB( sum, argsq, prod ) + prodlow;// sum*argsq - prod+prodlow;
		temp2 = __FMADD( prodlow, arg, prod*argl );  // prodlow*arg + prod*argl;	// mult. by arg
		temp = __FMADD( prod, arg, temp2 );			// prod*arg + temp2;
		temp2 = __FMSUB( prod, arg, temp ) + temp2;	// prod*arg - temp+temp2;
		
		res = __FADD( arg, temp );						// add arg for asin(arg)
		reslow = (arg - res + temp);					// exact
		resmid = __FADD( argl, temp2 );
		restiny = argl - resmid + temp2;
		p = __FADD( reslow, resmid );					// sum of middle terms
		q = reslow - p + resmid;						// possible residual
		reshi = __FADD( res, p );
		resbot = __FADD( (res - reshi + p), (q + restiny) ); // prepare to subtract from pi/4
		resinf = (res - reshi + p) - resbot + (q + restiny);
		
		if (xHead>0.0) {
			temp = kPiBy4;								// positive argument
			temp2 = kPiBy4Tail1;						// acos(x)=pi/4-.5 asin(arg,arg1)
			temp3 = kPiBy4Tail2;
		}
		else {
			temp = k3PiBy4;								// negative argument
			temp2 = k3PiBy4Tail1;						// acos(x)=3pi/4 +
			temp3 = k3PiBy4Tail2;						// 0.5 asin(arg,argl)
			reshi = -reshi;
			resbot = -resbot;
			resinf = -resinf;
		}
		
		res = __FNMSUB( 0.5, reshi, temp ); // temp - 0.5*reshi;
		reslow = __FNMSUB( 0.5, reshi, temp - res ) ; // (temp - res - 0.5*reshi);
		resmid = __FNMSUB( 0.5, resbot, temp2 ); // temp2 - 0.5*resbot;
		
		if (fabs  (temp2) > fabs  (0.5*resbot)) {
			/* restiny = temp2 - resmid - 0.5*resbot + temp3 - 0.5*resinf; */
			restiny = __FNMSUB( 0.5, resbot, temp2 - resmid ) + __FNMSUB( 0.5, resinf, temp3 );
		}
		else {
			/* restiny = temp2 - (0.5*resbot + resmid) + temp3 - 0.5*resinf; */
			restiny = temp2 - __FMADD( 0.5, resbot, resmid ) + __FNMSUB( 0.5, resinf, temp3 );
		}
		
		p = __FADD( reslow, resmid );
		q = reslow - p + resmid;
		reshi = __FADD( res, p );
		resbot = (res - reshi + p) + (q + restiny);
		
		ddx.d[0] = __FADD( reshi, resbot ); 			
		ddx.d[1] = (reshi - ddx.d[0]) + resbot; 
		
                FESETENVD(fenv);
		return (ddx.f);
	}
	else {	// "large arguments",  .866 < abs(xHead,xTail) <= 1.0
		
		if ((xHead - 1.0 + xTail) == 0.0) {				// for input of 1.0, return zero
                        FESETENVD(fenv);
			return (long double) 0.0;
		}
		
		if ((xHead + 1.0 - xTail) == 0.0) {				// for input of -1.0, return Pi
			ddx.d[0] = kPi;
			ddx.d[1] = kPiTail1;
                        FESETENVD(fenv);
			return ddx.f;
		}
		
		if (xHead < 0.0) {								// Use absolute value of input
			temp = -xHead;								// to work only in the first
			temp2 = -xTail;								// quadrant
		}
		else {											// For input x > 0.5, use identity
			temp = xHead;								// asin(x) =
			temp2 = xTail;								// pi/2-2asin(sqrt((1-x)/2))
		}
		
		h = __FNMSUB( 0.5, temp, 0.5 ); // 0.5 - 0.5*temp;					// exact
		argsq = __FNMSUB( 0.5, temp2, h ); // h - 0.5*temp2;
		argsq2 = __FNMSUB( 0.5, temp2, h - argsq ); // __FSUB( h, argsq ) - 0.5*temp2;		//square of reduced arg, exact
		
		ddx.f = __sqrtldextra(argsq , argsq2, &sqrtextra);	  
		
		sum = asinPtr[25].Head;							// Taylor Series
		
		for (i = 24; i >= kNterms/2+2; i--)		
			sum = __FMADD( sum, argsq, asinPtr[i].Head );// asinPtr[i].Head + sum*argsq; // working precison
		
		sumt = __FMADD( sum, argsq, asinPtr[kNterms/2+1].Head );// asinPtr[kNterms/2+1].Head + sum*argsq;
		suml = __FMADD( sum, argsq, asinPtr[kNterms/2+1].Head - sumt );// __FSUB( asinPtr[kNterms/2+1].Head, sumt ) + sum*argsq;
		sum = sumt;
		
		for (i = 1; i <= kNterms/2; i++) {				// remaining terms in quad prec.		
			temp = __FMADD( sum, argsq, asinPtr[kNterms/2-i+1].Head ); // asinPtr[kNterms/2-i+1].Head + sum*argsq;
			/* suml = asinPtr[kNterms/2-i+1].Head - temp + sum*argsq +
				(asinPtr[kNterms/2-i+1].Tail + sum*argsq2 + suml*argsq); */
			suml = __FMADD( sum, argsq, asinPtr[kNterms/2-i+1].Head - temp ) +
					__FMADD( suml, argsq, __FMADD( sum, argsq2, asinPtr[kNterms/2-i+1].Tail ) );
			sum = temp;
		}
		
		arg = ddx.d[0];
		argl = ddx.d[1];
		
		prodlow = __FMADD( suml, argsq, sum*argsq2 );	// suml*argsq + sum*argsq2;	// mult. by arg^2
		prod = __FMADD( sum, argsq, prodlow );			// sum*argsq + prodlow;
		prodlow = __FMSUB( sum, argsq, prod ) + prodlow;// sum*argsq - prod+prodlow;
		temp2 = __FMADD( prodlow, arg, prod*argl );		// prodlow*arg + prod*argl;	// multiply by arg
		temp = __FMADD( prod, arg, temp2 );				// prod*arg + temp2;
		temp2 = __FMSUB( prod, arg, temp ) + temp2;		// prod*arg - temp + temp2;
		
		res = __FADD( arg, temp );						// add arg for asin(arg)
		reslow = (arg - res + temp);					// exact
		resmid = __FADD( argl, temp2 );
		restiny = argl - resmid + temp2 + sqrtextra;
		p = __FADD( reslow, resmid );				// sum of middle terms
		q = reslow - p + resmid;						// possible residual
		reshi = __FADD( res, p );
		resbot = __FADD( (res - reshi + p), (q + restiny) );	//get ready to subtract from pi/4
		
		if (xHead > 0) {								// for first quadrant angles,
			ddx.d[0] = __FMADD( 2.0, reshi, 2.0*resbot ); // __FADD( 2.0*reshi, 2.0*resbot );			// we are finished			
			ddx.d[1] = __FMADD( 2.0, resbot, __FMSUB( 2.0, reshi, ddx.d[0] ) ); // (2.0*reshi - ddx.d[0]) + 2.0*resbot; 
                        FESETENVD(fenv);
			return (ddx.f);
		}
		
		resinf = (res - reshi +p ) - resbot + (q + restiny);	// for second quadrant need to
		res = __FNMSUB( 2.0, reshi, kPi ); // __FSUB( kPi, (2.0*reshi) );								// subtract from Pi.
		reslow = __FNMSUB( 2.0, reshi, kPi - res ); // (kPi - res - (2.0*reshi));
		resmid = __FNMSUB( 2.0, resbot, kPiTail1 ); // __FSUB( kPiTail1, (2.0*resbot) );
		
		if (kPiTail1 > fabs(2.0*resbot)) 
			/* restiny = kPiTail1 - resmid - (2.0*resbot) + (kPiTail2 - 2.0*resinf); */
			restiny = __FNMSUB( 2.0, resbot, kPiTail1 - resmid ) + __FNMSUB( 2.0, resinf, kPiTail2 );
		else
			/* restiny = kPiTail1 - (resmid + (2.0*resbot)) + (kPiTail2 - 2.0*resinf); */
			restiny = kPiTail1 - __FMADD( 2.0, resbot, resmid ) + __FNMSUB( 2.0, resinf, kPiTail2 );
		
		p = __FADD( reslow, resmid );
		q = reslow - p + resmid;
		reshi = __FADD( res, p );
		resbot = (res - reshi + p) + (q + restiny);
		
		ddx.d[0] = __FADD( reshi, resbot ); 			
		ddx.d[1] = (reshi - ddx.d[0]) + resbot; 
		
                FESETENVD(fenv);
		return (ddx.f);
	}
}
#endif
