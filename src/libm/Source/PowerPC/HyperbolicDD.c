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
//	HyperbolicDD.c
//	
//	Double-double Function Library
//	Copyright: © 1995-96 by Apple Computer, Inc., all rights reserved
//	
//	long double coshl(long double x);
//	long double sinhl(long double x);
//	long double tanhl(long double x);
//
//	Change history:
//		9/1/96	- PAF removed unnecessary tests (uncovered by MrC warning)
//

#include "math.h"
#include "fp_private.h"
#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)
#include "DD.h"

static const double recip = 0.1957294106339126123084757437350e-19;
static const double kBig  = 4.504699138998272e+15 ; // 2.0**52 + 2.0**40 = 0x43300100 00000000 

uint32_t TanhTbl[] = {
	0x00000000,	0x00000000,		0x00000000,	0x00000000,		0x00000000,	0x00000000, 
	0x3FB00000,	0x00030807,  	0x3FAFF559,	0x97E63AD9,  	0xBB6CF9C0,	0x2437818A, 
	0x3FC00000,	0x0000284A,  	0x3FBFD599,	0x2BC5078A,  	0xBB77F9D4,	0x704D912A, 
	0x3FC80000,	0x00005880,  	0x3FC7B8FF,	0x903C4CEC,  	0x3B47716F,	0xB54B60B5, 
	0x3FD00000,	0x00000CDC,  	0x3FCF597E,	0xA69A34B3,  	0xBB794C01,	0xC101435A, 
	0x3FD40000,	0x0000313C,  	0x3FD35F98,	0xA0EA91C7,  	0xBB904F63,	0x26BD63B6, 
	0x3FD80000,	0x000021EA,  	0x3FD6EF53,	0xDE8CAD3F,  	0xBB91172E,	0x8FAFB7A2, 
	0x3FDC0000,	0x00005214,  	0x3FDA5729,	0xEE48C464,  	0x3B9543D8,	0xC4943292, 
	0x3FE00000,	0x00001E86,  	0x3FDD9353,	0xD756BAF6,  	0xBB8F03A0,	0x34F7B469, 
	0x3FE20000,	0x00007115,  	0x3FE05086,	0xF2F72867,  	0xBB95D918,	0x3E3269EE, 
	0x3FE40000,	0x00005720,  	0x3FE1BF47,	0xEABBCBE9,  	0x3B918CCE,	0xA6F1FA3A, 
	0x3FE60000,	0x000021CF,  	0x3FE3157D,	0xFE9F8724,  	0xBBA1AC2C,	0x2B395168, 
	0x3FE80000,	0x00016908,  	0x3FE45323,	0xE553C98B,  	0xBBA14AF9,	0xB571AFA7, 
	0x3FEA0000,	0x00005202,  	0x3FE5788F,	0xF10D56AF,  	0xBBA3C229,	0xC063AD80, 
	0x3FEC0000,	0x00005742,  	0x3FE68665,	0x0B8C4C1B,  	0x3B8BE9FC,	0xF1B4428B, 
	0x3FEE0000,	0x00008582,  	0x3FE77D83,	0x8E34430D,  	0x3B829FA7,	0xF1623176, 
	0x3FF00000,	0x00005F76,  	0x3FE85EFA,	0xB51543C3,  	0xBB4B3592,	0x2498C785, 
	0x3FF10000,	0x00001F8D,  	0x3FE92BFB,	0x370DB380,  	0x3BA29188,	0x377121C3, 
	0x3FF20000,	0x0000130F,  	0x3FE9E5CB,	0x5BA45A90,  	0x3B7DFB4E,	0xF67C44E2, 
	0x3FF30000,	0x00004F18,  	0x3FEA8DBC,	0xBC31BABE,  	0x3B99C45D,	0xAE5D7827, 
	0x3FF40000,	0x00005131,  	0x3FEB2523,	0xBB6B5B77,  	0x3B85A025,	0x161129AB, 
	0x3FF50000,	0x000029D4,  	0x3FEBAD50,	0xA4A6A0D5,  	0xBB983007,	0x72D2D609, 
	0x3FF60000,	0x000007E1,  	0x3FEC278A,	0x52A4E807,  	0x3BA451F7,	0x31DE764B, 
	0x3FF70000,	0x0000185B,  	0x3FEC950A,	0x3340D299,  	0x3BAABF14,	0xE874463C, 
	0x3FF80000,	0x000024C4,  	0x3FECF6F9,	0x786E02C1,  	0x3B6B3F8E,	0x2446EA6B, 
	0x3FF90000,	0x0000BF0C,  	0x3FED4E6F,	0x4642C44F,  	0x3BA657E4,	0xDE96E741, 
	0x3FFA0000,	0x00006986,  	0x3FED9C6F,	0xAFE63ACE,  	0x3BA71F1B,	0xC2E3ED65, 
	0x3FFB0000,	0x00003690,  	0x3FEDE1EB,	0x59375F86,  	0x3B94D798,	0x08425B54, 
	0x3FFC0000,	0x000068CA,  	0x3FEE1FBF,	0x97E34D01,  	0x3BAD07C7,	0xA6BF9B47, 
	0x3FFD0000,	0x00008A24,  	0x3FEE56B6,	0xF3EFC7EE,  	0xBBA26658,	0xA2EB55F0, 
	0x3FFE0000,	0x0003C04A,  	0x3FEE8789,	0xECECBA51,  	0xBBA8B27F,	0x9026BBD4, 
	0x3FFF0000,	0x00000D68,  	0x3FEEB2DF,	0xEDD5EEB6,  	0x3B9EE74B,	0x12149464, 
	0x40000000,	0x00004DFD,  	0x3FEED950,	0x5E1BD9DE,  	0x3B971596,	0x795B2F55
};

