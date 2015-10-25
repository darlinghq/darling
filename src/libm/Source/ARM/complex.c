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
/****************************************************************************
**  File: complex.c
**
**  Contains:  C source code for implementations of floating-point
**             (double) complex functions defined in header file
**             "complex.h" for PowerPC Macintoshes in native mode.
**             Transcendental function algorithms are based on the
**             paper "Branch Cuts for Complex Elementary Functions"
**             by W. Kahan, May 17, 1987, and on Pascal and C source
**             code for the SANE 80-/96-bit extended type by Kenton
**             Hanson and Paul Finlayson, respectively.
**
**            
**  Written by: Jon Okada, SANEitation Engineer, ext. 4-4838
**   
**  Copyright:  c 1987-1993 by Apple Computer, Inc., all rights reserved.
**   
**  Change History (most recent first):
**
**     25 Aug 93   ali    Changed clog to cLog to avoid clashing with the
**                        stream i/o definition clog.
**     14 Jul 93   ali    Added #pragma fenv_access on
**     22 Feb 93   ali    Added a nomaf #pragma.
**     05 Feb 93   JPO    Modified calls to feclearexcept and feraiseexcept
**                        to reflect changes in "fenv.h".
**     18 Dec 92   JPO    First created.
**            
****************************************************************************/

#include "math.h"
#include "complex.h"
#include <stdint.h>

#define Real(z) (__real__ z)
#define Imag(z) (__imag__ z)

/****************************************************************************
   CONSTANTS used by complex functions

#include <stdio.h>
#include <math.h>
#include <float.h>
main()
{

float FPKASINHOM4f = asinhf(nextafterf(INFINITY,0.0f))/4.0f;
float FPKTHETAf = sqrtf(nextafterf(INFINITY,0.0f))/4.0f;
float FPKRHOf = 1.0f/FPKTHETAf;
float FPKLOVEREf = FLT_MIN/FLT_EPSILON;

printf("FPKASINHOM4 %16.7e %x\n", FPKASINHOM4f, *(int *)(&FPKASINHOM4f));
printf("FPKTHETA %16.7e %x\n", FPKTHETAf, *(int *)(&FPKTHETAf));
printf("FPKRHO %16.7e %x\n", FPKRHOf, *(int *)(&FPKRHOf));
printf("FPKLOVERE %16.7e %x\n", FPKLOVEREf, *(int *)(&FPKLOVEREf));
}
 
static const               // underflow threshold / round threshold 
   hexdouble FPKLOVERE = HEXDOUBLE(0x03600000,0x00000000);

static const               // underflow threshold / round threshold 
   hexsingle FPKLOVEREf = { 0xc000000 };

static const               // exp(709.0) 
   hexdouble FPKEXP709 = HEXDOUBLE(0x7fdd422d,0x2be5dc9b);

static const               // asinh(nextafterd(+infinity,0.0))/4.0 
   hexdouble FPKASINHOM4 = HEXDOUBLE(0x406633ce,0x8fb9f87e);

static const               // asinh(nextafterf(+infinity,0.0))/4.0 
   hexsingle FPKASINHOM4f = { 0x41b2d4fc };

static const               // sqrt(nextafterd(+infinity,0.0))/4.0 
   hexdouble FPKTHETA = HEXDOUBLE(0x5fcfffff,0xffffffff);

static const               // sqrt(nextafterf(+infinity,0.0))/4.0 
   hexsingle FPKTHETAf =  { 0x5e7fffff };

static const               // 4.0/sqrt(nextafterd(+infinity,0.0)) 
   hexdouble FPKRHO = HEXDOUBLE(0x20100000,0x00000000);

static const               // 4.0/sqrt(nextafterf(+infinity,0.0)) 
   hexsingle FPKRHOf = { 0x20800001 };

****************************************************************************/

static const double expOverflowThreshold_d = 0x1.62e42fefa39efp+9;
static const double expOverflowValue_d = 0x1.fffffffffff2ap+1023;   // exp(overflowThreshold)
static const double twiceExpOverflowThresh_d = 0x1.62e42fefa39efp+10;

static const double FPKASINHOM4 = 0x1.633ce8fb9f87ep+7;
static const float FPKASINHOM4f = 0x1.65a9f8p+4f;
static const double FPKTHETA = 0x1.fffffffffffffp+509;
static const float FPKTHETAf = 0x1.fffffep+61f;
static const double FPKRHO = 0x1p-510;
static const float FPKRHOf = 0x1.000002p-62f;

static void cosisin(double x, double complex *z);
static void cosisinf(float x, float complex *z);

static
double complex xdivc( double x, double complex y )   /* returns (real x) / (complex y) */
{
   double complex      z;
   double   r, denom;
   
   if ( fabs(Real(y)) >= fabs(Imag(y)) ) {     /* |Real(y)| >= |Imag(y)| */
      if (fabs(Real(y)) == INFINITY) {   /* Imag(y) and Real(y) are infinite */
         Real(z) = copysign(0.0,Real(y));
         Imag(z) = copysign(0.0,-Imag(y));
      }
      else {                             /* |Real(y)| >= finite |Imag(y)| */
         r = Imag(y)/Real(y);
         denom = Real(y) + Imag(y)*r;
         Real(z) = x/denom;
         Imag(z) = (-x*r)/denom;
      }
   }
   
   else {                                /* |Real(y)| !>= |Imag(y)| */
      r = Real(y)/Imag(y);
      denom = r*Real(y) + Imag(y);
      Real(z) = (r*x)/denom;
      Imag(z) = -x/denom;
   }
   
   return z;
}

static
float complex xdivcf( float x, float complex y )   /* returns (real x) / (complex y) */
{
   float complex      z;
   float   r, denom;
   
   if ( fabsf(Real(y)) >= fabsf(Imag(y)) ) {     /* |Real(y)| >= |Imag(y)| */
      if (fabsf(Real(y)) == INFINITY) {   /* Imag(y) and Real(y) are infinite */
         Real(z) = copysignf(0.0f,Real(y));
         Imag(z) = copysignf(0.0f,-Imag(y));
      }
      else {                             /* |Real(y)| >= finite |Imag(y)| */
         r = Imag(y)/Real(y);
         denom = Real(y) + Imag(y)*r;
         Real(z) = x/denom;
         Imag(z) = (-x*r)/denom;
      }
   }
   
   else {                                /* |Real(y)| !>= |Imag(y)| */
      r = Real(y)/Imag(y);
      denom = r*Real(y) + Imag(y);
      Real(z) = (r*x)/denom;
      Imag(z) = -x/denom;
   }
   
   return z;
}

/****************************************************************************
   double cabs(double complex z) returns the absolute value (magnitude) of its
   complex argument z, avoiding spurious overflow, underflow, and invalid
   exceptions.  The code is identical to hypot[fl].
****************************************************************************/

double cabs( double complex z ) { return hypot(Real(z), Imag(z)); }

float cabsf( float complex z ) { return hypotf(Real(z), Imag(z)); }

/****************************************************************************
   double carg(double complex z) returns the argument (in radians) of its
   complex argument z.  The algorithm is from Kahan's paper.
   
   The argument of a complex number z = x + i*y is defined as atan2(y,x)
   for finite x and y.
   
   CONSTANTS:  FPKPI2 = pi/2.0 to double precision
               FPKPI = pi to double precision
   
   Calls:  fpclassify, copysign, fabs, atan
****************************************************************************/

double carg ( double complex z ) { return atan2(Imag(z), Real(z)); }
   
float cargf ( float complex z ) { return atan2f(Imag(z), Real(z)); }

/****************************************************************************
   double complex csqrt(double complex z) returns the complex square root of its argument.
   The algorithm, which is from the Kahan paper, uses the following 
   identities:
   
      sqrt(x + i*y) = sqrt((|z| + Real(z))/2) + i*sqrt((|z| - Real(z))/2) and
      sqrt(x - i*y) = sqrt((|z| + Real(z))/2) - i*sqrt((|z| - Real(z))/2),
   
   where y is positive and x may be positive or negative.
   
   CONSTANTS:  FPKINF = +infinity
   
   Calls:  cssqs, scalb, fabs, sqrt, copysign.
****************************************************************************/

/* New Intel code written 9/26/06 by scanon
 * Uses extra precision to compute |z| instead of cssqs(), saving environment calls.
 * Note that we could also rescale using bits of the SSE2 code from ian's original intel hypot() routine, and will probably
 * want to do exactly that in the future, to move away from using x87 for this.
 */

