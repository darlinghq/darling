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

#include "math.h"
#include "complex.h" /* for its *double* prototypes */

#undef __LIBMLDBL_COMPAT
#if 0 /* Temporarily disable the plan-in-blue. */
#define __LIBMLDBL_64(sym) __asm("_" __STRING(sym) "$LDBL64")
#define __LIBMLDBL_GLOB(sym) __asm(".text"); __asm(".align 4"); __asm(".globl " "_" __STRING(sym))
#define __LIBMLDBL_NO_DECOR(sym) __asm("_" __STRING(sym) ": b " "_" __STRING(sym) "$LDBL64")
#define __LIBMLDBL_COMPAT(sym) __LIBMLDBL_64(sym) ; __LIBMLDBL_GLOB(sym) ; __LIBMLDBL_NO_DECOR(sym)
#else
#define __LIBMLDBL_64(sym) /* NOTHING */
#define __LIBMLDBL_GLOB(sym) __asm(".text"); __asm(".align 4"); __asm(".globl " "_" __STRING(sym) "$LDBL64")
#define __LIBMLDBL_NO_DECOR(sym) __asm("_" __STRING(sym) "$LDBL64" ": b " "_" __STRING(sym))
#define __LIBMLDBL_COMPAT(sym) __LIBMLDBL_64(sym) ; __LIBMLDBL_GLOB(sym) ; __LIBMLDBL_NO_DECOR(sym)
#endif

//
// Introduce prototypes and (trivial) implementations for long double == double scheme.
// Programs compiled with -mlong-double-64 see these.
//
extern long double complex cacosl( long double complex ) __LIBMLDBL_COMPAT(cacosl);
extern long double complex casinl( long double complex ) __LIBMLDBL_COMPAT(casinl);
extern long double complex catanl( long double complex ) __LIBMLDBL_COMPAT(catanl);

extern long double complex ccosl( long double complex ) __LIBMLDBL_COMPAT(ccosl);
extern long double complex csinl( long double complex ) __LIBMLDBL_COMPAT(csinl);
extern long double complex ctanl( long double complex ) __LIBMLDBL_COMPAT(ctanl);

extern long double complex cacoshl( long double complex ) __LIBMLDBL_COMPAT(cacoshl);
extern long double complex casinhl( long double complex ) __LIBMLDBL_COMPAT(casinhl);
extern long double complex catanhl( long double complex ) __LIBMLDBL_COMPAT(catanhl);

extern long double complex ccoshl( long double complex ) __LIBMLDBL_COMPAT(ccoshl);
extern long double complex csinhl( long double complex ) __LIBMLDBL_COMPAT(csinhl);
extern long double complex ctanhl( long double complex ) __LIBMLDBL_COMPAT(ctanhl);

extern long double complex cexpl( long double complex ) __LIBMLDBL_COMPAT(cexpl);
extern long double complex clogl( long double complex ) __LIBMLDBL_COMPAT(clogl);

extern long double cabsl( long double complex ) __LIBMLDBL_COMPAT(cabsl);
extern long double complex cpowl( long double complex, long double complex ) __LIBMLDBL_COMPAT(cpowl);
extern long double complex csqrtl( long double complex ) __LIBMLDBL_COMPAT(csqrtl);

extern long double cargl( long double complex ) __LIBMLDBL_COMPAT(cargl);
extern long double cimagl( long double complex ) __LIBMLDBL_COMPAT(cimagl);
extern long double complex conjl( long double complex ) __LIBMLDBL_COMPAT(conjl);
extern long double complex cprojl( long double complex ) __LIBMLDBL_COMPAT(cprojl);
extern long double creall( long double complex ) __LIBMLDBL_COMPAT(creall);

long double complex cacosl( long double complex z ) { return cacos( z); }
long double complex casinl( long double complex z ) { return casin( z); }
long double complex catanl( long double complex z ) { return catan( z); }

long double complex ccosl( long double complex z ) { return ccos( z); }
long double complex csinl( long double complex z ) { return csin( z); }
long double complex ctanl( long double complex z ) { return ctan( z); }

long double complex cacoshl( long double complex z ) { return cacosh( z); }
long double complex casinhl( long double complex z ) { return casinh( z); }
long double complex catanhl( long double complex z ) { return catanh( z); }

long double complex ccoshl( long double complex z ) { return ccosh( z); }
long double complex csinhl( long double complex z ) { return csinh( z); }
long double complex ctanhl( long double complex z ) { return ctanh( z); }

long double complex cexpl( long double complex z ) { return cexp( z); }
long double complex clogl( long double complex z ) { return clog( z); }

long double cabsl( long double complex z ) { return cabs( z); }
long double complex cpowl( long double complex x, long double complex y ) { return cpow( x,  y); }
long double complex csqrtl( long double complex z ) { return csqrt( z); }

long double cargl( long double complex z ) { return carg( z); }
long double cimagl( long double complex z ) { return cimag( z); }
long double complex conjl( long double complex z ) { return conj( z); }
long double complex cprojl( long double complex z ) { return cproj( z); }
long double creall( long double complex z ) { return creal( z); }
