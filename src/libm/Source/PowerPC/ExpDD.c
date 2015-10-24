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
//	ExpDD.c
//	
//	Double-double Function Library
//	Copyright: © 1995-96 by Apple Computer, Inc., all rights reserved
//	
//	long double expl( long double x );
//	long double exp2l( long double x );
//	long double expm1l( long double x );
//	
//			_ExpInnerLD() is exported for use by other functions.
//

#include "math.h"
#include "fp_private.h"
#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)
#include "DD.h"

// Floating-point constants

static const double kLn2  = 6.9314718055994529e-1;		// {0x3FE62E42, 0xFEFA39EF}
static const double kLn2b = 2.3190468138462996e-17;		// {0x3C7ABC9E, 0x3B39803F}
static const double kLn2c = 5.7077084384162121e-34;		// {0x3907B57A, 0x079A1934}
static const double kLn2d = -3.5824322106018114e-50;	// {0xB5AACE93, 0xA4EBE5D1}
static const double k1ByLn2 = 1.4426950408889634;		// {0x3FF71547, 0x652B82FE}
static const double kBig = 6.755399441055744e+15;		// {0x43380000, 0x00000000}
static const double r13 = 1.6059043836821613e-10;		// {0x3DE61246, 0x13A86D09}

static const Double kInfinityu = {{0x7ff00000, 0x0}};

static const double coeff[] = {
	6227020800.0, 3113510400.0, 1037836800.0, 259459200.0, 51891840.0, 8648640.0,
	1235520.0, 154440.0, 17160.0, 1716.0, 156.0, 13.0, 1.0
};

struct ExpTableEntry {
   double x;
   double fhead;
   double ftail;
};

extern uint32_t ExpTableLD[];

long double expl( long double x )
{
	double fpenv;
	DblDbl u;
	double extra;
	
	u.f = x;
	
	if (((u.i[0] & 0x7fffffffu) | u.i[1]) == 0)				// x = 0.0
		return 1.0L;
	
	if ((u.i[0] & 0x7ff00000u) != 0x7ff00000u) {				// x is not NaN, Infinity
                FEGETENVD(fpenv);
                FESETENVD(0.0);
		u.f = _ExpInnerLD(u.d[0], u.d[1], 0.0, &extra, 0);
		if ((u.i[0] & 0x7ff00000) == 0x7ff00000)
			u.d[1] = 0.0;
                FESETENVD(fpenv);
		return u.f;
	}
	
	if (u.d[0] != u.d[0])										// NaN case
		return x;
	
	if ((u.i[0] & 0xfff00000u) == 0x7ff00000u)				// +Infinity 
		return x;
	else														// -Infinity
		return 0.0L;
}

long double exp2l( long double x )
{
	double fpenv;
	DblDbl u;
	double head, tail, extra, uu, vv, ww, xx, yy, c, top, bottom;
	
	u.f = x;
	
	if (((u.i[0] & 0x7fffffffu) | u.i[1]) == 0)				// x = 0.0
		return 1.0L;
	
	if ((u.i[0] & 0x7ff00000u) != 0x7ff00000u) {  			// x is not NaN, Infinity
                FEGETENVD(fpenv);
                FESETENVD(0.0);
		head = u.d[0];
		tail = u.d[1];
		uu = __FADD( head * kLn2c, tail * kLn2b );
		vv = head * kLn2b;
		ww = __FMSUB( head, kLn2b, vv );
		xx = tail * kLn2;
		yy = __FMSUB( tail, kLn2, xx );
		c = ww + yy + uu;
		top = head * kLn2;
		ww = __FMSUB( head, kLn2, top );
		
		uu = __FADD( vv, xx );
		if (fabs(xx) > fabs(vv))
			c = xx - uu + vv + c;
		else
			c = vv - uu + xx + c;
			
		bottom = __FADD( uu, ww );
		if (fabs(ww) > fabs(uu))
			c = ww - bottom + uu + c;
		else
			c = uu - bottom + ww + c;
			
		head = __FADD( top, bottom );
		ww = (top - head) + bottom;
		tail = __FADD( ww, c );
		c = (ww - tail) + c;
		
		u.f = _ExpInnerLD(head, tail, c, &extra, 3);
		if ((u.i[0] & 0x7ff00000) == 0x7ff00000) {
			u.d[1] = 0.0;
		}
                FESETENVD(fpenv);
		return u.f;
	} 
	
	if (u.d[0] != u.d[0])									// NaN case
		return x;
	if ((u.i[0] & 0xfff00000u) == 0x7ff00000u)			// +Inifnity
		return x;
	else													// -Infinity
		return 0.0L;
}

