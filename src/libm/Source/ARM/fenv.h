/*
 * Copyright (c) 2002-2024 Apple Computer, Inc. All rights reserved.
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
*     File:  fenv.h  (Source/ARM private copy)                                 *
*                                                                              *
*     Contains:  typedefs and prototypes for the C99 floating-point            *
*                environment, ARM/ARM64 flavor.                                 *
*                                                                              *
*     This header is the *implementation-private* counterpart of the           *
*     public `<fenv.h>` shipped at src/libm/include/fenv.h. It MUST keep the   *
*     `fenv_t` layout in sync with the public header — every member name and  *
*     order has to match exactly, otherwise C99 user code reading an `fenv_t` *
*     filled in by fegetenv() will see the wrong bits.                         *
*                                                                              *
*******************************************************************************/

#ifndef __FENV__
#define __FENV__

#if defined(__arm__) && !defined(__SOFTFP__)
/* ============================ 32-bit ARM (VFP) =========================== */

#ifdef __cplusplus
extern "C" {
#endif

/*  fenv_t mirrors the public 32-bit ARM layout: a single FPSCR word plus     */
/*  three reserved words to keep the struct 16 bytes wide on all targets.    */
typedef struct {
    union {
        struct {
            unsigned int __fpscr;
            unsigned int __reserved0;
            unsigned int __reserved1;
            unsigned int __reserved2;
        };
#if defined(__GNUC__)
        struct {
            unsigned int __fpscr_cmp_n : 1;
            unsigned int __fpscr_cmp_z : 1;
            unsigned int __fpscr_cmp_c : 1;
            unsigned int __fpscr_cmp_v : 1;
            unsigned int __fpscr_do_not_modify_1 : 2;          /* zero */
            unsigned int __fpscr_default_nan_mode : 1;
            unsigned int __fpscr_flush_to_zero : 1;
            unsigned int __fpscr_rounding_mode : 2;
            unsigned int __fpscr_stride : 2;
            unsigned int __fpscr_do_not_modify_2 : 1;          /* zero */
            unsigned int __fpscr_len : 3;
            unsigned int __fpscr_trap_enable_subnormal : 1;
            unsigned int __fpscr_do_not_modify_3 : 2;          /* zero */
            unsigned int __fpscr_trap_enable_inexact : 1;
            unsigned int __fpscr_trap_enable_underflow : 1;
            unsigned int __fpscr_trap_enable_overflow : 1;
            unsigned int __fpscr_trap_enable_div_by_zero : 1;
            unsigned int __fpscr_trap_enable_invalid : 1;
            unsigned int __fpscr_fp_state_flag_subnormal : 1;
            unsigned int __fpscr_do_not_modify_4 : 2;          /* zero */
            unsigned int __fpscr_fp_state_flag_inexact : 1;
            unsigned int __fpscr_fp_state_flag_underflow : 1;
            unsigned int __fpscr_fp_state_flag_overflow : 1;
            unsigned int __fpscr_fp_state_flag_div_by_zero : 1;
            unsigned int __fpscr_fp_state_flag_invalid : 1;
        } __attribute__((packed));
#endif
    };
} fenv_t;

typedef unsigned short fexcept_t;

/* Exception flags (FPSCR bits 0..4 / +0x80 for FZ).                          */
#define FE_INEXACT           0x0010
#define FE_UNDERFLOW         0x0008
#define FE_OVERFLOW          0x0004
#define FE_DIVBYZERO         0x0002
#define FE_INVALID           0x0001
#define FE_ALL_EXCEPT        0x001F

/* Rounding direction bits live at FPSCR bits 22..23.                         */
#define FE_TONEAREST         0x00000000
#define FE_UPWARD            0x00400000
#define FE_DOWNWARD          0x00800000
#define FE_TOWARDZERO        0x00C00000

extern const fenv_t _FE_DFL_ENV;
#define FE_DFL_ENV (&_FE_DFL_ENV)

#ifdef __cplusplus
}
#endif

