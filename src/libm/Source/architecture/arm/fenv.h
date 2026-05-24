/*
 * Copyright (c) 2002-2013 Apple Computer, Inc. All rights reserved.
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
*     File:  fenv.h	                                                       *
*                                                                              *
*     Contains: typedefs and prototypes for C99 floating point environment.    *
*                                                                              *
*     This is the ARM / ARM64 counterpart of architecture/i386/fenv.h. The     *
*     top-level <fenv.h> dispatches here for __arm__ and __arm64__ targets.    *
*     Type and macro values are taken verbatim from Apple's shipping macOS     *
*     <fenv.h> ARM and ARM64 blocks.                                           *
*                                                                              *
*******************************************************************************/

#ifndef __FENV__
#define __FENV__

#if defined( __ppc__ ) || defined( __ppc64__ ) || defined( __i386__ ) || defined( __x86_64__ )
    #error  Wrong arch. This is ARM only.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*  ARM definitions of architecture-specific types and macros.                  *
*******************************************************************************/

#if defined( __arm__ ) && !defined( __SOFTFP__ )

typedef struct {
    unsigned int            __fpscr;
    unsigned int            __reserved0;
    unsigned int            __reserved1;
    unsigned int            __reserved2;
} fenv_t;

typedef unsigned short fexcept_t;

#define FE_INEXACT          0x0010
#define FE_UNDERFLOW        0x0008
#define FE_OVERFLOW         0x0004
#define FE_DIVBYZERO        0x0002
#define FE_INVALID          0x0001
/*  FE_FLUSHTOZERO
    An ARM-specific flag that is raised when a denormal is flushed to zero.
    This is also called the "input denormal exception"                        */
#define FE_FLUSHTOZERO      0x0080
#define FE_ALL_EXCEPT       0x009f

#define FE_TONEAREST        0x00000000
#define FE_UPWARD           0x00400000
#define FE_DOWNWARD         0x00800000
#define FE_TOWARDZERO       0x00C00000

/*  Masks for values that may be controlled in the FPSCR.  Modifying any other
    bits invokes undefined behavior.                                          */
enum {
    __fpscr_trap_invalid   = 0x00000100,
    __fpscr_trap_divbyzero = 0x00000200,
    __fpscr_trap_overflow  = 0x00000400,
    __fpscr_trap_underflow = 0x00000800,
    __fpscr_trap_inexact   = 0x00001000,
    __fpscr_trap_denormal  = 0x00008000,
    __fpscr_flush_to_zero  = 0x01000000,
    __fpscr_default_nan    = 0x02000000,
    __fpscr_saturation     = 0x08000000,
};

extern const fenv_t _FE_DFL_ENV;
#define FE_DFL_ENV &_FE_DFL_ENV

/*******************************************************************************
*  ARM64 definitions of architecture-specific types and macros.                *
*******************************************************************************/

#elif defined( __arm64__ ) || defined( __aarch64__ )

typedef struct {
    unsigned long long      __fpsr;
    unsigned long long      __fpcr;
} fenv_t;

typedef unsigned short fexcept_t;

#define FE_INEXACT          0x0010
#define FE_UNDERFLOW        0x0008
#define FE_OVERFLOW         0x0004
#define FE_DIVBYZERO        0x0002
#define FE_INVALID          0x0001
/*  FE_FLUSHTOZERO
    An ARM-specific flag that is raised when a denormal is flushed to zero.
    This is also called the "input denormal exception"                        */
#define FE_FLUSHTOZERO      0x0080
#define FE_ALL_EXCEPT       0x009f

#define FE_TONEAREST        0x00000000
#define FE_UPWARD           0x00400000
#define FE_DOWNWARD         0x00800000
#define FE_TOWARDZERO       0x00C00000

/*  Masks for values that may be controlled in the FPCR.  Modifying any other
    bits invokes undefined behavior.                                          */
enum {
    __fpcr_trap_invalid   = 0x00000100,
    __fpcr_trap_divbyzero = 0x00000200,
    __fpcr_trap_overflow  = 0x00000400,
    __fpcr_trap_underflow = 0x00000800,
    __fpcr_trap_inexact   = 0x00001000,
    __fpcr_trap_denormal  = 0x00008000,
    __fpcr_flush_to_zero  = 0x01000000,
};

/*  Mask for the QC bit of the FPSR                                           */
enum { __fpsr_saturation  = 0x08000000 };

extern const fenv_t _FE_DFL_ENV;
#define FE_DFL_ENV &_FE_DFL_ENV

/*  FE_DFL_DISABLE_DENORMS_ENV

    A pointer to a fenv_t object with the default floating-point state modified
    to set the FZ (flush to zero) bit in the FPCR.  When using this environment
    denormals encountered by floating-point calculations will be treated as
    zero.  Denormal results of floating-point operations will also be treated
    as zero.  This calculation mode is not IEEE-754 compliant, but it may
    prevent lengthy stalls that occur in code that encounters denormals.  It is
    suggested that you do not use this mode unless you have established that
    denormals are the source of measurable performance problems.

    Note that the math library, and other system libraries, are not guaranteed
    to do the right thing if called in this mode.  Edge cases may be incorrect.
    Use at your own risk.                                                     */
extern const fenv_t _FE_DFL_DISABLE_DENORMS_ENV;
#define FE_DFL_DISABLE_DENORMS_ENV &_FE_DFL_DISABLE_DENORMS_ENV

#else
    #error  Unsupported ARM variant (soft-float ARM has no hardware fenv).
#endif

/*******************************************************************************
*     The following functions provide high level access to the exception flags.*
*     The "int" input argument can be constructed by bitwise ORs of the        *
*     exception macros: for example: FE_OVERFLOW | FE_INEXACT.                 *
*******************************************************************************/

extern int  feclearexcept(int /*excepts*/);
extern int  fegetexceptflag(fexcept_t * /*flagp*/, int /*excepts*/);
extern int  feraiseexcept(int /*excepts*/);
extern int  fesetexceptflag(const fexcept_t * /*flagp*/, int /*excepts*/);
extern int  fetestexcept(int /*excepts*/);

/*******************************************************************************
*     The following functions provide control of rounding direction modes.     *
*******************************************************************************/

extern int  fegetround(void);
extern int  fesetround(int /*round*/);

/*******************************************************************************
*    The following functions manage the floating-point environment, exception  *
*    flags and dynamic modes, as one entity.                                   *
*******************************************************************************/

extern int  fegetenv(fenv_t * /*envp*/);
extern int  feholdexcept(fenv_t * /*envp*/);
extern int  fesetenv(const fenv_t * /*envp*/);
extern int  feupdateenv(const fenv_t * /*envp*/);

#ifdef __cplusplus
}
#endif

#endif /* __FENV__ */