double complex csqrt ( double complex z )
{
	static const double inf = __builtin_inf();
	double u,v;
	
	// Special case for infinite y:
	if (__builtin_fabs(Imag(z)) == inf)
		return inf + I*Imag(z);				// csqrt(x ± i∞) = ∞ ± i∞ for all x, including NaN.
	
	// Special cases for y = NaN:
	if (Imag(z) != Imag(z)) {
		if (Real(z) != Real(z))				// csqrt(NaN + iNaN) = NaN + iNaN, quietly. 
			return z; 
		else if (Real(z) == inf)			// csqrt(∞ + iNaN) = ∞ + iNaN
			return z; 
		else if (Real(z) == -inf)			// csqrt(-∞ ± iNaN) = NaN ± i∞.
			return Imag(z) + I*copysign(inf,Imag(z));
		else {								// csqrt(x + iNaN) = NaN + iNaN if x is finite.
			return Imag(z) + I*Imag(z);
		}
	}
	
	// At this point, we know that y is finite.  Deal with special cases for x:
	// Special case for x = NaN:
	if (Real(z) != Real(z)) {				// csqrt(NaN + ix) = NaN + iNaN.
		return Real(z) + I*copysign(Real(z),Imag(z));
	}
	
	// Special cases for x = 0:
	if (Real(z) == 0.0) {
		if (Imag(z) == 0.0)					// csqrt(±0 + i0) = 0 + i0, csqrt(±0 - i0) = 0 - i0.
			return I*Imag(z); 
		else {								// csqrt(0 ± iy) = sqrt(y/2) ± i sqrt(y/2).
			u = __builtin_sqrt(0.5*__builtin_fabs(Imag(z)));
			return u + I*copysign(u, Imag(z) );
		}
	}
	
	// Special cases for infinte x:
	if (Real(z) == inf)						// csqrt(∞ ± iy) = ∞ ± i0 for finite y.
		return inf + I*copysign(0.0,Imag(z));
	if (Real(z) == -inf)					// csqrt(-∞ ± iy) = 0 ± i∞ for finite y.
		return I*copysign(inf,Imag(z));
	
	// At this point, we know that x is finite, non-zero and y is finite.
	else {
		// We use extended (80-bit) precision to avoid over- or under-flow in computing |z|.
		long double x = __builtin_fabsl(Real(z));
		long double y = Imag(z);
		
		/* Compute
		 *         +----------------                       +----------------
		 *         |  |z| + |Re z|               Im z      |  |z| - |Re z|
		 *    u =  | --------------         v = ------ = ± | --------------
		 *        \|       2                      2u      \|       2
		 *
		 * There is no risk of drastic loss of precision due to cancellation using these formulas,
		 * as there would be if we used the second expression (involving the square root) for v.
		 *
		 * Overflow or Underflow is possible, but only if the actual result does not fit in double width.
		 */
		u = (double)__builtin_sqrtl(0.5L*(__builtin_sqrtl(x*x + y*y) + x));
		v = 0.5 * (Imag(z) / u);
		
		/* If x < 0, then sqrt(z) = |v| + I*copysign(u, Im z).
		 * Otherwise, sqrt(z) = u + I*v.
		 */
		if (Real(z) < 0.0) {
			return __builtin_fabs(v) + I*copysign(u,Imag(z));
		} else {
			return u + I*v;
		}
	}
}


float complex csqrtf ( float complex z )   
{
	static const float inf = __builtin_inff();
	float u,v;
	
	// Special case for infinite y:
	if (__builtin_fabsf(Imag(z)) == inf)
		return inf + I*Imag(z);			// csqrt(x ± i∞) = ∞ ± i∞ for all x, including NaN.
	
	// Special cases for y = NaN:
	if (Imag(z) != Imag(z)) {
		if (Real(z) != Real(z))			// csqrt(NaN + iNaN) = NaN + iNaN, quietly. 
			return z; 
		else if (Real(z) == inf)		// csqrt(∞ + iNaN) = ∞ + iNaN
			return z; 
		else if (Real(z) == -inf)		// csqrt(-∞ ± iNaN) = NaN ± i∞.
			return Imag(z) + I*copysignf(inf,Imag(z));
		else {							// csqrt(x + iNaN) = NaN + iNaN if x is finite.
			return Imag(z) + I*Imag(z);
		}
	}
	
	// At this point, we know that y is finite.  Deal with special cases for x:
	// Special case for x = NaN:
	if (Real(z) != Real(z)) {			// csqrt(NaN + ix) = NaN + iNaN.
		return Real(z) + I*copysignf(Real(z),Imag(z));
	}
	
	// Special cases for x = 0:
	if (Real(z) == 0.0f) {
		if (Imag(z) == 0.0f)			// csqrt(±0 + i0) = 0 + i0, csqrt(±0 - i0) = 0 - i0.
			return I*Imag(z); 
		else {							// csqrt(0 ± iy) = sqrt(y/2) ± i sqrt(y/2).
			u = __builtin_sqrtf(0.5f*__builtin_fabsf(Imag(z)));
			return u + I*copysign(u, Imag(z) );
		}
	}
	
	// Special cases for infinte x:
	if (Real(z) == inf)					// csqrt(∞ ± iy) = ∞ ± i0 for finite y.
		return inf + I*copysign(0.0f,Imag(z));
	if (Real(z) == -inf)				// csqrt(-∞ ± iy) = 0 ± i∞ for finite y.
		return I*copysign(inf,Imag(z));
	
	// At this point, we know that x is finite, non-zero and y is finite.
	else {
		// We use double (64-bit) precision to avoid over- or under-flow in computing |z|.
		double x = __builtin_fabs(Real(z));
		double y = Imag(z);
		
		/* Compute
		 *         +----------------                       +----------------
		 *         |  |z| + |Re z|               Im z      |  |z| - |Re z|
		 *    u =  | --------------         v = ------ = ± | --------------
		 *        \|       2                      2u      \|       2
		 *
		 * There is no risk of drastic loss of precision due to cancellation using these formulas,
		 * as there would be if we used the second expression (involving the square root) for v.
		 *
		 * Overflow or Underflow is possible, but only if the actual result does not fit in double width.
		 */
		u = (float)__builtin_sqrt(0.5*(__builtin_sqrt(x*x + y*y) + x));
		v = 0.5f * (Imag(z) / u);
		
		/* If x < 0, then sqrt(z) = |v| + I*copysign(u, Im z).
		 * Otherwise, sqrt(z) = u + I*v.
		 */
		if (Real(z) < 0.0f) {
			return __builtin_fabsf(v) + I*copysignf(u,Imag(z));
		} else {
			return u + I*v;
		}
	}	
} 

/****************************************************************************
   double complex clog(double complex z) returns the complex natural logarithm of its
   argument, using:
 
	 clog(x + iy) = [ log(x) + 0.5 * log1p(y^2/x^2) ] + I*carg(x + iy)     if x > y
								= [ log(y) + 0.5 * log1p(x^2/y^2) ] + I*carg(x + iy)     otherwise
 
	 the real part is "mathematically" equivalent to log |z|, but the alternative form
   is used to avoid spurious under/overflow.

   Calls:  fabs, log1p, log, carg. 
****************************************************************************/

double complex clog ( double complex z )   
{
	static const double inf = __builtin_inf();
	double large, small, temp;
	double complex w;
	long double ratio;
	
	Imag(w) = carg(z);
	
	// handle x,y = ∞
	if ((__builtin_fabs(Real(z)) == inf) || (__builtin_fabs(Imag(z)) == inf)) {
		Real(w) = inf;
		return w;
	}
	
	// handle x,y = NaN
	if (Real(z) != Real(z)) return Real(z) + I*copysign(Real(z),Imag(z));
	if (Imag(z) != Imag(z)) return Imag(z) + I*Imag(z);
	
	large = __builtin_fabs(Real(z));
	small = __builtin_fabs(Imag(z));
	if (large < small) {
		temp = large;
		large = small;
		small = temp;
	}
	
	Real(w) = log(large);
	
	// if small == 0, then Re(clog(z)) = log(large).  This sets div-by-zero when appropriate (if large is also 0).
	if (small == 0.0) return w;
	
	// if large == 1
	if (large == 1.0) {
		Real(w) = 0.5*log1p(small*small);  // any underflow here is deserved.
		return w;
	}
	
	// compute small/large in long double to avoid undue underflow.
	ratio = (long double)small / (long double)large;
	if (ratio > 0x1.0p-53L) {
		/* if ratio is smaller than 2^-53, then
		 * 1/2 log1p(ratio^2) ~ 2^-106 < 1/2 an ulp of log(large), so it can't affect the final result.
		 */
		Real(w) += 0.5*log1p((double)(ratio*ratio));
	}
	
	return w;
}

float complex clogf ( float complex z )   
{
	static const float inf = __builtin_inff();
	float large, small, temp;
	float complex w;
	double ratio;
	
	Imag(w) = cargf(z);
	
	// handle x,y = ∞
	if ((__builtin_fabsf(Real(z)) == inf) || (__builtin_fabsf(Imag(z)) == inf)) {
		Real(w) = inf;
		return w;
	}
	
	// handle x,y = NaN
	if (Real(z) != Real(z)) return Real(z) + I*copysignf(Real(z),Imag(z));
	if (Imag(z) != Imag(z)) return Imag(z) + I*Imag(z);
	
	large = __builtin_fabsf(Real(z));
	small = __builtin_fabsf(Imag(z));
	if (large < small) {
		temp = large;
		large = small;
		small = temp;
	}
	
	Real(w) = logf(large);
	
	// if small == 0, then Re(clog(z)) = log(large).  This sets div-by-zero when appropriate (if large is also 0).
	if (small == 0.0f) return w;
	
	// if large == 1
	if (large == 1.0f) {
		Real(w) = 0.5f*log1pf(small*small);  // underflow here is deserved.
		return w;
	}
	
	// compute small/large in double to avoid undue underflow.
	ratio = (double)small / (double)large;
	if (ratio > 0x1.0p-24) {
		Real(w) += 0.5f*log1pf((float)(ratio*ratio));
	}
	
	return w;
}

/****************************************************************************
 void cosisin(x, complex *z)
 returns cos(x) + i sin(x) computed using the x87 fsincos instruction.
 
 Called by:  cexp, csin, ccos, csinh, and ccosh.
 ****************************************************************************/
static void cosisin(double x, double complex *z) {
	Real(*z) = cos(x);
	Imag(*z) = sin(x);
}

static void cosisinf(float x, float complex *z) {
	Real(*z) = cosf(x);
	Imag(*z) = sinf(x);
}

/****************************************************************************
   double complex csin(double complex z) returns the complex sine of its argument.
   
      sin(z) = -i sinh(iz)

   Calls:  csinh
****************************************************************************/

double complex csin ( double complex z )   
{
	double complex iz, iw, w;
	Real(iz) = -Imag(z);
	Imag(iz) = Real(z);
	iw = csinh(iz);
	Real(w) = Imag(iw);
	Imag(w) = -Real(iw);
	return w;
}

