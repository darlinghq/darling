/*
 * arm64_builtins.c
 *
 * Thin wrappers for libm symbols whose clang `__builtin_*` lowers to ARM64
 * hardware FP instructions — empirically verified at -O2 with Apple clang 21,
 * Darling's minimum supported compiler floor:
 *
 *   sqrt[f]      → FSQRT          fabs[f]     → FABS
 *   ceil[f]      → FRINTP         floor[f]    → FRINTM
 *   trunc[f]     → FRINTZ         rint[f]     → FRINTX
 *   round[f]     → FRINTA         fmin[f]     → FMINNM
 *   fmax[f]      → FMAXNM         fma[f]      → FMADD
 *   lround[f]    → FCVTAS         llround[f]  → FCVTAS
 *   lrint[f]     → FRINTX + FCVTZS            (2 insns)
 *   llrint[f]    → FRINTX + FCVTZS            (2 insns)
 *   copysign[f]  → MVNI + BIF                 (3-4 SIMD insns; ARM64 has no
 *                                              single FCOPYSIGN, but the
 *                                              sequence is fully resident in
 *                                              vector registers — no libcall)
 *
 * Builtins whose `__builtin_X` lowers back to a libcall (sin, cos, exp, log,
 * pow, ldexp, frexp, modf, fmod, remainder, nextafter, scalbn, ilogb, logb,
 * and all transcendentals/Bessel/gamma/erf) MUST NOT appear here — they
 * would tail-call themselves and infinite-loop. Real implementations for
 * those come from the *_freeBSD.c sources (openlibm port) next to this file.
 *
 * Forms must be direct (`__builtin_sqrtf(x)`) — never `(float)__builtin_sqrt
 * ((double)x)`. At -O2 the optimizer collapses the cast back to the single-
 * precision instruction so the two are equivalent today, but relying on the
 * optimizer is fragile and the direct form has unambiguous register-allocator
 * and precision behavior across compiler versions.
 *
 * On arm64 Darwin, long double == double (64-bit IEEE 754), so the
 * libm_arm_vfp.a.alias file maps every `_Xl` symbol to its plain counterpart.
 * Likewise `_nearbyint[f]` are aliased to `_rint[f]` there — do not duplicate
 * those definitions here.
 */

#if defined(__aarch64__) || defined(__arm64__)

#include "math.h"

/* FSQRT */
double sqrt(double x)                 { return __builtin_sqrt(x); }
float  sqrtf(float x)                 { return __builtin_sqrtf(x); }

/* FABS */
double fabs(double x)                 { return __builtin_fabs(x); }
float  fabsf(float x)                 { return __builtin_fabsf(x); }

/* copysign — MVNI/BIF SIMD sign-bit transfer */
double copysign(double x, double y)   { return __builtin_copysign(x, y); }
float  copysignf(float x, float y)    { return __builtin_copysignf(x, y); }

/* FMINNM / FMAXNM */
double fmin(double x, double y)       { return __builtin_fmin(x, y); }
float  fminf(float x, float y)        { return __builtin_fminf(x, y); }
double fmax(double x, double y)       { return __builtin_fmax(x, y); }
float  fmaxf(float x, float y)        { return __builtin_fmaxf(x, y); }

/* FRINT* — IEEE 754 rounding-mode-aware variants
 * ceil  / floor / trunc      ⇒ explicit direction (P / M / Z)
 * rint                       ⇒ current rounding mode, may raise FE_INEXACT
 * round                      ⇒ ties-away (FRINTA); matches C99 semantics
 * nearbyint                  ⇒ alias of rint (link-time, see .alias file) */
double ceil(double x)                 { return __builtin_ceil(x); }
float  ceilf(float x)                 { return __builtin_ceilf(x); }
double floor(double x)                { return __builtin_floor(x); }
float  floorf(float x)                { return __builtin_floorf(x); }
double trunc(double x)                { return __builtin_trunc(x); }
float  truncf(float x)                { return __builtin_truncf(x); }
double rint(double x)                 { return __builtin_rint(x); }
float  rintf(float x)                 { return __builtin_rintf(x); }
double round(double x)                { return __builtin_round(x); }
float  roundf(float x)                { return __builtin_roundf(x); }

/* FCVT* — FP → integer with embedded rounding mode
 * lround  / llround : ties-away (FCVTAS)
 * lrint   / llrint  : current mode (FRINTX then FCVTZS toward zero on the
 *                     already-rounded result, raising FE_INEXACT correctly) */
long      lround(double x)            { return __builtin_lround(x); }
long      lroundf(float x)            { return __builtin_lroundf(x); }
long long llround(double x)           { return __builtin_llround(x); }
long long llroundf(float x)           { return __builtin_llroundf(x); }
long      lrint(double x)             { return __builtin_lrint(x); }
long      lrintf(float x)             { return __builtin_lrintf(x); }
long long llrint(double x)            { return __builtin_llrint(x); }
long long llrintf(float x)            { return __builtin_llrintf(x); }

/* FMADD — fused multiply-add (single rounding)
 * Defined here; fmaf historically had its own Source/ARM/fmaf.c wrapper —
 * removed in this pass since __builtin_fmaf lowers to FMADD just as cleanly. */
double fma(double x, double y, double z)  { return __builtin_fma(x, y, z); }
float  fmaf(float x, float y, float z)    { return __builtin_fmaf(x, y, z); }

#endif /* __aarch64__ || __arm64__ */
