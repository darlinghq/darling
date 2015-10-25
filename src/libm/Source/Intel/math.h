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

#include <sys/cdefs.h> /* For definition of __DARWIN_UNIX03 et al */

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


#if defined(__GNUC__)
	#define	HUGE_VAL	__builtin_huge_val()
	#define	HUGE_VALF	__builtin_huge_valf()
	#define	HUGE_VALL	__builtin_huge_vall()
    #define NAN         __builtin_nanf("0x7fc00000") /* Constant expression, can be used as initializer. */
    #define __MATH_H_ALWAYS_INLINE__		__attribute__ ((always_inline))
#else
    #define	HUGE_VAL	1e500
    #define	HUGE_VALF	1e50f
    #define	HUGE_VALL	1e5000L
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
#define FP_SUPERNORMAL  6 /* meaningful only on PowerPC */

/* fma() *function call* is more costly than equivalent (in-line) multiply and add operations    */
/* For single and double precision, the cost isn't too bad, because we can fall back on higher   */
/* precision hardware, with the necessary range to handle infinite precision products. However,  */
/* expect the long double fma to be at least an order of magnitude slower than a simple multiply */
/* and an add.                                                                                   */
#undef FP_FAST_FMA
#undef FP_FAST_FMAF
#undef FP_FAST_FMAL

/* The values returned by `ilogb' for 0 and NaN respectively. */
#define FP_ILOGB0	(-2147483647 - 1)
#define FP_ILOGBNAN	(-2147483647 - 1)

/* Bitmasks for the math_errhandling macro.  */
#define MATH_ERRNO	1	/* errno set by math functions.  */
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

#define fpclassify(x)	\
	(	sizeof (x) == sizeof(float )	?	__fpclassifyf((float)(x))	\
	:	sizeof (x) == sizeof(double)	?	__fpclassifyd((double)(x))	\
										:	__fpclassify ((long double)(x)))

extern int __fpclassifyf(float      );
extern int __fpclassifyd(double     );
extern int __fpclassify (long double);

#if defined( __GNUC__ ) && 0 == __FINITE_MATH_ONLY__ 
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

	static __inline__  int __inline_isfinitef	(float      ) __MATH_H_ALWAYS_INLINE__;
	static __inline__  int __inline_isfinited	(double     ) __MATH_H_ALWAYS_INLINE__;
	static __inline__  int __inline_isfinite	(long double) __MATH_H_ALWAYS_INLINE__;
	static __inline__  int __inline_isinff		(float      ) __MATH_H_ALWAYS_INLINE__;
	static __inline__  int __inline_isinfd		(double     ) __MATH_H_ALWAYS_INLINE__;
	static __inline__  int __inline_isinf		(long double) __MATH_H_ALWAYS_INLINE__;
	static __inline__  int __inline_isnanf		(float      ) __MATH_H_ALWAYS_INLINE__;
	static __inline__  int __inline_isnand		(double     ) __MATH_H_ALWAYS_INLINE__;
	static __inline__  int __inline_isnan		(long double) __MATH_H_ALWAYS_INLINE__;
	static __inline__  int __inline_isnormalf    (float      ) __MATH_H_ALWAYS_INLINE__;
	static __inline__  int __inline_isnormald    (double     ) __MATH_H_ALWAYS_INLINE__;
	static __inline__  int __inline_isnormal     (long double) __MATH_H_ALWAYS_INLINE__;
	static __inline__  int __inline_signbitf     (float      ) __MATH_H_ALWAYS_INLINE__;
	static __inline__  int __inline_signbitd     (double     ) __MATH_H_ALWAYS_INLINE__;
	static __inline__  int __inline_signbit      (long double) __MATH_H_ALWAYS_INLINE__;
	
	static __inline__  int __inline_isinff( float __x ) { return __builtin_fabsf(__x) == __builtin_inff(); }
	static __inline__  int __inline_isinfd( double __x ) { return __builtin_fabs(__x) == __builtin_inf(); }
	static __inline__  int __inline_isinf( long double __x ) { return __builtin_fabsl(__x) == __builtin_infl(); }
	static __inline__  int __inline_isfinitef( float __x ) { return __x == __x && __builtin_fabsf(__x) != __builtin_inff(); }
	static __inline__  int __inline_isfinited( double __x ) { return __x == __x && __builtin_fabs(__x) != __builtin_inf(); }
	static __inline__  int __inline_isfinite( long double __x ) { return __x == __x && __builtin_fabsl(__x) != __builtin_infl(); }
	static __inline__  int __inline_isnanf( float __x ) { return __x != __x; }
	static __inline__  int __inline_isnand( double __x ) { return __x != __x; }
	static __inline__  int __inline_isnan( long double __x ) { return __x != __x; }
	static __inline__  int __inline_signbitf( float __x ) { union{ float __f; unsigned int __u; }__u; __u.__f = __x; return (int)(__u.__u >> 31); }
	static __inline__  int __inline_signbitd( double __x ) { union{ double __f; unsigned int __u[2]; }__u; __u.__f = __x; return (int)(__u.__u[1] >> 31); }
	static __inline__  int __inline_signbit( long double __x ){ union{ long double __ld; struct{ unsigned int __m[2]; short __sexp; }__p; }__u; __u.__ld = __x; return (int) (((unsigned short) __u.__p.__sexp) >> 15); } 
	static __inline__  int __inline_isnormalf( float __x ) { float fabsf = __builtin_fabsf(__x); if( __x != __x ) return 0; return fabsf < __builtin_inff() && fabsf >= __FLT_MIN__; }  
	static __inline__  int __inline_isnormald( double __x ) { double fabsf = __builtin_fabs(__x); if( __x != __x ) return 0; return fabsf < __builtin_inf() && fabsf >= __DBL_MIN__; }  
	static __inline__  int __inline_isnormal( long double __x ) { long double fabsf = __builtin_fabsl(__x); if( __x != __x ) return 0; return fabsf < __builtin_infl() && fabsf >= __LDBL_MIN__; }  
	
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



