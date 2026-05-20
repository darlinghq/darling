/*
 * arm64_builtins.c
 *
 * Provide basic math functions for ARM64 that are implemented as
 * Intel assembly on x86_64. These use compiler builtins which lower
 * to efficient ARM64 hardware FP instructions or libcalls.
 *
 * On ARM64 Darwin, long double == double (64-bit IEEE 754).
 */

#if defined(__aarch64__) || defined(__arm64__)

#include <math.h>
#include <stdint.h>
#include <float.h>

/* === Trigonometric functions === */

/* sinf/cosf/tanf: The FreeBSD-derived implementations reference
 * kernel functions. We provide simple wrappers using double precision
 * to compute single-precision results accurately. */

/* These are implemented in the FreeBSD sources linked elsewhere,
 * but if they're still missing at link time, the compiler will
 * synthesize calls to the C library. The alias file handles the
 * long double variants (which equal double on ARM64). */

/* === Scaling / Exponent functions === */

/* ldexp: multiply x by 2^exp */
double ldexp(double x, int exp) {
    return __builtin_ldexp(x, exp);
}

float ldexpf(float x, int exp) {
    return __builtin_ldexpf(x, exp);
}

/* scalbn/scalbnf are aliased to ldexp/ldexpf via the alias file */

/* frexp: extract mantissa and exponent */
double frexp(double x, int *exp) {
    return __builtin_frexp(x, exp);
}

float frexpf(float x, int *exp) {
    return __builtin_frexpf(x, exp);
}

/* === Basic float functions === */

float sinf(float x) {
    return (float)__builtin_sin((double)x);
}

float cosf(float x) {
    return (float)__builtin_cos((double)x);
}

float tanf(float x) {
    return (float)__builtin_tan((double)x);
}

double sin(double x) {
    return __builtin_sin(x);
}

double cos(double x) {
    return __builtin_cos(x);
}

double tan(double x) {
    return __builtin_tan(x);
}

float logf(float x) {
    return (float)__builtin_log((double)x);
}

float expf(float x) {
    return (float)__builtin_exp((double)x);
}

float log2f(float x) {
    return (float)__builtin_log2((double)x);
}

float log10f(float x) {
    return (float)__builtin_log10((double)x);
}

float exp2f(float x) {
    return (float)__builtin_exp2((double)x);
}

float expm1f(float x) {
    return (float)__builtin_expm1((double)x);
}

float log1pf(float x) {
    return (float)__builtin_log1p((double)x);
}

float atan2f(float y, float x) {
    return (float)__builtin_atan2((double)y, (double)x);
}

float atanf(float x) {
    return (float)__builtin_atan((double)x);
}

float acosf(float x) {
    return (float)__builtin_acos((double)x);
}

float asinf(float x) {
    return (float)__builtin_asin((double)x);
}

float asinhf(float x) {
    return (float)__builtin_asinh((double)x);
}

float sinhf(float x) {
    return (float)__builtin_sinh((double)x);
}

float coshf(float x) {
    return (float)__builtin_cosh((double)x);
}

float tanhf(float x) {
    return (float)__builtin_tanh((double)x);
}

float acoshf(float x) {
    return (float)__builtin_acosh((double)x);
}

float atanhf(float x) {
    return (float)__builtin_atanh((double)x);
}

float hypotf(float x, float y) {
    return (float)__builtin_hypot((double)x, (double)y);
}

double hypot(double x, double y) {
    return __builtin_hypot(x, y);
}

float powf(float x, float y) {
    return (float)__builtin_pow((double)x, (double)y);
}

double pow(double x, double y) {
    return __builtin_pow(x, y);
}

float sqrtf(float x) {
    return __builtin_sqrtf(x);
}

double sqrt(double x) {
    return __builtin_sqrt(x);
}

float cbrtf(float x) {
    return (float)__builtin_cbrt((double)x);
}

/* === Rounding functions === */