long double expm1l( long double x )
{
	double fpenv;
	DblDbl u;
	double extra;
	
	u.f = x;
	
	if (((u.i[0] & 0x7fffffffu) | u.i[1]) == 0)			// x = +/-0.0
		return x;																			// changed 5-23-2006 to return x instead of 0.0L.
	
	if ((u.i[0] & 0x7ff00000u) != 0x7ff00000u) {		// x is not NaN, Infinity
                FEGETENVD(fpenv);
                FESETENVD(0.0);
		if ((u.i[0] & 0x7ff00000u) >= 0x2ef00000u)		// |x| > 2^(-110)
			u.f = _ExpInnerLD(u.d[0], u.d[1], 0.0, &extra, 2);
		if ((u.i[0] & 0x7ff00000) == 0x7ff00000)			// If the result is NaN or Infinity, clear out the tail.
			u.d[1] = 0.0;
                FESETENVD(fpenv);
		return u.f;
	}
	
	if (u.d[0] != u.d[0])														// NaN case
		return x;
	if ((u.i[0] & 0xfff00000u) == 0x7ff00000u)			// +Infinity
		return x;
	else																						// -Infinity
		return -1.0L;
}

//***********************************************************************************
//
//    FUNCTION:  long double _ExpInnerLD(double alpha, double beta, double gamma,
//               double *extra, int exptype)
//
//    DESCRIPTION: This routine is called internally by the following functions:
//
//                 long double Exp(long double);
//                 long double Exp2(long double);
//                 long double ExpMinus1(long double);
//                 long double Power(long double, long double);
//                 family of long double hyperbolic functions;
//
//                 1) exptype = 0 (called by Exp()):
//                    on entry: (alpha, beta)
//                    on exit:  (head, tail) of  Exp(alpha + beta) 
//                 
//                 2) exptype = 1 (called by hyperbolic functions):
//                    on entry: (alpha, beta)
//                    on exit:  (head, tail, extra) of Exp(alpha + beta)/2.0
//
//                 3) exptype = 2 (called by ExpMinus1()):
//                    on entry: (alpha, beta)
//                    on exit:  (head, tail) of Exp(alpha + beta) - 1.0
//
//                 4) exptype = 3 (called by Power(), Exp2()):
//                    on entry: (alpha, beta, gamma)
//                    on exit:  (head, tail) of Exp(alpha + beta + gamma)
//                 
//                 5) exptype = 4 (called by gamma functions):
//                    on entry: (alpha, beta, gamma)
//                    on exit:  (head, tail, extra) of Exp(alpha + beta + gamma)
//                 
//                 This routine assumes that the rounding direction on entry
//                 is round-to-nearest,  and infinity, NaN and 0 have been
//                 pre-filtered so that the input is a normal/denormal nonzero
//                 values.
//
//***********************************************************************************

