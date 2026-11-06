/*
 * arm64_misc.c
 *
 * Legacy and Apple-private libm symbols that openlibm does not provide:
 *
 *   __inf  __inff  __nan         — outline backings for the INFINITY / NAN
 *                                  macros in Apple's <math.h>. Both Apple
 *                                  and ARM compilers fold the macros at
 *                                  compile time, but a few historical .o's
 *                                  reference the externs.
 *
 *   _finite                      — SVID-era predicate, "true if x is not
 *                                  ±Inf or NaN" — identical to C99 isfinite.
 *
 *   _matherr                     — SVID-4 error-handling hook. Apple ships
 *                                  it as a weak symbol returning 0 ("library
 *                                  did not handle the error"); user code may
 *                                  override it at link time.
 *
 *   _rinttol / _roundtol         — Mac OS Classic <fp.h> long-integer
 *                                  conversions, kept for Carbon-era code.
 *                                  rinttol uses current rounding mode;
 *                                  roundtol rounds ties away from zero.
 */

#if defined(__aarch64__) || defined(__arm64__)

#include <math.h>

/* INFINITY / NAN macro backings.
 * Compilers inline __builtin_inf() / __builtin_nan("") at every use; these
 * out-of-line functions exist for the rare case the macros are taken by
 * address or referenced from a non-C source. Signatures match Apple SDK
 * architecture/arm/math.h:507-510 — note that `__nan` returns FLOAT (a
 * historical quirk Apple kept for ABI backward-compat with 10.3-era libm). */
double __inf(void)   { return __builtin_inf(); }
float  __inff(void)  { return __builtin_inff(); }
float  __nan(void)   { return __builtin_nanf(""); }

/* SVID4: finite(x) is the historical name for what C99 calls isfinite(x). */
int finite(double x) { return __builtin_isfinite(x); }

/* SVID4 math error hook. Apple SDK declares `int matherr(struct exception*)`
 * in <math.h>; we provide a weak stub returning 0 ("library, do your default
 * thing") so user code can override:
 *     int matherr(struct exception *e) { ...handle e->type... }
 */
__attribute__((weak))
int matherr(struct exception *exc)
{
    (void)exc;
    return 0;
}

/* __math_errhandling is provided by Source/ARM/math_errhandling.c as an
 * Apple-style function (not a const variable) — do not redefine here. */

/* Apple Mac OS Classic <fp.h> integer conversions, kept for source
 * compatibility with old Carbon code. */
long rinttol(double x)  { return (long)__builtin_rint(x); }
long roundtol(double x) { return (long)__builtin_round(x); }

/* 4.3BSD significand(x): scale x so the result lies in [1, 2). Defined as
 * `x * 2^(-ilogb(x))` for normal x; for ±0, ±Inf, NaN the function just
 * returns x unchanged (matching FreeBSD/NetBSD/Apple AMD64 libm). */
double significand(double x)
{
    if (!__builtin_isfinite(x) || x == 0.0)
        return x;
    return scalbn(x, -ilogb(x));
}

#endif /* __aarch64__ || __arm64__ */
