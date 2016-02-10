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
*     File:  fenv.c                                                             *
*                                                                              *
*     Contains:   C source code for PowerPC implementations of floating-point  *
*     environmental functions defined in C99.                                  *
*                                                                              *
*     Copyright © 1992-2001 Apple Computer, Inc.  All rights reserved.         *
*                                                                              *
*     Written by Jon Okada, started on November 1992.                          *
*     Modified by A. sazegari (ali) for MathLib v3.                            *
*     Modified and ported by Robert A. Murley (ram) for Mac OS X.              *
*                                                                              *
*     A MathLib v4 file.                                                       *
*                                                                              *
*     Change History (most recent first):                                      *
*                                                                              *
*     06 Nov 01   ram   commented out warning about Intel architectures.       *
*     08 Oct 01   ram   removed <CoreServices/CoreServices.h>.                 *
*                       changed compiler errors to warnings.                   *
*     18 Sep 01   ali   added <CoreServices/CoreServices.h> to get <fenv.h>.   *
*     10 Sep 01   ali   added macros to detect PowerPC and correct compiler.   *
*     09 Sep 01   ali   added more comments.                                   *
*     05 Sep 01   ram   added #ifdef __ppc__.                                  *
*     16 Jul 01   ram   Moved exception flag symbols to fenv_private.h.        *
*                       Replaced __setflm with FEGETENVD and FESETENVD.        *
*     29 Sep 94   ali   use __setflm for generating INEXACT, INVALID and       *
*                       DIVBYZERO in feraiseexecpt.                            *
*     28 Sep 94   PAF   Modified to use __setflm                               *
*     07 Oct 93   ali   corrected a mistake in setting flags for underflow     *
*                       and overflow in the feraiseexcept                      *
*     07 Apr 93   ali   included fenv.h and deleted the unnecessary macros     *
*     03 Feb 93   JPO   Reflecting changes in NCEG specification, changed      *
*                       the following functions from type int to void:         *
*                       feclearexcept, fegetexceptflag, feraiseexcept, and         *
*                       fesetexceptflag.  Changed the functionality of             *
*                       fesetexceptflag to match interpretation of the author      *
*                       of the NCEG spec.                                      *
*     21 Jan 93   JPO   Added default environment _FE_DFL_ENV.                 *
*     10 Dec 92   JPO   Put underscore in names of low level routines          *
*                       (_getFPSCR and _setFPSCR).                             *
*     30 Nov 92   JPO   First created.                                         *
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
*            optimization level 3 (-O3)                                        *
*            -fschedule-insns -finline-functions -funroll-all-loops            *
*                                                                              *
*******************************************************************************/

#include    "fp_private.h"
#include    "fenv_private.h"

/******************************************************************************
*   Implementations of functions which provide access to the exception flags. *
*   The "int" input argument is constructed by bitwise ORs of the exception   *
*   macros defined in fenv.h:  for example, FE_OVERFLOW | FE_INEXACT.         *
******************************************************************************/

static int _fegetexceptflag ( fexcept_t *flagp, int excepts )
{
   hexdouble temp;
   fenv_t excstate;
   uint32_t mask;
   
   mask = excepts & FE_ALL_EXCEPT;
   FEGETENVD_GRP( temp.d );
   excstate = temp.i.lo & FE_ALL_FLAGS;
   mask &= excstate;
   if (mask == 0)
      *flagp = 0;
   else {
      if ( (mask & FE_INVALID) == 0 ) excstate &= FE_NO_INVALID;
      mask |= FE_NO_EXCEPT;
      *flagp = excstate & mask;
   }
   
   return 0;
}