float complex csinf ( float complex z )   
{
	float complex iz, iw, w;
	Real(iz) = -Imag(z);
	Imag(iz) = Real(z);
	iw = csinhf(iz);
	Real(w) = Imag(iw);
	Imag(w) = -Real(iw);
	return w;
}

/****************************************************************************
   double complex ccos(double complex z) returns the complex cosine of its argument.
   
      cos(z) = cosh(iz)

   Calls:  ccosh
****************************************************************************/

double complex ccos ( double complex z )   
{
	double complex iz;
	Real(iz) = -Imag(z);
	Imag(iz) = Real(z);
	return ccosh(iz);
}

float complex ccosf ( float complex z )   
{
	float complex iz;
	Real(iz) = -Imag(z);
	Imag(iz) = Real(z);
	return ccoshf(iz);
}

/****************************************************************************
   double complex csinh(double complex z) returns the complex hyperbolic sine of its
   argument.  The algorithm is based upon the identity:
   
      sinh(x + i*y) = cos(y)*sinh(x) + i*sin(y)*cosh(x).
      
   Signaling of spurious overflows, underflows, and invalids is avoided where
   possible.

   Calls:  expm1, cosisin
****************************************************************************/

double complex csinh ( double complex z )   
{
	static const double INF = __builtin_inf();
	double complex w;
	
	// Handle x = NaN first
	if (Real(z) != Real(z)) {
		Real(w) = Real(z);
		if (Imag(z) == 0.0)   // cexp(NaN + 0i) = NaN + 0i
			Imag(w) = Imag(z);
		else                  // cexp(NaN + yi) = NaN + NaNi, for y ≠ 0
			Imag(w) = copysign(Real(z), Imag(z));
		return w;
	}
	
	// At this stage, x ≠ NaN.
	double absx = __builtin_fabs(Real(z));
	double reducedx = absx;
	
	cosisin(Imag(z), &w); // set w = cos y + i sin y.
	Real(w) *= copysign(1.0, Real(z)); // w = signof(x) cos y + i sin y
	
	// Handle x = ±∞ cases.
	if ((absx == INF) && ((Imag(z) == INF) || (Imag(z) != Imag(z)) || (Imag(z) == 0.0))) {
		Real(w) = copysign(INF, Real(z));
		return w;
	}
	
	// Handle x = 0 cases.
	if (absx == 0.0) {
		Real(w) = Real(z);   // sign of zero needs to be right.
		return w;
	}
	
	// Argument reduction, if need be.  (x is now a finite non-zero number)
	if ((reducedx < twiceExpOverflowThresh_d) && (reducedx > expOverflowThreshold_d)) {
		reducedx -= expOverflowThreshold_d;    // argument reduction, this is exact.
		Real(w) *= expOverflowValue_d;		     // not exact, but good enough.
		Imag(w) *= expOverflowValue_d;         // ditto.
	}
	
	double exm1 = expm1(reducedx);  // any overflow here is deserved.
	
	if (absx < 0x1p-27) { // |x|^2 is less than an ulp of 1, so only the leading terms of the series for
		Real(w) *= absx;    // cosh = 1 + .... and sinh = x + .... has any effect on the result.
	}
	
	else if (absx > 19.0) { // if |x| > 19, then e^-x is less than an ulp of e^x, so the smaller term in
		double halfExpX = 0.5 * (exm1 + 1.0); // cosh = (e^x + e^-x) / 2 has no effect, and similarly for
		Real(w) *= halfExpX;                  // sinh = (e^x - e^-x) / 2.
		 // only scale the imag part if non-zero (to prevent NaN in overflow*zero)
		if (Imag(z) != 0.0) Imag(w) *= halfExpX;
	}
	
	else { // the "normal" case, we need to be careful.
		double twiceExpX = 2.0 * (exm1 + 1.0);
		/* we use the following to get cosh(x):
		 *
		 *      expm1(x)*expm1(x)     2e^x + e^(2x) - 2e^x + 1     e^x + e^-x
		 * 1 + ------------------- = -------------------------- = ------------ = cosh(x)
		 *      2*(1 + expm1(x))                2e^x                   2
		 */
		Imag(w) *= 1.0 + (exm1*exm1)/twiceExpX;
		/* we use the following to get sinh(x) (up to sign):
		 *
		 *  1  /               expm1(x)   \    e^(2x) - e^x + e^x - 1     e^x - e^-x
		 * --- | expm1(x) + ------------- | = ------------------------ = ------------ = sinh(x)
		 *  2  \             1 + expm1(x) /             2e^x                  2
		 */
		Real(w) *= 0.5*exm1 + exm1/twiceExpX;
	}
	
	return w;
}

float complex csinhf ( float complex z )   
{
	static const float INFf = __builtin_inff();
	static const double INF = __builtin_inf();
	float complex w;
	double complex wd;
	
	// Handle x = NaN first
	if (Real(z) != Real(z)) {
		Real(w) = Real(z);
		if (Imag(z) == 0.0f)   // cexp(NaN + 0i) = NaN + 0i
			Imag(w) = Imag(z);
		else                  // cexp(NaN + yi) = NaN + NaNi, for y ≠ 0
			Imag(w) = copysignf(Real(z), Imag(z));
		return w;
	}
	
	// At this stage, x ≠ NaN.
	double absx = (double)__builtin_fabsf(Real(z));
	
	cosisin((double)Imag(z), &wd); // set w = cos y + i sin y.
	Real(wd) *= copysign(1.0, (double)Real(z)); // w = signof(x) cos y + i sin y
	
	// Handle x = ±∞ cases.
	if ((absx == INF) && ((Imag(z) == INFf) || (Imag(z) != Imag(z)) || (Imag(z) == 0.0f))) {
		Real(w) = copysignf(INFf, Real(z));
		Imag(w) = (float)Imag(wd);
		return w;
	}
	
	// Handle x = 0 cases.
	if (absx == 0.0) {
		Real(w) = Real(z);   // sign of zero needs to be right.
		Imag(w) = (float)Imag(wd);
		return w;
	}
	
	double exm1 = expm1(absx);  // any overflow here is deserved.
	
	if (absx < 0x1p-27) {  // |x|^2 is less than an ulp of 1, so only the leading terms of the series for
		Real(wd) *= absx;    // cosh = 1 + .... and sinh = x + .... has any effect on the result.
	}
	
	else if (absx > 19.0) { // if |x| > 19, then e^-x is less than an ulp of e^x, so the smaller term in
		double halfExpX = 0.5 * (exm1 + 1.0);  // cosh = (e^x + e^-x) / 2 has no effect, and similarly for
		Real(wd) *= halfExpX;                  // sinh = (e^x - e^-x) / 2.
		// only scale the imag part if non-zero (to prevent NaN in overflow*zero)
		if (Imag(z) != 0.0f) Imag(wd) *= halfExpX;
	}
	
	else { // the "normal" case, we need to be careful.
		double twiceExpX = 2.0 * (exm1 + 1.0);
		/* we use the following to get cosh(x):
		 *
		 *      expm1(x)*expm1(x)     2e^x + e^(2x) - 2e^x + 1     e^x + e^-x
		 * 1 + ------------------- = -------------------------- = ------------ = cosh(x)
		 *      2*(1 + expm1(x))                2e^x                   2
		 */
		Imag(wd) *= 1.0 + (exm1*exm1)/twiceExpX;
		/* we use the following to get sinh(x) (up to sign):
		 *
		 *  1  /               expm1(x)   \    e^(2x) - e^x + e^x - 1     e^x - e^-x
		 * --- | expm1(x) + ------------- | = ------------------------ = ------------ = sinh(x)
		 *  2  \             1 + expm1(x) /             2e^x                  2
		 */
		Real(wd) *= 0.5*exm1 + exm1/twiceExpX;
	}
	
	Real(w) = (float)Real(wd);
	Imag(w) = (float)Imag(wd);
	return w;
}

/****************************************************************************
   double complex ccosh(double complex z) returns the complex hyperbolic cosine of its
   argument.  The algorithm is based upon the identity:
   
      cosh(x + i*y) = cos(y)*cosh(x) + i*sin(y)*sinh(x).
      
   Signaling of spurious overflows, underflows, and invalids is avoided where
   possible.

   Calls:  expm1, cosisin
****************************************************************************/