long double _ExpInnerLD(double alpha, double beta, double gamma, double *extra,
	int exptype)
{
	int notnormal, i;
	double tail, factor, redarg, b, blow, redarg1, ca, carry;
	double arg, arg2a, c, d, arg2, sum, temp, suml, hi, prod, residual;
	double sum2, suml2, t1, t2, small, high, prodlow, bump, resnew;
	double close, top, residual2, residual3, bottom;
	Double test, accndx, en, rscale, scale, power;
	DblDbl result;
	struct ExpTableEntry *TblPtr = (struct ExpTableEntry *)ExpTableLD + 24;
	
	test.f = alpha;
	scale.f = 0.0;
	en.f = alpha*k1ByLn2 + kBig;
	factor = en.f - kBig;              
	redarg = __FNMSUB( kLn2, factor, alpha ); // alpha - kLn2*factor;   
	notnormal = 0;                   
	if ((test.i[0] & 0x7fffffffu) > 0x40862000u) { // |alpha| > 708 
		if (alpha > 0) {
			if (alpha <= 1026.0 * kLn2) {
				en.i[1] -= 4;
				notnormal = (1023 + 4) * 1048576;
			}
			else { // result overflow
				if ((exptype == 1) || (exptype == 4))
					*extra = 0.0;
				result.d[0] = kInfinityu.f;
				result.d[1] = 0.0;
				return result.f;
			}
		}
		else {
			if ((alpha >= -800.0) && (exptype != 2)) {
				en.i[1] += 256;       
				notnormal = (1023 - 256)*1048576;
			}
			else {      // result underflow or (alpha <= -708, and exptype = 2 or 4)
				if ((exptype == 1) || (exptype == 4))
					*extra = 0.0;
				result.d[0] = (exptype == 2) ? - 1.0 : 0.0;
				result.d[1] = 0.0;
				return result.f;
			}
		}
	}
	b = kLn2b*factor;              
	blow = __FMSUB( kLn2b, factor, b ); // kLn2b*factor - b;
	redarg1 = __FNMSUB( kLn2b, factor, beta );  // beta - b;
	accndx.f =  redarg*64.0 + kBig;  
	ca = kLn2c*factor;
	if (fabs(b) > fabs(beta))
		carry = beta - (b + redarg1) - blow;
	else
		carry = (beta - redarg1) - b - blow;
	redarg -= TblPtr[(int32_t)accndx.i[1]].x;     
	arg = __FADD( redarg, redarg1 );           
	arg2a = (redarg - arg + redarg1);
	c = __FSUB( carry, ca );                   
	d = __FNMSUB( kLn2c, factor, ca ) - (ca - (carry - c)) - kLn2d*factor;  // (ca - kLn2c*factor) - (ca - (carry - c)) - kLn2d*factor;  
	scale.i[0] = (en.i[1] + 1023) << 20;   
	arg2 = arg2a + c + d;
	if (exptype >= 3)
		arg2 += gamma;  // extpye = 3 or 4

	sum = coeff[12];
	sum = __FMADD( sum, arg, coeff[11] ); // coeff[11] + sum*arg;        
	sum = __FMADD( sum, arg, coeff[10] ); // coeff[10] + sum*arg;        
	sum = __FMADD( sum, arg, coeff[9] ); // coeff[ 9] + sum*arg;
	sum = __FMADD( sum, arg, coeff[8] ); // coeff[ 8] + sum*arg;
	temp = __FMADD( sum, arg, coeff[7] ); // coeff[7] + sum*arg;        
	suml = __FMADD( sum, arg, coeff[7] - temp ); // coeff[7] - temp + sum*arg;   
	sum = temp;
	for (i = 6; i > 0; i--) {
		hi = __FMADD( sum, arg, coeff[i] ); // coeff[i] + sum*arg;
		suml = __FMADD( suml, arg, __FMADD( sum, arg, coeff[i] - hi ) ); // coeff[i] - hi + sum*arg + suml*arg;
		sum = hi;
	}
	prod = sum*r13;                 
	residual = __FNMSUB( coeff[0], prod, sum ) + suml; // (sum - coeff[0]*prod) + suml;
	suml = residual*r13;
	sum2 = __FMADD( prod, arg, 1.0); // __FADD( 1.0, prod*arg );                
	suml2 = __FMADD( suml, arg, __FMADD( prod, arg, 1.0 - sum2 ) ); // 1.0 - sum2 + prod*arg + suml*arg;
	sum = __FMADD( suml2, arg, sum2*arg ); // sum2*arg + (suml2*arg);      
	suml = __FMADD( suml2, arg, __FMSUB( sum2, arg, sum ) ); // (sum2*arg - sum) + (suml2*arg);

	t1 = TblPtr[(int32_t)accndx.i[1]].fhead;    
	t2 = TblPtr[(int32_t)accndx.i[1]].ftail;
	small = t2 + sum*(t2 + arg2*t1);    
	prod = __FMUL( t1, sum );        
	high = __FADD( prod, t1 );        
	prodlow = __FMSUB( t1, sum, prod );     
	residual = (t1 - high) + prod; 
	     
	if (exptype == 2) {
		rscale.f = 0.0;
		rscale.i[0] = 2046*1048576 - scale.i[0];
		temp = __FSUB( high, rscale.f );  
		if (temp > 0) 
			bump = high - temp - rscale.f;
		else             
			bump = high - (temp + rscale.f);
		resnew = __FADD( residual, bump );  
		if (fabs(residual) > fabs(bump))
			small = ((residual - resnew) + bump) + small;  
		else
			small = ((bump - resnew) + residual) + small;
		high = temp;
		residual = resnew;
	}
	
	tail = small + arg2*t1 + prodlow + suml*t1;   
	close = __FADD( tail, residual );     
	top = __FADD( high, close );          
	residual2 = (residual - close) + tail;  
	residual3 = (high - top) + close;   
	bottom = __FADD( residual3, residual2 ); 
	  
	if ((exptype == 1) || (exptype == 4)) {
		if (exptype == 1) scale.f *= 0.5;   
		if (fabs(residual2) > fabs(residual3))
			*extra = (residual2 - bottom + residual3)*scale.f;
		else
			*extra = (residual3 - bottom + residual2)*scale.f;
	}
	
	if (notnormal != 0) {
		power.f = 0.0;
		power.i[0] = notnormal;
		top *= power.f;
		bottom *= power.f;
		if ((exptype == 1) || (exptype == 4))
			*extra *= power.f;
	}
	
	result.d[0] = top*scale.f;
	result.d[1] = bottom*scale.f;
	return result.f;
}
#endif