static int _fesetexceptflag ( const fexcept_t *flagp, int excepts )
{
   hexdouble ifpscr;
   uint32_t mask;
   
   mask = excepts & FE_ALL_EXCEPT;
   if (mask != 0) {                     // take action if mask != 0
      FEGETENVD_GRP( ifpscr.d );            // read current environment
      if (excepts & FE_INVALID)         // special case:  INVALID
         ifpscr.i.lo = (ifpscr.i.lo & FE_NO_INVALID) | (*flagp & FE_ALL_INVALID);
      ifpscr.i.lo = (ifpscr.i.lo & (~mask)) | (*flagp & mask);
      if (ifpscr.i.lo & FE_ALL_EXCEPT)
         ifpscr.i.lo |= FE_SET_FX;
      else
         ifpscr.i.lo &= FE_CLR_FX;
      FESETENVD_GRP( ifpscr.d );
   }

	return 0;
}

#if defined(BUILDING_FOR_CARBONCORE_LEGACY)

/* default environment object        */
const fenv_t _FE_DFL_ENV = (const fenv_t) 0L;

/****************************************************************************
   the "feclearexcept" function clears the exceptions represented by its
   argument. 
****************************************************************************/

int feclearexcept ( int excepts )
{
   hexdouble ifpscr;
   uint32_t mask;
   
   mask = excepts & FE_ALL_EXCEPT; 
   if (( excepts & FE_INVALID) != 0 ) mask |= FE_ALL_INVALID;     
   FEGETENVD_GRP( ifpscr.d );
   mask = ~mask;
   //if (( excepts & FE_INVALID) != 0 ) mask &= FE_NO_INVALID;
   ifpscr.i.lo &= mask;
   if (( ifpscr.i.lo & FE_ALL_EXCEPT ) == 0)
      ifpscr.i.lo &= FE_CLR_FX;
   FESETENVD_GRP( ifpscr.d );
   return 0;
}

/****************************************************************************
   the "feraiseexcept" function raises the exceptions represented by its
   argument.
****************************************************************************/

int feraiseexcept ( int excepts )
{
   uint32_t mask;
   hexdouble ifpscr;
   
   mask = excepts & FE_ALL_EXCEPT;
   
   FEGETENVD_GRP( ifpscr.d );
   
   if ((mask & FE_INVALID) != 0)
   {
       ifpscr.i.lo |= SET_INVALID;
       mask &= ~FE_INVALID;
   }
   
   if (( mask & ( FE_OVERFLOW | FE_UNDERFLOW | FE_DIVBYZERO | FE_INEXACT )) != 0)
       ifpscr.i.lo |= mask;
   
   FESETENVD_GRP( ifpscr.d );
	return 0;
}

/****************************************************************************
   The function "fetestexcept" determines which of the specified subset of
   the exception flags are currently set.  It returns the bitwise OR of a
   subset of the exception macros included in "excepts".
****************************************************************************/

int fetestexcept ( int excepts )
{
   hexdouble temp;
   
   FEGETENVD_GRP( temp.d );
   return (int) ((excepts & FE_ALL_EXCEPT) & temp.i.lo);
}


/*  The following functions provide control of rounding direction modes. */

/****************************************************************************
   The function "fegetround" returns the value of the rounding direction
   macro which represents the current rounding direction.
****************************************************************************/
   
int fegetround ( void )
{
   hexdouble temp;
   
   FEGETENVD_GRP( temp.d );
   return (int) (temp.i.lo & FE_ALL_RND);
}


/****************************************************************************
  The function "fesetround" establishes the rounding direction
   represented by its argument.  It returns zero if and only if
   the argument matches a rounding direction macro.  If not, the 
   rounding direction is not changed.
****************************************************************************/

int fesetround ( int round )
{   
   if ((round & FE_NO_RND))
       return 1;
   else
   {
       hexdouble temp;
        
       FEGETENVD_GRP( temp.d );
       temp.i.lo = (temp.i.lo & FE_NO_RND) | round;
       FESETENVD_GRP( temp.d );
       return 0;
   }
}


/*    The following functions manage the floating-point environment---
      exception flags and dynamic modes---as one entity.                  */