double complex ccosh ( double complex z )   
{
	static const double INF = __builtin_inf();
	double complex w;
	
	// Handle x = NaN first
	if (Real(z) != Real(z)) {
		Real(w) = Real(z);
	if (Imag(z) == 0.0)   // cexp(NaN + 0i) = NaN + 0i
		Imag(w) = Imag(z);
	else                  // cexp(NaN + yi) = NaN + NaNi, for y ≠ 0
		Imag(w) = copysign(Real(z), Imag(z));
	return w;
	}
	
	// At this stage, x ≠ NaN.
	double absx = __builtin_fabs(Real(z));
	double reducedx = absx;
	
	cosisin(Imag(z), &w); // set w = cos y + i sin y.
	Imag(w) *= copysign(1.0, Real(z)); // w = cos y + i sin y * signof(x)
	
	// Handle x = ±∞ cases.
	if ((absx == INF) && ((Imag(z) == INF) || (Imag(z) != Imag(z)) || (Imag(z) == 0.0))) {
		Real(w) = INF;
		return w;
	}
	
	// Handle x = 0 cases.
	if (absx == 0.0) {
		Imag(w) = Real(z) * copysign(1.0, Imag(z));   // finesse the sign of zero.
		return w;
	}
	
	// Argument reduction, if need be.  (x is now a finite non-zero number)
	if ((reducedx < twiceExpOverflowThresh_d) && (reducedx > expOverflowThreshold_d)) {
		reducedx -= expOverflowThreshold_d;    // argument reduction, this is exact.
		Real(w) *= expOverflowValue_d;		     // not exact, but good enough.
		Imag(w) *= expOverflowValue_d;         // ditto.
	}
	
	double exm1 = expm1(reducedx);  // any overflow here is deserved.
	
	if (absx < 0x1p-27) { // |x|^2 is less than an ulp of 1, so only the leading terms of the series for
		Imag(w) *= absx;    // cosh = 1 + .... and sinh = x + .... has any effect on the result.
	}
	
	else if (absx > 19.0) { // if |x| > 19, then e^-x is less than an ulp of e^x, so the smaller term in
		double halfExpX = 0.5 * (exm1 + 1.0); // cosh = (e^x + e^-x) / 2 has no effect, and similarly for
		Real(w) *= halfExpX;                  // sinh = (e^x - e^-x) / 2.
		 // only scale the imag part if non-zero (to prevent NaN in overflow*zero)
		if (Imag(z) != 0.0) Imag(w) *= halfExpX;
	}
	
	else { // the "normal" case, we need to be careful.
		double twiceExpX = 2.0 * (exm1 + 1.0);
		/* we use the following to get cosh(x):
		 *
		 *      expm1(x)*expm1(x)     2e^x + e^(2x) - 2e^x + 1     e^x + e^-x
		 * 1 + ------------------- = -------------------------- = ------------ = cosh(x)
		 *      2*(1 + expm1(x))                2e^x                   2
		 */
		Real(w) *= 1.0 + (exm1*exm1)/twiceExpX;
		/* we use the following to get sinh(x) (up to sign):
		 *
		 *  1  /               expm1(x)   \    e^(2x) - e^x + e^x - 1     e^x - e^-x
		 * --- | expm1(x) + ------------- | = ------------------------ = ------------ = sinh(x)
		 *  2  \             1 + expm1(x) /             2e^x                  2
		 */
		Imag(w) *= 0.5*exm1 + exm1/twiceExpX;
	}
	
	return w;
}

float complex ccoshf ( float complex z )   
{
	static const float INFf = __builtin_inff();
	static const double INF = __builtin_inf();
	double complex wd;
	float complex w;
	
	// Handle x = NaN first
	if (Real(z) != Real(z)) {
		Real(w) = Real(z);
		if (Imag(z) == 0.0f)   // cexp(NaN + 0i) = NaN + 0i
			Imag(w) = Imag(z);
		else                  // cexp(NaN + yi) = NaN + NaNi, for y ≠ 0
			Imag(w) = copysignf(Real(z), Imag(z));
		return w;
	}
	
	// At this stage, x ≠ NaN.
	double absx = (double)__builtin_fabsf(Real(z));
	
	cosisin((double)Imag(z), &wd); // set w = cos y + i sin y.
	Imag(wd) *= copysign(1.0, (double)Real(z)); // w = cos y + i sin y * signof(x)
	
	// Handle x = ±∞ cases.
	if ((absx == INF) && ((Imag(z) == INFf) || (Imag(z) != Imag(z)) || (Imag(z) == 0.0f))) {
		Real(w) = INFf;
		Imag(w) = (float)Imag(wd);
		return w;
	}
	
	// Handle x = 0 cases.
	if (absx == 0.0) {
		Imag(w) = Real(z) * copysignf(1.0f, Imag(z));   // finesse the sign of zero.
		Real(w) = (float)Real(wd);
		return w;
	}
	
	double exm1 = expm1(absx);  // any overflow here is deserved.
	
	if (absx < 0x1p-27) {  // |x|^2 is less than an ulp of 1, so only the leading terms of the series for
		Imag(wd) *= absx;    // cosh = 1 + .... and sinh = x + .... has any effect on the result.
	}
	
	else if (absx > 19.0) { // if |x| > 19, then e^-x is less than an ulp of e^x, so the smaller term in
		double halfExpX = 0.5 * (exm1 + 1.0);  // cosh = (e^x + e^-x) / 2 has no effect, and similarly for
		Real(wd) *= halfExpX;                  // sinh = (e^x - e^-x) / 2.
		// only scale the imag part if non-zero (to prevent NaN in overflow*zero)
		if (Imag(z) != 0.0) Imag(wd) *= halfExpX;
	}
	
	else { // the "normal" case, we need to be careful.
		double twiceExpX = 2.0 * (exm1 + 1.0);
		/* we use the following to get cosh(x):
		 *
		 *      expm1(x)*expm1(x)     2e^x + e^(2x) - 2e^x + 1     e^x + e^-x
		 * 1 + ------------------- = -------------------------- = ------------ = cosh(x)
		 *      2*(1 + expm1(x))                2e^x                   2
		 */
		Real(wd) *= 1.0 + (exm1*exm1)/twiceExpX;
		/* we use the following to get sinh(x) (up to sign):
		 *
		 *  1  /               expm1(x)   \    e^(2x) - e^x + e^x - 1     e^x - e^-x
		 * --- | expm1(x) + ------------- | = ------------------------ = ------------ = sinh(x)
		 *  2  \             1 + expm1(x) /             2e^x                  2
		 */
		Imag(wd) *= 0.5*exm1 + exm1/twiceExpX;
	}
		
	Real(w) = (float)Real(wd);
	Imag(w) = (float)Imag(wd);
	return w;
}

/****************************************************************************
   double complex cexp(double complex z) returns the complex exponential of its
   argument.  The algorithm is based upon the identity:
   
      exp(x + i*y) = cos(y)*exp(x) + i*sin(y)*exp(x).
      
   Signaling of spurious overflows and invalids is avoided where
   possible.
   
   CONSTANTS:  expOverflowValue_d = exp(709.0) to double precision

   Calls:  cosisin and exp.
****************************************************************************/

double complex cexp ( double complex z )   
{
	static const double INF = __builtin_inf();
	double complex w;
	
	// Handle x = NaN first
	if (Real(z) != Real(z)) {
		Real(w) = Real(z);
		if (Imag(z) == 0.0)   // cexp(NaN + 0i) = NaN + 0i
			Imag(w) = Imag(z);
		else                  // cexp(NaN + yi) = NaN + NaNi, for y ≠ 0
			Imag(w) = copysign(Real(z), Imag(z));
		return w;
	}
	
	// Handle x = -∞, y = ∞ or NaN:
	if ((Real(z) == -INF) && ((__builtin_fabs(Imag(z)) == INF) || (Imag(z) != Imag(z)))) {
		Real(w) = 0.0;
		Imag(w) = copysign(0.0, Imag(z));
		return w;
	}
	
	if (Imag(z) == 0.0) {  // exact exp(x + 0i) case.
		Real(w) = exp(Real(z));
		Imag(w) = 0.0;
		return w;
	}
	
	// At this stage, x ≠ NaN, and extraordinary x = -∞ cases are sorted.  y ≠ 0.
	cosisin(Imag(z), &w); // set w = cos(y) + i sin(y)
	
	// Handle x = +∞ cases.
	if ((Real(z) == INF) && ((Imag(z) == INF) || (Imag(z) != Imag(z)))) {
		Real(w) = INF;      // cexp(∞ + yi) = ∞ + NaNi for y = NaN or ∞.
		return w;           // cexp(∞ + yi) for finite y falls through.
	}
	
	// At this point, x ≠ NaN, +inf, y ≠ 0, and all remaining cases fall through
	double x = Real(z);
	
	if ((x < twiceExpOverflowThresh_d) && (x > expOverflowThreshold_d)) {
		x -= expOverflowThreshold_d;       // argument reduction, this is exact.
		Real(w) *= expOverflowValue_d;     // not exact, but good enough.
		Imag(w) *= expOverflowValue_d;     // ditto.
	}
	
	double scale = exp(x);
	Real(w) *= scale;
	Imag(w) *= scale;
	return w;
}

float complex cexpf ( float complex z )   
{
	static const float INFf = __builtin_inff();
	float complex w;
	
	// Handle x = NaN first
	if (Real(z) != Real(z)) {
		Real(w) = Real(z);
		if (Imag(z) == 0.0f)   // cexp(NaN + 0i) = NaN + 0i
			Imag(w) = Imag(z);
		else                  // cexp(NaN + yi) = NaN + NaNi, for y ≠ 0
			Imag(w) = copysignf(Real(z), Imag(z));
		return w;
	}
	
	// Handle x = -∞, y = ∞ or NaN:
	if ((Real(z) == -INFf) && ((__builtin_fabsf(Imag(z)) == INFf) || (Imag(z) != Imag(z)))) {
		Real(w) = 0.0f;
		Imag(w) = copysignf(0.0f, Imag(z));
		return w;
	}
	
	if (Imag(z) == 0.0f) {  // exact exp(x + 0i) case.
		Real(w) = expf(Real(z));
		Imag(w) = 0.0f;
		return w;
	}
	
	double complex wd;
	// At this stage, x ≠ NaN, and extraordinary x = -∞ cases are sorted.  y ≠ 0.
	cosisin((double)Imag(z), &wd); // set w = cos(y) + i sin(y)
	
	// Handle x = +∞ cases.
	if ((Real(z) == INFf) && ((Imag(z) == INFf) || (Imag(z) != Imag(z)))) {
		Real(w) = INFf;      // cexp(∞ + yi) = ∞ + NaNi for y = NaN or ∞.
		Imag(w) = (float)Imag(wd);
		return w;           // cexp(∞ + yi) for finite y falls through.
	}
	
	// At this point, x ≠ NaN, +inf, y ≠ 0, and all remaining cases fall through

	double scale = exp((double)Real(z));
	Real(w) = (float)(scale*Real(wd));
	Imag(w) = (float)(scale*Imag(wd));
	return w;
}
      
/****************************************************************************
   double complex cpow(double complex x, double complex y) returns the complex result of x^y.
   The algorithm is based upon the identity:
   
      x^y = cexp(y*clog(x)).
      
   Calls:  clog, cexp.
****************************************************************************/

