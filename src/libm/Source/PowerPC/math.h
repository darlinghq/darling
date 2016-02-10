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
 
/*******************************************************************************
*                                                                              *
*     File:  math.h	                                                       *
*                                                                              *
*     Contains: typedefs, prototypes, and macros germane to C99 floating point.*
*                                                                              *
*******************************************************************************/
#ifndef __MATH__
#define __MATH__

#include "sys/cdefs.h" /* For definition of __DARWIN_UNIX03 et al */

#if (!defined(__WANT_LONG_DOUBLE_FORMAT__))
#if defined(__APPLE_CC__) && defined(__LONG_DOUBLE_128__)
#define __WANT_LONG_DOUBLE_FORMAT__ 128
#else
#define __WANT_LONG_DOUBLE_FORMAT__ 64
#endif
#endif

#if ( __WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L )
#define __LIBMLDBL_COMPAT(sym) __asm("_" __STRING(sym) "$LDBL128")
#elif ( __WANT_LONG_DOUBLE_FORMAT__ - 0L == 64L )
#define __LIBMLDBL_COMPAT(sym) /* NOTHING */
#else
#define __LIBMLDBL_COMPAT(sym) /* NOTHING */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
*       Floating point data types                                             *
******************************************************************************/

/*	Define float_t and double_t per C standard, ISO/IEC 9899:1999 7.12 2,
	taking advantage of GCC's __FLT_EVAL_METHOD__ (which a compiler may
	define anytime and GCC does) that shadows FLT_EVAL_METHOD (which a compiler
	must and may define only in float.h).
*/
#if __FLT_EVAL_METHOD__ == 0
	typedef float float_t;
	typedef double double_t;
#elif __FLT_EVAL_METHOD__ == 1
	typedef double float_t;
	typedef double double_t;
#elif __FLT_EVAL_METHOD__ == 2 || __FLT_EVAL_METHOD__ == -1
	typedef long double float_t;
	typedef long double double_t;
#else /* __FLT_EVAL_METHOD__ */
	#error "Unsupported value of __FLT_EVAL_METHOD__."
#endif /* __FLT_EVAL_METHOD__ */


#if defined(__GNUC__) && ! defined( __XLC__ )
	#define	HUGE_VAL	__builtin_huge_val()
	#define	HUGE_VALF	__builtin_huge_valf()
	#define	HUGE_VALL	__builtin_huge_vall()
	#define NAN			__builtin_nanf("0x7fc00000") /* Constant expression, can be used as initializer. */
	#define __MATH_H_ALWAYS_INLINE__		__attribute__ ((always_inline))
#else
	#define	HUGE_VAL	1e500
	#define	HUGE_VALF	1e50f
	#define	HUGE_VALL	1e500L
	#define NAN		__nan( )
	#define __MATH_H_ALWAYS_INLINE__
#endif

#define INFINITY	HUGE_VALF

/******************************************************************************
*      Taxonomy of floating point data types                                  *
******************************************************************************/

#define FP_NAN          1
#define FP_INFINITE     2
#define FP_ZERO         3
#define FP_NORMAL       4
#define FP_SUBNORMAL    5
#define FP_SUPERNORMAL  6

/* fma() *function call* is more costly than equivalent (in-line) multiply and add operations */
#undef FP_FAST_FMA
#undef FP_FAST_FMAF
#undef FP_FAST_FMAL

/* The values returned by `ilogb' for 0 and NaN respectively.  */
#define FP_ILOGB0	(-2147483647)
#define FP_ILOGBNAN	(2147483647)

/* Bitmasks for the math_errhandling macro.  */
#define MATH_ERRNO		1	/* errno set by math functions.  */
#define MATH_ERREXCEPT	2	/* Exceptions raised by math functions.  */

#define math_errhandling (__math_errhandling())
extern int __math_errhandling ( void );

/********************************************************************************
*                                                                               *
*                              Inquiry macros                                   *
*                                                                               *
*   fpclassify      Returns one of the FP_* values.                             *
*   isnormal        Non-zero if and only if the argument x is normalized.       *
*   isfinite        Non-zero if and only if the argument x is finite.           *
*   isnan           Non-zero if and only if the argument x is a NaN.            *
*   signbit         Non-zero if and only if the sign of the argument x is       *
*                   negative.  This includes, NaNs, infinities and zeros.       *
*                                                                               *
********************************************************************************/