/********************************************************************************
*                                                                               *
*                              Math Functions                                   *
*                                                                               *
********************************************************************************/

extern double  acos( double );
extern float  acosf( float );

extern double  asin( double );
extern float  asinf( float );

extern double  atan( double );
extern float  atanf( float );

extern double  atan2( double, double );
extern float  atan2f( float, float );

extern double  cos( double );
extern float  cosf( float );

extern double  sin( double );
extern float  sinf( float );

extern double  tan( double );
extern float  tanf( float );

extern double  acosh( double );
extern float  acoshf( float );

extern double  asinh( double );
extern float  asinhf( float );

extern double  atanh( double );
extern float  atanhf( float );

extern double  cosh( double );
extern float  coshf( float );

extern double  sinh( double );
extern float  sinhf( float );

extern double  tanh( double );
extern float  tanhf( float );

extern double exp ( double );
extern float expf ( float );

extern double exp2 ( double ); 
extern float exp2f ( float );

extern double expm1 ( double ); 
extern float expm1f ( float );

extern double log ( double );
extern float logf ( float );

extern double log10 ( double );
extern float log10f ( float );

extern double log2 ( double );
extern float log2f ( float );

extern double log1p ( double );
extern float log1pf ( float );

extern double logb ( double );
extern float logbf ( float );

extern double modf ( double, double * );
extern float modff ( float, float * );

extern double ldexp ( double, int );
extern float ldexpf ( float, int );

extern double frexp ( double, int * );
extern float frexpf ( float, int * );

extern int ilogb ( double );
extern int ilogbf ( float );

extern double scalbn ( double, int );
extern float scalbnf ( float, int );

extern double scalbln ( double, long int );
extern float scalblnf ( float, long int );

extern double  fabs( double );
extern float  fabsf( float );

extern double  cbrt( double );
extern float  cbrtf( float );

extern double hypot ( double, double );
extern float hypotf ( float, float );

extern double pow ( double, double );
extern float powf ( float, float );

extern double  sqrt( double );
extern float  sqrtf( float );

extern double  erf( double );
extern float  erff( float );

extern double  erfc( double );
extern float  erfcf( float );

