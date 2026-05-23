/*
 *  fenv.c
 *  cLibm
 *
 *  Originally created by Ian Ollmann on 7/15/07.
 *  Copyright 2007-2024 Apple Inc. All rights reserved.
 *
 *  C99 floating-point environment for ARM and ARM64. Reads and writes
 *  FPCR/FPSR on ARM64, the legacy single-word FPSCR on ARM32.
 */

#if defined(ARMLIBM_FENV_SUPPORT)

#include "fenv.h"
#include <stdint.h>
#include "required_arithmetic.h"

#if defined(__GNUC__)
    #define ALWAYS_INLINE __attribute__((__always_inline__))
#else
    #define ALWAYS_INLINE
#endif

#if !defined(__arm__) && !defined(__aarch64__) && !defined(__arm64__)
    #error This file is for use on ARM / ARM64 targets
#endif

/* =====================================================================
 *  Hardware accessors
 *  ARM32: a single 32-bit FPSCR holds both exception flags and control.
 *  ARM64: FPSR holds the cumulative status flags; FPCR holds the
 *         rounding mode, exception enables, and FZ/DN modes. They are
 *         architecturally separate and must be MRS/MSR'd individually.
 * ===================================================================== */

#if defined(__aarch64__) || defined(__arm64__)

/* MRS/MSR accessors come from Source/ARM/fenv.h (mirroring openlibm's
 * include/openlibm_fenv_aarch64.h). Thin static-inline wrappers keep
 * type signatures unambiguous at the call sites below. */
static inline uint64_t _get_fpsr(void) ALWAYS_INLINE;
static inline uint64_t _get_fpsr(void) { uint64_t v; __mrs_fpsr(v); return v; }

static inline uint64_t _get_fpcr(void) ALWAYS_INLINE;
static inline uint64_t _get_fpcr(void) { uint64_t v; __mrs_fpcr(v); return v; }

static inline void _set_fpsr(uint64_t v) ALWAYS_INLINE;
static inline void _set_fpsr(uint64_t v) { __msr_fpsr(v); }

static inline void _set_fpcr(uint64_t v) ALWAYS_INLINE;
static inline void _set_fpcr(uint64_t v) { __msr_fpcr(v); }

#else /* __arm__ */

static inline uint32_t _get_fpscr(void) ALWAYS_INLINE;
static inline uint32_t _get_fpscr(void)
{
    uint32_t v;
    __asm__ __volatile__("fmrx %0, fpscr" : "=r"(v));
    return v;
}

static inline void _set_fpscr(uint32_t v) ALWAYS_INLINE;
static inline void _set_fpscr(uint32_t v)
{
    __asm__ __volatile__("fmxr fpscr, %0" : : "r"(v));
}

#endif

/* =====================================================================
 *  Default environments
 *  _FE_DFL_ENV:                 all zeros — to-nearest, no traps, no FZ.
 *  _FE_DFL_DISABLE_DENORMS_ENV: ARM64 only — flush-to-zero set in FPCR.
 * ===================================================================== */

#if defined(__aarch64__) || defined(__arm64__)
const fenv_t _FE_DFL_ENV                  = { 0, 0 };
const fenv_t _FE_DFL_DISABLE_DENORMS_ENV  = { 0, __fpcr_flush_to_zero };
#else
const fenv_t _FE_DFL_ENV = {{{ 0, 0, 0, 0 }}};
#endif

/* =====================================================================
 *  Exception-flag manipulation (status flags only — FPSR on ARM64,
 *  low bits of FPSCR on ARM32).
 * ===================================================================== */

static inline int _fesetexceptflag_private(const fexcept_t *flagp, int excepts) ALWAYS_INLINE;
static inline int _fesetexceptflag_private(const fexcept_t *flagp, int excepts)
{
    excepts &= FE_ALL_EXCEPT;
#if defined(__aarch64__) || defined(__arm64__)
    uint64_t fpsr = _get_fpsr();
    fpsr &= ~(uint64_t)excepts;
    fpsr |= (uint64_t)(*flagp & excepts);
    _set_fpsr(fpsr);
#else
    uint32_t fpscr = _get_fpscr();
    fpscr &= ~(uint32_t)excepts;
    fpscr |= (uint32_t)(*flagp & excepts);
    _set_fpscr(fpscr);
#endif
    return 0;
}