uint32_t TanhCoeff[] = {
	0x3FF00000,	0x00000000,		0x00000000,	0x00000000, 
	0xBFD55555,	0x55555555,  	0xBC755555,	0x55555554, 
	0x3FC11111,	0x11111111,  	0x3C411111,	0x10FF0F6D, 
	0xBFABA1BA,	0x1BA1BA1C,  	0x3C47917A,	0xA287E6B6, 
	0x3F9664F4,	0x882C10FA,  	0xBC0A8F5F,	0x684BD9FF, 
	0xBF8226E3,	0x55E6C238,  	0x3C01097B,	0x425ED098, 
	0x3F6D6D3D,	0x0E154787,  	0xBC0BA781,	0x948B0FCF, 
	0xBF57DA36,	0x446C8BDA,  	0xBBD2F684,	0xBDA12F6B, 
	0x3F435580,	0xBCDA12F7,  	0xBBEED097,	0xB425ED0A, 
	0xBF2F545A,	0x781948B1,  	0x3B7948B0,	0xFCD6E991, 
	0x3F17B291,	0x61F9ADD4,  	0xBBAF9ADD,	0x3C0CA458
};

struct TanhTblEntry { 
   double One;
   double Two;
   double Three;
} TanhTblEntry;

struct TanhCoeffEntry { 
   double One;				
   double Two;
} TanhCoeffEntry;


extern long double  _ExpInnerLD(double alpha, double beta, double gamma,
								double *pextra, int normflag);

