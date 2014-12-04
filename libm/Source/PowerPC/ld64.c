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

#define __WANT_LONG_DOUBLE_FORMAT__ 0 /* Defeat long double prototypes in math.h */
#include "math.h" /* for its *double* prototypes */

#undef __LIBMLDBL_COMPAT
#if 0 /* Temporarily disable the plan-in-blue. */
#define __LIBMLDBL_64(sym) __asm("_" __STRING(sym) "$LDBL64")
#define __LIBMLDBL_GLOB(sym) __asm(".text"); __asm(".align 4"); __asm(".globl " "_" __STRING(sym))
#define __LIBMLDBL_NO_DECOR(sym) __asm("_" __STRING(sym) ": b " "_" __STRING(sym) "$LDBL64")
#define __LIBMLDBL_COMPAT(sym) __LIBMLDBL_64(sym) ; __LIBMLDBL_GLOB(sym) ; __LIBMLDBL_NO_DECOR(sym)
#else
#define __LIBMLDBL_64(sym) /* NOTHING */
#define __LIBMLDBL_GLOB(sym) __asm(".text"); __asm(".align 4"); __asm(".globl " "_" __STRING(sym) "$LDBL64")
#define __LIBMLDBL_NO_DECOR(sym) __asm("_" __STRING(sym) "$LDBL64" ": b " "_" __STRING(sym))
#define __LIBMLDBL_COMPAT(sym) __LIBMLDBL_64(sym) ; __LIBMLDBL_GLOB(sym) ; __LIBMLDBL_NO_DECOR(sym)
#endif
//
// Introduce prototypes and (trivial) implementations for long double == double scheme.
// Programs compiled with -mlong-double-64 see these.
//
extern double acosl( double )							 __LIBMLDBL_COMPAT(acosl);
extern double asinl( double )							 __LIBMLDBL_COMPAT(asinl);
extern double atanl( double )							 __LIBMLDBL_COMPAT(atanl);
extern double atan2l( double, double )			 __LIBMLDBL_COMPAT(atan2l);
extern double cosl( double )							 __LIBMLDBL_COMPAT(cosl);
extern double sinl( double )							 __LIBMLDBL_COMPAT(sinl);
extern double tanl( double )							 __LIBMLDBL_COMPAT(tanl);
extern double acoshl( double )						 __LIBMLDBL_COMPAT(acoshl);
extern double asinhl( double )						 __LIBMLDBL_COMPAT(asinhl);
extern double atanhl( double )						 __LIBMLDBL_COMPAT(atanhl);
extern double coshl( double )							 __LIBMLDBL_COMPAT(coshl);
extern double sinhl( double )							 __LIBMLDBL_COMPAT(sinhl);
extern double tanhl( double )							 __LIBMLDBL_COMPAT(tanhl);
extern double expl( double )							 __LIBMLDBL_COMPAT(expl);
extern double exp2l( double )							 __LIBMLDBL_COMPAT(exp2l);
extern double expm1l( double )						 __LIBMLDBL_COMPAT(expm1l);
extern double logl( double )							 __LIBMLDBL_COMPAT(logl);
extern double log10l( double )						 __LIBMLDBL_COMPAT(log10l);
extern double log2l( double )							 __LIBMLDBL_COMPAT(log2l);
extern double log1pl( double )						 __LIBMLDBL_COMPAT(log1pl);
extern double logbl( double )							 __LIBMLDBL_COMPAT(logbl);
extern double modfl( double, double * )			 __LIBMLDBL_COMPAT(modfl);
extern double ldexpl( double, int )					 __LIBMLDBL_COMPAT(ldexpl);
extern double frexpl( double, int * )					 __LIBMLDBL_COMPAT(frexpl);
extern int		   ilogbl( double )						 __LIBMLDBL_COMPAT(ilogbl);
extern double scalbnl( double, int )					 __LIBMLDBL_COMPAT(scalbnl);
extern double scalblnl( double, long int )			 __LIBMLDBL_COMPAT(scalblnl);
extern double fabsl( double )							 __LIBMLDBL_COMPAT(fabsl);
extern double cbrtl( double )							 __LIBMLDBL_COMPAT(cbrtl);
extern double hypotl( double, double )			 __LIBMLDBL_COMPAT(hypotl);
extern double powl( double, double )				 __LIBMLDBL_COMPAT(powl);
extern double sqrtl( double )							 __LIBMLDBL_COMPAT(sqrtl);
extern double erfl( double )							 __LIBMLDBL_COMPAT(erfl);
extern double erfcl( double )							 __LIBMLDBL_COMPAT(erfcl);
extern double lgammal( double )						 __LIBMLDBL_COMPAT(lgammal);
extern double tgammal( double )						 __LIBMLDBL_COMPAT(tgammal);
extern double ceill( double )							 __LIBMLDBL_COMPAT(ceill);
extern double floorl( double )						 __LIBMLDBL_COMPAT(floorl);
extern double nearbyintl( double )					 __LIBMLDBL_COMPAT(nearbyintl);
extern double rintl( double )							 __LIBMLDBL_COMPAT(rintl);
extern long int    lrintl( double )						 __LIBMLDBL_COMPAT(lrintl);
extern long long int llrintl( double )						 __LIBMLDBL_COMPAT(llrintl);
extern double roundl( double )						 __LIBMLDBL_COMPAT(roundl);
extern long int    lroundl( double )						 __LIBMLDBL_COMPAT(lroundl);
extern long long int llroundl( double )					 __LIBMLDBL_COMPAT(llroundl);
extern double truncl( double )						 __LIBMLDBL_COMPAT(truncl);
extern double fmodl( double, double)				 __LIBMLDBL_COMPAT(fmodl);
extern double remainderl( double, double )		 __LIBMLDBL_COMPAT(remainderl);
extern double remquol( double, double, int * )	 __LIBMLDBL_COMPAT(remquol);
extern double copysignl( double, double )		 __LIBMLDBL_COMPAT(copysignl);
extern double nextafterl( double, double )		 __LIBMLDBL_COMPAT(nextafterl);
extern double	   nexttoward( double, double )			 __LIBMLDBL_COMPAT(nexttoward);
extern float	   nexttowardf( float, double )			 __LIBMLDBL_COMPAT(nexttowardf);
extern double nexttowardl( double, double )		 __LIBMLDBL_COMPAT(nexttowardl);
extern double fdiml( double, double )			 __LIBMLDBL_COMPAT(fdiml);
extern double fmaxl( double, double )			 __LIBMLDBL_COMPAT(fmaxl);
extern double fminl( double, double )			 __LIBMLDBL_COMPAT(fminl);
extern double fmal( double, double, double )  __LIBMLDBL_COMPAT(fmal);