double complex cpow ( double complex x, double complex y )     /* (complex x)^(complex y) */
{
   double complex logval,z;
   
   logval = clog(x);                        /* complex logarithm of x */
   Real(z) = Real(y)*Real(logval) - Imag(y)*Imag(logval);  /* multiply by y */
   Imag(z) = Real(y)*Imag(logval) + Imag(y)*Real(logval);
   return (cexp(z));                        /* return complex exponential */
}

float complex cpowf ( float complex x, float complex y )     /* (complex x)^(complex y) */
{
   float complex logval,z;
   
   logval = clogf(x);                        /* complex logarithm of x */
   Real(z) = Real(y)*Real(logval) - Imag(y)*Imag(logval);  /* multiply by y */
   Imag(z) = Real(y)*Imag(logval) + Imag(y)*Real(logval);
   return (cexpf(z));                        /* return complex exponential */
}

/****************************************************************************
   double complex ctanh(double complex x) returns the complex hyperbolic tangent of its
   argument.  The algorithm is from Kahan's paper and is based on the
   identity:
   
      tanh(x+i*y) = (sinh(2*x) + i*sin(2*y))/(cosh(2*x) + cos(2*y))
                  = (cosh(x)*sinh(x)*cscsq + i*tan(y))/(1+cscsq*sinh(x)*sinh(x)),
   
   where cscsq = 1/(cos(y)*cos(y).  For large values of ze.re, spurious
   overflow and invalid signaling is avoided.
   
   CONSTANTS:  FPKASINHOM4 = asinh(nextafterd(+infinity,0.0))/4.0 to double
                  precision
               FPKINF = +infinity
      
   Calls:  tan, sinh, sqrt, fabs, feholdexcept, feraiseexcept, feclearexcept,
           and feupdateenv.
****************************************************************************/

double complex ctanh( double complex z )
{
	static const double INF = __builtin_inf();
	double x = __builtin_fabs(Real(z));
	double y = __builtin_fabs(Imag(z));
	double sinhval, coshval, tanval, exm1, cscsq;
	double complex w;
	
	if (x == INF) {
		w = 1.0 + I*copysign(0.0, sin(2.0*y));  // ctanh(∞ + iy) = 1.0 ± i0
	}
	
	else if (Imag(z) != Imag(z) || Real(z) != Real(z)) {
		if (Imag(z) == 0.0) {
			w = Real(z) + I*0.0;                  // ctanh(NaN + i0) = NaN + i0
		} else {
			Real(w) = Real(z) + Imag(z);          // ctanh(NaN) = NaN + iNaN
			Imag(w) = Real(w);
		}
	}
	
	else if (y == INF) {
		Real(w) = y - y;                        // ctanh(x + i∞) = NaN + iNaN (invalid)
		Imag(w) = Real(w);
	}
	
	else if (x > 19.0) {
		w = 1.0 + I*copysign(0.0, sin(2.0*y));  // if x is big, tanh(z) = 1 ± i0
	}
	
	else {                                    // edge case free!
		tanval = tan(y);
		cscsq = 1.0 + tanval*tanval;            // cscsq = 1/cos^2(y)
		
		if (x < 0x1p-27) {
			coshval = 1.0;
			sinhval = x;
		} else {
			exm1 = expm1(x);
			coshval = 1.0 + 0.5*(exm1*exm1)/(exm1 + 1.0);
			sinhval = 0.5*(exm1 + exm1/(exm1 + 1.0));
		}
		
		Real(w) = cscsq * coshval * sinhval / (1.0 + cscsq * sinhval * sinhval);
		Imag(w) = tanval / (1.0 + cscsq * sinhval * sinhval);
	}
	
	// Patch up signs of return value
	Real(w) = copysign(Real(w),Real(z));
	Imag(w) *= copysign(1.0,Imag(z));
	
	return w;
}

float complex ctanhf( float complex z )
{
	static const float INFf = __builtin_inff();
	float x = __builtin_fabsf(Real(z));
	float y = __builtin_fabsf(Imag(z));
	double sinhval, coshval, tanval, exm1, cscsq;
	float complex w;
	
	if (x == INFf) {
		w = 1.0f + I*copysignf(0.0f, sinf(2.0f*y));  // ctanh(∞ + iy) = 1.0 ± i0
	}
	
	else if (Imag(z) != Imag(z) || Real(z) != Real(z)) {
		if (Imag(z) == 0.0f) {
			w = Real(z) + I*0.0f;                  // ctanh(NaN + i0) = NaN + i0
		} else {
			Real(w) = Real(z) + Imag(z);          // ctanh(NaN) = NaN + iNaN
			Imag(w) = Real(w);
		}
	}
	
	else if (y == INFf) {
		Real(w) = y - y;                        // ctanh(x + i∞) = NaN + iNaN (invalid)
		Imag(w) = Real(w);
	}
	
	else if (x > 19.0f) {
		w = 1.0f + I*copysignf(0.0f, sinf(2.0f*y));  // if x is big, tanh(z) = 1 ± i0
	}
	
	else {                                    // edge case free!
		tanval = (double)tanf(y);
		cscsq = 1.0 + tanval*tanval;            // cscsq = 1/cos^2(y)
		
		if (x < 0x1p-13f) {
			coshval = 1.0;
			sinhval = x;
		} else {
			exm1 = (double)expm1f(x);
			coshval = 1.0 + 0.5*(exm1*exm1)/(exm1 + 1.0);
			sinhval = 0.5*(exm1 + exm1/(exm1 + 1.0));
		}
		
		Real(w) = (float)(cscsq * coshval * sinhval / (1.0 + cscsq * sinhval * sinhval));
		Imag(w) = (float)(tanval / (1.0 + cscsq * sinhval * sinhval));
	}
	
	// Patch up signs of return value
	Real(w) = copysignf(Real(w),Real(z));
	Imag(w) *= copysignf(1.0f,Imag(z));
	
	return w;
}

/****************************************************************************
   double complex ctan(double complex x) returns the complex hyperbolic tangent of its
   argument.  Per C99,
 
			i ctan(z) = ctanh(iz)
      
   Calls:  ctanh
****************************************************************************/

double complex ctan( double complex z )
{
	double complex iz, iw, w;
	Real(iz) = -Imag(z);
	Imag(iz) = Real(z);
	iw = ctanh(iz);
	Real(w) = Imag(iw);
	Imag(w) = -Real(iw);
	return w;
}

float complex ctanf( float complex z )
{
	float complex iz, iw, w;
	Real(iz) = -Imag(z);
	Imag(iz) = Real(z);
	iw = ctanhf(iz);
	Real(w) = Imag(iw);
	Imag(w) = -Real(iw);
	return w;
}
   
/****************************************************************************
   double complex casin(double complex z) returns the complex inverse sine of its
   argument.  The algorithm is from Kahan's paper and is based on the
   formulae:
   
       real(casin(z)) = atan (real(z)/real(csqrt(1.0-z)*csqrt(1.0+z)))
       imag(casin(z)) = arcsinh(imag(csqrt(1.0-cconj(z))*csqrt(1.0+z)))
      
   Calls:  arcsinh, csqrt, atan, feholdexcept, feclearexcept, feupdateenv.
****************************************************************************/

double complex casin ( double complex z )
{
	double complex iz, iw, w;
	Real(iz) = -Imag(z);
	Imag(iz) = Real(z);
	iw = casinh(iz);
	Real(w) = Imag(iw);
	Imag(w) = -Real(iw);
	return w;
}

float complex casinf ( float complex z )
{
	float complex iz, iw, w;
	Real(iz) = -Imag(z);
	Imag(iz) = Real(z);
	iw = casinhf(iz);
	Real(w) = Imag(iw);
	Imag(w) = -Real(iw);
	return w;
}

/****************************************************************************
   double complex casinh(double complex z) returns the complex inverse hyperbolic sine of
   its argument.  We compute the function only in the upper-right quadrant of the complex
   plane, and use the facts that casinh(conj(z)) = conj(casinh(z)) and casinh is odd to
   get the values on the rest of the plane.
   
	 within the upper-right quadrant, we use:

     casinh(z) = z if |z| is small,
                 ln(2z) if |z| is big,
                 and a rather complicated expression for other values of z.
      
   Calls:  asinh, csqrt, atan2.
****************************************************************************/