/*	lgamma and lgammaf are not thread-safe.  The thread-safe variants
 *	lgamma_r and lgammaf_r are available on OS X 10.6 and later.
 *
 *	To use the thread-safe variants, you must define the _REENTRANT symbol.
 */
extern double  lgamma( double );
extern float  lgammaf( float );

extern double  tgamma( double );
extern float  tgammaf( float );

extern double ceil ( double );
extern float ceilf ( float );

extern double floor ( double );
extern float floorf ( float );

extern double nearbyint ( double );
extern float nearbyintf ( float );

extern double rint ( double );
extern float rintf ( float );

extern long int lrint ( double );
extern long int lrintf ( float );

extern double round ( double );
extern float roundf ( float );

extern long int lround ( double );
extern long int lroundf ( float );
    
#if !(__DARWIN_NO_LONG_LONG)
    /* long long is not part of C90. Make sure you are passing -std=c99 or -std=gnu99 or better if you need this. */
    extern long long int llrint ( double );
    extern long long int llrintf ( float );
    extern long long int llround ( double );
    extern long long int llroundf ( float );
#endif /* !(__DARWIN_NO_LONG_LONG) */

extern double trunc ( double );
extern float truncf ( float );

extern double fmod ( double, double );
extern float fmodf ( float, float );

extern double remainder ( double, double );
extern float remainderf ( float, float );

extern double remquo ( double, double, int * );
extern float remquof ( float, float, int * );

extern double copysign ( double, double );
extern float copysignf ( float, float );

extern double nan( const char * );
extern float nanf( const char * );

extern double nextafter ( double, double );
extern float nextafterf ( float, float );

extern double fdim ( double, double );
extern float fdimf ( float, float );

extern double fmax ( double, double );
extern float fmaxf ( float, float );

extern double fmin ( double, double );
extern float fminf ( float, float );

extern double fma ( double, double, double );
extern float fmaf ( float, float, float );

extern long double acosl(long double);
extern long double asinl(long double);
extern long double atanl(long double);
extern long double atan2l(long double, long double);
extern long double cosl(long double);
extern long double sinl(long double);
extern long double tanl(long double);
extern long double acoshl(long double);
extern long double asinhl(long double);
extern long double atanhl(long double);
extern long double coshl(long double);
extern long double sinhl(long double);
extern long double tanhl(long double);
extern long double expl(long double);
extern long double exp2l(long double);
extern long double expm1l(long double);
extern long double logl(long double);
extern long double log10l(long double);
extern long double log2l(long double);
extern long double log1pl(long double);
extern long double logbl(long double);
extern long double modfl(long double, long double *);
extern long double ldexpl(long double, int);
extern long double frexpl(long double, int *);
extern int ilogbl(long double);
extern long double scalbnl(long double, int);
extern long double scalblnl(long double, long int);
extern long double fabsl(long double);
extern long double cbrtl(long double);
extern long double hypotl(long double, long double);
extern long double powl(long double, long double);
extern long double sqrtl(long double);
extern long double erfl(long double);
extern long double erfcl(long double);
	
/*	lgammal is not thread-safe.
 *	The thread-safe variant lgammal_r is available on OS X 10.6 and later.
 *
 *	To use the thread-safe variant, you must define the _REENTRANT symbol.
 */
extern long double lgammal(long double);
	
extern long double tgammal(long double);
extern long double ceill(long double);
extern long double floorl(long double);
extern long double nearbyintl(long double);
extern long double rintl(long double);
extern long int lrintl(long double);
extern long double roundl(long double);
extern long int lroundl(long double);

#if !(__DARWIN_NO_LONG_LONG)
    /* long long is not part of C90. Make sure you are passing -std=c99 or -std=gnu99 or better if you need this. */
    extern long long int llrintl(long double);
    extern long long int llroundl(long double);
#endif /* !(__DARWIN_NO_LONG_LONG) */

extern long double truncl(long double);
extern long double fmodl(long double, long double);
extern long double remainderl(long double, long double);
extern long double remquol(long double, long double, int *);
extern long double copysignl(long double, long double);
extern long double nanl(const char *);
extern long double nextafterl(long double, long double);
extern double nexttoward(double, long double);
extern float nexttowardf(float, long double);
extern long double nexttowardl(long double, long double);
extern long double fdiml(long double, long double);
extern long double fmaxl(long double, long double);
extern long double fminl(long double, long double);
extern long double fmal(long double, long double, long double);