/****************************************************************************
   The function "fgetenv" stores the current environment in the
   object pointed to by its pointer argument "envp".
****************************************************************************/
   
int fegetenv ( fenv_t *envp )
{
   hexdouble temp;
   
   FEGETENVD_GRP( temp.d );
   *envp = temp.i.lo;
	return 0;
}


/****************************************************************************
   The function "feholdexcept" saves the current environment in
   the object pointed to by its argument "envp" and clears the
   exception flags.  It returns zero.  This function supersedes
   the SANE function "procentry".
****************************************************************************/
   
int feholdexcept ( fenv_t *envp )
{
   hexdouble ifpscr;
   
   FEGETENVD_GRP( ifpscr.d );
   *envp = ifpscr.i.lo;
   ifpscr.i.lo &= (FE_NO_FLAGS & FE_NO_ENABLES);
   FESETENVD_GRP( ifpscr.d );
   return 0;
}


/****************************************************************************
   The function "fesetenv" establishes the floating-point environment
   represented by the object pointed to by its argument "envp".  The
   value of "*env_p" must be set by a call to "fegetenv" or
   "feholdexcept", by the macro "FE_DFL_ENV", or by an implementation-
   defined macro of type "fenv_t".
****************************************************************************/
   
int fesetenv ( const fenv_t *envp )
{
   hexdouble temp;
   
   temp.i.lo = *envp;
   FESETENVD_GRP( temp.d );

	return 0;
}


/****************************************************************************
   The function "feupdateenv" saves the current exceptions in its
   automatic storage, installs the environment pointed to by its
   pointer argument "envp", and then re-raises the saved exceptions.
   This function, which supersedes the SANE function "procexit", can
   be used in conjunction with "feholdexcept" to write routines which
   hide spurious exceptions from their callers.
****************************************************************************/
   
int feupdateenv ( const fenv_t *envp )
{
   int newexc;
   hexdouble temp;
   
   FEGETENVD_GRP( temp.d );
   newexc = temp.i.lo & FE_ALL_EXCEPT;
   temp.i.lo = *envp;
   FESETENVD_GRP( temp.d );
   feraiseexcept(newexc);
	return 0;
}

/* Legacy entry point */
void fegetexcept ( fexcept_t *flagp, int excepts )
{
    _fegetexceptflag (flagp, excepts );
}

/* Legacy entry point */
void fesetexcept ( fexcept_t *flagp, int excepts )
{
    _fesetexceptflag ( flagp, excepts );
}

#else /* !BUILDING_FOR_CARBONCORE_LEGACY */

/****************************************************************************
   "fegetexceptflag" stores a representation of the exception flags indicated by
   the argument "excepts" through the pointer argument "flagp".
****************************************************************************/

int fegetexceptflag ( fexcept_t *flagp, int excepts )
{
   return _fegetexceptflag (flagp, excepts );
}
      

/****************************************************************************
   "fesetexceptflag" sets the exception flags indicated by the argument "excepts",
   to the corresponding state represented in the object pointed to by "flagp".
   This function does not raise exceptions, but only sets the state of the
   flags.
****************************************************************************/

int fesetexceptflag ( const fexcept_t *flagp, int excepts )
{
   return _fesetexceptflag ( flagp, excepts );
}

/****************************************************************************
    The float.h macro FLT_ROUNDS has a value derived from fegetround() --
    Addition rounds to 0: zero, 1: nearest, 2: +inf, 3: -inf, -1: unknown 
****************************************************************************/

int __fegetfltrounds( void ) 
{
    switch ( fegetround() )
    {
    case FE_TONEAREST:
        return 1;
    case FE_TOWARDZERO:
        return 0;
    case FE_UPWARD:
        return 2;
    case FE_DOWNWARD:
        return 3;
    default:
        return -1;
    }
}

#endif /* !BUILDING_FOR_CARBONCORE_LEGACY */