double acosl( double x )				{ return (double)acos((double) x); }
double asinl( double x )				{ return (double)asin((double) x); }
double atanl( double x )				{ return (double)atan((double) x); }
double atan2l( double y, double x ) 
												{ return (double)atan2((double) y, (double) x); }
double cosl( double x )				{ return (double)cos((double) x); }
double sinl( double x )				{ return (double)sin((double) x); }
double tanl( double x )				{ return (double)tan((double) x); }
double acoshl( double x )				{ return (double)acosh((double) x); }
double asinhl( double x )				{ return (double)asinh((double) x); }
double atanhl( double x )				{ return (double)atanh((double) x); }
double coshl( double x )				{ return (double)cosh((double) x); }
double sinhl( double x )				{ return (double)sinh((double) x); }
double tanhl( double x )				{ return (double)tanh((double) x); }
double expl( double x )				{ return (double)exp((double) x); }
double exp2l( double x )				{ return (double)exp2((double) x); }
double expm1l( double x )				{ return (double)expm1((double) x); }
double logl( double x )				{ return (double)log((double) x); }
double log10l( double x )				{ return (double)log10((double) x); }
double log2l( double x )				{ return (double)log2((double) x); }
double log1pl( double x )				{ return (double)log1p((double) x); }
double logbl( double x )				{ return (double)logb((double) x); }
double modfl( double x, double *iptr ) 
												{ return (double)modf((double) x, (double *)iptr); }
double ldexpl( double x, int n )		{ return (double)ldexp((double) x, n); }
double frexpl( double x, int *exp )	{ return (double)frexp((double) x, exp); }
int			ilogbl( double x )				{ return ilogb((double) x); }
double scalbnl( double x, int n )		{ return (double)scalbn((double) x, n); }
double scalblnl( double x, long int n ) { return (double)scalbln((double) x, n); }
double fabsl( double x )				{ return (double)fabs((double) x); }
double cbrtl( double x )				{ return (double)cbrt((double) x); }
double hypotl( double x, double y ) 
												{ return (double)hypot((double) x, (double) y); }
double powl( double x, double y ) 
												{ return (double)pow((double) x, (double) y); }
double sqrtl( double x )				{ return (double)sqrt((double) x); }
double erfl( double x )				{ return (double)erf((double) x); }
double erfcl( double x )				{ return (double)erfc((double) x); }
double lgammal( double x )			{ return (double)lgamma((double) x); }
double tgammal( double x )			{ return (double)tgamma((double) x); }
double ceill( double x )				{ return (double)ceil((double) x); }
double floorl( double x )				{ return (double)floor((double) x); }
double nearbyintl( double x )			{ return (double)nearbyint((double) x); }
double rintl( double x )				{ return (double)rint((double) x); }
long int    lrintl( double x )				{ return (long int)lrint((double) x); }
long long int llrintl( double x )			{ return (long long int)llrint((double) x); }
double roundl( double x )				{ return (double)round((double) x); }
long int    lroundl( double x )			{ return (long int)lround((double) x); }
long long int llroundl( double x )			{ return (long long int)llround((double) x); }
double truncl( double x )				{ return (double)trunc((double) x); }
double fmodl( double x, double y){ return (double)fmod((double) x, (double) y); }
double remainderl( double x, double y ) 
												{ return (double)remainder((double) x, (double) y); }
double remquol( double x, double y, int *quo ) 
												{ return (double)remquo((double) x, (double) y, quo); }
double copysignl( double x, double y ) 
												{ return (double)copysign((double) x, (double) y); }
double nextafterl( double x, double y ) 
												{ return (double)nextafter((double) x, (double) y); }
double      nexttoward( double x, double y ) { return (double)nextafter((double) x, (double) y); }
float       nexttowardf( float x, double y ) 
{
if ((double) x == y || y != y) return (float)y; /* 7.2.11.4  paragraph 2*/
else if ((double)x > y) return nextafterf(x, -INFINITY);
        else return nextafterf(x, INFINITY);
}

double nexttowardl( double x, double y )                
                                                { return (double)nextafter((double) x, (double) y); }
double fdiml( double x, double y ) { return (double)fdim((double) x, (double) y); }
double fmaxl( double x, double y ) { return (double)fmax((double) x, (double) y); }
double fminl( double x, double y ) { return (double)fmin((double) x, (double) y); }
double fmal( double x, double y, double z ) 
												{ return (double)fma((double) x, (double) y, (double) z); }
	