float ceilf(float x) {
    return __builtin_ceilf(x);
}

double ceil(double x) {
    return __builtin_ceil(x);
}

float floorf(float x) {
    return __builtin_floorf(x);
}

double floor(double x) {
    return __builtin_floor(x);
}

float truncf(float x) {
    return __builtin_truncf(x);
}

double trunc(double x) {
    return __builtin_trunc(x);
}

float roundf(float x) {
    return __builtin_roundf(x);
}

double round(double x) {
    return __builtin_round(x);
}

float rintf(float x) {
    return __builtin_rintf(x);
}

long lroundf(float x) {
    return __builtin_lroundf(x);
}

long lrintf(float x) {
    return __builtin_lrintf(x);
}

float nearbyintf(float x) {
    return __builtin_nearbyintf(x);
}

/* === Double-precision functions needed as alias bases === */
/* The alias file creates long double variants (_Xl) from these.
 * On ARM64 Darwin, long double == double. */

double atan(double x) {
    return __builtin_atan(x);
}

double atan2(double y, double x) {
    return __builtin_atan2(y, x);
}

double cbrt(double x) {
    return __builtin_cbrt(x);
}

double log(double x) {
    return __builtin_log(x);
}

double log2(double x) {
    return __builtin_log2(x);
}

double nextafter(double x, double y) {
    return __builtin_nextafter(x, y);
}

/* === Misc === */

double copysign(double x, double y) {
    return __builtin_copysign(x, y);
}

float fabsf(float x) {
    return __builtin_fabsf(x);
}

double fabs(double x) {
    return __builtin_fabs(x);
}

float fmaxf(float x, float y) {
    return __builtin_fmaxf(x, y);
}

float fminf(float x, float y) {
    return __builtin_fminf(x, y);
}

double fmax(double x, double y) {
    return __builtin_fmax(x, y);
}

double fmin(double x, double y) {
    return __builtin_fmin(x, y);
}

float modff(float x, float *iptr) {
    return __builtin_modff(x, iptr);
}

double modf(double x, double *iptr) {
    return __builtin_modf(x, iptr);
}

double fmod(double x, double y) {
    return __builtin_fmod(x, y);
}

float nextafterf(float x, float y) {
    return __builtin_nextafterf(x, y);
}

/* === Integer rounding functions === */

long lrint(double x) {
    return __builtin_lrint(x);
}

long lround(double x) {
    return __builtin_lround(x);
}

long long llrint(double x) {
    return __builtin_llrint(x);
}

long long llround(double x) {
    return __builtin_llround(x);
}

long long llrintf(float x) {
    return __builtin_llrintf(x);
}

long long llroundf(float x) {
    return __builtin_llroundf(x);
}

double rint(double x) {
    return __builtin_rint(x);
}

/* nearbyint/nearbyintl are aliased to rint via the alias file */

/* === Complex multiplication builtins === */
/* These are normally provided by compiler_rt but we need them here
 * to break the circular dependency between libsystem_m and libcompiler_rt.
 * Implements C99 Annex G semantics for complex multiply with NaN/Inf handling. */

