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
#ifndef __FENV_H__
#define __FENV_H__

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(__ppc__) || defined(__ppc64__))
#include "architecture/ppc/fenv.h"
#elif (defined (__i386__) || defined( __x86_64__ ))
#include "architecture/i386/fenv.h"
#elif defined(__arm__) && !defined(__SOFTFP__)
#include "architecture/arm/fenv.h"
#elif defined(__arm64__) || defined(__aarch64__)

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
#define FE_FLUSHTOZERO      0x0080
#define FE_ALL_EXCEPT       0x009f

#define FE_TONEAREST        0x00000000
#define FE_UPWARD           0x00400000
#define FE_DOWNWARD         0x00800000
#define FE_TOWARDZERO       0x00C00000

enum {
    __fpcr_trap_invalid   = 0x00000100,
    __fpcr_trap_divbyzero = 0x00000200,
    __fpcr_trap_overflow  = 0x00000400,
    __fpcr_trap_underflow = 0x00000800,
    __fpcr_trap_inexact   = 0x00001000,
    __fpcr_trap_denormal  = 0x00008000,
    __fpcr_flush_to_zero  = 0x01000000,
};

enum { __fpsr_saturation  = 0x08000000 };

extern const fenv_t _FE_DFL_ENV;
#define FE_DFL_ENV &_FE_DFL_ENV

extern const fenv_t _FE_DFL_DISABLE_DENORMS_ENV;
#define FE_DFL_DISABLE_DENORMS_ENV &_FE_DFL_DISABLE_DENORMS_ENV

#else
/* Unknown or soft-float architectures */
typedef int fenv_t;
typedef unsigned short fexcept_t;
#define FE_ALL_EXCEPT       0
#define FE_TONEAREST        0
extern const fenv_t _FE_DFL_ENV;
#define FE_DFL_ENV &_FE_DFL_ENV
#endif

extern int feclearexcept(int);
extern int fegetexceptflag(fexcept_t *, int);
extern int feraiseexcept(int);
extern int fesetexceptflag(const fexcept_t *, int);
extern int fetestexcept(int);
extern int fegetround(void);
extern int fesetround(int);
extern int fegetenv(fenv_t *);
extern int feholdexcept(fenv_t *);
extern int fesetenv(const fenv_t *);
extern int feupdateenv(const fenv_t *);

#ifdef __cplusplus
}
#endif

#endif /* __FENV_H__ */