#define isgreater(x, y) __builtin_isgreater ((x),(y))
#define isgreaterequal(x, y) __builtin_isgreaterequal ((x),(y))
#define isless(x, y) __builtin_isless ((x),(y))
#define islessequal(x, y) __builtin_islessequal ((x),(y))
#define islessgreater(x, y) __builtin_islessgreater ((x),(y))
#define isunordered(x, y) __builtin_isunordered ((x),(y))

extern double  		__inf( void );
extern float  		__inff( void );
extern long double  __infl( void );
extern float  		__nan( void ); /* 10.3 (and later) must retain in ABI for backward compatability */

#if !defined(_ANSI_SOURCE)
extern double j0 ( double );

extern double j1 ( double );

extern double jn ( int, double );

extern double y0 ( double );

extern double y1 ( double );

extern double yn ( int, double );

extern double scalb ( double, double ); 


#define M_E         2.71828182845904523536028747135266250   /* e */
#define M_LOG2E     1.44269504088896340735992468100189214   /* log 2e */
#define M_LOG10E    0.434294481903251827651128918916605082  /* log 10e */
#define M_LN2       0.693147180559945309417232121458176568  /* log e2 */
#define M_LN10      2.30258509299404568401799145468436421   /* log e10 */
#define M_PI        3.14159265358979323846264338327950288   /* pi */
#define M_PI_2      1.57079632679489661923132169163975144   /* pi/2 */
#define M_PI_4      0.785398163397448309615660845819875721  /* pi/4 */
#define M_1_PI      0.318309886183790671537767526745028724  /* 1/pi */
#define M_2_PI      0.636619772367581343075535053490057448  /* 2/pi */
#define M_2_SQRTPI  1.12837916709551257389615890312154517   /* 2/sqrt(pi) */
#define M_SQRT2     1.41421356237309504880168872420969808   /* sqrt(2) */
#define M_SQRT1_2   0.707106781186547524400844362104849039  /* 1/sqrt(2) */

#define	MAXFLOAT	((float)3.40282346638528860e+38)
extern int signgam;     /* required for unix 2003 */


#endif /* !defined(_ANSI_SOURCE) */

#if !defined(__NOEXTENSIONS__) && (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))
#define __WANT_EXTENSIONS__
#endif

#ifdef __WANT_EXTENSIONS__

#define FP_SNAN		FP_NAN
#define FP_QNAN		FP_NAN

/* Legacy API: please use C99 lrint() instead. */
extern long int rinttol ( double );

/* Legacy API: please use C99 lround() instead. */
extern long int roundtol ( double );

/*
 * XOPEN/SVID
 */
#if !defined(_ANSI_SOURCE) && (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))
#if (!defined(_XOPEN_SOURCE) || defined(_DARWIN_C_SOURCE))
#if !defined(__cplusplus)
/* used by matherr below */
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
    
/* Legacy API: please use C99 isfinite() instead. */
extern int finite ( double );
    
/* Legacy API: please use C99 tgamma() instead. */
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

/* Legacy API: please use C99 remainder() instead. */
extern double drem ( double, double );
	
/*
 * Reentrant version of lgamma; passes signgam back by reference
 * as the second argument; user must allocate space for signgam.
 */
	
#ifdef _REENTRANT
#include <AvailabilityMacros.h>
	// Available on OS X 10.6 and later.
	extern float  lgammaf_r ( float, int * ) AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER;
	extern double lgamma_r ( double, int * ) AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER;
	extern long double lgammal_r ( long double, int * ) AVAILABLE_MAC_OS_X_VERSION_10_6_AND_LATER;
#endif /* _REENTRANT */
	
#endif /* (!_XOPEN_SOURCE || _DARWIN_C_SOURCE) */
#endif /* !_ANSI_SOURCE && (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */

#endif /* __WANT_EXTENSIONS__ */

#ifdef __cplusplus
}
#endif

#endif /* __MATH__ */
