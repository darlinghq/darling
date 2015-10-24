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
*     File:  minmaxdim.c                                                       *
*                                                                              *
*     Contains: C99 fmin, fmax, fdim, and fma				       *
*                                                                              *
*     Copyright © 2001 Apple Computer, Inc.  All rights reserved.              *
*                                                                              *
*     Written by Stephen C. Peters, started in November 2001.                  *
*                                                                              *
*     A MathLib v5 file.                                                       *
*                                                                              *
*     Change History (most recent first):                                      *
*                                                                              *
*     21 Nov 01   scp   First created.                                         *
*                                                                              *
*     W A R N I N G:                                                           *
*     These routines require a 64-bit double precision IEEE-754 model.         *
*     They are written for PowerPC only and are expecting the compiler         *
*     to generate the correct sequence of multiply-add fused instructions.     *
*                                                                              *
*     These routines are not intended for 32-bit Intel architectures.          *
*                                                                              *
*     A version of gcc higher than 932 is required.                            *
*                                                                              *
*     GCC compiler options:                                                    *
*           optimization level 3 (-O3)                                         *
*           -fschedule-insns -finline-functions -funroll-all-loops             *
*                                                                              *
*******************************************************************************/

#include "fp_private.h"
#include "math.h"

#if defined(BUILDING_FOR_CARBONCORE_LEGACY)

double fdim ( double x, double y )
{
    if (unlikely((x != x) || (y != y)))
        return ( x + y );
    else if (x > y)
        return ( x - y );
    else
        return 0.0;
}

//
// N.B. max/min (-0, 0) allows implementation dependent result
//
#define __fmax(x, y) \
({ \
    double __value, __argx = (x), __argy = (y); \
    asm volatile ( \
        "fcmpu 		cr0,%1,%2 ; 	/* Compare unordered */ 				\n \
         blt		cr0, 0f ; 	/* Order discerned? Then we have our answer */ 		\n \
         bnu+		cr0, 1f ; 	/* Opposite order discerned? Then we have our answer */ \n \
         fcmpu 		cr1,%2,%2 ; 	/* x, y or both are NAN. Is y NAN? */			\n \
         bun-		cr1, 1f ; 	/* If so, x is our answer */ 				\n \
    0:	 fmr		%0, %2; 	/* Else y is our answer */ 				\n \
         b		2f									\n \
    1:	 fmr		%0,%1;									\n \
    2:												\n \
        ": "=f"(__value) : "f" (__argx), "f" (__argy)); \
    __value; \
})  

#if 0
//
// N.B. max/min (-0, 0) allows implementation dependent result
//
double fmax ( double x, double y )
{
    if (x != x)
        return y;
    else if (y != y)
        return x;
    else if (x < y)
        return y;
    else
        return x;
}
#else
double fmax ( double x, double y )
{
    return __fmax( x, y );
}
#endif

#define __fmin(x, y) \
({ \
    double __value, __argx = (x), __argy = (y); \
    asm volatile ( \
        "fcmpu 		cr0,%1,%2 ; 	/* Compare unordered */ 				\n \
         bgt		cr0, 0f ; 	/* Order discerned? Then we have our answer */ 		\n \
         bnu+		cr0, 1f ; 	/* Opposite order discerned? Then we have our answer */ \n \
         fcmpu 		cr1,%2,%2 ; 	/* x, y or both are NAN. Is y NAN? */			\n \
         bun-		cr1, 1f ; 	/* If so, x is our answer */ 				\n \
    0:	 fmr		%0, %2; 	/* Else y is our answer */ 				\n \
         b		2f									\n \
    1:	 fmr		%0,%1;									\n \
    2:												\n \
        ": "=f"(__value) : "f" (__argx), "f" (__argy)); \
    __value; \
})  

#if 0
double fmin ( double x, double y )
{
    if (x != x)
        return y;
    else if (y != y)
        return x;
    else if (x > y)
        return y;
    else
        return x;
}
#else
double fmin ( double x, double y )
{
    return __fmin( x, y );
}
#endif

#else /* !BUILDING_FOR_CARBONCORE_LEGACY */

float fdimf ( float x, float y )
{
    if (unlikely((x != x) || (y != y)))
        return ( x + y );
    else if (x > y)
        return ( x - y );
    else
        return 0.0f;
}


#define __fmaxf(x, y) \
({ \
    float __value, __argx = (x), __argy = (y); \
    asm volatile ( \
        "fcmpu 		cr0,%1,%2 ; 	/* Compare unordered */ 				\n \
         blt		cr0, 0f ; 	/* Order discerned? Then we have our answer */ 		\n \
         bnu+		cr0, 1f ; 	/* Opposite order discerned? Then we have our answer */ \n \
         fcmpu 		cr1,%2,%2 ; 	/* x, y or both are NAN. Is y NAN? */			\n \
         bun-		cr1, 1f ; 	/* If so, x is our answer */ 				\n \
    0:	 fmr		%0, %2; 	/* Else y is our answer */ 				\n \
         b		2f									\n \
    1:	 fmr		%0,%1;									\n \
    2:												\n \
        ": "=f"(__value) : "f" (__argx), "f" (__argy)); \
    __value; \
})  

#if 0
float fmaxf ( float x, float y )
{
    if (x != x)
        return y;
    else if (y != y)
        return x;
    else if (x < y)
        return y;
    else
        return x;
}
#else
float fmaxf ( float x, float y )
{
    return __fmaxf( x, y );
}
#endif

#define __fminf(x, y) \
({ \
    float __value, __argx = (x), __argy = (y); \
    asm volatile ( \
        "fcmpu 		cr0,%1,%2 ; 	/* Compare unordered */ 				\n \
         bgt		cr0, 0f ; 	/* Order discerned? Then we have our answer */ 		\n \
         bnu+		cr0, 1f ; 	/* Opposite order discerned? Then we have our answer */ \n \
         fcmpu 		cr1,%2,%2 ; 	/* x, y or both are NAN. Is y NAN? */			\n \
         bun-		cr1, 1f ; 	/* If so, x is our answer */ 				\n \
    0:	 fmr		%0, %2; 	/* Else y is our answer */ 				\n \
         b		2f									\n \
    1:	 fmr		%0,%1;									\n \
    2:												\n \
        ": "=f"(__value) : "f" (__argx), "f" (__argy)); \
    __value; \
})  

#if 0
float fminf ( float x, float y )
{
    if (x != x)
        return y;
    else if (y != y)
        return x;
    else if (x > y)
        return y;
    else
        return x; 
}
#else
float fminf ( float x, float y )
{
    return __fminf( x, y );
}
#endif

double fma ( double x, double y, double z )
{
    return __FMADD(x, y, z);
}

float fmaf ( float x, float y, float z )
{
    return __FMADDS(x, y, z);
}
#endif /* !BUILDING_FOR_CARBONCORE_LEGACY */
