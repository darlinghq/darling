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

#pragma option nomaf
#pragma STDC FENV_ACCESS ON

#include "math.h"
#include "complex.h"
#include "fenv.h"
#include "fp_private.h"

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

****************************************************************************/
static const               /* underflow threshold / round threshold */
   hexdouble FPKLOVERE = HEXDOUBLE(0x03600000,0x00000000);

static const               /* underflow threshold / round threshold */
   hexsingle FPKLOVEREf = { 0xc000000 };

static const               /* exp(709.0) */
   hexdouble FPKEXP709 = HEXDOUBLE(0x7fdd422d,0x2be5dc9b);

static const               /* asinh(nextafterd(+infinity,0.0))/4.0 */
   hexdouble FPKASINHOM4 = HEXDOUBLE(0x406633ce,0x8fb9f87e);

static const               /* asinh(nextafterf(+infinity,0.0))/4.0 */
   hexsingle FPKASINHOM4f = { 0x41b2d4fc };

static const               /* sqrt(nextafterd(+infinity,0.0))/4.0 */
   hexdouble FPKTHETA = HEXDOUBLE(0x5fcfffff,0xffffffff);

static const               /* sqrt(nextafterf(+infinity,0.0))/4.0 */
   hexsingle FPKTHETAf =  { 0x5e7fffff };

static const               /* 4.0/sqrt(nextafterd(+infinity,0.0)) */
   hexdouble FPKRHO = HEXDOUBLE(0x20100000,0x00000000);

static const               /* 4.0/sqrt(nextafterf(+infinity,0.0)) */
   hexsingle FPKRHOf = { 0x20800001 };

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


#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