double complex casinh ( double complex z )   
{
	static const double INF = __builtin_inf();
	static const double ln2 =     0x1.62e42fefa39ef358p-1;
	static const double sqrt1_2 = 0x1.6a09e667f3bcc908p-1;
	double complex w;
	double x = __builtin_fabs(Real(z));
	double y = __builtin_fabs(Imag(z));
	double u, xSquared, tmp;
	double complex sqrt1Plusiz, sqrt1PlusizBar;
	
	// If |z| == inf, then casinh(z) = inf + carg(z)
	if ((x == INF) || (y == INF)) {
		Real(w) = INF;
		Imag(w) = atan2(y,x);
	}
	
	// If z = NaN, casinh(z) = NaN, with the special case that casinh(NaN + i0) = NaN + i0.
	else if ((x != x) || (y != y)) {
		if (y == 0.0)
			w = z;
		else {
			Real(w) = x + y;
			Imag(w) = x + y;
		}
	}

	// at this point x,y are finite, non-nan.
	else {
		// If z is small, then casinh(z) = z - z^3/6 + ... = z within half an ulp
		if ((x < 0x1p-27) && (y < 0x1p-27)) {
			Real(w) = x;
			Imag(w) = y;
		}
		
		// If z is big, then casinh(z) = log2 + log(z) + terms smaller than half an ulp
		else if ((x > 0x1p27) || (y > 0x1p27)) {
			w = clog(x + I*y);
			Real(w) += ln2;
		}
		
		/* Otherwise, use the expressions from Kahan's "Much ado about nothing's sign bit..."
		 *
		 * Derivation of these formulats boggles the mind, but they are easily verified with the
		 * Monodromy theorem.
		 */
		else {
			// Compute sqrt1Plusiz = sqrt(1-y + ix)
			u = 1.0 - y;
			xSquared = (x < 0x1p-106 ? 0.0 : x*x); // Avoid underflows.  Faster via mask?
			
			if (u == 0.0) {
				Real(sqrt1Plusiz) = sqrt1_2 * __builtin_sqrt(x); // Avoid spurious underflows in this case
				Imag(sqrt1Plusiz) = Real(sqrt1Plusiz);           // by using the simpler formula.
			}
			
			else {  // No underflow or overflow is possible.
				Real(sqrt1Plusiz) = __builtin_sqrt(0.5*(__builtin_sqrt(u*u + xSquared) + __builtin_fabs(u)));
				tmp = 0.5 * (x / Real(sqrt1Plusiz));
				
				if (u < 0.0) {
					Imag(sqrt1Plusiz) = Real(sqrt1Plusiz);
					Real(sqrt1Plusiz) = tmp;
				} else {
					Imag(sqrt1Plusiz) = tmp;
				}
			}
			
			// Compute sqrt1PlusizBar = sqrt(1+y + ix).  No underflow or overflow is possible.
			u = 1.0 + y;
			Real(sqrt1PlusizBar) = __builtin_sqrt(0.5*(__builtin_sqrt(u*u + xSquared) + u));
			Imag(sqrt1PlusizBar) = x / (2.0*Real(sqrt1PlusizBar));
			
			// Magic formulas from Kahan.
			Real(w) = asinh(Real(sqrt1Plusiz)*Imag(sqrt1PlusizBar) + Imag(sqrt1Plusiz)*Real(sqrt1PlusizBar));
			Imag(w) = atan2(y, Real(sqrt1Plusiz)*Real(sqrt1PlusizBar) + Imag(sqrt1Plusiz)*Imag(sqrt1PlusizBar));
		}
	}
	
	// Patch up signs to handle z in quadrants II - IV, using symmetry.
	Real(w) = copysign(Real(w), Real(z));
	Imag(w) = copysign(Imag(w), Imag(z));
	
	return w;
}

float complex casinhf ( float complex z )   
{
	static const float INFf = __builtin_inff();
	static const float ln2f = 0x1.62e42fefa39ef358p-1f;
	static const float sqrt1_2f = 0x1.6a09e667f3bcc908p-1f;
	float complex w;
	float x = __builtin_fabsf(Real(z));
	float y = __builtin_fabsf(Imag(z));
	float u, xSquared, tmp;
	float complex sqrt1Plusiz, sqrt1PlusizBar;
	
	// If |z| == inf, then casinh(z) = inf + carg(z)
	if ((x == INFf) || (y == INFf)) {
		Real(w) = INFf;
		Imag(w) = atan2f(y,x);
	}
	
	// If z = NaN, casinh(z) = NaN, with the special case that casinh(NaN + i0) = NaN + i0.
	else if ((x != x) || (y != y)) {
		if (y == 0.0f)
			w = z;
		else {
			Real(w) = x + y;
			Imag(w) = x + y;
		}
	}
	
	// at this point x,y are finite, non-nan.
	else {
		// If z is small, then casinhf(z) = z - z^3/6 + ... = z within half an ulp
		if ((x < 0x1p-13f) && (y < 0x1p-13f)) {
			Real(w) = x;
			Imag(w) = y;
		}
		
		// If z is big, then casinh(z) = log2 + log(z) + terms smaller than half an ulp
		else if ((x > 0x1p13f) || (y > 0x1p13f)) {
			w = clogf(x + I*y);
			Real(w) += ln2f;
		}
		
		/* Otherwise, use the expressions from Kahan's "Much ado about nothing's sign bit..."
		 *
		 * Derivation of these formulats boggles the mind, but they are easily verified with the
		 * Monodromy theorem.
		 */
		else {
			// Compute sqrt1Plusiz = sqrt(1-y + ix)
			u = 1.0f - y;
			xSquared = (x < 0x1p-52f ? 0.0f : x*x); // Avoid underflows.  Faster via mask?
			
			if (u == 0.0f) {
				Real(sqrt1Plusiz) = sqrt1_2f * __builtin_sqrtf(x); // Avoid spurious underflows in this case
				Imag(sqrt1Plusiz) = Real(sqrt1Plusiz);             // by using the simpler formula.
			}
			
			else {  // No underflow or overflow is possible.
				Real(sqrt1Plusiz) = __builtin_sqrtf(0.5f*(__builtin_sqrtf(u*u + xSquared) + __builtin_fabsf(u)));
				tmp = 0.5f * (x / Real(sqrt1Plusiz));
				
				if (u < 0.0f) {
					Imag(sqrt1Plusiz) = Real(sqrt1Plusiz);
					Real(sqrt1Plusiz) = tmp;
				} else {
					Imag(sqrt1Plusiz) = tmp;
				}
			}
			
			// Compute sqrt1PlusizBar = sqrt(1+y + ix).  No underflow or overflow is possible.
			u = 1.0f + y;
			Real(sqrt1PlusizBar) = __builtin_sqrtf(0.5f*(__builtin_sqrtf(u*u + xSquared) + u));
			Imag(sqrt1PlusizBar) = x / (2.0f*Real(sqrt1PlusizBar));
			
			// Magic formulas from Kahan.
			Real(w) = asinhf(Real(sqrt1Plusiz)*Imag(sqrt1PlusizBar) + Imag(sqrt1Plusiz)*Real(sqrt1PlusizBar));
			Imag(w) = atan2f(y, Real(sqrt1Plusiz)*Real(sqrt1PlusizBar) + Imag(sqrt1Plusiz)*Imag(sqrt1PlusizBar));
		}
	}
	
	// Patch up signs to handle z in quadrants II - IV, using symmetry.
	Real(w) = copysignf(Real(w), Real(z));
	Imag(w) = copysignf(Imag(w), Imag(z));
	
	return w;
}

/****************************************************************************
   double complex cacos(double complex z) returns the complex inverse cosine of its
   argument.  The algorithm is from Kahan's paper and is based on the
   formulae:
   
       real(cacos(z)) = 2.0*atan(real(csqrt(1.0-z)/real(csqrt(1.0+z))))
       imag(cacos(z)) = arcsinh(imag(csqrt(1.0-z)*csqrt(cconj(1.0+z))))
      
   Calls:  arcsinh, csqrt, atan, feholdexcept, feclearexcept, feupdateenv.
****************************************************************************/

double complex cacos ( double complex z )
{
	static const double INF = __builtin_inf();
	static const double ln2 =     0x1.62e42fefa39ef358p-1;
	static const double sqrt1_2 = 0x1.6a09e667f3bcc908p-1;
	static const double pi2 =     0x1.921fb54442d1846ap0;

	double complex w;
	double x = __builtin_fabs(Real(z));
	double y = __builtin_fabs(Imag(z));
	double u, ySquared, tmp;
	double complex sqrt1Plusz, sqrt1Minusz;
	
	// If |z| == inf, then cacos(z) = carg(z) - inf * I
	if ((x == INF) || (y == INF)) {
		Imag(w) = -INF;
		Real(w) = atan2(y,x);
	}
	
	// If z = NaN, cacos(z) = NaN, with the special case that cacos(0 + iNaN) = π/2 + iNaN.
	else if ((x != x) || (y != y)) {
		if (x == 0.0)
			Real(w) = pi2;
		else
			Real(w) = x + y;
		Imag(w) = x + y;
	}
	
	// at this point x,y are finite, non-nan.
	else {
		// If z is small, then cacos(z) = π/2 - z + z^3/6 + ... = π/2 - z within half an ulp
		if ((x < 0x1p-27) && (y < 0x1p-27)) {
			Real(w) = pi2 - x;
			Imag(w) = -y;
		}
		
		// If z is big, then cacos(z) = -i * (log2 + log(z)) + terms smaller than half an ulp
		else if ((x > 0x1p27) || (y > 0x1p27)) {
			w = -I*(clog(x + I*y) + ln2);
		}
		
		/* Otherwise, use the expressions from Kahan's "Much ado about nothing's sign bit..."
		 *
		 * Real(w) = 2*atan2( Re(sqrt(1-z)), Re(sqrt(1+z)) )
		 * Imag(w) = asinh( Im( sqrt(1-z)*sqrt(1+conj(z)) ) )
		 *
		 * Derivation of these formulats boggles the mind, but they are easily verified with the
		 * Monodromy theorem.  Analysis of roundoff is a bit harder, but goes though just fine.
		 */
		else {
			ySquared = (y < 0x1p-106 ? 0.0 : y*y); // Avoid underflows.  Faster via mask?
			
			// Compute sqrt1Plusz = sqrt(1+x + iy)
			u = 1.0 + x;
			Real(sqrt1Plusz) = __builtin_sqrt(0.5*(__builtin_sqrt(u*u + ySquared) + u));
			Imag(sqrt1Plusz) = 0.5 * (y / Real(sqrt1Plusz));
			
			// Compute sqrt1Minusz = sqrt(1-x - iy)
			u = 1.0 - x;
			
			if (u == 0.0) {
				Real(sqrt1Minusz) = sqrt1_2 * __builtin_sqrt(y); // Avoid spurious underflows in this case
				Imag(sqrt1Minusz) = -Real(sqrt1Minusz);          // by using the simpler formula.
			}
			
			else {  // No underflow or overflow is possible.
				Real(sqrt1Minusz) = __builtin_sqrt(0.5*(__builtin_sqrt(u*u + ySquared) + __builtin_fabs(u)));
				tmp = 0.5 * (y / Real(sqrt1Minusz));
			
				if (u < 0.0) {
					Imag(sqrt1Minusz) = -Real(sqrt1Minusz);
					Real(sqrt1Minusz) = tmp;
				} else {
					Imag(sqrt1Minusz) = -tmp;
				}
			}
			
			// Magic formulas from Kahan.
			Real(w) = 2.0 * atan2(Real(sqrt1Minusz), Real(sqrt1Plusz));
			Imag(w) = asinh( Real(sqrt1Plusz)*Imag(sqrt1Minusz) - Imag(sqrt1Plusz)*Real(sqrt1Minusz) );
		}
	}
	
	// Patch up signs to handle z in quadrants II, III & IV, using symmetry.
	Imag(w) = copysign(Imag(w), -Imag(z));
	
	if (Real(z) < 0.0)
		Real(w) = 2.0 * pi2 - Real(w); // No undue cancellation is possible here - Real(w) < π/2.
	
	return w;
}
	