double _Complex __muldc3(double __a, double __b, double __c, double __d) {
    double __ac = __a * __c;
    double __bd = __b * __d;
    double __ad = __a * __d;
    double __bc = __b * __c;
    double _Complex z;
    __real__ z = __ac - __bd;
    __imag__ z = __ad + __bc;
    if (__builtin_isnan(__real__ z) && __builtin_isnan(__imag__ z)) {
        int __recalc = 0;
        if (__builtin_isinf(__a) || __builtin_isinf(__b)) {
            double a2 = __builtin_copysign(__builtin_isinf(__a) ? 1.0 : 0.0, __a);
            double b2 = __builtin_copysign(__builtin_isinf(__b) ? 1.0 : 0.0, __b);
            if (__builtin_isnan(__c)) __c = __builtin_copysign(0.0, __c);
            if (__builtin_isnan(__d)) __d = __builtin_copysign(0.0, __d);
            __recalc = 1;
            __a = a2; __b = b2;
        }
        if (__builtin_isinf(__c) || __builtin_isinf(__d)) {
            double c2 = __builtin_copysign(__builtin_isinf(__c) ? 1.0 : 0.0, __c);
            double d2 = __builtin_copysign(__builtin_isinf(__d) ? 1.0 : 0.0, __d);
            if (__builtin_isnan(__a)) __a = __builtin_copysign(0.0, __a);
            if (__builtin_isnan(__b)) __b = __builtin_copysign(0.0, __b);
            __recalc = 1;
            __c = c2; __d = d2;
        }
        if (!__recalc && (__builtin_isinf(__ac) || __builtin_isinf(__bd) ||
                          __builtin_isinf(__ad) || __builtin_isinf(__bc))) {
            if (__builtin_isnan(__a)) __a = __builtin_copysign(0.0, __a);
            if (__builtin_isnan(__b)) __b = __builtin_copysign(0.0, __b);
            if (__builtin_isnan(__c)) __c = __builtin_copysign(0.0, __c);
            if (__builtin_isnan(__d)) __d = __builtin_copysign(0.0, __d);
            __recalc = 1;
        }
        if (__recalc) {
            __real__ z = __builtin_huge_val() * (__a * __c - __b * __d);
            __imag__ z = __builtin_huge_val() * (__a * __d + __b * __c);
        }
    }
    return z;
}

float _Complex __mulsc3(float __a, float __b, float __c, float __d) {
    float __ac = __a * __c;
    float __bd = __b * __d;
    float __ad = __a * __d;
    float __bc = __b * __c;
    float _Complex z;
    __real__ z = __ac - __bd;
    __imag__ z = __ad + __bc;
    if (__builtin_isnan(__real__ z) && __builtin_isnan(__imag__ z)) {
        int __recalc = 0;
        if (__builtin_isinf(__a) || __builtin_isinf(__b)) {
            float a2 = __builtin_copysignf(__builtin_isinf(__a) ? 1.0f : 0.0f, __a);
            float b2 = __builtin_copysignf(__builtin_isinf(__b) ? 1.0f : 0.0f, __b);
            if (__builtin_isnan(__c)) __c = __builtin_copysignf(0.0f, __c);
            if (__builtin_isnan(__d)) __d = __builtin_copysignf(0.0f, __d);
            __recalc = 1;
            __a = a2; __b = b2;
        }
        if (__builtin_isinf(__c) || __builtin_isinf(__d)) {
            float c2 = __builtin_copysignf(__builtin_isinf(__c) ? 1.0f : 0.0f, __c);
            float d2 = __builtin_copysignf(__builtin_isinf(__d) ? 1.0f : 0.0f, __d);
            if (__builtin_isnan(__a)) __a = __builtin_copysignf(0.0f, __a);
            if (__builtin_isnan(__b)) __b = __builtin_copysignf(0.0f, __b);
            __recalc = 1;
            __c = c2; __d = d2;
        }
        if (!__recalc && (__builtin_isinf(__ac) || __builtin_isinf(__bd) ||
                          __builtin_isinf(__ad) || __builtin_isinf(__bc))) {
            if (__builtin_isnan(__a)) __a = __builtin_copysignf(0.0f, __a);
            if (__builtin_isnan(__b)) __b = __builtin_copysignf(0.0f, __b);
            if (__builtin_isnan(__c)) __c = __builtin_copysignf(0.0f, __c);
            if (__builtin_isnan(__d)) __d = __builtin_copysignf(0.0f, __d);
            __recalc = 1;
        }
        if (__recalc) {
            __real__ z = __builtin_huge_valf() * (__a * __c - __b * __d);
            __imag__ z = __builtin_huge_valf() * (__a * __d + __b * __c);
        }
    }
    return z;
}

#endif /* __aarch64__ || __arm64__ */