static inline int _fegetexceptflag_private(fexcept_t *flagp, int excepts) ALWAYS_INLINE;
static inline int _fegetexceptflag_private(fexcept_t *flagp, int excepts)
{
#if defined(__aarch64__) || defined(__arm64__)
    *flagp = (fexcept_t)(_get_fpsr() & (uint64_t)excepts);
#else
    *flagp = (fexcept_t)(_get_fpscr() & (uint32_t)excepts);
#endif
    return 0;
}

int feclearexcept(int excepts)
{
    fexcept_t zero = 0;
    return _fesetexceptflag_private(&zero, excepts);
}

int feraiseexcept(int excepts)
{
    /* Generate the requested exceptions by performing arithmetic that
     * provokes them. This is portable across implementations and is the
     * approach the Intel libm uses too. */
    int inexact_set = 0;

    if (excepts & FE_OVERFLOW) {
        required_add_float(0x1.0p127f, 0x1.0p127f);
        inexact_set = 1;
    }
    if (excepts & FE_UNDERFLOW) {
        required_multiply_float(0x1.0p-126f, 0x1.0p-126f);
        inexact_set = 1;
    }
    if (excepts & FE_DIVBYZERO) {
        required_divide_float(1.0f, 0.0f);
    }
    if (excepts & FE_INVALID) {
        required_divide_float(0.0f, 0.0f);
    }
    if ((excepts & FE_INEXACT) && !inexact_set) {
        required_add_float(0x1.0p23f, 1.0f);
    }
    return 0;
}

int fegetexceptflag(fexcept_t *flagp, int excepts)
{
    return _fegetexceptflag_private(flagp, excepts);
}

int fesetexceptflag(const fexcept_t *flagp, int excepts)
{
    return _fesetexceptflag_private(flagp, excepts);
}

int fetestexcept(int excepts)
{
    fexcept_t f;
    _fegetexceptflag_private(&f, excepts);
    return (int)f;
}

/* =====================================================================
 *  Rounding-direction modes
 *  Rounding lives in FPCR bits 22..23 on ARM64 and FPSCR bits 22..23 on
 *  ARM32, with identical encoding. FE_* macros are pre-shifted to match
 *  the in-register layout so we just mask & assign.
 * ===================================================================== */

#define FE_ALL_RND (FE_TONEAREST | FE_TOWARDZERO | FE_UPWARD | FE_DOWNWARD)

int fegetround(void)
{
#if defined(__aarch64__) || defined(__arm64__)
    return (int)(_get_fpcr() & (uint64_t)FE_ALL_RND);
#else
    return (int)(_get_fpscr() & (uint32_t)FE_ALL_RND);
#endif
}

int fesetround(int round)
{
    if ((unsigned)round & ~(unsigned)FE_ALL_RND)
        return 1; /* invalid */

#if defined(__aarch64__) || defined(__arm64__)
    uint64_t fpcr = _get_fpcr();
    fpcr &= ~(uint64_t)FE_ALL_RND;
    fpcr |= (uint64_t)round;
    _set_fpcr(fpcr);
#else
    uint32_t fpscr = _get_fpscr();
    fpscr &= ~(uint32_t)FE_ALL_RND;
    fpscr |= (uint32_t)round;
    _set_fpscr(fpscr);
#endif
    return 0;
}

/* =====================================================================
 *  Whole-environment operations
 * ===================================================================== */

int fegetenv(fenv_t *envp)
{
#if defined(__aarch64__) || defined(__arm64__)
    envp->__fpsr = _get_fpsr();
    envp->__fpcr = _get_fpcr();
#else
    envp->__fpscr     = _get_fpscr();
    envp->__reserved0 = 0;
    envp->__reserved1 = 0;
    envp->__reserved2 = 0;
#endif
    return 0;
}