float complex cacosf ( float complex z )
{
	static const float INFf = __builtin_inff();
	static const float ln2f = 0x1.62e42fefa39ef358p-1f;
	static const float pi2f = 0x1.921fb54442d1846ap0f;
	static const float sqrt1_2f = 0x1.6a09e667f3bcc908p-1f;
	
	float complex w;
	float x = __builtin_fabsf(Real(z));
	float y = __builtin_fabsf(Imag(z));
	float u, ySquared, tmp;
	float complex sqrt1Plusz, sqrt1Minusz;
	
	// If |z| == inf, then cacos(z) = carg(z) - inf i
	if ((x == INFf) || (y == INFf)) {
		Imag(w) = -INFf;
		Real(w) = atan2f(y,x);
	}
	
	// If z = NaN, cacos(z) = NaN, with the special case that cacos(0 + iNaN) = π/2 + iNaN.
	else if ((x != x) || (y != y)) {
		
		if (x == 0.0f)
			Real(w) = pi2f;
		else
			Real(w) = x + y;
		
		Imag(w) = x + y;
	}
	
	// at this point x,y are finite, non-nan.
	else {
		// If z is small, then cacos(z) = π/2 - z + z^3/6 + ... = π/2 - z within half an ulp
		if ((x < 0x1p-13f) && (y < 0x1p-13f)) {
			Real(w) = pi2f - x;
			Imag(w) = -y;
		}
		
		// If z is big, then cacos(z) = -i * (log2 + log(z)) + terms smaller than half an ulp
		else if ((x > 0x1p13f) || (y > 0x1p13f)) {
			w = -I*(clogf(x + I*y) + ln2f);
		}
		
		/* Otherwise, use the expressions from Kahan's "Much ado about nothing's sign bit..."
		 *
		 * Real(w) = 2*atan2( Re(sqrt(1-z)), Re(sqrt(1+z)) )
		 * Imag(w) = asinh( Im( sqrt(1-z)*sqrt(1+conj(z)) ) )
		 *
		 * Derivation of these formulats boggles the mind, but they are easily verified with the
		 * Monodromy theorem.  Analysis of roundoff is a bit harder, but goes though just fine.
		 */
		else {
			ySquared = (y < 0x1p-52f ? 0.0f : y*y); // Avoid underflows.  Faster via mask?
			
			// Compute sqrt1Plusz = sqrt(1+x + iy)
			u = 1.0f + x;
			Real(sqrt1Plusz) = __builtin_sqrtf(0.5f*(__builtin_sqrtf(u*u + ySquared) + u));
			Imag(sqrt1Plusz) = 0.5f * (y / Real(sqrt1Plusz));
			
			// Compute sqrt1Minusz = sqrt(1-x - iy)
			u = 1.0f - x;
			
			if (u == 0.0f) {
				Real(sqrt1Minusz) = sqrt1_2f * __builtin_sqrtf(y);
				Imag(sqrt1Minusz) = -Real(sqrt1Minusz);
			}
			
			else {
				Real(sqrt1Minusz) = __builtin_sqrtf(0.5f*(__builtin_sqrtf(u*u + ySquared) + __builtin_fabsf(u)));
				tmp = 0.5f * (y / Real(sqrt1Minusz));
			
				if (u < 0.0f) {
					Imag(sqrt1Minusz) = -Real(sqrt1Minusz);
					Real(sqrt1Minusz) = tmp;
				} else {
					Imag(sqrt1Minusz) = -tmp;
				}
			}
			
			// Magic formulas from Kahan.
			Real(w) = 2.0f * atan2f(Real(sqrt1Minusz),Real(sqrt1Plusz));
			Imag(w) = asinhf( Real(sqrt1Plusz)*Imag(sqrt1Minusz) - Imag(sqrt1Plusz)*Real(sqrt1Minusz) );
		}
	}
	
	// Patch up signs to handle z in quadrants II, III & IV, using symmetry.
	Imag(w) = copysignf(Imag(w), -Imag(z));
	
	if (Real(z) < 0.0f)
		Real(w) = 2.0f * pi2f - Real(w); // No undue cancellation is possible here - Real(w) < π/2.
	
	return w;
}

/****************************************************************************
   double complex cacosh(double complex z) returns the complex inverse hyperbolic`cosine
   of its argument.  The algorithm is from Kahan's paper and is based on the
   formulae:
   
       real(cacosh(z)) = arcsinh(real(csqrt(cconj(z)-1.0)*csqrt(z+1.0)))
       imag(cacosh(z)) = 2.0*atan(imag(csqrt(z-1.0)/real(csqrt(z+1.0))))
      
   Calls:  arcsinh, csqrt, atan, feholdexcept, feclearexcept, feupdateenv.
****************************************************************************/

double complex cacosh ( double complex z )
{
	static const double INF = __builtin_inf();
	static const double ln2 =     0x1.62e42fefa39ef358p-1;
	static const double sqrt1_2 = 0x1.6a09e667f3bcc908p-1;
	static const double pi2 =     0x1.921fb54442d1846ap0;
	
	double complex w;
	double x = __builtin_fabs(Real(z));
	double y = __builtin_fabs(Imag(z));
	double u, ySquared, tmp;
	double complex sqrtzPlus1, sqrtzMinus1;
	
	// If |z| == inf, then cacosh(z) = inf + carg(z) * I
	if ((x == INF) || (y == INF)) {
		Imag(w) = atan2(y,x);
		Real(w) = INF;
	}
	
	// If z = NaN, cacosh(z) = NaN.
	else if ((x != x) || (y != y)) {
		Real(w) = x + y;
		Imag(w) = x + y;
	}
	
	// at this point x,y are finite, non-nan.
	else {
		// If z is small, then cacosh(z) = I*(π/2 - z + z^3/6 + ...) = I*(π/2 - z) within half an ulp
		if ((x < 0x1p-27) && (y < 0x1p-27)) {
			Real(w) = y;
			Imag(w) = pi2 - x;
		}
		
		// If z is big, then cacosh(z) = (log2 + log(z)) + terms smaller than half an ulp
		else if ((x > 0x1p27) || (y > 0x1p27)) {
			w = clog(x + I*y) + ln2;
		}
		
		/* Otherwise, use the expressions from Kahan's "Much ado about nothing's sign bit..."
		 *
		 * Real(w) = asinh(real(csqrt(cconj(z)-1.0)*csqrt(z+1.0)))
		 * Imag(w) = 2.0*atan2(imag(csqrt(z-1.0))/real(csqrt(z+1.0)))
		 *
		 * Derivation of these formulats boggles the mind, but they are easily verified with the
		 * Monodromy theorem.  Analysis of roundoff is a bit harder, but goes though just fine.
		 */
		else {
			ySquared = (y < 0x1p-106 ? 0.0 : y*y); // Avoid underflows.  Faster via mask?
			
			// Compute sqrt1Plusz = sqrt(x+1 + iy)
			u = x + 1.0;
			Real(sqrtzPlus1) = __builtin_sqrt(0.5*(__builtin_sqrt(u*u + ySquared) + u));
			Imag(sqrtzPlus1) = 0.5 * (y / Real(sqrtzPlus1));
			
			// Compute sqrt1Minusz = sqrt(x-1 + iy)
			u = x - 1.0;
			
			if (u == 0.0) {
				Real(sqrtzMinus1) = sqrt1_2 * __builtin_sqrt(y); // Avoid spurious underflows in this case
				Imag(sqrtzMinus1) = Real(sqrtzMinus1);           // by using the simpler formula.
			}
			
			else {  // No underflow or overflow is possible.
				Real(sqrtzMinus1) = __builtin_sqrt(0.5*(__builtin_sqrt(u*u + ySquared) + __builtin_fabs(u)));
				tmp = 0.5 * (y / Real(sqrtzMinus1));
				
				if (u < 0.0) {
					Imag(sqrtzMinus1) = Real(sqrtzMinus1);
					Real(sqrtzMinus1) = tmp;
				} else {
					Imag(sqrtzMinus1) = tmp;
				}
			}
			
			// Magic formulas from Kahan.
			Real(w) = asinh( Real(sqrtzPlus1)*Real(sqrtzMinus1) + Imag(sqrtzPlus1)*Imag(sqrtzMinus1) );
			Imag(w) = 2.0*atan2( Imag(sqrtzMinus1) , Real(sqrtzPlus1) );
		}
	}
	
	// Patch up signs to handle z in quadrants II, III & IV, using symmetry.
	if (Real(z) < 0.0)
		Imag(w) = 2.0 * pi2 - Imag(w); // No undue cancellation is possible here - Real(w) < π/2.
	
	Imag(w) = copysign(Imag(w), Imag(z));
	
	return w;
}