long double sinhl(long double x)
{
	double xHead, xTail, xHeadx, xTailx;
 	double p, q, r1, r2, r3, r4, r5, t1, t2;
	double residual, residual1, residual2, residual3, residual4, residual5, residual6;
	double res, resnew, reshi, reslow, restiny, resbot, resmid; 
	double remainder, partial, extraword;
	double argsq, argsq2;
	double prod, prodlow, sum, suml, temp, temp2,  templow;
	double bottom;
	uint32_t hibits;
	int i;
	double fpenv;
	DblDbl ddx;
	double coeff[16] = {
		0.62132974171578525315635255289e-14, 0.577836659795680285435407874e-11,
		0.469203367754092391773551193841e-8, 0.329380764163372859025032938e-5,
		0.1976284584980237154150197628458498e-2, 1.0, 420.0, 143640.0, 39070080.0,
		8204716800.0, 1279935820800.0, 140792940288000.0, 10137091700736000.0,
		425757851430912000.0, 8515157028618240000.0, 51090942171709440000.0
	};

	ddx.f = x;
	xHead = ddx.d[0];
	xTail = ddx.d[1];
	hibits = ddx.i[0] & 0x7FFFFFFFu;	// highest 32 bits as uint32_t
	
	// NaNs and infinities
	if (hibits >= 0x7ff00000u)			// special cases: NaN and infinity
		return x;

	// x = zero hence xHead is zero
	if ((hibits | ddx.i[1]) == 0)  return x;      

		
	// x is not zero, infinity, or NaN
    
	// finite  non-zero x 
        FEGETENVD(fpenv);
        FESETENVD(0.0);


	// for small x, |x| <= 0.693147 use power series

	if (fabs(xHead) <= 0.693147 ) {
		temp=2.0*xHead*xTail;																// small argument
	 	argsq=__FMADD( xHead, xHead, temp );				// xHead*xHead+temp;			// use power series
		argsq2=__FMADD( xTail, xTail, __FMSUB( xHead, xHead, argsq ) + temp ); // xHead*xHead-argsq+temp+xTail*xTail;
	 	sum=coeff[0];
	 	suml=0.0;
	 	sum=__FMADD( argsq, sum, coeff[1] );				// coeff[1]+argsq*sum;
	 	sum=__FMADD( argsq, sum, coeff[2] );				// coeff[2]+argsq*sum;
	 	sum=__FMADD( argsq, sum, coeff[3] );				// coeff[3]+argsq*sum;
		for ( i=4; i<15; i++) {	
			templow=__FMADD( suml, argsq, sum*argsq2 );		// suml*argsq+sum*argsq2;
			temp=__FMADD( sum, argsq, templow );			// sum*argsq+templow;
			bottom=__FMSUB( sum, argsq, temp) + templow;	// sum*argsq-temp+templow;
			sum=__FADD( coeff[i], temp );
			residual=coeff[i]-sum+temp;
			suml=bottom+residual;
		}
	 	prodlow=__FMADD( suml, argsq, sum*argsq2 );			// suml*argsq+sum*argsq2;		// mult. by arg^2
	 	prod=__FMADD( sum, argsq, prodlow );				// sum*argsq+prodlow;
	 	prodlow=__FMSUB( sum, argsq, prod) + prodlow;		// sum*argsq-prod+prodlow;
	 	temp2=__FMADD( prodlow, xHead, prod*xTail );		// prodlow*xHead+prod*xTail;	// mult. by xHead
	 	temp=__FMADD( prod, xHead, temp2 );					// prod*xHead+temp2;
	 	temp2=__FMSUB(prod, xHead, temp ) + temp2;			// prod*xHead-temp+temp2;
	 	sum=temp*recip;																		// sum of series ---
	 	remainder=__FNMSUB( sum, coeff[15], temp );			// (temp-sum*coeff[15]);
	 	partial=__FADD( remainder, temp2 );
	 	residual=remainder-partial+temp2;
	 	suml=__FMADD( partial, recip, residual*recip );		// partial*recip+(residual*recip);
	 	res=__FADD( xHead, sum );															// except for argument
	 	reslow=(xHead-res+sum);																// exact
	 	resmid=__FADD( xTail, suml );
	 	restiny=xTail-resmid+suml;
	 	p=__FADD( reslow, resmid );															// sum of middle terms
	 	q=reslow-p+resmid;																	// possible residual
	 	reshi=__FADD( res, p );
	 	resbot=(res-reshi+p)+(q+restiny);
		ddx.d[0] = __FADD( reshi, resbot );													// build a cannonical result
		ddx.d[1] = (reshi - ddx.d[0]) + resbot; 
                FESETENVD(fpenv);
		return (ddx.f);
	}

	// |x| > 0.693147 
	// else if (fabs(xHead) > 0.693147 ) {  <-- Unnecessary test
	else {
  		if (xHead < 0) {
	 		xHeadx = -xHead;
	 		xTailx = -xTail;
  		} 
		else {
	 		xHeadx = xHead;
	 		xTailx = xTail;
  		}

		ddx.f = _ExpInnerLD(xHeadx, xTailx, 0.0 , &extraword , 1); 	//get .5*e^x  
  		
		if (fabs(xHead) > 39.0) {
		
            if (xHead < 0) ddx.f = - ddx.f;

			reshi = ddx.d[0];
			ddx.d[0] =  __FADD( ddx.d[0],  ddx.d[1] );
			if((ddx.i[0] & 0x7FFFFFFFu)>= 0x7ff00000u) {
				// special cases : NaN and infinity
				ddx.d[1] = 0.0;
			}
			else {
				ddx.d[1] = (reshi -  ddx.d[0]) + ddx.d[1];
			}
                        FESETENVD(fpenv);
			return (ddx.f);
		} 
		else {
	 		t1 = ddx.d[0];																// zres = .5*e^x
	 		t2 = ddx.d[1];																// Def. of sinh: (e^x - e^-x)/2
	 		r1=0.25/t1;
	 		residual=__FNMSUB( t2, r1, __FNMSUB( t1, r1, 0.25 ) );	// (0.25-t1*r1)-t2*r1;
	 		r2=residual*(4.0*r1);														// (r1,r2)=.5*e^-x
	 		residual1 =__FNMSUB( t2, r1, __FNMSUB( t1, r1, 0.25 ) - residual ); // (0.25-t1*r1)-residual-t2*r1;
	 		r3=__FMSUB(t1, r2, residual );		//(t1*r2-residual);						// (reversed sign)
	 		r4=__FMADD( extraword, r1, t2*r2 );	//(extraword*r1+(t2*r2));				// (reversed sign)
	 		reshi=__FSUB( t1, r1 );
	 		reslow=__FSUB( t2, r2 );
	 		residual=(t1-reshi)-r1;														// exact
	 		if (fabs(t2) > fabs(r2)) 
				residual2=(t2-reslow)-r2;												// exact
	 		else 
				residual2=t2-(reslow+r2);												// exact
	 		r5=(r3+r4-residual1)*(4.0*r1);
	 		resnew=__FADD( reshi, reslow );
	 		residual3=reshi-resnew+reslow;												// exact
	 		residual4=__FADD( residual, residual3 );
	 		reshi=__FADD( resnew, residual4 );
	 		residual5=residual-residual4+residual3;
	 		residual6=resnew -reshi+residual4;
	 		reslow=(residual2+(extraword+r5))+residual5+residual6;
	 		if (xHead <	0.0) {															// Reverse signs for
				reshi = -reshi;															// negative arguments
				reslow = -reslow;
			}
			ddx.d[0] = __FADD( reshi, reslow );											// build a cannonical result
			ddx.d[1] = (reshi - ddx.d[0]) + reslow; 
                        FESETENVD(fpenv);
			return (ddx.f);
		}
	}       // end of abs(xHead) > 0.693147

}