#elif defined(__aarch64__) || defined(__arm64__)
/* =============================== ARM64 =================================== *
 * Self-contained ARM64 floating-point-environment declarations, mirroring the
 * role of Source/Intel/fenv.h for x86: this is the implementation-private
 * header that Source/ARM/*.c sources include directly. Its `fenv_t` layout
 * and FE_* values MUST stay byte-for-byte identical to the public ABI in
 * include/architecture/arm/fenv.h. */

typedef struct {
    unsigned long long __fpsr;
    unsigned long long __fpcr;
} fenv_t;

typedef unsigned short fexcept_t;

/* Exception flag bits (FPSR.IOC=0, DZC=1, OFC=2, UFC=3, IXC=4, IDC=7).       */
#define FE_INVALID           0x0001
#define FE_DIVBYZERO         0x0002
#define FE_OVERFLOW          0x0004
#define FE_UNDERFLOW         0x0008
#define FE_INEXACT           0x0010
#define FE_FLUSHTOZERO       0x0080
#define FE_ALL_EXCEPT        0x009F

/* Rounding direction bits (FPCR bits 22..23).                                */
#define FE_TONEAREST         0x00000000
#define FE_UPWARD            0x00400000
#define FE_DOWNWARD          0x00800000
#define FE_TOWARDZERO        0x00C00000

/* FPCR trap-enable bits (bits 8..12, 15).                                    */
enum {
    __fpcr_trap_invalid    = 0x00000100,
    __fpcr_trap_divbyzero  = 0x00000200,
    __fpcr_trap_overflow   = 0x00000400,
    __fpcr_trap_underflow  = 0x00000800,
    __fpcr_trap_inexact    = 0x00001000,
    __fpcr_trap_denormal   = 0x00008000,
    __fpcr_flush_to_zero   = 0x01000000,
};

enum { __fpsr_saturation   = 0x08000000 };

/*
 * Implementation-private FPCR / FPSR register accessors — not part of the
 * public ABI. Layout mirrors openlibm's include/openlibm_fenv_aarch64.h so
 * any later feenableexcept / fedisableexcept work stays aligned with upstream.
 */
#define __mrs_fpcr(__r)  __asm__ __volatile__("mrs %0, fpcr" : "=r"(__r))
#define __msr_fpcr(__r)  __asm__ __volatile__("msr fpcr, %0" : : "r"(__r))
#define __mrs_fpsr(__r)  __asm__ __volatile__("mrs %0, fpsr" : "=r"(__r))
#define __msr_fpsr(__r)  __asm__ __volatile__("msr fpsr, %0" : : "r"(__r))

/* Default environment object: rounding=to-nearest, no traps, no flush.       */
extern const fenv_t _FE_DFL_ENV;
#define FE_DFL_ENV (&_FE_DFL_ENV)

/* Default environment with subnormals flushed to zero (Apple extension).     */
extern const fenv_t _FE_DFL_DISABLE_DENORMS_ENV;
#define FE_DFL_DISABLE_DENORMS_ENV (&_FE_DFL_DISABLE_DENORMS_ENV)

#else
#error "Source/ARM/fenv.h is for ARM / ARM64 targets only"
#endif

/* ===================== Function prototypes (both ABIs) ===================== */

#ifdef __cplusplus
extern "C" {
#endif

extern int feclearexcept(int /*excepts*/);
extern int fegetexceptflag(fexcept_t * /*flagp*/, int /*excepts*/);
extern int feraiseexcept(int /*excepts*/);
extern int fesetexceptflag(const fexcept_t * /*flagp*/, int /*excepts*/);
extern int fetestexcept(int /*excepts*/);

extern int fegetround(void);
extern int fesetround(int /*round*/);

extern int fegetenv(fenv_t * /*envp*/);
extern int feholdexcept(fenv_t * /*envp*/);
extern int fesetenv(const fenv_t * /*envp*/);
extern int feupdateenv(const fenv_t * /*envp*/);

#ifdef __cplusplus
}
#endif

#endif /* __FENV__ */