float complex cacoshf ( float complex z )
{
	static const float INFf = __builtin_inff();
	static const float ln2f =     0x1.62e42fefa39ef358p-1f;
	static const float sqrt1_2f = 0x1.6a09e667f3bcc908p-1f;
	static const float pi2f =     0x1.921fb54442d1846ap0f;
	
	float complex w;
	float x = __builtin_fabsf(Real(z));
	float y = __builtin_fabsf(Imag(z));
	float u, ySquared, tmp;
	float complex sqrtzPlus1, sqrtzMinus1;
	
	// If |z| == inf, then cacosh(z) = inf + carg(z) * I
	if ((x == INFf) || (y == INFf)) {
		Imag(w) = atan2f(y,x);
		Real(w) = INFf;
	}
	
	// If z = NaN, cacosh(z) = NaN.
	else if ((x != x) || (y != y)) {
		Real(w) = x + y;
		Imag(w) = x + y;
	}
	
	// at this point x,y are finite, non-nan.
	else {
		// If z is small, then cacosh(z) = I*(π/2 - z + z^3/6 + ...) = I*(π/2 - z) within half an ulp
		if ((x < 0x1p-13f) && (y < 0x1p-13f)) {
			Real(w) = y;
			Imag(w) = pi2f - x;
		}
		
		// If z is big, then cacosh(z) = (log2 + log(z)) + terms smaller than half an ulp
		else if ((x > 0x1p13f) || (y > 0x1p13f)) {
			w = clogf(x + I*y) + ln2f;
		}
		
		/* Otherwise, use the expressions from Kahan's "Much ado about nothing's sign bit..."
		 *
		 * Real(w) = asinh(real(csqrt(cconj(z)-1.0)*csqrt(z+1.0)))
		 * Imag(w) = 2.0*atan2(imag(csqrt(z-1.0))/real(csqrt(z+1.0)))
		 *
		 * Derivation of these formulats boggles the mind, but they are easily verified with the
		 * Monodromy theorem.  Analysis of roundoff is a bit harder, but goes though just fine.
		 */
		else {
			ySquared = (y < 0x1p-52f ? 0.0f : y*y); // Avoid underflows.  Faster via mask?
			
			// Compute sqrt1Plusz = sqrt(x+1 + iy)
			u = x + 1.0f;
			Real(sqrtzPlus1) = __builtin_sqrtf(0.5f*(__builtin_sqrtf(u*u + ySquared) + u));
			Imag(sqrtzPlus1) = 0.5f * (y / Real(sqrtzPlus1));
			
			// Compute sqrt1Minusz = sqrt(x-1 + iy)
			u = x - 1.0f;
			
			if (u == 0.0f) {
				Real(sqrtzMinus1) = sqrt1_2f * __builtin_sqrtf(y); // Avoid spurious underflows in this case
				Imag(sqrtzMinus1) = Real(sqrtzMinus1);           // by using the simpler formula.
			}
			
			else {  // No underflow or overflow is possible.
				Real(sqrtzMinus1) = __builtin_sqrtf(0.5f*(__builtin_sqrtf(u*u + ySquared) + __builtin_fabsf(u)));
				tmp = 0.5f * (y / Real(sqrtzMinus1));
				
				if (u < 0.0f) {
					Imag(sqrtzMinus1) = Real(sqrtzMinus1);
					Real(sqrtzMinus1) = tmp;
				} else {
					Imag(sqrtzMinus1) = tmp;
				}
			}
			
			// Magic formulas from Kahan.
			Real(w) = asinhf( Real(sqrtzPlus1)*Real(sqrtzMinus1) + Imag(sqrtzPlus1)*Imag(sqrtzMinus1) );
			Imag(w) = 2.0f*atan2f( Imag(sqrtzMinus1) , Real(sqrtzPlus1) );
		}
	}
	
	// Patch up signs to handle z in quadrants II, III & IV, using symmetry.
	if (Real(z) < 0.0f)
		Imag(w) = 2.0f * pi2f - Imag(w); // No undue cancellation is possible here - Real(w) < π/2.
	
	Imag(w) = copysignf(Imag(w), Imag(z));
	
	return w;
}

/****************************************************************************
   double complex catan(double complex z) returns the complex inverse tangent
   of its argument.  The algorithm is from Kahan's paper and is based on
   the formula:
   
      catan(z) = i*(clog(1.0-i*z) - clog(1+i*z))/2.0.
   
   CONSTANTS:  FPKTHETA = sqrt(nextafterd(+INF,0.0))/4.0
               FPKRHO = 1.0/FPKTHETA
               FPKPI2 = pi/2.0
               FPKPI = pi
            
   Calls:  copysign, fabs, xdivc, sqrt, log, atan, log1p, and carg.
****************************************************************************/

double complex catan ( double complex z )
{
	double complex iz, iw, w;
	Real(iz) = -Imag(z);
	Imag(iz) = Real(z);
	iw = catanh(iz);
	Real(w) = Imag(iw);
	Imag(w) = -Real(iw);
	return w;
}

float complex catanf ( float complex z )
{
	float complex iz, iw, w;
	Real(iz) = -Imag(z);
	Imag(iz) = Real(z);
	iw = catanhf(iz);
	Real(w) = Imag(iw);
	Imag(w) = -Real(iw);
	return w;
}

/****************************************************************************
   double complex catanh(double complex z) returns the complex inverse hyperbolic tangent
   of its argument.  The algorithm is from Kahan's paper and is based on
   the formula:
   
      catanh(z) = (clog(1.0 + z) - clog(1 - z))/2.0.
   
   CONSTANTS:  FPKTHETA = sqrt(nextafterd(+INF,0.0))/4.0
               FPKRHO = 1.0/FPKTHETA
               FPKPI2 = pi/2.0
               FPKPI = pi
            
   Calls:  copysign, fabs, xdivc, sqrt, log, atan, log1p, and carg.
****************************************************************************/

double complex catanh( double complex z )   
{
   double complex ctemp, w;
   double t1, t2, xi, eta, beta;
   
   beta = copysign(1.0,Real(z));             /* copes with unsigned zero */
   
   Imag(z) = -beta*Imag(z);                     /* transform real & imag components */
   Real(z) = beta*Real(z);
   
   if ((Real(z) > FPKTHETA) || (fabs(Imag(z)) > FPKTHETA)) {
      eta = copysign(M_PI_2,Imag(z));   /* avoid overflow */
      ctemp = xdivc(1.0,z);
      xi = Real(ctemp);
   }
      
   else if (Real(z) == 1.0) {
      t1 = fabs(Imag(z)) + FPKRHO;
      xi = log(sqrt(sqrt(4.0 + t1*t1))/sqrt(fabs(Imag(z))));
      eta = 0.5*copysign(M_PI-atan(2.0/(fabs(Imag(z))+FPKRHO)),Imag(z));
   }
   
   else {                                 /* usual case */
      t2 = fabs(Imag(z)) + FPKRHO;
      t1 = 1.0 - Real(z);
      t2 = t2*t2;
      xi = 0.25*log1p(4.0*Real(z)/(t1*t1 + t2));
      Real(ctemp) = (1.0 - Real(z))*(1.0 + Real(z)) - t2;
      Imag(ctemp) = Imag(z) + Imag(z);
      eta = 0.5*carg(ctemp);
   }
   
   Real(w) = beta*xi;                        /* fix up signs of result */
   Imag(w) = -beta*eta;
   return w;
}

float complex catanhf( float complex z )   
{
   float complex ctemp, w;
   float t1, t2, xi, eta, beta;
   
   beta = copysignf(1.0f,Real(z));             /* copes with unsigned zero */
   
   Imag(z) = -beta*Imag(z);                     /* transform real & imag components */
   Real(z) = beta*Real(z);
   
   if ((Real(z) > FPKTHETAf) || (fabsf(Imag(z)) > FPKTHETAf)) {
      eta = copysignf((float) M_PI_2,Imag(z));   /* avoid overflow */
      ctemp = xdivcf(1.0f,z);
      xi = Real(ctemp);
   }
      
   else if (Real(z) == 1.0f) {
      t1 = fabsf(Imag(z)) + FPKRHOf;
      xi = logf(sqrtf(sqrtf(4.0f + t1*t1))/sqrtf(fabsf(Imag(z))));
      eta = 0.5f*copysignf((float)( M_PI-atan(2.0f/(fabsf(Imag(z))+FPKRHOf))),Imag(z));
   }
   
   else {                                 /* usual case */
      t2 = fabsf(Imag(z)) + FPKRHOf;
      t1 = 1.0f - Real(z);
      t2 = t2*t2;
      xi = 0.25f*log1pf(4.0f*Real(z)/(t1*t1 + t2));
      Real(ctemp) = (1.0f - Real(z))*(1.0f + Real(z)) - t2;
      Imag(ctemp) = Imag(z) + Imag(z);
      eta = 0.5f*cargf(ctemp);
   }
   
   Real(w) = beta*xi;                        /* fix up signs of result */
   Imag(w) = -beta*eta;
   return w;
}

/* conj(), creal(), and cimag() are gcc built ins. */
double creal( double complex z )
{
	return __builtin_creal(z);
}

float crealf( float complex z )
{
	return __builtin_crealf(z);
}

double cimag( double complex z )
{
	return __builtin_cimag(z);
}

float cimagf( float complex z )
{
	return __builtin_cimagf(z);
}

double complex conj( double complex z )
{
	return __builtin_conj(z);
}

float complex conjf( float complex z )
{
	return __builtin_conjf(z);
}

double complex cproj( double complex z )
{
	static const double inf = __builtin_inf();
	double u = __builtin_fabs(Real(z));
	double v = __builtin_fabs(Imag(z));
    
	if ((u == inf) || (v == inf))
		return inf + I*copysign(0.0, Imag(z));
	else
		return z;
}

float complex cprojf( float complex z )
{
	static const double inf = __builtin_inff();
	float u = __builtin_fabsf(Real(z));
	float v = __builtin_fabsf(Imag(z));
	
	if ((u == inf) || (v == inf))
		return inf + I*copysignf(0.0f, Imag(z));
	else
		return z;	
}