long double coshl(long double x)
{
	double xHead, xTail, xHeadx, xTailx;
 	double r1, r2, r3, r4, r5, t1, t2;
	double residual, residual1, residual2, residual3, residual4, residual5, residual6;
	double  res, resnew, reshi, reslow;
	double remainder, partial, extraword;
	double argsq, argsq2;
	double prod, prodlow, sum, suml, temp, templow;
	double bottom;
	uint32_t hibits, signx;
	int i;
    double fpenv;
	DblDbl ddx;
	double coeff[9] = {
		306.0,  73440.0,  13366080.0,  1764322560.0,  158789030400.0,  8892185702400.0,  
		266765571072000.0,  3201186852864000.0, 6402373705728000.0
	};
	double recip = 1.561920696858622646221636435005e-16;

	ddx.f = x;
	xHead = ddx.d[0];
	xTail = ddx.d[1];
	hibits = ddx.i[0] & 0x7FFFFFFFu;		// highest 32 bits as uint32_t
    signx = (ddx.i[0] >> 31) & 1u;			// get sign of head
	
	// NaNs and infinities
	
	if (hibits >= 0x7ff00000u){			// special cases: NaN and infinity
		if (xHead != xHead) {				// x is a NaN
			return x;
		}
		else {								// For +/- infinity return +infinity
			if (signx)
				return (-x);
			else
				return x;
		}
	}


	// x = zero hence xHead is zero

	if ((hibits | ddx.i[1]) == 0)
		return (long double) 1.0;      

	// x is not zero, infinity nor a NaN
	
        FEGETENVD(fpenv);
        FESETENVD(0.0);

	//for small x, |x| <= 0.125 use power series
	if (hibits <= 0x3fc00000u) {
		temp=2.0*xHead*xTail;        
		argsq=__FMADD( xHead, xHead, temp );				// xHead*xHead+temp;         
		argsq2=__FMADD( xTail, xTail, __FMSUB( xHead, xHead, argsq ) + temp ); // xHead*xHead-argsq+temp+xTail*xTail;
		sum=1.0;
		suml=0.0;
	 	sum=__FMADD( argsq, sum, coeff[0] );				// coeff[0]+argsq*sum;
	 	sum=__FMADD( argsq, sum, coeff[1] );				// coeff[1]+argsq*sum;
	 	sum=__FMADD( argsq, sum, coeff[2] );				// coeff[2]+argsq*sum;
		for (i=3; i< 8; i++) {
			templow=__FMADD( suml, argsq, sum*argsq2 );		// suml*argsq+sum*argsq2;
			temp=__FMADD( sum, argsq, templow );			// sum*argsq+templow;
			bottom=__FMSUB( sum, argsq, temp) + templow;	// sum*argsq-temp+templow;
			sum=__FADD( coeff[i], temp );
			residual=coeff[i]-sum+temp;
			suml=bottom+residual;
		}
	 	prodlow=__FMADD( suml, argsq, sum*argsq2 );			// suml*argsq+sum*argsq2;		// mult. by arg^2
	 	prod=__FMADD( sum, argsq, prodlow );				// sum*argsq+prodlow;
	 	prodlow=__FMSUB( sum, argsq, prod) + prodlow;		// sum*argsq-prod+prodlow;
	 	sum=prod*recip;																		// sum of series ---
	 	remainder=__FNMSUB( sum, coeff[8], prod );			// (prod-sum*coeff[8]);
	 	partial=__FADD( remainder, prodlow );
		residual=remainder-partial+prodlow;
	 	suml=__FMADD( partial, recip, residual*recip );		// partial*recip+(residual*recip);
		res=__FADD(1.0, sum );																// except for 1.0 term.
	 	reslow=1.0 - res+sum+suml;
		ddx.d[0] = __FADD( res, reslow );													// build a cannonical result
		ddx.d[1] = (res - ddx.d[0]) + reslow; 
                FESETENVD(fpenv);
		return (ddx.f);
	}

	// |x| > 0.125 
	// else if (hibits > 0x3fc00000u) {   	// abs(xHead) > 0.125  <-- Unnecessary test
	else {   								// abs(xHead) > 0.125
		if (signx) {
			xHeadx = -xHead;
			xTailx = -xTail;
		}
		else{
			xHeadx = xHead;
			xTailx = xTail;
		} 
		ddx.f = _ExpInnerLD (xHeadx, xTailx, 0.0 , &extraword , 1); 	// get .5*e^x 

		// |x| > 40.0		
		if (hibits > 0x40440000u) { 		// abs(xHead) > 40.0

			reshi = ddx.d[0];
			ddx.d[0] =  __FADD( ddx.d[0],  ddx.d[1] );
			if((ddx.i[0] & 0x7FFFFFFFu)>= 0x7ff00000u) {  // special cases : NaN and infinity
				ddx.d[1] = 0.0;
			}
			else {
				ddx.d[1] = (reshi -  ddx.d[0]) + ddx.d[1];
			}
                        FESETENVD(fpenv);
			return (ddx.f);
		}
		// 0.125 <= |x| < 40.0	
		else{
	 		t1 = ddx.d[0];																// zres=.5*e^x
	 		t2 = ddx.d[1];																// def. of cosh: (e^x + e^-x)/2
	 		r1=0.25/t1;
	 		residual=__FNMSUB( t2, r1, __FNMSUB( t1, r1, 0.25 ) );	// (0.25-t1*r1)-t2*r1;
	 		r2=residual*(4.0*r1);														// (r1,r2)=.5*e^-x + errors< 4 ulps
	 		residual1 =__FNMSUB( t2, r1, __FNMSUB( t1, r1, 0.25 ) - residual ); // (0.25-t1*r1)-residual-t2*r1;
											// rest of rem. from HO divisor only
	 		r3=__FMSUB(t1, r2, residual );		//(t1*r2-residual);						// (reversed sign)...LO quotient
			r4=__FMADD( extraword, r1, t2*r2 );	//(extraword*r1+(t2*r2));				// (reversed sign)...LO quotient
	 		reshi=__FADD( t1, r1 );
	 		reslow=__FADD( t2, r2 );
	 		residual=(t1-reshi)+r1;														// exact
	 		if (fabs(t2) >  fabs(r2)) 
	   			residual2=(t2-reslow)+r2;												// exact
	 		else
	  			 residual2=r2-reslow+t2;												// exact
	 		r5=(r3+r4-residual1)*(4.0*r1);												// reversed sign
	 		resnew=__FADD( reshi, reslow );
	 		residual3=reshi-resnew+reslow;												// exact
	 		residual4=__FADD( residual, residual3 );
	 		reshi=__FADD( resnew, residual4 );
	 		residual5=residual-residual4+residual3;
	 		residual6=resnew-reshi+residual4;
	 		reslow=(residual2+(extraword-r5))+residual5+residual6;
			ddx.d[0] = __FADD( reshi, reslow );											// build a cannonical result
			ddx.d[1] = (reshi - ddx.d[0]) + reslow; 
                        FESETENVD(fpenv);
			return (ddx.f);
		}
	}       // end of abs(xHead) > 0.125
}


