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
*     File:  fenv.h	                                                       *
*                                                                              *
*     Contains: typedefs and prototypes for C99 floating point environment.    *
*                                                                              *
*******************************************************************************/

#ifndef __FENV__
#define __FENV__

#if defined( __i386__ ) || defined( __x86_64__ )
    #error  Wrong arch. This is PowerPC only.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
    A collection of functions designed to provide access to the floating
    point environment for numerical programming. It is modeled after
    the floating-point requirements in C9X.
    
    The file <fenv.h> declares many functions in support of numerical
    programming. Programs that test flags or run under
    non-default modes must do so under the effect of an enabling
    "fenv_access" pragma.
*/

/********************************************************************************
*                                                                               *
*    fenv_t         is a type for representing the entire floating-point        *
*                   environment in a single object.                             *
*                                                                               *
*    fexcept_t      is a type for representing the floating-point               *
*                   exception flag state collectively.                          *
*                                                                               *
********************************************************************************/
typedef unsigned int			fenv_t;
typedef unsigned int			fexcept_t;

/*    Definitions of floating-point exception macros                          */
#define FE_INEXACT      0x02000000
#define FE_DIVBYZERO    0x04000000
#define FE_UNDERFLOW    0x08000000
#define FE_OVERFLOW     0x10000000
#define FE_INVALID      0x20000000
#define FE_ALL_EXCEPT   0x3E000000

/*    Definitions of rounding direction macros                                */
#define FE_TONEAREST    0x00000000
#define FE_TOWARDZERO   0x00000001
#define FE_UPWARD       0x00000002
#define FE_DOWNWARD     0x00000003

/* default environment object        */
extern const fenv_t _FE_DFL_ENV;
#define FE_DFL_ENV &_FE_DFL_ENV          /* pointer to default environment    */

/*******************************************************************************
*     The following functions provide access to the exception flags.  The      *
*     "int" input argument can be constructed by bitwise ORs of the exception  *
*     macros: for example: FE_OVERFLOW | FE_INEXACT.                           *
*******************************************************************************/

/*******************************************************************************
*     The function "feclearexcept" clears the supported exceptions represented *
*     by its argument.                                                         *
*******************************************************************************/

extern int  feclearexcept(int);


/*******************************************************************************
*    The function "fegetexceptflag" stores a representation of the exception   *
*     flags indicated by its integer argument through the fexcept_t pointer    *
*     argument.                                                                *
*******************************************************************************/

extern int  fegetexceptflag(fexcept_t *, int);


/*******************************************************************************
*     The function "feraiseexcept" raises the supported exceptions             *
*     represented by its argument.                                             *
*******************************************************************************/

extern int  feraiseexcept(int);


/*******************************************************************************
*     The function "fesetexceptflag" sets or clears the exception flags indicated  *
*     by the its integer argument according to the representation in the       *
*     object pointed to by the fexcept_t pointer argument.  The value of the   *
*     object must have been set by a previous call to "fegetexceptflag".       *
*     This function does not raise exceptions; it just sets the state of       *
*     the flags.                                                               *
*******************************************************************************/

extern int  fesetexceptflag(const fexcept_t *, int);


/*******************************************************************************
*     The function "fetestexcept" determines which of the specified subset of  *
*     the exception flags are currently set.  The integer argument specifies   *
*     the exception flags to be queried as a bitwise OR of the exception       *
*     macros.  This function returns the bitwise OR of the exception macros    *
*     corresponding to the currently set exceptions included in "excepts".     *
*******************************************************************************/

extern int  fetestexcept(int);


/*******************************************************************************
*     The following functions provide control of rounding direction modes.     *
*******************************************************************************/

/*******************************************************************************
*     The function "fegetround" returns the value of the rounding direction    *
*     macro which represents the current rounding direction.                   *
*******************************************************************************/

extern int  fegetround(void);


/*******************************************************************************
*     The function "fesetround" establishes the rounding direction represented *
*     by its argument.  It returns zero if and only if the argument matches    *
*     a rounding direction macro.  If not, the rounding direction is not       *
*     changed.                                                                 *
*******************************************************************************/

extern int  fesetround(int);


/*******************************************************************************
*    The following functions manage the floating-point environment, exception  *
*    flags and dynamic modes, as one entity.                                   *
*******************************************************************************/

extern int  fegetenv(fenv_t *);
extern int   feholdexcept(fenv_t *);
extern int  fesetenv(const fenv_t *);
extern int  feupdateenv(const fenv_t *);


#ifdef __cplusplus
}
#endif

#endif /* __FENV__ */

