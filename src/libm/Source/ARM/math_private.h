/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * from: @(#)fdlibm.h 5.1 93/09/24
 * $FreeBSD: src/lib/msun/src/math_private.h,v 1.17 2005/02/04 20:05:39 das Exp $
 */

#ifndef _MATH_PRIVATE_H_
#define	_MATH_PRIVATE_H_

#include <sys/types.h>
#include <machine/endian.h>

/*
 * The original fdlibm code used statements like:
 *	n0 = ((*(int*)&one)>>29)^1;		* index of high word *
 *	ix0 = *(n0+(int*)&x);			* high word of x *
 *	ix1 = *((1-n0)+(int*)&x);		* low word of x *
 * to dig two 32 bit words out of the 64 bit IEEE floating point
 * value.  That is non-ANSI, and, moreover, the gcc instruction
 * scheduler gets it wrong.  We instead use the following macros.
 * Unlike the original code, we determine the endianness at compile
 * time, not at run time; I don't see much benefit to selecting
 * endianness at run time.
 */

/*
 * A union which permits us to convert between a double and two 32 bit
 * ints.
 */

#if BYTE_ORDER == BIG_ENDIAN

typedef union
{
  double value;
  struct
  {
    u_int32_t msw;
    u_int32_t lsw;
  } parts;
} ieee_double_shape_type;

#endif

#if BYTE_ORDER == LITTLE_ENDIAN

typedef union
{
  double value;
  struct
  {
    u_int32_t lsw;
    u_int32_t msw;
  } parts;
} ieee_double_shape_type;

#endif

/* Get two 32 bit ints from a double.  */

#define EXTRACT_WORDS(ix0,ix1,d)				\
do {								\
  ieee_double_shape_type ew_u;					\
  ew_u.value = (d);						\
  (ix0) = ew_u.parts.msw;					\
  (ix1) = ew_u.parts.lsw;					\
} while (0)

/* Get the more significant 32 bit int from a double.  */

#define GET_HIGH_WORD(i,d)					\
do {								\
  ieee_double_shape_type gh_u;					\
  gh_u.value = (d);						\
  (i) = gh_u.parts.msw;						\
} while (0)

/* Get the less significant 32 bit int from a double.  */

#define GET_LOW_WORD(i,d)					\
do {								\
  ieee_double_shape_type gl_u;					\
  gl_u.value = (d);						\
  (i) = gl_u.parts.lsw;						\
} while (0)

/* Set a double from two 32 bit ints.  */

#define INSERT_WORDS(d,ix0,ix1)					\
do {								\
  ieee_double_shape_type iw_u;					\
  iw_u.parts.msw = (ix0);					\
  iw_u.parts.lsw = (ix1);					\
  (d) = iw_u.value;						\
} while (0)

/* Set the more significant 32 bits of a double from an int.  */

#define SET_HIGH_WORD(d,v)					\
do {								\
  ieee_double_shape_type sh_u;					\
  sh_u.value = (d);						\
  sh_u.parts.msw = (v);						\
  (d) = sh_u.value;						\
} while (0)

/* Set the less significant 32 bits of a double from an int.  */

#define SET_LOW_WORD(d,v)					\
do {								\
  ieee_double_shape_type sl_u;					\
  sl_u.value = (d);						\
  sl_u.parts.lsw = (v);						\
  (d) = sl_u.value;						\
} while (0)

/*
 * A union which permits us to convert between a float and a 32 bit
 * int.
 */

typedef union
{
  float value;
  /* FIXME: Assumes 32 bit int.  */
  unsigned int word;
} ieee_float_shape_type;

/* Get a 32 bit int from a float.  */

#define GET_FLOAT_WORD(i,d)					\
do {								\
  ieee_float_shape_type gf_u;					\
  gf_u.value = (d);						\
  (i) = gf_u.word;						\
} while (0)

/* Set a float from a 32 bit int.  */

#define SET_FLOAT_WORD(d,i)					\
do {								\
  ieee_float_shape_type sf_u;					\
  sf_u.word = (i);						\
  (d) = sf_u.value;						\
} while (0)

/*
 * ieee style elementary functions
 *
 * We rename functions here to improve other sources' diffability
 * against fdlibm.
 */
#define	__ieee754_sqrt	sqrt
#define	__ieee754_acos	acos
#define	__ieee754_acosh	acosh
#define	__ieee754_log	log
#define	__ieee754_atanh	atanh
#define	__ieee754_asin	asin
#define	__ieee754_atan2	atan2
#define	__ieee754_exp	exp
#define	__ieee754_cosh	cosh
#define	__ieee754_fmod	fmod
#define	__ieee754_pow	pow
#define	__ieee754_lgamma lgamma
#define	__ieee754_gamma	gamma
#define	__ieee754_lgamma_r lgamma_r
#define	__ieee754_gamma_r gamma_r
#define	__ieee754_log10	log10
#define	__ieee754_sinh	sinh
#define	__ieee754_hypot	hypot
#define	__ieee754_j0	j0
#define	__ieee754_j1	j1
#define	__ieee754_y0	y0
#define	__ieee754_y1	y1
#define	__ieee754_jn	jn
#define	__ieee754_yn	yn
#define	__ieee754_remainder remainder
#define	__ieee754_scalb	scalb
#define	__ieee754_sqrtf	sqrtf
#define	__ieee754_acosf	acosf
#define	__ieee754_acoshf acoshf
#define	__ieee754_logf	logf
#define	__ieee754_atanhf atanhf
#define	__ieee754_asinf	asinf
#define	__ieee754_atan2f atan2f
#define	__ieee754_expf	expf
#define	__ieee754_coshf	coshf
#define	__ieee754_fmodf	fmodf
#define	__ieee754_powf	powf
#define	__ieee754_lgammaf lgammaf
#define	__ieee754_gammaf gammaf
#define	__ieee754_lgammaf_r lgammaf_r
#define	__ieee754_gammaf_r gammaf_r
#define	__ieee754_log10f log10f
#define	__ieee754_sinhf	sinhf
#define	__ieee754_hypotf hypotf
#define	__ieee754_j0f	j0f
#define	__ieee754_j1f	j1f
#define	__ieee754_y0f	y0f
#define	__ieee754_y1f	y1f
#define	__ieee754_jnf	jnf
#define	__ieee754_ynf	ynf
#define	__ieee754_remainderf remainderf
#define	__ieee754_scalbf scalbf

/*
 * The openlibm-ported sources (Source/ARM/*_freeBSD.c) use STRICT_ASSIGN to
 * force C99 evaluation semantics for intermediate floating-point assignments
 * on platforms where FLT_EVAL_METHOD > 0. On ARM/ARM64 FLT_EVAL_METHOD is 0
 * (no excess precision), so a plain assignment is sufficient.
 */
#ifndef STRICT_ASSIGN
#define	STRICT_ASSIGN(type, lval, rval)	((lval) = (rval))
#endif

/* Helper used by the openlibm e_pow / e_powf to propagate quiet NaN payloads
 * while making the float-flavour exception path agree with double. */
#ifndef nan_mix
#define	nan_mix(x, y)	(((x) + 0.0L) + ((y) + 0))
#endif

/*
 * Efficient round-to-int-nearest, for use inside argument-reduction kernels
 * (e_rem_pio2 / e_rem_pio2f). openlibm provides amd64 (`cvtsd2si`) and
 * i386 (`fistl`) versions in src/math_private.h but nothing for aarch64.
 *
 * On ARM64 the single-instruction equivalent is FCVTNS — "convert FP to
 * signed integer, rounding to nearest, ties to even" — which matches the
 * semantics x86 needs (round-to-nearest), so we get the same fast path.
 */
#if defined(__aarch64__) || defined(__arm64__)
static __inline int
irint(double x)
{
	int n;
	__asm__ ("fcvtns %w0, %d1" : "=r"(n) : "w"(x));
	return n;
}
#define HAVE_EFFICIENT_IRINT
#endif

/* fdlibm / openlibm kernel-function prototypes. These are referenced from
 * the ported Source/ARM/*_freeBSD.c implementations of sin/cos/tan and the
 * argument-reduction kernels. Declarations live here rather than as private
 * extern at each call site so clang doesn't trip its implicit-function-decl
 * hard-error on -O2 builds. Function names match what the Darling-side
 * freeBSD sources actually emit (the openlibm `__ieee754_rem_pio2`
 * gets renamed to the prefix-less `rem_pio2` during our port). */
#include <stdint.h>
extern int	  __kernel_rem_pio2(double*, double*, int, int, int, const int32_t*);
extern int32_t	rem_pio2(double, double*);
extern int32_t	rem_pio2f(float, double*);
extern double	__kernel_sin(double, double, int);
extern double	__kernel_cos(double, double);
extern double	__kernel_tan(double, double, int);
extern float	__kernel_sindf(double);
extern float	__kernel_cosdf(double);
extern float	__kernel_tandf(double, int);
extern double	__ldexp_exp(double, int);
extern float	__ldexp_expf(float, int);

#endif /* !_MATH_PRIVATE_H_ */