long double tanhl(long double x)
{

	double r, res, res2, res3, res4,res5, resfin, reslow;
	double rem, rem1, rem2;
	uint32_t signx, hibits, ndx ;
	double q, qq, qa, qb, qc, q1, quot, quot2, quot3;
	double arg, arg1, arg2, argsq, argsq2;
	double sum, sum2, suml, suml2;
	double frac, frac2, fac, fac2;
	double ra, rb, rc, rd, re, rf;
	double ans, ansx, ansl, ansmid, anstiny, anslow, almost;
	double prod, prodlow, prodx, prodlowx, proderr, pl;
	double top, top2, den, den1, denf, denf2;
	double rg, t2, t3, tsq, tsql;
	double az, bz, temp, temp1, templow, carry, carry1, extra;
	double bump, residual, tiny, recip;
	Double DeN;
	double fpenv;
	double xHead, xTail;
	DblDbl ddx;
	int i;

	struct TanhTblEntry *TanhTblPtr = (struct TanhTblEntry *)TanhTbl;
	struct TanhCoeffEntry *TanhCoeffPtr = (struct TanhCoeffEntry *) TanhCoeff;

	ddx.f = x;
	xHead = ddx.d[0];
	xTail = ddx.d[1];
	hibits = ddx.i[0] & 0x7FFFFFFFu;			// highest 32 bits as uint32_t
    signx = (ddx.i[0] >> 31) & 1u;				// get sign of head
	
	// NaN or zero
	
	if ((xHead != xHead)||((hibits | ddx.i[1]) == 0))
		return x;

        FEGETENVD(fpenv);
        FESETENVD(0.0);
		
	//for infinity or large x , |xHead| > 40.0 

	if (hibits > 0x40440000u) {				// results = 1, for big args.
        FESETENVD(fpenv);
		if (signx)
			return (long double) (-1.0);		// sign of result= sign of arg
		else
			return (long double) (1.0);
    }

	// x is not zero, infinity nor a NaN
	
	if (signx) {
		xHead = -xHead;
		xTail = -xTail;
	}


	// values of |xHead| with 2.0<= |xHead| <= 40.0

	if (hibits > 0x40000000u) {	  
		//****************************************************************************
		//                                                                           *
		//                         1.0                                               *
		//      tanh(x)= 1 -  -------------           |x| > 2.0                      *
		//                    .5 +.5*e^(2x)                                          *
		//                                                                           *
		//****************************************************************************
		xHead=2.0*xHead;						// double arg
		xTail=2.0*xTail;
       
		ddx.f = _ExpInnerLD (xHead, xTail, 0.0 , &extra, 1 );							// compute e^(2*arg)
		az = ddx.d[0];
       	bz = ddx.d[1];
       	temp=__FADD( az, 0.5 );															// az >= 0.5
       	carry=az-temp+0.5;																// exact
       	temp1=__FADD( carry, bz );
       	carry1=carry-temp1+bz;
       	den=__FADD( temp, temp1 );
       	den1=temp-den+temp1+carry1;
       	r=1.0/den;																		// guess recip. of denominator
       	rem=__FNMSUB( den, r, 1.0 );			// 1.0-den*r;							// first remainder
       	rem1=__FNMSUB( r, den1, rem );			// rem-r*den1;							// (rem1,rem2) is better rem.
       	if (fabs(rem) > fabs(r*den1)) 
        	rem2=__FNMSUB( r, den1, rem-rem1 ); // rem-rem1-r*den1;
       	else {
          	rem1=__FNMSUB( r, den1, rem ); // rem-(r*den1);
          	/* rem2=-(r*den1)-rem1+rem -(r*den1-(r*den1)); */
			rem2 = rem - __FMADD( r, den1, rem1 ) - __FMSUB( r, den1, (r*den1) );
       	} 
       	qq=__FMADD( r, rem1, r ); // r+r*rem1; 							// going for the full quotient
       	qa=__FMADD( r, rem1, r-qq ); // r-qq+r*rem1;
       	qb=__FMADD( r, rem2, qa ); // qa+r*rem2;
       	q=__FADD( qq, qb );
       	// qc=qa-qb+r*rem2-(extra*r)*r;
		qc = __FNMSUB( extra*r, r, __FMADD( r, rem2, qa - qb ) );
       	q1=qq-q+qb+qc;
       	res=__FSUB( 1.0, q );															// high order result
       	res2=1.0-res-q;																	// exact low order
       	res3=__FSUB( res2, q1 );
       	resfin=__FADD( res, res3 );
       	res4=res2-res3-q1;
       	res5=res-resfin+res3;
       	reslow=res4+res5;
       	if (signx) {
          	resfin=-resfin;
          	reslow=-reslow;
       	} 
		ddx.d[0] = __FADD( resfin, reslow );											// build a cannonical result
		ddx.d[1] = (resfin - ddx.d[0]) + reslow; 
                FESETENVD(fpenv);
		return (ddx.f);
	}

	// else if (hibits <=  0x40000000u) {		// |xHead| < 2.0  <-- Unnecessary test
	else {										// |xHead| < 2.0
	
		//***************************************************************************
		//                                                                          *
		//     Accurate table is used to reduce small arguments such that the       *
		//     range of the |reduced argument| < 1/32. The tanh addition formula    *
		//     is used to piece together three tanhs                                *
		//                                                                          *
		//***************************************************************************

		DeN.f = __FMADD( xHead, 16.0, kBig ); // kBig+xHead*16.0;				// compute table lookup index
 	 	ndx = DeN.i[1];
       	if (hibits <  0x3fb00000u) ndx=0;		// |xHead| < 0.0625
       	arg1=xHead-TanhTblPtr[(int32_t)ndx].One;			// reduce argument by table value
												// which is close to ndx/16 (exact)
       	arg=__FADD( arg1, xTail );							// full 53 bit argument
       	arg2=arg1-arg+xTail;					// low order argument
		//************************************************************************************
		//     The argument has been broken up as follows:                             		 *
		//                   Zarg=TanhTblPtr[ndx].One +arg+arg2                              *
		//                                                                             		 *
		//     tanh(TanhTblPtr[ndx].One) is read from tantbl(2,ndx),tantbl(3,ndx)            *
		//         with at least 122 bits of precision                                 		 *
		//                   tanh(arg2)=arg2+o(2^-172)                                 		 *
		//                                                                             		 *
		//     Compute tanh(arg) by economized power series.                           		 *
		//     the task is to piece together the three parts                     			 *
		//                                                                             		 *
		//                                      (tanh(xTail)+tanh(c))(1-tanh^2(xHead))    	 *
		// tanh(xHead+xTail+c)=tanh(xHead) +   ------------------------------------------    *
		//                         1+tanh(xHead) tanh(xTail)+tanh(c)(tanh(xHead)+tanh(xTail))*
		//                                                                             		 *
		//************************************************************************************
     	sum=TanhCoeffPtr[10].One;
      	suml=TanhCoeffPtr[10].Two;
      	argsq=arg*arg;
      	argsq2=__FMSUB( arg, arg, argsq ); //	arg*arg-argsq;					// arg^2 for series
     	for (i = 9; i > 0; i--) { 
    		pl=__FMADD( suml, argsq, sum*argsq2 ); // suml*argsq+sum*argsq2;
         	prod=__FMADD( sum, argsq, pl ); // sum*argsq+pl;
         	prodlow=__FMSUB( sum, argsq, prod ) + pl; // sum*argsq-prod+pl;
         	sum=__FADD( TanhCoeffPtr[i].One, prod );		// add in the next coefficient
         	sum2=TanhCoeffPtr[i].Two+prodlow;
         	suml=TanhCoeffPtr[i].One-sum+prod+sum2;
		}
      	pl=__FMADD( suml, argsq, sum*argsq2 ); // suml*argsq+sum*argsq2;
      	temp=__FMADD( sum, argsq, pl ); // sum*argsq+pl;
      	templow=__FMSUB( sum, argsq, temp ) + pl; // sum*argsq-temp+pl;
      	pl=templow*arg;							// last multiplication by arg
      	prodx=__FMADD( temp, arg, pl );			// temp*arg+pl;
      	prodlowx=__FMSUB( temp, arg, prodx ) + pl; // temp*arg-prodx+pl;				// tanh(arg)-1.0  is done.
      	prod=__FADD( arg, prodx );
      	prodlow=arg-prod+prodx+prodlowx;		// tanh(arg) is done.
      
	  	if (hibits <  0x3fb00000u) {			// |xHead| < 0.0625, trivial: tanh(xHead)=0 in formula
         	proderr=(arg-prod+prodx)-prodlow+prodlowx;
         	bump=__FNMSUB( prod, prod*arg2, arg2 ); // arg2-prod*prod*arg2;
         	reslow=__FADD( prodlow, bump );
         	residual=prodlow-reslow+bump+proderr;
         	res=__FADD( reslow, prod );
         	reslow=prod-res+reslow+residual;
         	if (signx) {
            	res=-res;
            	reslow=-reslow;
         	} 

			ddx.d[0] = __FADD( res, reslow );			// build a cannonical result
			ddx.d[1] = (res - ddx.d[0]) + reslow; 
                        FESETENVD(fpenv);
			return (ddx.f);
      	} 
      
		tiny=arg2*(TanhTblPtr[(int32_t)ndx].Two+prod);	// The last addend in denominator
		
      	// pl=TanhTblPtr[(int32_t)ndx].Two*prodlow+TanhTblPtr[(int32_t)ndx].Three*prod;  // tanh(xHead)*tanh(xTail)
		pl = __FMADD( TanhTblPtr[(int32_t)ndx].Two, prodlow, TanhTblPtr[(int32_t)ndx].Three*prod );
		
      	// den=TanhTblPtr[(int32_t)ndx].Two*prod+pl;		// denominator completed except
		den = __FMADD( TanhTblPtr[(int32_t)ndx].Two, prod, pl );
		
      	// den1=TanhTblPtr[(int32_t)ndx].Two*prod-den+pl+tiny;			// for the +1 term.
		den1 = __FMSUB( TanhTblPtr[(int32_t)ndx].Two, prod, den ) + pl + tiny;
		
      	suml2=prodlow+arg2;						// start on numerator
      	t2=2.0*TanhTblPtr[(int32_t)ndx].Two*TanhTblPtr[(int32_t)ndx].Three;	// table look up value
		
      	// tsq=TanhTblPtr[(int32_t)ndx].Two*TanhTblPtr[(int32_t)ndx].Two+t2;		// squared.
		tsq = __FMADD( TanhTblPtr[(int32_t)ndx].Two, TanhTblPtr[(int32_t)ndx].Two, t2 );
		
      	// tsql=TanhTblPtr[(int32_t)ndx].Two*TanhTblPtr[(int32_t)ndx].Two-tsq+t2;
		tsql = __FMSUB( TanhTblPtr[(int32_t)ndx].Two, TanhTblPtr[(int32_t)ndx].Two, tsq ) + t2;
		
      	/* t3=2.0*TanhTblPtr[(int32_t)ndx].Two*TanhTblPtr[(int32_t)ndx].Three-t2 +
    		TanhTblPtr[(int32_t)ndx].Three*TanhTblPtr[(int32_t)ndx].Three; */
		t3 = __FMADD( TanhTblPtr[(int32_t)ndx].Three, TanhTblPtr[(int32_t)ndx].Three, 
				__FMSUB( 2.0*TanhTblPtr[(int32_t)ndx].Two, TanhTblPtr[(int32_t)ndx].Three, t2 ) );
				
      	denf=__FADD( 1.0, den );							// denominator is really done now
      	denf2=1.0-denf+den+den1;
      	fac=__FSUB( 1.0, tsq );							// compute 1-tanhtble(ndx)^2 becomes
      	residual=1.0-fac-tsq;					// (fac, fac2)
      	fac2=residual-tsql-t3;
      	pl=__FMADD( fac, suml2, fac2*prod ); // fac*suml2+fac2*prod;					// (xTail+c)(1-xHead*xHead) ... above formula
      	top=__FMADD( fac, prod, pl ); // fac*prod+pl;
      	top2=__FMSUB( fac, prod, top ) + pl; // fac*prod-top+pl;					// doing division
      	recip=1.0/denf;
      	quot=top*recip;
      	ra=__FNMSUB( quot, denf, top ); // top-quot*denf;						// 3 part remainder:
      	rb=-(quot*denf2);						// rem=ra+rb+top2
      	rc= - __FMADD(quot, denf2, rb ); // -(quot*denf2+rb);					// 3rd order remainder
      	rd=__FADD( top2, rb );								// sum 2nd order rems. into rf
      	if (fabs(top2) > fabs(rb))    
			re=top2-rd+rb;
      	else 
			re=rb-rd+top2;
      	rf=__FADD( ra, rd );
      	if (fabs(ra) > fabs(rd)) 
         	rg=ra-rf+rd;						// more 3rd order rems.
      	else									// rc+re+rg
         	rg=rd-rf+ra;
      	quot2=rf*recip;
      	quot3=__FMSUB( rf, recip, quot2 ); // rf*recip-quot2;
      	frac=__FADD( quot, quot2 );
      	// frac2=quot-frac+quot2+(quot3+recip *(rc+re+rg));
		frac2 = quot-frac+quot2 + __FMADD( recip, (rc+re+rg), quot3 );


      	ansx=__FADD( TanhTblPtr[(int32_t)ndx].Two, frac );			// paste together the result
      	ansl=__FADD( TanhTblPtr[(int32_t)ndx].Three, frac2 );
      	ansmid=TanhTblPtr[(int32_t)ndx].Two-ansx+frac;
      	anstiny=TanhTblPtr[(int32_t)ndx].Three-ansl+frac2;
      	almost=__FADD( ansmid, ansl );
      	ans=__FADD( ansx, almost );
      	anslow=(ansx-ans+almost)+((ansmid-almost+ansl) +anstiny);
      	if (signx) {
         	ans=-ans;
         	anslow=-anslow;
      	} 
 
 		ddx.d[0] = __FADD( ans, anslow );				// build a cannonical result
		ddx.d[1] = (ans - ddx.d[0]) + anslow; 
                FESETENVD(fpenv);
		return (ddx.f);
	}
}
#endif