#if (__WANT_LONG_DOUBLE_FORMAT__ - 0L == 128L)
	#define fpclassify(x)	\
		(	sizeof (x) == sizeof(float )	?	__fpclassifyf((float)x)	\
		:	sizeof (x) == sizeof(double)	?	__fpclassifyd((double)x)	\
											:	__fpclassify ((long double)x))

	extern int __fpclassifyf(float      );
	extern int __fpclassifyd(double     );
	extern int __fpclassify (long double);

	#if defined( __GNUC__ ) && ! defined( __XLC__ ) && 0 == __FINITE_MATH_ONLY__
    /*  Yes, that's right. You only get the fast iswhatever() macros if you do NOT turn on -ffast-math.  */
    /*  These inline functions require the compiler to be compiling to standard in order to work.        */
    /*  -ffast-math, among other things, implies that NaNs don't happen. The compiler can in that case   */
    /*  optimize x != x to be false always, wheras it would be true for NaNs. That breaks __inline_isnan() */
    /*  below.                                                                                           */

		#define isnormal(x)	\
			(	sizeof (x) == sizeof(float )	?	__inline_isnormalf((float)(x))	\
			:	sizeof (x) == sizeof(double)	?	__inline_isnormald((double)(x))	\
												:	__inline_isnormal ((long double)(x)))

		#define isfinite(x)	\
			(	sizeof (x) == sizeof(float )	?	__inline_isfinitef((float)(x))	\
			:	sizeof (x) == sizeof(double)	?	__inline_isfinited((double)(x))	\
												:	__inline_isfinite ((long double)(x)))

		#define isinf(x)	\
			(	sizeof (x) == sizeof(float )	?	__inline_isinff((float)(x))	\
			:	sizeof (x) == sizeof(double)	?	__inline_isinfd((double)(x))	\
												:	__inline_isinf ((long double)(x)))

		#define isnan(x)	\
			(	sizeof (x) == sizeof(float )	?	__inline_isnanf((float)(x))	\
			:	sizeof (x) == sizeof(double)	?	__inline_isnand((double)(x))	\
												:	__inline_isnan ((long double)(x)))

		#define signbit(x)	\
			(	sizeof (x) == sizeof(float )	?	__inline_signbitf((float)(x))	\
			:	sizeof (x) == sizeof(double)	?	__inline_signbitd((double)(x))	\
												:	__inline_signbit((long double)(x)))

        /* Developers who are calling __isnan, __isnormal, __isinf, etc. and now encountering errors are calling private APIs       */ 
        /* that are deprecated.  Please use the official C99 sanctioned macros listed above instead.                                */

		static __inline__ int __inline_isfinitef	(float      ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isfinited	(double     ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isfinite		(long double) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isinff		(float      ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isinfd		(double     ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isinf		(long double) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isnanf		(float      ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isnand		(double     ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isnan		(long double) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isnormalf    (float      ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isnormald    (double     ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isnormal     (long double) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_signbitf     (float      ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_signbitd     (double     ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_signbit      (long double) __MATH_H_ALWAYS_INLINE__;
		
		static __inline__ int __inline_isinff( float __x ) { return __builtin_fabsf(__x) == __builtin_inff(); }
		static __inline__ int __inline_isinfd( double __x ) { return __builtin_fabs(__x) == __builtin_inf(); }
		static __inline__ int __inline_isinf( long double __x ) { return __builtin_fabsl(__x) == __builtin_infl(); }
		static __inline__ int __inline_isfinitef( float __x ) { return __x == __x && __builtin_fabsf(__x) != __builtin_inff(); }
		static __inline__ int __inline_isfinited( double __x ) { return __x == __x && __builtin_fabs(__x) != __builtin_inf(); }
		static __inline__ int __inline_isfinite( long double __x ) { return __x == __x && __builtin_fabsl(__x) != __builtin_infl(); }
		static __inline__ int __inline_isnanf( float __x ) { return __x != __x; }
		static __inline__ int __inline_isnand( double __x ) { return __x != __x; }
		static __inline__ int __inline_isnan( long double __x ) { return __x != __x; }
		static __inline__ int __inline_signbitf( float __x ) { union{ float __f; unsigned int __u; }__u; __u.__f = __x; return (int)(__u.__u >> 31); }
		static __inline__ int __inline_signbitd( double __x ) { union{ double __f; unsigned long __u; }__u; __u.__f = __x; return (int)(__u.__u >> ( 8 * sizeof( __u.__u) - 1)); }
		static __inline__ int __inline_signbit( long double __x ){ union{ long double __ld; unsigned long __p; }__u; __u.__ld = __x; return (int) (__u.__p >> ( 8 * sizeof( __u.__p) - 1)); } 
		static __inline__ int __inline_isnormalf( float __x ) { float fabsf = __builtin_fabsf(__x); if( __x != __x ) return 0; return fabsf < __builtin_inff() && fabsf >= __FLT_MIN__; }  
		static __inline__ int __inline_isnormald( double __x ) { double fabsf = __builtin_fabs(__x); if( __x != __x ) return 0; return fabsf < __builtin_inf() && fabsf >= __DBL_MIN__; }  
		static __inline__ int __inline_isnormal( long double __x ) { long double fabsf = __builtin_fabsl(__x); if( __x != __x ) return 0; return fabsf < __builtin_infl() && fabsf >= __LDBL_MIN__; }  
		
	#else

		#define isnormal(x)	\
			(	sizeof (x) == sizeof(float )	?	__isnormalf((float)(x))	\
			:	sizeof (x) == sizeof(double)	?	__isnormald((double)(x))	\
												:	__isnormal ((long double)(x)))

		#define isfinite(x)	\
			(	sizeof (x) == sizeof(float )	?	__isfinitef((float)(x))	\
			:	sizeof (x) == sizeof(double)	?	__isfinited((double)(x))	\
												:	__isfinite ((long double)(x)))

		#define isinf(x)	\
			(	sizeof (x) == sizeof(float )	?	__isinff((float)(x))	\
			:	sizeof (x) == sizeof(double)	?	__isinfd((double)(x))	\
												:	__isinf ((long double)(x)))

		#define isnan(x)	\
			(	sizeof (x) == sizeof(float )	?	__isnanf((float)(x))	\
			:	sizeof (x) == sizeof(double)	?	__isnand((double)(x))	\
												:	__isnan ((long double)(x)))

		#define signbit(x)	\
			(	sizeof (x) == sizeof(float )	?	__signbitf((float)(x))	\
			:	sizeof (x) == sizeof(double)	?	__signbitd((double)(x))	\
												:	__signbitl((long double)(x)))


		extern int __isnormalf  (float      );
		extern int __isnormald  (double     );
		extern int __isnormal   (long double);

		extern int __isfinitef  (float      );
		extern int __isfinited  (double     );
		extern int __isfinite   (long double);

		extern int __isinff     (float      );
		extern int __isinfd     (double     );
		extern int __isinf      (long double);

		extern int __isnanf     (float      );
		extern int __isnand     (double     );
		extern int __isnan      (long double);

		extern int __signbitf   (float      );
		extern int __signbitd   (double     );
		extern int __signbitl   (long double);

	#endif

#else
	#define fpclassify(x)	\
		(	sizeof (x) == sizeof(float )	?	__fpclassifyf((float)(x))	:	__fpclassifyd((double)(x))	)

	extern int __fpclassifyf(float      );
	extern int __fpclassifyd(double     );

	#if defined( __GNUC__ ) && ! defined( __XLC__ ) && 0 == __FINITE_MATH_ONLY__
    /*  Yes, that's right. You only get the fast iswhatever() macros if you do NOT turn on -ffast-math.  */
    /*  These inline functions require the compiler to be compiling to standard in order to work.        */
    /*  -ffast-math, among other things, implies that NaNs don't happen. The compiler can in that case   */
    /*  optimize x != x to be false always, wheras it would be true for NaNs. That breaks __inline_isnan() */
    /*  below.                                                                                           */
		#define isnormal(x)	\
			(	sizeof (x) == sizeof(float )	?	__inline_isnormalf((float)(x))	:	__inline_isnormald((double)(x))	)

		#define isfinite(x)	\
			(	sizeof (x) == sizeof(float )	?	__inline_isfinitef((float)(x))	:	__inline_isfinited((double)(x))	)

		#define isinf(x)	\
			(	sizeof (x) == sizeof(float )	?	__inline_isinff((float)(x))		:	__inline_isinfd((double)(x))	)

		#define isnan(x)	\
			(	sizeof (x) == sizeof(float )	?	__inline_isnanf((float)(x))		:	__inline_isnand((double)(x))	)

		#define signbit(x)	\
			(	sizeof (x) == sizeof(float )	?	__inline_signbitf((float)(x))	:	__inline_signbitd((double)(x))	)

        /* Developers who are calling __isnan, __isnormal, __isinf, etc. and now encountering errors are calling private APIs           */ 
        /* that are deprecated.  Please use the official C99 sanctioned macros listed above instead.                                    */

		static __inline__ int __inline_isfinitef	(float      ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isfinited	(double     ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isfinite		(long double) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isinff		(float      ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isinfd		(double     ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isinf		(long double) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isnanf		(float      ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isnand		(double     ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isnan		(long double) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isnormalf    (float      ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isnormald    (double     ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_isnormal     (long double) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_signbitf     (float      ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_signbitd     (double     ) __MATH_H_ALWAYS_INLINE__;
		static __inline__ int __inline_signbit      (long double) __MATH_H_ALWAYS_INLINE__;
		
		static __inline__ int __inline_isinff( float __x ) { return __builtin_fabsf(__x) == __builtin_inff(); }
		static __inline__ int __inline_isinfd( double __x ) { return __builtin_fabs(__x) == __builtin_inf(); }
		static __inline__ int __inline_isinf( long double __x ) { return __builtin_fabsl(__x) == __builtin_infl(); }
		static __inline__ int __inline_isfinitef( float __x ) { return __x == __x && __builtin_fabsf(__x) != __builtin_inff(); }
		static __inline__ int __inline_isfinited( double __x ) { return __x == __x && __builtin_fabs(__x) != __builtin_inf(); }
		static __inline__ int __inline_isfinite( long double __x ) { return __x == __x && __builtin_fabsl(__x) != __builtin_infl(); }
		static __inline__ int __inline_isnanf( float __x ) { return __x != __x; }
		static __inline__ int __inline_isnand( double __x ) { return __x != __x; }
		static __inline__ int __inline_isnan( long double __x ) { return __x != __x; }
		static __inline__ int __inline_signbitf( float __x ) { union{ float __f; unsigned int __u; }__u; __u.__f = __x; return (int)(__u.__u >> 31); }
		static __inline__ int __inline_signbitd( double __x ) { union{ double __f; unsigned long __u; }__u; __u.__f = __x; return (int)(__u.__u >> ( 8 * sizeof( __u.__u) - 1)); }
		static __inline__ int __inline_signbit( long double __x ){ union{ long double __ld; unsigned long __p; }__u; __u.__ld = __x; return (int) (__u.__p >> ( 8 * sizeof( __u.__p) - 1)); } 
		static __inline__ int __inline_isnormalf( float __x ) { float fabsf = __builtin_fabsf(__x); if( __x != __x ) return 0; return fabsf < __builtin_inff() && fabsf >= __FLT_MIN__; }  
		static __inline__ int __inline_isnormald( double __x ) { double fabsf = __builtin_fabs(__x); if( __x != __x ) return 0; return fabsf < __builtin_inf() && fabsf >= __DBL_MIN__; }  
		static __inline__ int __inline_isnormal( long double __x ) { long double fabsf = __builtin_fabsl(__x); if( __x != __x ) return 0; return fabsf < __builtin_infl() && fabsf >= __LDBL_MIN__; }  
		
	#else

		#define isnormal(x)	\
			(	sizeof (x) == sizeof(float )	?	__isnormalf((float)(x))	:	__isnormald((double)(x))	)

		#define isfinite(x)	\
			(	sizeof (x) == sizeof(float )	?	__isfinitef((float)(x))	:	__isfinited((double)(x))	)

		#define isinf(x)	\
			(	sizeof (x) == sizeof(float )	?	__isinff((float)(x))	:	__isinfd((double)(x))		)

		#define isnan(x)	\
			(	sizeof (x) == sizeof(float )	?	__isnanf((float)(x))	:	__isnand((double)(x))		)

		#define signbit(x)	\
			(	sizeof (x) == sizeof(float )	?	__signbitf((float)(x))	:	__signbitd((double)(x))		)


		extern int __isnormalf  (float      );
		extern int __isnormald  (double     );

		extern int __isfinitef  (float      );
		extern int __isfinited  (double     );

		extern int __isinff     (float      );
		extern int __isinfd     (double     );

		extern int __isnanf     (float      );
		extern int __isnand     (double     );

		extern int __signbitf   (float      );
		extern int __signbitd   (double     );

	#endif
							  
#endif /* __WANT_LONG_DOUBLE_FORMAT__ */
                              

/********************************************************************************
*                                                                               *
*                              Math Functions                                   *
*                                                                               *
********************************************************************************/

extern double acos( double );
extern float acosf( float );

extern double asin( double );
extern float asinf( float );

extern double atan( double );
extern float atanf( float );

extern double atan2( double, double );
extern float atan2f( float, float );

extern double cos( double );
extern float cosf( float );

extern double sin( double );
extern float sinf( float );

extern double tan( double );
extern float tanf( float );

extern double acosh( double );
extern float acoshf( float );

extern double asinh( double );
extern float asinhf( float );

extern double atanh( double );
extern float atanhf( float );

extern double cosh( double );
extern float coshf( float );

extern double sinh( double );
extern float sinhf( float );

extern double tanh( double );
extern float tanhf( float );

extern double exp( double );
extern float expf( float );

extern double exp2( double );
extern float exp2f( float );

extern double expm1( double );
extern float expm1f( float );

extern double log( double );
extern float logf( float );

extern double log10( double );
extern float log10f( float );

extern double log2( double );
extern float log2f( float );

extern double log1p( double );
extern float log1pf( float );

extern double logb( double );
extern float logbf( float );

extern double modf( double, double * );
extern float modff( float, float * );

extern double ldexp( double, int );
extern float ldexpf( float, int );

extern double frexp( double, int * );
extern float frexpf( float, int * );

extern int ilogb( double );
extern int ilogbf( float );

extern double scalbn( double, int );
extern float scalbnf( float, int );

extern double scalbln( double, long int );
extern float scalblnf( float, long int );

extern double fabs( double );
extern float fabsf( float );

extern double cbrt( double );
extern float cbrtf( float );

extern double hypot( double, double );
extern float hypotf( float, float );

extern double pow( double, double );
extern float powf( float, float );

extern double sqrt( double );
extern float sqrtf( float );

extern double erf( double );
extern float erff( float );

extern double erfc( double );
extern float erfcf( float );

extern double lgamma( double );
extern float lgammaf( float );

extern double tgamma( double );
extern float tgammaf( float );

extern double ceil( double );
extern float ceilf( float );

extern double floor( double );
extern float floorf( float );

extern double nearbyint( double );
extern float nearbyintf( float );

extern double rint( double );
extern float rintf( float );

extern long int lrint( double );
extern long int lrintf( float );

extern double round( double );
extern float roundf( float );

extern long int lround( double );
extern long int lroundf( float );

#if ( defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L ) || ! defined( __STRICT_ANSI__ )  || ! defined( __GNUC__ )
    /* C90 doesn't know about long long. Make sure you are passing -std=c99 or -std=gnu99 or better if you need this. */
    extern long long int llrint( double );
    extern long long int llrintf( float );

    extern long long int llround( double );
    extern long long int llroundf( float );
#endif /*#if ( defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L ) || ! defined( __STRICT_ANSI__ )  || ! defined( __GNUC__ )  */

extern double trunc( double );
extern float truncf( float );

extern double fmod( double, double );
extern float fmodf( float, float );

extern double remainder( double, double );
extern float remainderf( float, float );

extern double remquo( double, double, int * );
extern float remquof( float, float, int * );

extern double copysign( double, double );
extern float copysignf( float, float );

extern double nan( const char * );
extern float nanf( const char * );

extern double nextafter( double, double );
extern float nextafterf( float, float );


extern double fdim( double, double );
extern float fdimf( float, float );

extern double fmax( double, double );
extern float fmaxf( float, float );

extern double fmin( double, double );
extern float fminf( float, float );

extern double fma( double, double, double );
extern float fmaf( float, float, float );

#if ( __WANT_LONG_DOUBLE_FORMAT__ - 0L > 0L ) 
extern long double acosl( long double ) __LIBMLDBL_COMPAT(acosl);
extern long double asinl( long double ) __LIBMLDBL_COMPAT(asinl);
extern long double atanl( long double ) __LIBMLDBL_COMPAT(atanl);
extern long double atan2l( long double, long double ) __LIBMLDBL_COMPAT(atan2l);
extern long double cosl( long double ) __LIBMLDBL_COMPAT(cosl);
extern long double sinl( long double ) __LIBMLDBL_COMPAT(sinl);
extern long double tanl( long double ) __LIBMLDBL_COMPAT(tanl);
extern long double acoshl( long double ) __LIBMLDBL_COMPAT(acoshl);
extern long double asinhl( long double ) __LIBMLDBL_COMPAT(asinhl);
extern long double atanhl( long double ) __LIBMLDBL_COMPAT(atanhl);
extern long double coshl( long double ) __LIBMLDBL_COMPAT(coshl);
extern long double sinhl( long double ) __LIBMLDBL_COMPAT(sinhl);
extern long double tanhl( long double ) __LIBMLDBL_COMPAT(tanhl);
extern long double expl( long double ) __LIBMLDBL_COMPAT(expl);
extern long double exp2l( long double ) __LIBMLDBL_COMPAT(exp2l);
extern long double expm1l( long double ) __LIBMLDBL_COMPAT(expm1l);
extern long double logl( long double ) __LIBMLDBL_COMPAT(logl);
extern long double log10l( long double ) __LIBMLDBL_COMPAT(log10l);
extern long double log2l( long double ) __LIBMLDBL_COMPAT(log2l);
extern long double log1pl( long double ) __LIBMLDBL_COMPAT(log1pl);
extern long double logbl( long double ) __LIBMLDBL_COMPAT(logbl);
extern long double modfl( long double, long double * ) __LIBMLDBL_COMPAT(modfl);
extern long double ldexpl( long double, int ) __LIBMLDBL_COMPAT(ldexpl);
extern long double frexpl( long double, int * ) __LIBMLDBL_COMPAT(frexpl);
extern int ilogbl( long double ) __LIBMLDBL_COMPAT(ilogbl);
extern long double scalbnl( long double, int ) __LIBMLDBL_COMPAT(scalbnl);
extern long double scalblnl( long double, long int ) __LIBMLDBL_COMPAT(scalblnl);
extern long double fabsl( long double ) __LIBMLDBL_COMPAT(fabsl);
extern long double cbrtl( long double ) __LIBMLDBL_COMPAT(cbrtl);
extern long double hypotl( long double, long double ) __LIBMLDBL_COMPAT(hypotl);
extern long double powl( long double, long double ) __LIBMLDBL_COMPAT(powl);
extern long double sqrtl( long double ) __LIBMLDBL_COMPAT(sqrtl);
extern long double erfl( long double ) __LIBMLDBL_COMPAT(erfl);
extern long double erfcl( long double ) __LIBMLDBL_COMPAT(erfcl);
extern long double lgammal( long double ) __LIBMLDBL_COMPAT(lgammal);
extern long double tgammal( long double ) __LIBMLDBL_COMPAT(tgammal);
extern long double ceill( long double ) __LIBMLDBL_COMPAT(ceill);
extern long double floorl( long double ) __LIBMLDBL_COMPAT(floorl);
extern long double nearbyintl( long double ) __LIBMLDBL_COMPAT(nearbyintl);
extern long double rintl( long double ) __LIBMLDBL_COMPAT(rintl);
extern long int lrintl( long double ) __LIBMLDBL_COMPAT(lrintl);
extern long double roundl( long double ) __LIBMLDBL_COMPAT(roundl);
extern long int lroundl( long double ) __LIBMLDBL_COMPAT(lroundl);

#if ( defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L ) || ! defined( __STRICT_ANSI__ ) || ! defined( __GNUC__ )
        /* C90 doesn't know about long long. Make sure you are passing -std=c99 or -std=gnu99 or better if you need this. */
    extern long long int llrintl( long double ) __LIBMLDBL_COMPAT(llrintl);
    extern long long int llroundl( long double ) __LIBMLDBL_COMPAT(llroundl);
#endif /* #if ( defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L ) || ! defined( __STRICT_ANSI__ )  || ! defined( __GNUC__ ) */

extern long double truncl( long double ) __LIBMLDBL_COMPAT(truncl);
extern long double fmodl( long double, long double) __LIBMLDBL_COMPAT(fmodl);
extern long double remainderl( long double, long double ) __LIBMLDBL_COMPAT(remainderl);
extern long double remquol( long double, long double, int * ) __LIBMLDBL_COMPAT(remquol);
extern long double copysignl( long double, long double ) __LIBMLDBL_COMPAT(copysignl);
extern long double nanl( const char * ) __LIBMLDBL_COMPAT(nanl);
extern long double nextafterl( long double, long double ) __LIBMLDBL_COMPAT(nextafterl);
extern double nexttoward( double, long double ) __LIBMLDBL_COMPAT(nexttoward);
extern float nexttowardf( float, long double ) __LIBMLDBL_COMPAT(nexttowardf);
extern long double nexttowardl( long double, long double ) __LIBMLDBL_COMPAT(nexttowardl);
extern long double fdiml( long double, long double ) __LIBMLDBL_COMPAT(fdiml);
extern long double fmaxl( long double, long double ) __LIBMLDBL_COMPAT(fmaxl);
extern long double fminl( long double, long double ) __LIBMLDBL_COMPAT(fminl);
extern long double fmal( long double, long double, long double ) __LIBMLDBL_COMPAT(fmal);
#endif /* __WANT_LONG_DOUBLE_FORMAT__ */

#define isgreater(x, y) __builtin_isgreater ((x),(y))
#define isgreaterequal(x, y) __builtin_isgreaterequal ((x),(y))
#define isless(x, y) __builtin_isless ((x),(y))
#define islessequal(x, y) __builtin_islessequal ((x),(y))
#define islessgreater(x, y) __builtin_islessgreater ((x),(y))
#define isunordered(x, y) __builtin_isunordered ((x),(y))

extern double  __inf( void );
extern float  __inff( void );
extern float  __nan( void ); /* 10.3 (and later) must retain in ABI for backward compatability */

#if !defined(_ANSI_SOURCE)
extern double j0 ( double );
extern double j1 ( double );
extern double jn ( int, double );

extern double y0 ( double );
extern double y1 ( double );
extern double yn ( int, double );

/* 
 * Scalb Travellers' advisory:
 * ---------------------------
 *
 * Reduction of Information advisory -- This advisory may constitute "too much information". Readers who are easily panicked 
 *                                      or confused may be needlessly panicked or confused by this advisory.
 *
 * We are in the process of retiring the legacy scalb.  IEEE-754 did not specify what the argument types should be
 * for scalb.  We guessed scalb(double, int) -- ints are faster to use here -- but our guess and what later standards
 * standard eventually settled on did not agree. To be compliant with these standards, our scalb needs to be scalb(double, double).
 * Unfortunately, we have a commitment to be binary compatible with old software compiled against scalb(double, int)
 * for older operating systems, so the old symbol _scalb must live on in perpetuity in the __ppc__ binary interface to service 
 * this need. To deal with this problem, we have introduced a new binary symbol _scalb$UNIX2003 and did some magic below
 * so that when you now compile against scalb() on a __ppc__ application, you get linked to _scalb$UNIX2003 instead. Thus, 
 * this constitutes a source level *** API CHANGE *** from  scalb( double, int ) to scalb( double, double) on __ppc__ only 
 * that your source will need to contend with if you compile with this header.  On __ppc__, all ints are exactly representable 
 * as doubles so from an arithmetic standpoint, this should cause no changes arithmetically from parameters of int type, but there 
 * remains the danger of triggering various compiler warnings that might balloon to more serious problems under -Werror. 
 *
 * On __ppc64__, __i386__ and future archs, scalb has always been scalb( double, double) and will continue to be so. Thus, this change
 * will make scalb on all platforms behave identically, with the same parameter types. The change will also eliminate GCC warnings about 
 * the math.h scalb declaration not matching the gcc4 builtin version. 
 *
 * The intent is that you will "never know" that a change occurred, and your code should "just do the right thing" without modification.
 * However, if you would like to sidestep any problems associated with this move, it is suggested that you use the C99 scalbn or scalbln 
 * or single/long double variants as appropriate instead. Their behavior and type is rigorously defined. There should be no hidden arithmetic 
 * "gotchas" if you simply replace all legacy calls to scalb with scalbn, since they essentially do the same thing.  If you 
 * decide to take this step, you should exercise due diligence to make sure that scalbn is present in the oldest version of 
 * MacOS X that you support. Otherwise, your application may fail to load on older systems. C99 support was introduced in MacOS X.3.9. 
 *
 * Use of the symbol _scalb$UNIX2003 should not in itself be construed to mean that scalb$UNIX2003 necessarily is UNIX 2003 compliant. 
 * UNIX is a registered trademark of The Open Group. 
 */
 
/* maps to _scalb$UNIX2003 on __ppc__ and _scalb elsewhere */
#if defined( __ppc__ )
    #if defined( __GNUC__ ) && ! defined( __XLC__ ) && ( ! defined( __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) || __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__-0 >= 1040 )
        extern double scalb ( double, double )  __asm("_scalb$UNIX2003" ); 
    #else
        extern double scalb ( double , int );  
    #endif
#else
	extern double scalb ( double, double );
#endif

#define M_E         2.71828182845904523536028747135266250   /* e */
#define M_LOG2E     1.44269504088896340735992468100189214   /* log_2(e) */
#define M_LOG10E    0.434294481903251827651128918916605082  /* log_10(e) */
#define M_LN2       0.693147180559945309417232121458176568  /* log_e(2) */
#define M_LN10      2.30258509299404568401799145468436421   /* log_e(10) */
#define M_PI        3.14159265358979323846264338327950288   /* pi */
#define M_PI_2      1.57079632679489661923132169163975144   /* pi/2 */
#define M_PI_4      0.785398163397448309615660845819875721  /* pi/4 */
#define M_1_PI      0.318309886183790671537767526745028724  /* 1/pi */
#define M_2_PI      0.636619772367581343075535053490057448  /* 2/pi */
#define M_2_SQRTPI  1.12837916709551257389615890312154517   /* 2/sqrt(pi) */
#define M_SQRT2     1.41421356237309504880168872420969808   /* sqrt(2) */
#define M_SQRT1_2   0.707106781186547524400844362104849039  /* 1/sqrt(2) */

#define	MAXFLOAT	((float)3.40282346638528860e+38)
extern int signgam;

#endif /* !defined(_ANSI_SOURCE) */

#if !defined(__NOEXTENSIONS__) && (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))
#define __WANT_EXTENSIONS__
#endif

#ifdef __WANT_EXTENSIONS__

#define FP_SNAN		FP_NAN
#define FP_QNAN		FP_NAN

extern long int rinttol ( double );

extern long int roundtol ( double );

typedef struct __complex_s {
        double Real;
        double Imag;
} __complex_t;

/*
 * XOPEN/SVID
 */
#if !defined(_ANSI_SOURCE) && (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))

#if (!defined(_XOPEN_SOURCE) || defined(_DARWIN_C_SOURCE))
enum fdversion {_fdlibm_ieee = -1, _fdlibm_svid, _fdlibm_xopen, _fdlibm_posix}; /* Legacy fdlibm constructs */
#define fdlibm_ieee _fdlibm_ieee
#define fdlibm_svid _fdlibm_svid
#define fdlibm_xopen _fdlibm_xopen
#define fdlibm_posix _fdlibm_posix

#define _LIB_VERSION_TYPE enum fdversion
#define _LIB_VERSION _fdlib_version  

/* if global variable _LIB_VERSION is not desirable, one may 
 * change the following to be a constant by: 
 *	#define _LIB_VERSION_TYPE const enum version
 * In that case, after one initializes the value _LIB_VERSION (see
 * s_lib_version.c) during compile time, it cannot be modified
 * in the middle of a program
 */ 
extern  _LIB_VERSION_TYPE  _LIB_VERSION;

#define _IEEE_  fdlibm_ieee
#define _SVID_  fdlibm_svid
#define _XOPEN_ fdlibm_xopen
#define _POSIX_ fdlibm_posix

#if !defined(__cplusplus)
struct exception {
	int type;
	char *name;
	double arg1;
	double arg2;
	double retval;
};
#endif

#define	HUGE		MAXFLOAT

/* 
 * set X_TLOSS = pi*2**52, which is possibly defined in <values.h>
 * (one may replace the following line by "#include <values.h>")
 */

#define X_TLOSS		1.41484755040568800000e+16 

#define	DOMAIN		1
#define	SING		2
#define	OVERFLOW	3
#define	UNDERFLOW	4
#define	TLOSS		5
#define	PLOSS		6

#endif /* (!_XOPEN_SOURCE || _DARWIN_C_SOURCE) */
#endif /* !_ANSI_SOURCE && (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */

#if !defined( __STRICT_ANSI__) && !defined(_ANSI_SOURCE) && (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))

extern int finite ( double );

extern double gamma ( double );

#if (!defined(_XOPEN_SOURCE) || defined(_DARWIN_C_SOURCE))

#if !defined(__cplusplus)
extern int matherr ( struct exception * );
#endif

/*
 * IEEE Test Vector
 */
extern double significand ( double );

/*
 * BSD math library entry points
 */
extern double drem ( double, double );

/*
 * Reentrant version of gamma & lgamma; passes signgam back by reference
 * as the second argument; user must allocate space for signgam.
 */
#ifdef _REENTRANT
extern double gamma_r ( double, int * );
extern double lgamma_r ( double, int * );
#endif /* _REENTRANT */
#endif /* (!_XOPEN_SOURCE || _DARWIN_C_SOURCE) */
#endif /* !_ANSI_SOURCE && (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */

#endif /* __WANT_EXTENSIONS__ */

/*
 * The following replacements for libm's floor, floorf, ceil, and ceilf are activated
 * when the flag "-ffast-math" is passed to the gcc compiler. These functions do not
 * distinguish between -0.0 and 0.0, so are not IEC6509 compliant for argument -0.0.
 */
#if defined(__FAST_MATH__) && !defined(__cplusplus)

#define __FSELS(e,t,f) (((e) >= 0.0f) ? (t) : (f))
#define __FSEL(e,t,f) (((e) >= 0.0) ? (t) : (f))

static __inline__ float __fastmath_floorf( float f ) __attribute__((__always_inline__));
static __inline__ float __fastmath_floorf( float f )
{
    float b, c, d, e, g, h, t;

    c = __FSELS( f, -8388608.f, 8388608.f );          b = fabsf( f ); 
    d = f - c;                                          e = b - 8388608.f;                        
	__asm__("" : "+f" (d));	/* Tell compiler value of d cannot be optimized away. */
	d = d + c;
    g = f - d;
    h = __FSELS( g, 0.0f, 1.0f );
    t = d - h;
    return __FSELS( e, f, t );
}

static __inline__ float __fastmath_ceilf( float f ) __attribute__((__always_inline__));
static __inline__ float __fastmath_ceilf( float f )
{
    float b, c, d, e, g, h, t;

    c = __FSELS( f, -8388608.f, 8388608.f );          b = fabsf( f ); 
    d = f - c;                                          e = b - 8388608.f;                        
	__asm__("" : "+f" (d));	/* Tell compiler value of d cannot be optimized away. */
	d = d + c;
    g = d - f;
    h = __FSELS( g, 0.0f, 1.0f );
    t = d + h;
    return __FSELS( e, f, t );
}

static __inline__ double __fastmath_floor( double f ) __attribute__((__always_inline__));
static __inline__ double __fastmath_floor( double f )
{
    double b, c, d, e, g, h, t;

    c = __FSEL( f, -4503599627370496., 4503599627370496. );             b = fabs( f );      
    d = f - c;                                          e = b - 4503599627370496.;                    
	__asm__("" : "+f" (d));	/* Tell compiler value of d cannot be optimized away. */
	d = d + c;
    g = f - d;
    h = __FSEL( g, 0.0, 1.0 );
    t = d - h;
    return __FSEL( e, f, t );
}

static __inline__ double __fastmath_ceil( double f ) __attribute__((__always_inline__));
static __inline__ double __fastmath_ceil( double f )
{
    double b, c, d, e, g, h, t;

    c = __FSEL( f, -4503599627370496., 4503599627370496. );             b = fabs( f );      
    d = f - c;                                          e = b - 4503599627370496.;                    
	__asm__("" : "+f" (d));	/* Tell compiler value of d cannot be optimized away. */
	d = d + c;
    g = d - f;
    h = __FSEL( g, 0.0, 1.0 );
    t = d + h;
    return __FSEL( e, f, t );
}

#define floorf(x) __fastmath_floorf((x))
#define ceilf(x) __fastmath_ceilf((x))
#define floor(x) __fastmath_floor((x))
#define ceil(x) __fastmath_ceil((x))

#endif /* __FAST_MATH__ && !__cplusplus */

#ifdef __cplusplus
}
#endif

#endif /* __MATH__ */
