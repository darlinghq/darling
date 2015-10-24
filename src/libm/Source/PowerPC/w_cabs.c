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

#include "math.h"
#include "fenv.h"
#include "fp_private.h"
#include  "complex.h"

#define complex _Complex

#define Real(z) (__real__ z)
#define Imag(z) (__imag__ z)

static const               /* sqrt(2.0) */
   hexdouble FPKSQT2 = HEXDOUBLE(0x3ff6a09e,0x667f3bcd);

static const               /* sqrt(2.0) + 1.0 to double */
   hexdouble FPKR2P1 = HEXDOUBLE(0x4003504f,0x333f9de6);

static const             /* sqrt(2.0) + 1.0 - FPKR2P1 to double */
   hexdouble FPKT2P1 = HEXDOUBLE(0x3ca21165,0xf626cdd6);

static const               
   hexdouble infinity = HEXDOUBLE(0x7ff00000, 0x00000000);

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

double cabs ( double complex z )
{
    double a,b,s,t;
    fenv_t env;
    double FPR_inf = infinity.d;
    
    a = fabs(Real(z));
    b = fabs(Imag(z));
    
    if (unlikely( (a == FPR_inf) || (b == FPR_inf) ))
	return FPR_inf;
	    
    if (unlikely( (a != a) || (b != b) ))
	return __FABS ( a + b );
    
    if (unlikely((a == 0.0) || (b == 0.0) ))
	return __FABS ( a + b );

    /* both components of z are finite, nonzero */
    {
      (void)feholdexcept(&env);         /* save environment, clear flags */
      (void)fesetround(FE_TONEAREST);   /* set default rounding */

      s = 0.0;
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
            s += sqrt(1.0 + s*s);
         }
         else                           /* a - b <= b */
         {
            s = t/b;
            t = (2.0 + s)*s;
            s = ((FPKT2P1.d+t/(FPKSQT2.d+sqrt(2.0+t)))+s)+FPKR2P1.d;
         }
         
         s = b/s;                        /* may spuriously underflow */
         feclearexcept(FE_UNDERFLOW);
      }
      
      feupdateenv(&env);                 /* restore environment */
      return (a + s);                    /* deserved overflow occurs here */
   }                                     /* finite, nonzero case */
}