static
long double complex xdivcl( long double x, long double complex y )   /* returns (real x) / (complex y) */
{
   long double complex      z;
   long double   r, denom;
   
   if ( fabsl(Real(y)) >= fabsl(Imag(y)) ) {     /* |Real(y)| >= |Imag(y)| */
      if (fabsl(Real(y)) == INFINITY) {   /* Imag(y) and Real(y) are infinite */
         Real(z) = copysignl(0.0L,Real(y));
         Imag(z) = copysignl(0.0L,-Imag(y));
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
#endif

/****************************************************************************
   double cabs(double complex z) returns the absolute value (magnitude) of its
   complex argument z, avoiding spurious overflow, underflow, and invalid
   exceptions.  The algorithm is from Kahan's paper.
   
   CONSTANTS:  FPKSQT2 = sqrt(2.0) to double precision
               FPKR2P1 = sqrt(2.0) + 1.0 to double precision
               FPKT2P1 = sqrt(2.0) + 1.0 - FPKR2P1 to double precision, so
                  that FPKR2P1 + FPKT2P1 = sqrt(2.0) + 1.0 to double
                  double precision.
            
   Calls:  fpclassify, fabs, sqrt, feholdexcept, fesetround, feclearexcept,
           and feupdateenv.
****************************************************************************/

extern double cabs( double complex );
float cabsf( float complex z ){ return (float) cabs((double complex) z); }

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

static const long double FPKEXP709L = 8.2184074615549721892413723865978e+307L;

#define M_FPKT2P1Q -.0000000000000000000000000000000000014303281246230519268233202620092676L

long double cabsl ( long double complex z )
{
    long double a,b,s,t;
    fenv_t env;
    long double FPR_inf = INFINITY;
    
    a = fabsl(Real(z));
    b = fabsl(Imag(z));
    
    if (unlikely( (a == FPR_inf) || (b == FPR_inf) ))
	return FPR_inf;
	    
    if (unlikely( (a != a) || (b != b) ))
	return __FABS ( a + b );
    
    if (unlikely((a == 0.0L) || (b == 0.0L) ))
	return __FABS ( a + b );
    
    /* both components of z are finite, nonzero */
    {
      (void)feholdexcept(&env);         /* save environment, clear flags */
      (void)fesetround(FE_TONEAREST);   /* set default rounding */

      s = 0.0L;
      if (a < b)                        /* order a >= b */
      {
         t = a;
         a = b;
         b = t;
      }
      t = a - b;                        /* magnitude difference */
      
      if (t != a)                       /* b not negligible relative to a */
      {
         if (t > b)                     /* a - b > b */
         {
            s = a/b;
            s += sqrtl(1.0L + s*s);
         }
         else                           /* a - b <= b */
         {
            s = t/b;
            t = (2.0L + s)*s;
            s = ((M_FPKT2P1Q+t/(M_SQRT2+sqrt(2.0L+t)))+s)+(1.0L + M_SQRT2);
         }
         
         s = b/s;                        /* may spuriously underflow */
         feclearexcept(FE_UNDERFLOW);
      }
      
      feupdateenv(&env);                 /* restore environment */
      return (a + s);                    /* deserved overflow occurs here */
   }                                     /* finite, nonzero case */
}
#endif

/****************************************************************************
   double carg(double complex z) returns the argument (in radians) of its
   complex argument z.  The algorithm is from Kahan's paper.
   
   The argument of a complex number z = x + i*y is defined as atan2(y,x)
   for finite x and y.
   
   CONSTANTS:  FPKPI2 = pi/2.0 to double precision
               FPKPI = pi to double precision
   
   Calls:  fpclassify, copysign, fabs, atan
****************************************************************************/

double carg ( double complex z )
{
   double a,b,argr;
   int   clre,clim;
   
   a = Real(z);
   b = Imag(z);
   clre = fpclassify(a);
   clim = fpclassify(b);
   
   if ((clre == FP_ZERO) && (clim == FP_ZERO)) {  /* zero real and imag parts */
      a = copysign(1.0, a);
   }
   
   if ((clre == FP_INFINITE) && (clim == FP_INFINITE)) {   /* both parts INF */
      a = copysign(1.0, a);
      b = copysign(1.0, b);
   }
         
   if (fabs(b) > fabs(a))               /* |imag| > |real| */
      argr = copysign(M_PI_2, b) - atan(a/b);

   else {
      if (a < 0.0)                      /* |real| >= |imag| */
         argr = copysign(M_PI, b) + atan(b/a);
      else
         argr = atan(b/a);
   }
   
   return argr;
}
   
float cargf ( float complex z )
{
   float a,b,argr;
   int   clre,clim;
   
   a = Real(z);
   b = Imag(z);
   clre = fpclassify(a);
   clim = fpclassify(b);
   
   if ((clre == FP_ZERO) && (clim == FP_ZERO)) {  /* zero real and imag parts */
      a = copysignf(1.0f, a);
   }
   
   if ((clre == FP_INFINITE) && (clim == FP_INFINITE)) {   /* both parts INF */
      a = copysignf(1.0f, a);
      b = copysignf(1.0f, b);
   }
         
   if (fabsf(b) > fabsf(a))               /* |imag| > |real| */
      argr = copysignf((float) M_PI_2, b) - atanf(a/b);

   else {
      if (a < 0.0f)                      /* |real| >= |imag| */
         argr = copysignf((float) M_PI, b) + atanf(b/a);
      else
         argr = atanf(b/a);
   }
   
   return argr;
}
   
#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

long double cargl ( long double complex z )
{
   long double a,b,argr;
   int   clre,clim;
   
   a = Real(z);
   b = Imag(z);
   clre = fpclassify(a);
   clim = fpclassify(b);
   
   if ((clre == FP_ZERO) && (clim == FP_ZERO)) {  /* zero real and imag parts */
      a = copysignl(1.0L, a);
   }
   
   if ((clre == FP_INFINITE) && (clim == FP_INFINITE)) {   /* both parts INF */
      a = copysignl(1.0L, a);
      b = copysignl(1.0L, b);
   }
         
   if (fabsl(b) > fabsl(a))               /* |imag| > |real| */
      argr = copysignl(M_PI_2, b) - atanl(a/b);

   else {
      if (a < 0.0L)                      /* |real| >= |imag| */
         argr = copysignl(M_PI, b) + atanl(b/a);
      else
         argr = atanl(b/a);
   }
   
   return argr;
}
#endif

/****************************************************************************
   static double cssqs(double complex z, long int *k) returns rho = |z/(2^*k)|^2
   with scale factor *k set to avoid overflow/underflow.  Algorithm is
   from the Kahan paper.
   
   CONSTANTS:  FPKINF = +infinity
               FPKLOVERE = (double underflow threshold)/(double round threshold)
                         = (4.0*nextafterd(1.0,0.0)/nextafterd(INF,0.0))
						   /(1.0 - nextafterd(1.0,0)
   
   Calls:  fabs, logb, scalb, fmax, feholdexcept, fetestexcept, feclearexcept,
           and feupdateenv.
   
   Called by:  csqrt and clog.
****************************************************************************/

static double cssqs ( double complex z, int *k)
{
   double a,b,rho;
   fenv_t env;
   int iscale;
   
   iscale = 0;
   a = fabs(Real(z));
   b = fabs(Imag(z));
   (void)feholdexcept(&env);          /* save environment, clr flags */
   rho = a*a + b*b;                    /* preliminary result */
   
   if ((a == INFINITY) || (b == INFINITY)) {
      rho = INFINITY;       /* +INF result if Real(z) or Imag(z) is infinite */
   }
   
   else if (fetestexcept(FE_OVERFLOW) || (fetestexcept(FE_UNDERFLOW) && (rho < FPKLOVERE.d))) {
      iscale = ilogb(fmax(a,b));       /* scaling necessary */
      a = scalbn(a,-iscale);
      b = scalbn(b,-iscale);
      rho = a*a + b*b;                 /* re-calculate scaled square magnitude */
   }
   
   feclearexcept(FE_OVERFLOW + FE_UNDERFLOW);
   feupdateenv(&env);                  /* restore environment */
   *k = iscale;                        /* store scale value */
   return (rho);
}

static float cssqsf ( float complex z, int *k)
{
   float a,b,rho;
   fenv_t env;
   int iscale;
   
   iscale = 0;
   a = fabsf(Real(z));
   b = fabsf(Imag(z));
   (void)feholdexcept(&env);          /* save environment, clr flags */
   rho = a*a + b*b;                    /* preliminary result */
   
   if ((a == INFINITY) || (b == INFINITY)) {
      rho = INFINITY;       /* +INF result if Real(z) or Imag(z) is infinite */
   }
   
   else if (fetestexcept(FE_OVERFLOW) || (fetestexcept(FE_UNDERFLOW) && (rho < FPKLOVEREf.fval))) {
      iscale = logbf(fmaxf(a,b));       /* scaling necessary */
      a = scalbnf(a,-iscale);
      b = scalbnf(b,-iscale);
      rho = a*a + b*b;                 /* re-calculate scaled square magnitude */
   }
   
   feclearexcept(FE_OVERFLOW + FE_UNDERFLOW);
   feupdateenv(&env);                  /* restore environment */
   *k = iscale;                        /* store scale value */
   return (rho);
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

static long double cssqsl ( long double complex z, int *k)
{
   long double a,b,rho;
   fenv_t env;
   int iscale;
   
   iscale = 0;
   a = fabsl(Real(z));
   b = fabsl(Imag(z));
   (void)feholdexcept(&env);          /* save environment, clr flags */
   rho = a*a + b*b;                    /* preliminary result */
   
   if ((a == INFINITY) || (b == INFINITY)) {
      rho = INFINITY;       /* +INF result if Real(z) or Imag(z) is infinite */
   }
   
   else if (fetestexcept(FE_OVERFLOW) || (fetestexcept(FE_UNDERFLOW) && (rho < FPKLOVERE.d))) {
      iscale = logbl(fmaxl(a,b));       /* scaling necessary */
      a = scalbnl(a,-iscale);
      b = scalbnl(b,-iscale);
      rho = a*a + b*b;                 /* re-calculate scaled square magnitude */
   }
   
   feclearexcept(FE_OVERFLOW + FE_UNDERFLOW);
   feupdateenv(&env);                  /* restore environment */
   *k = iscale;                        /* store scale value */
   return (rho);
}
#endif   

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

double complex csqrt ( double complex z )   
{
   double rho,x,y;
   double complex w;
   int k;

   rho = cssqs(z,&k);                   /* scaled square magnitude */
   
   if (Real(z) == Real(z))
      rho = scalbn(fabs(Real(z)),-k) + sqrt(rho);   /* scaled |Real(z)| + |z| */
   
   if (k%2)                             /* k is odd */
      k = (k-1)/2;
   else {                               /* k is even */
      k = k/2 - 1;
      rho = rho + rho;
   }
   
   rho = scalbn(sqrt(rho),k);           /* sqrt((|Real(z)| + |z|)/2) */
   x = rho;
   y = Imag(z);
   
   if (rho != 0.0) {
      if (fabs(y) != INFINITY)
         y = (y/rho)*0.5;								/* signbit(Imag(z))*sqrt((|z|-|Real(z)|)/2 */
      if (Real(z) < 0.0) {
         x = fabs(y);
         y = copysign(rho,Imag(z));
      }
   }
   
   Real(w) = x;
   Imag(w) = y;
   return w;
}

float complex csqrtf ( float complex z )   
{
   float rho,x,y;
   float complex w;
   int k;

   rho = cssqsf(z,&k);                   /* scaled square magnitude */
   
   if (Real(z) == Real(z))
      rho = scalbnf(fabsf(Real(z)),-k) + sqrtf(rho);   /* scaled |Real(z)| + |z| */
   
   if (k%2)                             /* k is odd */
      k = (k-1)/2;
   else {                               /* k is even */
      k = k/2 - 1;
      rho = rho + rho;
   }
   
   rho = scalbnf(sqrtf(rho),k);         /* sqrt((|Real(z)| + |z|)/2) */
   x = rho;
   y = Imag(z);
   
   if (rho != 0.0f) {
      if (fabsf(y) != INFINITY)
         y = (y/rho)*0.5f;							/* signbit(Imag(z))*sqrt((|z|-|Real(z)|)/2 */
      if (Real(z) < 0.0f) {
         x = fabsf(y);
         y = copysignf(rho,Imag(z));
      }
   }
   
   Real(w) = x;
   Imag(w) = y;
   return w;
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

long double complex csqrtl ( long double complex z )   
{
   long double rho,x,y;
   long double complex w;
   int k;

   rho = cssqsl(z,&k);                   /* scaled square magnitude */
   
   if (Real(z) == Real(z))
      rho = scalbnl(fabsl(Real(z)),-k) + sqrtl(rho);   /* scaled |Real(z)| + |z| */
   
   if (k%2)                             /* k is odd */
      k = (k-1)/2;
   else {                               /* k is even */
      k = k/2 - 1;
      rho = rho + rho;
   }
   
   rho = scalbnl(sqrtl(rho),k);            /* sqrt((|Real(z)| + |z|)/2) */
   x = rho;
   y = Imag(z);
   
   if (rho != 0.0L) {
      if (fabsl(y) != INFINITY)
         y = (y/rho)*0.5L;          /* signbit(Imag(z))*sqrt((|z|-|Real(z)|)/2 */
      if (Real(z) < 0.0L) {
         x = fabsl(y);
         y = copysignl(rho,Imag(z));
      }
   }
   
   Real(w) = x;
   Imag(w) = y;
   return w;
}
#endif

/****************************************************************************
   double complex clog(double complex z) returns the complex natural logarithm of its
   argument.  The algorithm, which is from the Kahan paper, avoids spurious
   underflow or overflow.
   
   CONSTANTS:  FPKSQRTHALF = sqrt(0.5) to double precision
               FPKLOG2 = log(2.0) to double precision

   Calls:  fabs, cssqs, log1p, log, carg. 
****************************************************************************/

double complex clog ( double complex z )   
{
   double rho,dmax,dmin,temp;
   double complex w;
   int k;
   
   dmax = fabs(Real(z));  /* order real and imaginary parts of z by magnitude */
   dmin = fabs(Imag(z));
   if (dmax < dmin) {
      temp = dmax;
      dmax = dmin;
      dmin = temp;
   }
   
   rho = cssqs(z,&k);                     /* scaled |z*z| */
   
   if ((k == 0) && (dmax > M_SQRT1_2) && ((dmax <= 1.25) || (rho < 3.0)))
      Real(w) = log1p((dmax - 1.0)*(dmax + 1.0) + dmin*dmin)*0.5; /* |z| near 1.0 */
   else
      Real(w) = log(rho)*0.5 + k*M_LN2;   /* more naive approximation */
   
   Imag(w) = carg(z);                        /* imaginary part of logarithm */

   return w;
}

float complex clogf ( float complex z )   
{
   float rho,dmax,dmin,temp;
   float complex w;
   int k;
   
   dmax = fabsf(Real(z));  /* order real and imaginary parts of z by magnitude */
   dmin = fabsf(Imag(z));
   if (dmax < dmin) {
      temp = dmax;
      dmax = dmin;
      dmin = temp;
   }
   
   rho = cssqsf(z,&k);                     /* scaled |z*z| */
   
   if ((k == 0) && (dmax > M_SQRT1_2) && ((dmax <= 1.25f) || (rho < 3.0f)))
      Real(w) = log1pf((dmax - 1.0f)*(dmax + 1.0f) + dmin*dmin)*0.5f; /* |z| near 1.0 */
   else
      Real(w) = logf(rho)*0.5f + (float)((double)k*M_LN2);   /* more naive approximation */
   
   Imag(w) = cargf(z);                        /* imaginary part of logarithm */

   return w;
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

long double complex clogl ( long double complex z )   
{
   long double rho,dmax,dmin,temp;
   long double complex w;
   int k;
   
   dmax = fabsl(Real(z));  /* order real and imaginary parts of z by magnitude */
   dmin = fabsl(Imag(z));
   if (dmax < dmin) {
      temp = dmax;
      dmax = dmin;
      dmin = temp;
   }
   
   rho = cssqsl(z,&k);                     /* scaled |z*z| */
   
   if ((k == 0) && (dmax > M_SQRT1_2) && ((dmax <= 1.25L) || (rho < 3.0L)))
      Real(w) = log1pl((dmax - 1.0L)*(dmax + 1.0L) + dmin*dmin)*0.5L; /* |z| near 1.0 */
   else
      Real(w) = logl(rho)*0.5L + k*M_LN2;   /* more naive approximation */
   
   Imag(w) = cargl(z);                        /* imaginary part of logarithm */

   return w;
}
#endif
   
/****************************************************************************
   static double coshmul(double x, double y) returns y*cosh(x) while
   avoiding spurious overflow and invalid exceptions.
   
   CONSTANTS:  FPKEXP709 = exp(709.0) in double precision
   
   Calls:  cosh, exp, fpclassify, fabs, and scalb.
   
   Called by:  csin, ccos, csinh, and ccosh.
****************************************************************************/

static double coshmul ( double x, double y )
{
   double absx, result;
   
   absx = fabs(x);
   if (absx <= 709.0) {                  /* cosh(x) is finite */
      return y*cosh(x);
   }
   
   else if (fpclassify(x) < FP_ZERO) {   /* x is NaN or infinite */
      return (y*absx);
   }
   
   else if (absx > 1460.0) {             /* probable overflow case */
      return (scalbn(y,2100));
   }
   
   else {                        /* cosh(x) overflows but y*cosh(x) may not */
      result = (0.5 * FPKEXP709.d) * y;   /* initialize result to cosh(709) */
      absx -= 709.0;                     /* reduce exponential argument */
      while (absx > 709.0) {             /* exponential reduction loop */
         result *= FPKEXP709.d;
         absx -= 709.0;
      }
      return (result*exp(absx));         /* final multiplication */
   }
}

static float coshmulf ( float x, float y )
{
   float absx;
   
   absx = fabsf(x);
   if (absx <= 89.0f) {                  /* coshf(x) is finite */
      return y*coshf(x);
   }
   
   else if (fpclassify(x) < FP_ZERO) {   /* x is NaN or infinite */
      return (y*absx);
   }
   
   return (float)coshmul((double)x, (double)y);
}

// long double has same range as double
#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

static long double coshmull ( long double x, long double y )
{
   long double absx, result;
   
   absx = fabsl(x);
   if (absx <= 709.0L) {                  /* cosh(x) is finite */
      return y*coshl(x);
   }
   
   else if (fpclassify(x) < FP_ZERO) {   /* x is NaN or infinite */
      return (y*absx);
   }
   
   else if (absx > 1460.0L) {             /* probable overflow case */
      return (scalbnl(y,2100));
   }
   
   else {                        /* cosh(x) overflows but y*cosh(x) may not */
      long double t = FPKEXP709L;
	  
      result = (0.5L * t) * y;   /* initialize result to cosh(709) */
      absx -= 709.0L;                     /* reduce exponential argument */
      while (absx > 709.0L) {             /* exponential reduction loop */
         result *= t;
         absx -= 709.0L;
      }
      return (result*expl(absx));         /* final multiplication */
   }
}
#endif   

/****************************************************************************
   static double sinhmul(double x, double y) returns y*sinh(x) while
   avoiding spurious overflow and invalid exceptions.
   
   CONSTANTS:  FPKEXP709 = exp(709.0) in double precision
   
   Calls:  sinh, exp, fpclassify, fabs, and scalb.
   
   Called by:  csin, ccos, csinh, and ccosh.
****************************************************************************/

static double sinhmul ( double x, double y )
{
   double absx, result;
   
   absx = fabs(x);
   if (absx <= 709.0) {                  /* sinh(x) is finite */
      return y*sinh(x);
   }
   
   else if (fpclassify(x) < FP_ZERO) {   /* x is NaN or infinite */
      return (y*x);
   }
   
   else if (absx > 1460.0) {             /* probable overflow case */
      return (scalbn(y,2100));
   }
   
   else {                       /* sinh(x) overflows but y*sinh(x) may not */
      result = (0.5*FPKEXP709.d)*y;  /* initialize result to |sinh(709)| */
      absx -= 709.0;                    /* reduce exponential argument */
      if (signbit(x) != 0) result = -result;   /* take care of sign of result */
      while (absx > 709.0) {            /* exponential reduction loop */
         result *= FPKEXP709.d;
         absx -= 709.0;
      }
      return (result*exp(absx));        /* final multiplication */
   }
}

static float sinhmulf ( float x, float y )
{
   float absx;
   
   absx = fabsf(x);
   if (absx <= 709.0f) {                  /* sinhf(x) is finite */
      return y*sinhf(x);
   }
   
   else if (fpclassify(x) < FP_ZERO) {   /* x is NaN or infinite */
      return (y*x);
   }
   
   return (float)sinhmul((double)x, (double)y);
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

static long double sinhmull ( long double x, long double y )
{
   long double absx, result;
   
   absx = fabsl(x);
   if (absx <= 709.0L) {                  /* sinh(x) is finite */
      return y*sinhl(x);
   }
   
   else if (fpclassify(x) < FP_ZERO) {   /* x is NaN or infinite */
      return (y*x);
   }
   
   else if (absx > 1460.0L) {             /* probable overflow case */
      return (scalbnl(y,2100));
   }
   
   else {                       /* sinh(x) overflows but y*sinh(x) may not */
      long double t = FPKEXP709L;
	  
      result = (0.5L*t)*y;  /* initialize result to y*|sinh(709)| */
      absx -= 709.0L;                    /* reduce exponential argument */
      if (signbit(x) != 0) result = -result;   /* take care of sign of result */
      while (absx > 709.0L) {            /* exponential reduction loop */
         result *= t;
         absx -= 709.0L;
      }
      return (result*expl(absx));        /* final multiplication */
   }
}
#endif
   

/****************************************************************************
   double complex csin(double complex z) returns the complex sine of its argument.  The 
   algorithm is based upon the identity:
   
      sin(x + i*y) = sin(x)*cosh(y) + i*cos(x)*sinh(y).
      
   Signaling of spurious overflows, underflows, and invalids is avoided where
   possible.

   Calls:  sin, cos, coshmul, sinhmul, feholdexcept, feclearexcept, and
           feupdateenv.
****************************************************************************/

double complex csin ( double complex z )   
{
   fenv_t env;
   double sinval, cosval;
   double complex w;
   
   (void)feholdexcept(&env);         /* save environment, clear flags */
   sinval = sin(Real(z));                /* sine of real part of argument */
   cosval = cos(Real(z));                /* cosine of real part of argument */
   Real(w) = coshmul(Imag(z),sinval);       /* real result = sinval*cosh(Imag(z)) */
   Imag(w) = sinhmul(Imag(z),cosval);       /* imag result = cosval*sinh(Imag(z)) */
   feclearexcept(FE_UNDERFLOW);       /* underflows don't matter */
   feupdateenv(&env);                 /* update environment */
   return w;
}

float complex csinf ( float complex z )   
{
   fenv_t env;
   float sinval, cosval;
   float complex w;
   
   (void)feholdexcept(&env);         /* save environment, clear flags */
   sinval = sinf(Real(z));                /* sine of real part of argument */
   cosval = cosf(Real(z));                /* cosine of real part of argument */
   Real(w) = coshmulf(Imag(z),sinval);       /* real result = sinval*cosh(Imag(z)) */
   Imag(w) = sinhmulf(Imag(z),cosval);       /* imag result = cosval*sinh(Imag(z)) */
   feclearexcept(FE_UNDERFLOW);       /* underflows don't matter */
   feupdateenv(&env);                 /* update environment */
   return w;
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

long double complex csinl ( long double complex z )   
{
   fenv_t env;
   long double sinval, cosval;
   long double complex w;
   
   (void)feholdexcept(&env);         /* save environment, clear flags */
   sinval = sinl(Real(z));                /* sine of real part of argument */
   cosval = cosl(Real(z));                /* cosine of real part of argument */
   Real(w) = coshmull(Imag(z),sinval);       /* real result = sinval*cosh(Imag(z)) */
   Imag(w) = sinhmull(Imag(z),cosval);       /* imag result = cosval*sinh(Imag(z)) */
   feclearexcept(FE_UNDERFLOW);       /* underflows don't matter */
   feupdateenv(&env);                 /* update environment */
   return w;
}
#endif

/****************************************************************************
   double complex ccos(double complex z) returns the complex cosine of its argument.  The 
   algorithm is based upon the identity:
   
      cos(x + i*y) = cos(x)*cosh(y) - i*sin(x)*sinh(y).
      
   Signaling of spurious overflows, underflows, and invalids is avoided where
   possible.

   Calls:  sin, cos, coshmul, sinhmul, feholdexcept, feclearexcept, and
           feupdateenv.
****************************************************************************/

double complex ccos ( double complex z )   
{
   fenv_t env;
   double sinval, cosval;
   double complex w;
   
   (void)feholdexcept(&env);         /* save environment, clear flags */
   sinval = sin(Real(z));                /* sine of real part of argument */
   cosval = cos(Real(z));                /* cosine of real part of argument */
   Real(w) = coshmul(Imag(z),cosval);       /* real result = cosval*cosh(Imag(z)) */
   Imag(w) = sinhmul(Imag(z),-sinval);      /* imag result = -sinval*sinh(Imag(z)) */
   feclearexcept(FE_UNDERFLOW);       /* underflows don't matter */
   feupdateenv(&env);                 /* update environment */
   return w;
}

float complex ccosf ( float complex z )   
{
   fenv_t env;
   float sinval, cosval;
   float complex w;
   
   (void)feholdexcept(&env);         /* save environment, clear flags */
   sinval = sinf(Real(z));                /* sine of real part of argument */
   cosval = cosf(Real(z));                /* cosine of real part of argument */
   Real(w) = coshmulf(Imag(z),cosval);       /* real result = cosval*cosh(Imag(z)) */
   Imag(w) = sinhmulf(Imag(z),-sinval);      /* imag result = -sinval*sinh(Imag(z)) */
   feclearexcept(FE_UNDERFLOW);       /* underflows don't matter */
   feupdateenv(&env);                 /* update environment */
   return w;
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

long double complex ccosl ( long double complex z )   
{
   fenv_t env;
   long double sinval, cosval;
   long double complex w;
   
   (void)feholdexcept(&env);         /* save environment, clear flags */
   sinval = sinl(Real(z));                /* sine of real part of argument */
   cosval = cosl(Real(z));                /* cosine of real part of argument */
   Real(w) = coshmull(Imag(z),cosval);       /* real result = cosval*cosh(Imag(z)) */
   Imag(w) = sinhmull(Imag(z),-sinval);      /* imag result = -sinval*sinh(Imag(z)) */
   feclearexcept(FE_UNDERFLOW);       /* underflows don't matter */
   feupdateenv(&env);                 /* update environment */
   return w;
}
#endif


/****************************************************************************
   double complex csinh(double complex z) returns the complex hyperbolic sine of its
   argument.  The algorithm is based upon the identity:
   
      sinh(x + i*y) = cos(y)*sinh(x) + i*sin(y)*cosh(x).
      
   Signaling of spurious overflows, underflows, and invalids is avoided where
   possible.

   Calls:  sin, cos, coshmul, sinhmul, feholdexcept, feclearexcept, and
           feupdateenv.
****************************************************************************/

double complex csinh ( double complex z )   
{
   fenv_t env;
   double sinval, cosval;
   double complex w;
   
   (void)feholdexcept(&env);         /* save environment, clear flags */
   sinval = sin(Imag(z));                /* sine of imaginary part of argument */
   cosval = cos(Imag(z));                /* cosine of imaginary part of argument */
   Real(w) = sinhmul(Real(z),cosval);       /* real result = cosval*sinh(Real(z)) */
   Imag(w) = coshmul(Real(z),sinval);       /* imag result = sinval*cosh(Real(z)) */
   feclearexcept(FE_UNDERFLOW);       /* underflows don't matter */
   feupdateenv(&env);                 /* update environment */
   return w;
}

float complex csinhf ( float complex z )   
{
   fenv_t env;
   float sinval, cosval;
   float complex w;
   
   (void)feholdexcept(&env);         /* save environment, clear flags */
   sinval = sinf(Imag(z));                /* sine of imaginary part of argument */
   cosval = cosf(Imag(z));                /* cosine of imaginary part of argument */
   Real(w) = sinhmulf(Real(z),cosval);       /* real result = cosval*sinh(Real(z)) */
   Imag(w) = coshmulf(Real(z),sinval);       /* imag result = sinval*cosh(Real(z)) */
   feclearexcept(FE_UNDERFLOW);       /* underflows don't matter */
   feupdateenv(&env);                 /* update environment */
   return w;
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

long double complex csinhl ( long double complex z )   
{
   fenv_t env;
   long double sinval, cosval;
   long double complex w;
   
   (void)feholdexcept(&env);         /* save environment, clear flags */
   sinval = sinl(Imag(z));                /* sine of imaginary part of argument */
   cosval = cosl(Imag(z));                /* cosine of imaginary part of argument */
   Real(w) = sinhmull(Real(z),cosval);       /* real result = cosval*sinh(Real(z)) */
   Imag(w) = coshmull(Real(z),sinval);       /* imag result = sinval*cosh(Real(z)) */
   feclearexcept(FE_UNDERFLOW);       /* underflows don't matter */
   feupdateenv(&env);                 /* update environment */
   return w;
}
#endif


/****************************************************************************
   double complex ccosh(double complex z) returns the complex hyperbolic cosine of its
   argument.  The algorithm is based upon the identity:
   
      cosh(x + i*y) = cos(y)*cosh(x) + i*sin(y)*sinh(x).
      
   Signaling of spurious overflows, underflows, and invalids is avoided where
   possible.

   Calls:  sin, cos, coshmul, sinhmul, feholdexcept, feclearexcept, and
           feupdateenv.
****************************************************************************/

double complex ccosh ( double complex z )   
{
   fenv_t env;
   double sinval, cosval;
   double complex w;
   
   (void)feholdexcept(&env);         /* save environment, clear flags */
   sinval = sin(Imag(z));                /* sine of imaginary part of argument */
   cosval = cos(Imag(z));                /* cosine of imaginary part of argument */
   Real(w) = coshmul(Real(z),cosval);       /* real result = cosval*cosh(Real(z)) */
   Imag(w) = sinhmul(Real(z),sinval);       /* imag result = sinval*sinh(Real(z)) */
   feclearexcept(FE_UNDERFLOW);       /* underflows don't matter */
   feupdateenv(&env);                 /* update environment */
   return w;
}

float complex ccoshf ( float complex z )   
{
   fenv_t env;
   float sinval, cosval;
   float complex w;
   
   (void)feholdexcept(&env);         /* save environment, clear flags */
   sinval = sinf(Imag(z));                /* sine of imaginary part of argument */
   cosval = cosf(Imag(z));                /* cosine of imaginary part of argument */
   Real(w) = coshmulf(Real(z),cosval);       /* real result = cosval*cosh(Real(z)) */
   Imag(w) = sinhmulf(Real(z),sinval);       /* imag result = sinval*sinh(Real(z)) */
   feclearexcept(FE_UNDERFLOW);       /* underflows don't matter */
   feupdateenv(&env);                 /* update environment */
   return w;
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

long double complex ccoshl ( long double complex z )   
{
   fenv_t env;
   long double sinval, cosval;
   long double complex w;
   
   (void)feholdexcept(&env);         /* save environment, clear flags */
   sinval = sinl(Imag(z));                /* sine of imaginary part of argument */
   cosval = cosl(Imag(z));                /* cosine of imaginary part of argument */
   Real(w) = coshmull(Real(z),cosval);       /* real result = cosval*cosh(Real(z)) */
   Imag(w) = sinhmull(Real(z),sinval);       /* imag result = sinval*sinh(Real(z)) */
   feclearexcept(FE_UNDERFLOW);       /* underflows don't matter */
   feupdateenv(&env);                 /* update environment */
   return w;
}
#endif


/****************************************************************************
   double complex cexp(double complex z) returns the complex exponential of its
   argument.  The algorithm is based upon the identity:
   
      exp(x + i*y) = cos(y)*exp(x) + i*sin(y)*exp(x).
      
   Signaling of spurious overflows and invalids is avoided where
   possible.
   
   CONSTANTS:  FPKEXP709 = exp(709.0) to double precision

   Calls:  sin, cos, exp, fpclassify, and scalb.
   
   Called by:  cepwry, cxpwri, cxpwre, and cxpwry
****************************************************************************/

double complex cexp ( double complex z )   
{
   double sinval, cosval, expval, exparg;
   double complex w;
   
   sinval = sin(Imag(z));                /* sine of imaginary part of argument */
   cosval = cos(Imag(z));                /* cosine of imaginary part of argument */
   
   if (Real(z) <= 709.0) {               /* exp(Real(z)) is finite */
      expval = exp(Real(z));             /* evaluate exponential */
      Real(w) = cosval*expval;           /* real result = cos(Imag(z))*exp(Real(z)) */
      Imag(w) = sinval*expval;           /* imag result = sin(Imag(z))*exp(Real(z)) */
   }
   
   else if (fpclassify(Real(z)) < FP_ZERO) {      /* Real(z) = +INF or a NaN */
      Real(w) = cosval*Real(z);             /* deserved invalid may occur */
      Imag(w) = sinval*Real(z);             /* deserved invalid may occur */
   }
   
   else if (Real(z) > 1460.0) {          /* probable overflow case */
      Real(w) = scalbn(cosval,2100);
      Imag(w) = scalbn(sinval,2100);
   }
   
   else {           /* exp(Real(z)) overflows but product with sin or cos may not */
      Real(w) = cosval*FPKEXP709.d;   /* initialize real result */
      Imag(w) = sinval*FPKEXP709.d;   /* initialize imag result */
      exparg = Real(z) - 709.0;          /* initialize reduced exponent argument */
      while (exparg > 709.0) {        /* exponential reduction loop */
         Real(w) *= FPKEXP709.d;
         Imag(w) *= FPKEXP709.d;
         exparg -= 709.0;
      }
      expval = exp(exparg);           /* final exponential value */
      Real(w) *= expval;                 /* final multiplication steps */
      Imag(w) *= expval;
   }
   
   return w;                          /* done */
}

float complex cexpf ( float complex z )   
{
   float sinval, cosval, expval;
   float complex w;
   
   sinval = sinf(Imag(z));                /* sine of imaginary part of argument */
   cosval = cosf(Imag(z));                /* cosine of imaginary part of argument */
   
   if (Real(z) <= 88.0f) {               /* exp(Real(z)) is finite */
      expval = expf(Real(z));             /* evaluate exponential */
      Real(w) = cosval*expval;           /* real result = cos(Imag(z))*exp(Real(z)) */
      Imag(w) = sinval*expval;           /* imag result = sin(Imag(z))*exp(Real(z)) */
   }
   
   else // Handle edge cases in double
	  w = (float complex)cexp((double complex)z);
   
   return w;                          /* done */
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

// long double has same range as double
long double complex cexpl ( long double complex z )   
{
   long double sinval, cosval, expval, exparg;
   long double complex w;
   
   sinval = sinl(Imag(z));                /* sine of imaginary part of argument */
   cosval = cosl(Imag(z));                /* cosine of imaginary part of argument */
   
   if (Real(z) <= 709.0L) {               /* exp(Real(z)) is finite */
      expval = expl(Real(z));             /* evaluate exponential */
      Real(w) = cosval*expval;           /* real result = cos(Imag(z))*exp(Real(z)) */
      Imag(w) = sinval*expval;           /* imag result = sin(Imag(z))*exp(Real(z)) */
   }
   
   else if (fpclassify(Real(z)) < FP_ZERO) {      /* Real(z) = +INF or a NaN */
      Real(w) = cosval*Real(z);             /* deserved invalid may occur */
      Imag(w) = sinval*Real(z);             /* deserved invalid may occur */
   }
   
   else if (Real(z) > 1460.0L) {          /* probable overflow case */
      Real(w) = scalbnl(cosval,2100);
      Imag(w) = scalbnl(sinval,2100);
   }
   
   else {           /* exp(Real(z)) overflows but product with sin or cos may not */
	  long double t = FPKEXP709L;
	  
      Real(w) = cosval*t;   /* initialize real result */
      Imag(w) = sinval*t;   /* initialize imag result */
      exparg = Real(z) - 709.0L;          /* initialize reduced exponent argument */
      while (exparg > 709.0) {        /* exponential reduction loop */
         Real(w) *= t;
         Imag(w) *= t;
         exparg -= 709.0L;
      }
      expval = expl(exparg);           /* final exponential value */
      Real(w) *= expval;                 /* final multiplication steps */
      Imag(w) *= expval;
   }
   
   return w;                          /* done */
}
#endif
      
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

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

long double complex cpowl ( long double complex x, long double complex y )     /* (complex x)^(complex y) */
{
   long double complex logval,z;
   
   logval = clogl(x);                        /* complex logarithm of x */
   Real(z) = Real(y)*Real(logval) - Imag(y)*Imag(logval);  /* multiply by y */
   Imag(z) = Real(y)*Imag(logval) + Imag(y)*Real(logval);
   return (cexpl(z));                        /* return complex exponential */
}
#endif

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
   fenv_t env;
   double tanval, beta, sinhval, coshval, denom;
   double complex w;
   
   (void)feholdexcept(&env);            /* save environment and clear flags */
   
   if (fabs(Real(z)) > FPKASINHOM4.d) {  /* avoid overflow for large |Real(z)| */
      Real(w) = copysign(1.0,Real(z));            /* real result has unit magnitude */
      Imag(w) = copysign(0.0,Imag(z));            /* imag result is signed zero */
      if (fabs(Real(z)) != INFINITY)        /* set inexact for finite Real(z) */
         feraiseexcept(FE_INEXACT);
      feupdateenv(&env);                    /* update environment */
   }                                     /* end large |Real(z)| case */
   
   
   else {                                /* usual case */
      tanval = tan(Imag(z));                   /* evaluate tangent */
      feclearexcept(FE_DIVBYZERO);          /* in case tangent is infinite */
      feupdateenv(&env);                    /* update environment */
      beta = 1.0 + tanval*tanval;           /* 1/(cos(Imag(z)))^2 */
      sinhval = sinh(Real(z));                 /* evaluate sinh */
      coshval = sqrt(1.0+sinhval*sinhval);  /* evaluate cosh */
      
      if (fabs(tanval) == INFINITY) {    /* infinite tangent */
         Real(w) = coshval/sinhval;
         Imag(w) = 1.0/tanval;
      }
      
      else {                                /* finite tangent */
         denom = 1.0 + beta*sinhval*sinhval;
         Real(w) = beta*coshval*sinhval/denom;
         Imag(w) = tanval/denom;
      }
   }                                     /* end usual case */
   
   return w;                             /* done */
}

float complex ctanhf( float complex z )
{
   fenv_t env;
   float tanval, beta, sinhval, coshval, denom;
   float complex w;
   
   (void)feholdexcept(&env);            /* save environment and clear flags */
   
   if (fabsf(Real(z)) > FPKASINHOM4f.fval) {  /* avoid overflow for large |Real(z)| */
      Real(w) = copysignf(1.0f,Real(z));            /* real result has unit magnitude */
      Imag(w) = copysignf(0.0f,Imag(z));            /* imag result is signed zero */
      if (fabsf(Real(z)) != INFINITY)        /* set inexact for finite Real(z) */
         feraiseexcept(FE_INEXACT);
      feupdateenv(&env);                    /* update environment */
   }                                     /* end large |Real(z)| case */
   
   
   else {                                /* usual case */
      tanval = tanf(Imag(z));                   /* evaluate tangent */
      feclearexcept(FE_DIVBYZERO);          /* in case tangent is infinite */
      feupdateenv(&env);                    /* update environment */
      beta = 1.0f + tanval*tanval;           /* 1/(cos(Imag(z)))^2 */
      sinhval = sinhf(Real(z));                 /* evaluate sinh */
      coshval = sqrtf(1.0f+sinhval*sinhval);  /* evaluate cosh */
      
      if (fabs(tanval) == INFINITY) {    /* infinite tangent */
         Real(w) = coshval/sinhval;
         Imag(w) = 1.0f/tanval;
      }
      
      else {                                /* finite tangent */
         denom = 1.0f + beta*sinhval*sinhval;
         Real(w) = beta*coshval*sinhval/denom;
         Imag(w) = tanval/denom;
      }
   }                                     /* end usual case */
   
   return w;                             /* done */
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

long double complex ctanhl( long double complex z )
{
   fenv_t env;
   long double tanval, beta, sinhval, coshval, denom;
   long double complex w;
   
   (void)feholdexcept(&env);            /* save environment and clear flags */
   
   if (fabsl(Real(z)) > FPKASINHOM4.d) {  /* avoid overflow for large |Real(z)| */
      Real(w) = copysignl(1.0L,Real(z));            /* real result has unit magnitude */
      Imag(w) = copysignl(0.0L,Imag(z));            /* imag result is signed zero */
      if (fabsl(Real(z)) != INFINITY)        /* set inexact for finite Real(z) */
         feraiseexcept(FE_INEXACT);
      feupdateenv(&env);                    /* update environment */
   }                                     /* end large |Real(z)| case */
   
   
   else {                                /* usual case */
      tanval = tanl(Imag(z));                   /* evaluate tangent */
      feclearexcept(FE_DIVBYZERO);          /* in case tangent is infinite */
      feupdateenv(&env);                    /* update environment */
      beta = 1.0L + tanval*tanval;           /* 1/(cos(Imag(z)))^2 */
      sinhval = sinhl(Real(z));                 /* evaluate sinh */
      coshval = sqrtl(1.0L+sinhval*sinhval);  /* evaluate cosh */
      
      if (fabsl(tanval) == INFINITY) {    /* infinite tangent */
         Real(w) = coshval/sinhval;
         Imag(w) = 1.0L/tanval;
      }
      
      else {                                /* finite tangent */
         denom = 1.0L + beta*sinhval*sinhval;
         Real(w) = beta*coshval*sinhval/denom;
         Imag(w) = tanval/denom;
      }
   }                                     /* end usual case */
   
   return w;                             /* done */
}
#endif

/****************************************************************************
   double complex ctan(double complex x) returns the complex hyperbolic tangent of its
   argument.  The algorithm is from Kahan's paper and is based on the
   identity:
   
      tan(x + i*y) = (sin(2*x) + i*sinh(2*y))/(cos(2*x) + cosh(2*y))
                   = (tan(x)+i*cosh(y)*sinh(y)*cscsq)/(1+cscsq*sinh(y)*sinh(y)),
   
   where cscsq = 1/(cos(x)*cos(x).  For large values of ze.im, spurious
   overflow and invalid signaling is avoided.
   
   CONSTANTS:  FPKASINHOM4 = asinh(nextafterd(+infinity,0.0))/4.0 to double
                  precision
               FPKINF = +infinity
      
   Calls:  tan, sinh, sqrt, fabs, feholdexcept, feraiseexcept, feclearexcept,
           and feupdateenv.
****************************************************************************/

double complex ctan( double complex z )
{
   fenv_t env;
   double tanval, beta, sinhval, coshval, denom;
   double complex w;
   
   (void)feholdexcept(&env);            /* save environment and clear flags */
   
   if (fabs(Imag(z)) > FPKASINHOM4.d) {  /* avoid overflow for large |Imag(z)| */
      Real(w) = copysign(0.0,Real(z));            /* real result has zero magnitude */
      Imag(w) = copysign(1.0,Imag(z));            /* imag result has unit magnitude */
      if (fabs(Imag(z)) != INFINITY)        /* set inexact for finite Real(z) */
         feraiseexcept(FE_INEXACT);
      feupdateenv(&env);                    /* update environment */
   }                                     /* end large |Real(z)| case */
   
   
   else {                                /* usual case */
      tanval = tan(Real(z));                   /* evaluate tangent */
      feclearexcept(FE_DIVBYZERO);          /* in case tangent is infinite */
      feupdateenv(&env);                    /* update environment */
      beta = 1.0 + tanval*tanval;           /* 1/(cos(Real(z)))^2 */
      sinhval = sinh(Imag(z));                 /* evaluate sinh */
      coshval = sqrt(1.0+sinhval*sinhval);  /* evaluate cosh */
      
      if (fabs(tanval) == INFINITY)   {  /* infinite tangent */
         Real(w) = 1.0/tanval;
         Imag(w) = coshval/sinhval;
      }
      
      else {                                /* finite tangent */
         denom = 1.0 + beta*sinhval*sinhval;
         Real(w) = tanval/denom;
         Imag(w) = beta*coshval*sinhval/denom;
      }
   }                                     /* end usual case */
   
   return w;                             /* done */
}

float complex ctanf( float complex z )
{
   fenv_t env;
   float tanval, beta, sinhval, coshval, denom;
   float complex w;
   
   (void)feholdexcept(&env);            /* save environment and clear flags */
   
   if (fabsf(Imag(z)) > FPKASINHOM4f.fval) {  /* avoid overflow for large |Imag(z)| */
      Real(w) = copysignf(0.0f,Real(z));            /* real result has zero magnitude */
      Imag(w) = copysignf(1.0f,Imag(z));            /* imag result has unit magnitude */
      if (fabsf(Imag(z)) != INFINITY)        /* set inexact for finite Real(z) */
         feraiseexcept(FE_INEXACT);
      feupdateenv(&env);                    /* update environment */
   }                                     /* end large |Real(z)| case */
   
   
   else {                                /* usual case */
      tanval = tanf(Real(z));                   /* evaluate tangent */
      feclearexcept(FE_DIVBYZERO);          /* in case tangent is infinite */
      feupdateenv(&env);                    /* update environment */
      beta = 1.0f + tanval*tanval;           /* 1/(cos(Real(z)))^2 */
      sinhval = sinhf(Imag(z));                 /* evaluate sinh */
      coshval = sqrtf(1.0f+sinhval*sinhval);  /* evaluate cosh */
      
      if (fabsf(tanval) == INFINITY)   {  /* infinite tangent */
         Real(w) = 1.0f/tanval;
         Imag(w) = coshval/sinhval;
      }
      
      else {                                /* finite tangent */
         denom = 1.0f + beta*sinhval*sinhval;
         Real(w) = tanval/denom;
         Imag(w) = beta*coshval*sinhval/denom;
      }
   }                                     /* end usual case */
   
   return w;                             /* done */
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

long double complex ctanl( long double complex z )
{
   fenv_t env;
   long double tanval, beta, sinhval, coshval, denom;
   long double complex w;
   
   (void)feholdexcept(&env);            /* save environment and clear flags */
   
   if (fabsl(Imag(z)) > FPKASINHOM4.d) {  /* avoid overflow for large |Imag(z)| */
      Real(w) = copysignl(0.0L,Real(z));            /* real result has zero magnitude */
      Imag(w) = copysignl(1.0L,Imag(z));            /* imag result has unit magnitude */
      if (fabsl(Imag(z)) != INFINITY)        /* set inexact for finite Real(z) */
         feraiseexcept(FE_INEXACT);
      feupdateenv(&env);                    /* update environment */
   }                                     /* end large |Real(z)| case */
   
   
   else {                                /* usual case */
      tanval = tanl(Real(z));                   /* evaluate tangent */
      feclearexcept(FE_DIVBYZERO);          /* in case tangent is infinite */
      feupdateenv(&env);                    /* update environment */
      beta = 1.0L + tanval*tanval;           /* 1/(cos(Real(z)))^2 */
      sinhval = sinhl(Imag(z));                 /* evaluate sinh */
      coshval = sqrtl(1.0L+sinhval*sinhval);  /* evaluate cosh */
      
      if (fabsl(tanval) == INFINITY)   {  /* infinite tangent */
         Real(w) = 1.0L/tanval;
         Imag(w) = coshval/sinhval;
      }
      
      else {                                /* finite tangent */
         denom = 1.0L + beta*sinhval*sinhval;
         Real(w) = tanval/denom;
         Imag(w) = beta*coshval*sinhval/denom;
      }
   }                                     /* end usual case */
   
   return w;                             /* done */
}
#endif
   
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
   double complex zp1, zm, zm1, w;
   fenv_t env;
   
   Real(zp1) = 1.0 + Real(z);               /* evaluate zp1 = csqrt(1.0+z) */
   Imag(zp1) = Imag(z);
   zp1 = csqrt(zp1);
   
   Real(zm) = 1.0 - Real(z);                /* evaluate zm1 = csqrt(1.0-z) */
   Imag(zm) = -Imag(z);
   zm1 = csqrt(zm);
   
   (void)feholdexcept(&env);         /* save environ., clr flags/enables */
   Real(w) = atan(Real(z)/(Real(zp1)*Real(zm1) -Imag(zp1)*Imag(zm1))); /* real result */
   feclearexcept(FE_DIVBYZERO);       /* clear spurious DIVBYZERO exception */
   feupdateenv(&env);                 /* restore environment with new flags */
   
   Imag(zm) = Imag(z);                      /* evaluate zm1 = csqrt(1.0-cconj(z)) */
   zm1 = csqrt(zm);
   
   Imag(w) = asinh(Real(zp1)*Imag(zm1) + Imag(zp1)*Real(zm1));  /* imag result */
   
   return w;
}

float complex casinf ( float complex z )
{
   float complex zp1, zm, zm1, w;
   fenv_t env;
   
   Real(zp1) = 1.0f + Real(z);               /* evaluate zp1 = csqrt(1.0+z) */
   Imag(zp1) = Imag(z);
   zp1 = csqrtf(zp1);
   
   Real(zm) = 1.0f - Real(z);                /* evaluate zm1 = csqrt(1.0-z) */
   Imag(zm) = -Imag(z);
   zm1 = csqrtf(zm);
   
   (void)feholdexcept(&env);         /* save environ., clr flags/enables */
   Real(w) = atanf(Real(z)/(Real(zp1)*Real(zm1) -Imag(zp1)*Imag(zm1))); /* real result */
   feclearexcept(FE_DIVBYZERO);       /* clear spurious DIVBYZERO exception */
   feupdateenv(&env);                 /* restore environment with new flags */
   
   Imag(zm) = Imag(z);                      /* evaluate zm1 = csqrt(1.0-cconj(z)) */
   zm1 = csqrtf(zm);
   
   Imag(w) = asinhf(Real(zp1)*Imag(zm1) + Imag(zp1)*Real(zm1));  /* imag result */
   
   return w;
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

long double complex casinl ( long double complex z )
{
   long double complex zp1, zm, zm1, w;
   fenv_t env;
   
   Real(zp1) = 1.0L + Real(z);               /* evaluate zp1 = csqrt(1.0+z) */
   Imag(zp1) = Imag(z);
   zp1 = csqrtl(zp1);
   
   Real(zm) = 1.0L - Real(z);                /* evaluate zm1 = csqrt(1.0-z) */
   Imag(zm) = -Imag(z);
   zm1 = csqrtl(zm);
   
   (void)feholdexcept(&env);         /* save environ., clr flags/enables */
   Real(w) = atanl(Real(z)/(Real(zp1)*Real(zm1) -Imag(zp1)*Imag(zm1))); /* real result */
   feclearexcept(FE_DIVBYZERO);       /* clear spurious DIVBYZERO exception */
   feupdateenv(&env);                 /* restore environment with new flags */
   
   Imag(zm) = Imag(z);                      /* evaluate zm1 = csqrt(1.0-cconj(z)) */
   zm1 = csqrtl(zm);
   
   Imag(w) = asinhl(Real(zp1)*Imag(zm1) + Imag(zp1)*Real(zm1));  /* imag result */
   
   return w;
}
#endif

/****************************************************************************
   double complex casinh(double complex z) returns the complex inverse hyperbolic sine of
   its argument.  The algorithm is from Kahan's paper and is based on the
   formula:
   
       casinh(z) = -i*casin(i*z).
      
   Calls:  arcsinh, csqrt, atan, feholdexcept, feclearexcept, feupdateenv.
****************************************************************************/

double complex casinh ( double complex z )   
{
   double complex zp1, zm, zm1, w;
   fenv_t env;
   
   Real(zp1) = 1.0 - Imag(z);                /* evaluate zp1 = csqrt(1.0+i*z) */
   Imag(zp1) = Real(z);
   zp1 = csqrt(zp1);
   
   Real(zm) = 1.0 + Imag(z);                 /* evaluate zm1 = csqrt(1.0-cconj(i*z)) */
   Imag(zm) = Real(z);
   zm1 = csqrt(zm);
   
   Real(w) = asinh(Real(zp1)*Imag(zm1) + Imag(zp1)*Real(zm1)); /* imag result */
   
   Imag(zm) = -Real(z);                      /* evaluate zm1 = csqrt(1.0-cconj(z)) */
   zm1 = csqrt(zm);
   
   (void)feholdexcept(&env);          /* save environ., clr flags/enables */
   Imag(w) = atan(Imag(z)/(Real(zp1)*Real(zm1) -Imag(zp1)*Imag(zm1))); /* real result */
   feclearexcept(FE_DIVBYZERO);        /* clear spurious DIVBYZERO exception */
   feupdateenv(&env);                  /* restore environment with new flags */
   
   return w;
}

float complex casinhf ( float complex z )   
{
   float complex zp1, zm, zm1, w;
   fenv_t env;
   
   Real(zp1) = 1.0f - Imag(z);                /* evaluate zp1 = csqrt(1.0+i*z) */
   Imag(zp1) = Real(z);
   zp1 = csqrt(zp1);
   
   Real(zm) = 1.0f + Imag(z);                 /* evaluate zm1 = csqrt(1.0-cconj(i*z)) */
   Imag(zm) = Real(z);
   zm1 = csqrtf(zm);
   
   Real(w) = asinhf(Real(zp1)*Imag(zm1) + Imag(zp1)*Real(zm1)); /* imag result */
   
   Imag(zm) = -Real(z);                      /* evaluate zm1 = csqrt(1.0-cconj(z)) */
   zm1 = csqrtf(zm);
   
   (void)feholdexcept(&env);          /* save environ., clr flags/enables */
   Imag(w) = atanf(Imag(z)/(Real(zp1)*Real(zm1) -Imag(zp1)*Imag(zm1))); /* real result */
   feclearexcept(FE_DIVBYZERO);        /* clear spurious DIVBYZERO exception */
   feupdateenv(&env);                  /* restore environment with new flags */
   
   return w;
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

long double complex casinhl ( long double complex z )   
{
   long double complex zp1, zm, zm1, w;
   fenv_t env;
   
   Real(zp1) = 1.0L - Imag(z);                /* evaluate zp1 = csqrt(1.0+i*z) */
   Imag(zp1) = Real(z);
   zp1 = csqrtl(zp1);
   
   Real(zm) = 1.0L + Imag(z);                 /* evaluate zm1 = csqrt(1.0-cconj(i*z)) */
   Imag(zm) = Real(z);
   zm1 = csqrtl(zm);
   
   Real(w) = asinhl(Real(zp1)*Imag(zm1) + Imag(zp1)*Real(zm1)); /* imag result */
   
   Imag(zm) = -Real(z);                      /* evaluate zm1 = csqrt(1.0-cconj(z)) */
   zm1 = csqrtl(zm);
   
   (void)feholdexcept(&env);          /* save environ., clr flags/enables */
   Imag(w) = atanl(Imag(z)/(Real(zp1)*Real(zm1) -Imag(zp1)*Imag(zm1))); /* real result */
   feclearexcept(FE_DIVBYZERO);        /* clear spurious DIVBYZERO exception */
   feupdateenv(&env);                  /* restore environment with new flags */
   
   return w;
}
#endif

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
   double complex zp, zp1, zm1, w;
   fenv_t env;
   
   Real(zp) = 1.0 + Real(z);                /* zp1 = csqrt(1.0 + z) */
   Imag(zp) = Imag(z);
   zp1 = csqrt(zp);
   
   Real(zm1) = 1.0 - Real(z);               /* zm1 = csqrt(1.0 - z) */
   Imag(zm1) = -Imag(z);
   zm1 = csqrt(zm1);
   
   (void)feholdexcept(&env);         /* save environment, clr flags/enables */
   Real(w) = 2.0*atan(Real(zm1)/Real(zp1));    /* real result */
   feclearexcept(FE_DIVBYZERO);       /* clr possible spurious div-by-zero flag */
   feupdateenv(&env);                 /* update environment with new flags */
   
   Imag(zp) = -Imag(z);                     /* zp1 = csqrt(1.0 + cconj(z)) */
   zp1 = csqrt(zp);
   
   Imag(w) = asinh(Real(zp1)*Imag(zm1) + Imag(zp1)*Real(zm1)); /* imag result */
   
   return w;
}

float complex cacosf ( float complex z )
{
   float complex zp, zp1, zm1, w;
   fenv_t env;
   
   Real(zp) = 1.0f + Real(z);                /* zp1 = csqrt(1.0 + z) */
   Imag(zp) = Imag(z);
   zp1 = csqrtf(zp);
   
   Real(zm1) = 1.0f - Real(z);               /* zm1 = csqrt(1.0 - z) */
   Imag(zm1) = -Imag(z);
   zm1 = csqrtf(zm1);
   
   (void)feholdexcept(&env);         /* save environment, clr flags/enables */
   Real(w) = 2.0f*atanf(Real(zm1)/Real(zp1));    /* real result */
   feclearexcept(FE_DIVBYZERO);       /* clr possible spurious div-by-zero flag */
   feupdateenv(&env);                 /* update environment with new flags */
   
   Imag(zp) = -Imag(z);                     /* zp1 = csqrt(1.0 + cconj(z)) */
   zp1 = csqrtf(zp);
   
   Imag(w) = asinhf(Real(zp1)*Imag(zm1) + Imag(zp1)*Real(zm1)); /* imag result */
   
   return w;
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

long double complex cacosl ( long double complex z )
{
   long double complex zp, zp1, zm1, w;
   fenv_t env;
   
   Real(zp) = 1.0L + Real(z);                /* zp1 = csqrt(1.0 + z) */
   Imag(zp) = Imag(z);
   zp1 = csqrtl(zp);
   
   Real(zm1) = 1.0L - Real(z);               /* zm1 = csqrt(1.0 - z) */
   Imag(zm1) = -Imag(z);
   zm1 = csqrtl(zm1);
   
   (void)feholdexcept(&env);         /* save environment, clr flags/enables */
   Real(w) = 2.0L*atanl(Real(zm1)/Real(zp1));    /* real result */
   feclearexcept(FE_DIVBYZERO);       /* clr possible spurious div-by-zero flag */
   feupdateenv(&env);                 /* update environment with new flags */
   
   Imag(zp) = -Imag(z);                     /* zp1 = csqrt(1.0 + cconj(z)) */
   zp1 = csqrtl(zp);
   
   Imag(w) = asinhl(Real(zp1)*Imag(zm1) + Imag(zp1)*Real(zm1)); /* imag result */
   
   return w;
}
#endif


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
   double complex zp1, zm, zm1, w;
   fenv_t env;
   
   Real(zp1) = Real(z) + 1.0;              /* zp1 = csqrt(z + 1.0) */
   Imag(zp1) = Imag(z);
   zp1 = csqrt(zp1);
   
   Real(zm) = Real(z) - 1.0;               /* zm1 = csqrt(z - 1.0) */
   Imag(zm) = Imag(z);
   zm1 = csqrt(zm);
   
   (void)feholdexcept(&env);        /* save environment, clr flags/enables */
   Imag(w) = 2.0*atan(Imag(zm1)/Real(zp1));   /* imag result */
   feclearexcept(FE_DIVBYZERO);      /* clr possible spurious div-by-zero flag */
   feupdateenv(&env);                /* restore environment with new flags */
   
   Imag(zm) = -Imag(z);                    /* zm1 = csqrt(cconj(z) - 1.0) */
   zm1 = csqrt(zm);
   
   Real(w) = asinh(Real(zp1)*Real(zm1) - Imag(zp1)*Imag(zm1)); /* real result */
   
   return w;
}

float complex cacoshf ( float complex z )
{
   float complex zp1, zm, zm1, w;
   fenv_t env;
   
   Real(zp1) = Real(z) + 1.0f;              /* zp1 = csqrt(z + 1.0) */
   Imag(zp1) = Imag(z);
   zp1 = csqrtf(zp1);
   
   Real(zm) = Real(z) - 1.0f;               /* zm1 = csqrt(z - 1.0) */
   Imag(zm) = Imag(z);
   zm1 = csqrtf(zm);
   
   (void)feholdexcept(&env);        /* save environment, clr flags/enables */
   Imag(w) = 2.0f*atanf(Imag(zm1)/Real(zp1));   /* imag result */
   feclearexcept(FE_DIVBYZERO);      /* clr possible spurious div-by-zero flag */
   feupdateenv(&env);                /* restore environment with new flags */
   
   Imag(zm) = -Imag(z);                    /* zm1 = csqrt(cconj(z) - 1.0) */
   zm1 = csqrtf(zm);
   
   Real(w) = asinhf(Real(zp1)*Real(zm1) - Imag(zp1)*Imag(zm1)); /* real result */
   
   return w;
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

long double complex cacoshl ( long double complex z )
{
   long double complex zp1, zm, zm1, w;
   fenv_t env;
   
   Real(zp1) = Real(z) + 1.0L;              /* zp1 = csqrt(z + 1.0) */
   Imag(zp1) = Imag(z);
   zp1 = csqrtl(zp1);
   
   Real(zm) = Real(z) - 1.0L;               /* zm1 = csqrt(z - 1.0) */
   Imag(zm) = Imag(z);
   zm1 = csqrtl(zm);
   
   (void)feholdexcept(&env);        /* save environment, clr flags/enables */
   Imag(w) = 2.0L*atanl(Imag(zm1)/Real(zp1));   /* imag result */
   feclearexcept(FE_DIVBYZERO);      /* clr possible spurious div-by-zero flag */
   feupdateenv(&env);                /* restore environment with new flags */
   
   Imag(zm) = -Imag(z);                    /* zm1 = csqrt(cconj(z) - 1.0) */
   zm1 = csqrtl(zm);
   
   Real(w) = asinhl(Real(zp1)*Real(zm1) - Imag(zp1)*Imag(zm1)); /* real result */
   
   return w;
}
#endif

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
   double complex ctemp, w;
   double t1, t2, xi, eta, beta;
   
   xi = -Imag(z);
   beta = copysign(1.0,xi);               /* copes with unsigned zero */
   
   Imag(z) = -beta*Real(z);                     /* transform real & imag components */
   Real(z) = beta*xi;
   
   if ((Real(z) > FPKTHETA.d) || (fabs(Imag(z)) > FPKTHETA.d)) {
      xi = copysign(M_PI_2,Imag(z));    /* avoid spurious overflow */
      ctemp = xdivc(1.0,z);
      eta = Real(ctemp);
   }
      
   else if (Real(z) == 1.0) {
      t1 = fabs(Imag(z)) + FPKRHO.d;
      xi = log(sqrt(sqrt(4.0 + t1*t1))/sqrt(fabs(Imag(z))));
      eta = 0.5*copysign(M_PI-atan(2.0/(fabs(Imag(z))+FPKRHO.d)),Imag(z));
   }
   
   else {                                 /* usual case */
      t2 = fabs(Imag(z)) + FPKRHO.d;
      t1 = 1.0 - Real(z);
      t2 = t2*t2;
      xi = 0.25*log1p(4.0*Real(z)/(t1*t1 + t2));
      Real(ctemp) = (1.0 - Real(z))*(1.0 + Real(z)) - t2;
      Imag(ctemp) = Imag(z) + Imag(z);
      eta = 0.5*carg(ctemp);
   }
   
   Real(w) = -beta*eta;                      /* fix up signs of result */
   Imag(w) = -beta*xi;
   return w;
}

float complex catanf ( float complex z )
{
   float complex ctemp, w;
   float t1, t2, xi, eta, beta;
   
   xi = -Imag(z);
   beta = copysignf(1.0f,xi);               /* copes with unsigned zero */
   
   Imag(z) = -beta*Real(z);                     /* transform real & imag components */
   Real(z) = beta*xi;
   
   if ((Real(z) > FPKTHETAf.fval) || (fabsf(Imag(z)) > FPKTHETAf.fval)) {
      xi = copysignf((float) M_PI_2,Imag(z));    /* avoid spurious overflow */
      ctemp = xdivcf(1.0f,z);
      eta = Real(ctemp);
   }
      
   else if (Real(z) == 1.0f) {
      t1 = fabsf(Imag(z)) + FPKRHOf.fval;
      xi = logf(sqrtf(sqrtf(4.0f + t1*t1))/sqrtf(fabsf(Imag(z))));
      eta = 0.5f*copysignf((float)( M_PI-atan(2.0/(fabsf(Imag(z))+FPKRHOf.fval))),Imag(z));
   }
   
   else {                                 /* usual case */
      t2 = fabsf(Imag(z)) + FPKRHOf.fval;
      t1 = 1.0f - Real(z);
      t2 = t2*t2;
      xi = 0.25f*log1pf(4.0f*Real(z)/(t1*t1 + t2));
      Real(ctemp) = (1.0f - Real(z))*(1.0f + Real(z)) - t2;
      Imag(ctemp) = Imag(z) + Imag(z);
      eta = 0.5f*cargf(ctemp);
   }
   
   Real(w) = -beta*eta;                      /* fix up signs of result */
   Imag(w) = -beta*xi;
   return w;
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)
static const
	hexdbldbl FPKRHOl = {{0x20100000, 0x00000001, 0x9cbfffff, 0xffffffff}};
	
long double complex catanl ( long double complex z )
{
   long double complex ctemp, w;
   long double t1, t2, xi, eta, beta;
   
   xi = -Imag(z);
   beta = copysignl(1.0L,xi);               /* copes with unsigned zero */
   
   Imag(z) = -beta*Real(z);                     /* transform real & imag components */
   Real(z) = beta*xi;
   
   if ((Real(z) > FPKTHETA.d) || (fabsl(Imag(z)) > FPKTHETA.d)) {
      xi = copysignl(M_PI_2,Imag(z));    /* avoid spurious overflow */
      ctemp = xdivcl(1.0L,z);
      eta = Real(ctemp);
   }
      
   else if (Real(z) == 1.0L) {
      t1 = fabsl(Imag(z)) + FPKRHOl.ld;
      xi = logl(sqrtl(sqrtl(4.0L + t1*t1))/sqrtl(fabsl(Imag(z))));
      eta = 0.5L*copysignl(M_PI-atanl(2.0L/(fabsl(Imag(z))+FPKRHOl.ld)),Imag(z));
   }
   
   else {                                 /* usual case */
      t2 = fabsl(Imag(z)) + FPKRHOl.ld;
      t1 = 1.0L - Real(z);
      t2 = t2*t2;
      xi = 0.25L*log1pl(4.0L*Real(z)/(t1*t1 + t2));
      Real(ctemp) = (1.0L - Real(z))*(1.0L + Real(z)) - t2;
      Imag(ctemp) = Imag(z) + Imag(z);
      eta = 0.5L*cargl(ctemp);
   }
   
   Real(w) = -beta*eta;                      /* fix up signs of result */
   Imag(w) = -beta*xi;
   return w;
}
#endif

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
   
   if ((Real(z) > FPKTHETA.d) || (fabs(Imag(z)) > FPKTHETA.d)) {
      eta = copysign(M_PI_2,Imag(z));   /* avoid overflow */
      ctemp = xdivc(1.0,z);
      xi = Real(ctemp);
   }
      
   else if (Real(z) == 1.0) {
      t1 = fabs(Imag(z)) + FPKRHO.d;
      xi = log(sqrt(sqrt(4.0 + t1*t1))/sqrt(fabs(Imag(z))));
      eta = 0.5*copysign(M_PI-atan(2.0/(fabs(Imag(z))+FPKRHO.d)),Imag(z));
   }
   
   else {                                 /* usual case */
      t2 = fabs(Imag(z)) + FPKRHO.d;
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
   
   if ((Real(z) > FPKTHETAf.fval) || (fabsf(Imag(z)) > FPKTHETAf.fval)) {
      eta = copysignf((float) M_PI_2,Imag(z));   /* avoid overflow */
      ctemp = xdivcf(1.0f,z);
      xi = Real(ctemp);
   }
      
   else if (Real(z) == 1.0f) {
      t1 = fabsf(Imag(z)) + FPKRHOf.fval;
      xi = logf(sqrtf(sqrtf(4.0f + t1*t1))/sqrtf(fabsf(Imag(z))));
      eta = 0.5f*copysignf((float)( M_PI-atan(2.0f/(fabsf(Imag(z))+FPKRHOf.fval))),Imag(z));
   }
   
   else {                                 /* usual case */
      t2 = fabsf(Imag(z)) + FPKRHOf.fval;
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

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)
long double complex catanhl( long double complex z )   
{
   long double complex ctemp, w;
   long double t1, t2, xi, eta, beta;
   
   beta = copysignl(1.0L,Real(z));             /* copes with unsigned zero */
   
   Imag(z) = -beta*Imag(z);                     /* transform real & imag components */
   Real(z) = beta*Real(z);
   
   if ((Real(z) > FPKTHETA.d) || (fabsl(Imag(z)) > FPKTHETA.d)) {
      eta = copysignl(M_PI_2,Imag(z));   /* avoid overflow */
      ctemp = xdivcl(1.0L,z);
      xi = Real(ctemp);
   }
      
   else if (Real(z) == 1.0L) {
      t1 = fabsl(Imag(z)) + FPKRHOl.ld;
      xi = logl(sqrtl(sqrtl(4.0L + t1*t1))/sqrtl(fabsl(Imag(z))));
      eta = 0.5L*copysignl(M_PI-atanl(2.0L/(fabsl(Imag(z))+FPKRHOl.ld)),Imag(z));
   }
   
   else {                                 /* usual case */
      t2 = fabsl(Imag(z)) + FPKRHOl.ld;
      t1 = 1.0L - Real(z);
      t2 = t2*t2;
      xi = 0.25L*log1pl(4.0L*Real(z)/(t1*t1 + t2));
      Real(ctemp) = (1.0L - Real(z))*(1.0L + Real(z)) - t2;
      Imag(ctemp) = Imag(z) + Imag(z);
      eta = 0.5L*cargl(ctemp);
   }
   
   Real(w) = beta*xi;                        /* fix up signs of result */
   Imag(w) = -beta*eta;
   return w;
}
#endif

/* conj(), creal(), and cimag() are gcc built ins. */
double creal( double complex z )
{
	return __builtin_creal(z);
}

float crealf( float complex z )
{
	return __builtin_crealf(z);
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

long double creall( long double complex z )
{
	return __builtin_creall(z);
}
#endif

double cimag( double complex z )
{
	return __builtin_cimag(z);
}

float cimagf( float complex z )
{
	return __builtin_cimagf(z);
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

long double cimagl( long double complex z )
{
	return __builtin_cimagl(z);
}
#endif

double complex conj( double complex z )
{
	return __builtin_conj(z);
}

float complex conjf( float complex z )
{
	return __builtin_conjf(z);
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)

long double complex conjl( long double complex z )
{
	return __builtin_conjl(z);
}
#endif

double complex cproj( double complex z )
{
	static const double inf = __builtin_inf();
	double u = __builtin_fabs(Real(z));
	double v = __builtin_fabs(Imag(z));
    
	if (unlikely((u == inf) || (v == inf)))
		return inf + I*copysign(0.0, Imag(z));
	else
		return z;
}

float complex cprojf( float complex z )
{
	static const double inf = __builtin_inff();
	float u = __builtin_fabsf(Real(z));
	float v = __builtin_fabsf(Imag(z));
	
	if (unlikely((u == inf) || (v == inf)))
		return inf + I*copysignf(0.0f, Imag(z));
	else
		return z;	
}

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)
// PPC long double
long double complex cprojl( long double complex z )
{
	static const double inf = __builtin_infl();
	float u = __builtin_fabsl(Real(z));
	float v = __builtin_fabsl(Imag(z));
	
	if (unlikely((u == inf) || (v == inf)))
		return inf + I*copysignl(0.0l, Imag(z));
	else
		return z;	
}
#endif