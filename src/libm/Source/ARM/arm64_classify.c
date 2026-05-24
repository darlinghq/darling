/*
 * arm64_classify.c
 *
 * Out-of-line backings for the `<math.h>` classification family:
 *   __fpclassify  __isfinite  __isinf  __isnan  __isnormal  __signbit
 * and their explicit-type variants (`Xd` for double, `Xf` for float,
 * `Xl` for long double — which is identical to double on arm64 Darwin and
 * is mapped via the link-time alias list).
 *
 * Apple's `<math.h>` is written so that compilers with `__builtin_isnan`,
 * `__builtin_fpclassify`, etc. inline these checks at every call site — but
 * when user code uses a function pointer, includes math.h from a non-C
 * source, or otherwise prevents the inline expansion, the macro falls back
 * to one of these `___XYZ` externs. Without them the libm fails to link.
 *
 * Signatures must match the prototypes Apple's SDK math.h declares
 * (architecture/arm/math.h lines 128..241): the non-suffixed forms accept
 * `long double`, the `Xd`/`Xf` forms accept double/float. On arm64 Darwin
 * `long double` is the same as `double` so the `Xl`/non-suffixed forms are
 * effectively double-precision but their declared type must match the
 * header to avoid -Werror=incompatible-pointer-types build failures.
 *
 * Each function is one line of clang `__builtin_*`, which lowers to:
 *   isnan/isinf/isfinite/signbit  → 2-3 ARM64 instructions (fcmp + cset / and)
 *   isnormal                      → ~6 instructions (range test on biased exp)
 *   fpclassify                    → inline 5-way branch, no libcall
 *
 * Verified at clang -O2 -arch arm64 — none of these emits a libcall.
 */

#if defined(__aarch64__) || defined(__arm64__)

#include <math.h>

/* fpclassify — return FP_NAN, FP_INFINITE, FP_NORMAL, FP_SUBNORMAL, FP_ZERO
 * using Apple's macro values from <math.h> so the result is consistent with
 * the inline expansion in user code. */
int __fpclassify(long double x)
{
    return __builtin_fpclassify(FP_NAN, FP_INFINITE, FP_NORMAL,
                                FP_SUBNORMAL, FP_ZERO, x);
}

int __fpclassifyd(double x)
{
    return __builtin_fpclassify(FP_NAN, FP_INFINITE, FP_NORMAL,
                                FP_SUBNORMAL, FP_ZERO, x);
}

int __fpclassifyf(float x)
{
    return __builtin_fpclassify(FP_NAN, FP_INFINITE, FP_NORMAL,
                                FP_SUBNORMAL, FP_ZERO, x);
}

/* isfinite — true if x is neither NaN nor ±Inf. */
int __isfinite(long double x) { return __builtin_isfinite(x); }
int __isfinited(double x)     { return __builtin_isfinite(x); }
int __isfinitef(float x)      { return __builtin_isfinite(x); }

/* isinf — true if x is +Inf or -Inf. */
int __isinf(long double x)    { return __builtin_isinf(x); }
int __isinfd(double x)        { return __builtin_isinf(x); }
int __isinff(float x)         { return __builtin_isinf(x); }

/* isnan — true if x is NaN. */
int __isnan(long double x)    { return __builtin_isnan(x); }
int __isnand(double x)        { return __builtin_isnan(x); }
int __isnanf(float x)         { return __builtin_isnan(x); }

/* isnormal — true if x is a normalized finite value (not zero/sub/inf/NaN). */
int __isnormal(long double x) { return __builtin_isnormal(x); }
int __isnormald(double x)     { return __builtin_isnormal(x); }
int __isnormalf(float x)      { return __builtin_isnormal(x); }

/* signbit — true if the sign bit of x is set (handles -0 and -NaN).
 * The Apple SDK declares __signbitl(long double) — on arm64 long double is
 * the same as double, so __signbitl shares the implementation with
 * __signbitd via a link-time alias (see libm_arm_vfp.a.alias). */
int __signbitd(double x)      { return __builtin_signbit(x); }
int __signbitf(float x)       { return __builtin_signbit(x); }

#endif /* __aarch64__ || __arm64__ */
