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
*     File fp_private.h                                                        *
*     Masks used for single and double floating point representations          *
*     on PowerPC.                                                              *
*                                                                              *
*******************************************************************************/
#ifndef __FP_PRIVATE__
#define __FP_PRIVATE__
#include "stdint.h"

/******************************************************************************
*       Functions used internally                                             *
******************************************************************************/
double   copysign ( double arg2, double arg1 );
double	 fabs ( double x );
double   nan   ( const char *string );

/* gcc inlines fabs() and fabsf()  */ 
#define      __FABS(x)	__builtin_fabs(x)
#define      __FABSF(x)	__builtin_fabsf(x)

#if defined(__APPLE_CC__)
#define likely(x) __builtin_expect(!!(x), 1) 
#define unlikely(x) __builtin_expect((x), 0) 
#else 
#define likely(x) (x) 
#define unlikely(x) (x) 
#endif 

#include "ppc_intrinsics.h"

#define __FMADD __fmadd
#define __FMADDS __fmadds
#define __FMSUB __fmsub
#define __FNMSUB __fnmsub
#define __FMUL __fmul
#define __FADD __fadd
#define __FSUB __fsub

static inline double __fadd (double a, double b) __attribute__((always_inline));
static inline double
__fadd (double  a, double b)
{
  double result;
  __asm__ ("fadd %0, %1, %2"
           /* outputs:  */ : "=f" (result)
           /* inputs:   */ : "f" (a), "f" (b));
  return result;
}

static inline double __fsub (double a, double b) __attribute__((always_inline));
static inline double
__fsub (double  a, double b)
{
  double result;
  __asm__ ("fsub %0, %1, %2"
           /* outputs:  */ : "=f" (result)
           /* inputs:   */ : "f" (a), "f" (b));
  return result;
}

// The following macros are invoked for side-effect. Not written as inline functions because the
// compiler could discard the code as an optimization.
#define __NOOP \
({  __label__ L1, L2; L1: (void)&&L1;\
    asm volatile ( "nop" ); /* NOOP */ \
    L2: (void)&&L2; \
})  

#define __ENSURE(x, y, z) \
({ \
    double __value, __argx = (x), __argy = (y), __argz = (z); \
    asm volatile ("fmadd %0,%1,%2,%3" : "=f" (__value): "f" (__argx), "f" (__argy), "f" (__argz)); \
    __value; \
}) 
 
#define __PROD(x, y) \
({ \
    double __value, __argx = (x), __argy = (y); \
    asm volatile ("fmul %0,%1,%2" : "=f" (__value): "f" (__argx), "f" (__argy)); \
    __value; \
})  

#define __PROG_INEXACT( x ) (void)__PROD( x, x ) /* Raises INEXACT for suitable choice of x */

#define __PROG_UF_INEXACT( x ) (void)__PROD( x, x ) /* Raises UNDERFLOW and INEXACT for suitable choice of x e.g. MIN_NORMAL */

#define __PROG_OF_INEXACT( x ) (void)__PROD( x, x ) /* Raises OVERFLOW and INEXACT for suitable choice of x e.g. MAX_NORMAL */

/******************************************************************************
*       Single precision                                                      *
******************************************************************************/

#define       fQuietNan           0x00400000

typedef union {
       int32_t       lval;
       float          fval;
} hexsingle;

/******************************************************************************
*       Double precision                                                      *
******************************************************************************/

#define       dQuietNan           0x00080000

#if defined(__BIG_ENDIAN__)

typedef union {
       struct {
		uint32_t hi;
		uint32_t lo;
	} i;
       double            d;
} hexdouble;

#define HEXDOUBLE(hi, lo) { { hi, lo } }

#elif defined(__LITTLE_ENDIAN__)

typedef union {
       struct {
		uint32_t lo;
		uint32_t hi;
	} i;
       double            d;
} hexdouble;

#define HEXDOUBLE(hi, lo) { { lo, hi } }

#else
#error Unknown endianness
#endif

typedef union {
	uint32_t i[4];
	struct {
	    hexdouble hexhead;
	    hexdouble hextail;
	} hh;
	struct {
		double head;
		double tail;
	} dd;
	long double ld;
} hexdbldbl;
	  
#endif      /* __FP_PRIVATE__ */