int feholdexcept(fenv_t *envp)
{
#if defined(__aarch64__) || defined(__arm64__)
    uint64_t fpsr = _get_fpsr();
    uint64_t fpcr = _get_fpcr();
    envp->__fpsr = fpsr;
    envp->__fpcr = fpcr;
    /* Clear status flags and disable traps for all standard exceptions. */
    fpsr &= ~(uint64_t)FE_ALL_EXCEPT;
    fpcr &= ~(uint64_t)(__fpcr_trap_invalid | __fpcr_trap_divbyzero |
                        __fpcr_trap_overflow | __fpcr_trap_underflow |
                        __fpcr_trap_inexact);
    _set_fpsr(fpsr);
    _set_fpcr(fpcr);
#else
    uint32_t fpscr = _get_fpscr();
    envp->__fpscr     = fpscr;
    envp->__reserved0 = 0;
    envp->__reserved1 = 0;
    envp->__reserved2 = 0;
    /* Clear status flags AND trap-enable bits (bits 8..12). */
    fpscr &= ~(uint32_t)(FE_ALL_EXCEPT | (FE_ALL_EXCEPT << 8));
    _set_fpscr(fpscr);
#endif
    return 0;
}

int fesetenv(const fenv_t *envp)
{
#if defined(__aarch64__) || defined(__arm64__)
    _set_fpcr(envp->__fpcr);
    _set_fpsr(envp->__fpsr);
#else
    _set_fpscr(envp->__fpscr);
#endif
    return 0;
}

int feupdateenv(const fenv_t *envp)
{
#if defined(__aarch64__) || defined(__arm64__)
    uint64_t raised = _get_fpsr() & (uint64_t)FE_ALL_EXCEPT;
    _set_fpcr(envp->__fpcr);
    _set_fpsr(envp->__fpsr | raised);
    if (raised)
        feraiseexcept((int)raised);
#else
    uint32_t raised = _get_fpscr() & (uint32_t)FE_ALL_EXCEPT;
    _set_fpscr(envp->__fpscr);
    if (raised)
        feraiseexcept((int)raised);
#endif
    return 0;
}

/* =====================================================================
 *  GNU / Apple-private extensions
 *  fegetexcept / fesetexcept   — read/write the trap-enable mask (FPCR
 *                                 bits 8..12 on ARM64; FPSCR bits 8..12
 *                                 on ARM32). Note these manipulate which
 *                                 exceptions raise a SIGFPE-style trap;
 *                                 they do NOT touch the sticky status
 *                                 flags (that's fesetexceptflag).
 *  __fegetfltrounds            — Apple-internal hook backing FLT_ROUNDS
 *                                 from <float.h>. Returns the same
 *                                 0/1/2/3 encoding the C standard
 *                                 mandates for that macro.
 * ===================================================================== */

#if defined(__aarch64__) || defined(__arm64__)

/* Map FE_* exception bits (FPSR layout, bits 0..4) onto the FPCR
 * trap-enable bits (bits 8..12). */
#define __FE_TRAP_SHIFT  8

int fegetexcept(void)
{
    uint64_t fpcr = _get_fpcr();
    return (int)((fpcr >> __FE_TRAP_SHIFT) & (uint64_t)FE_ALL_EXCEPT);
}

int fesetexcept(int excepts)
{
    excepts &= FE_ALL_EXCEPT;
    uint64_t fpcr = _get_fpcr();
    fpcr &= ~((uint64_t)FE_ALL_EXCEPT << __FE_TRAP_SHIFT);
    fpcr |=  ((uint64_t)excepts        << __FE_TRAP_SHIFT);
    _set_fpcr(fpcr);
    return 0;
}

#else /* ARM32: FPSCR holds traps at bits 8..12 too */

#define __FE_TRAP_SHIFT  8

int fegetexcept(void)
{
    uint32_t fpscr = _get_fpscr();
    return (int)((fpscr >> __FE_TRAP_SHIFT) & (uint32_t)FE_ALL_EXCEPT);
}

int fesetexcept(int excepts)
{
    excepts &= FE_ALL_EXCEPT;
    uint32_t fpscr = _get_fpscr();
    fpscr &= ~((uint32_t)FE_ALL_EXCEPT << __FE_TRAP_SHIFT);
    fpscr |=  ((uint32_t)excepts        << __FE_TRAP_SHIFT);
    _set_fpscr(fpscr);
    return 0;
}

#endif

/* __fegetfltrounds backs <float.h>'s FLT_ROUNDS. The encoding is fixed by
 * ISO C: -1 = indeterminate, 0 = toward zero, 1 = to nearest,
 *         2 = toward +inf, 3 = toward -inf. clang's __builtin_flt_rounds()
 * already returns exactly this on every supported target. */
int __fegetfltrounds(void)
{
    return __builtin_flt_rounds();
}

#endif /* ARMLIBM_FENV_SUPPORT */
