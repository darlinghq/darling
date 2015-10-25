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
*     File:  fenv.h                                                            *
*                                                                              *
*     Contains: typedefs and prototypes for C99 floating point environment.    *
*                                                                              *
*******************************************************************************/

#ifndef __FENV__
#define __FENV__

/*  We require VFP for this set of interfaces to work  */
#if !defined(__VFP_FP__) || defined(__SOFTFP__)
	#warning The <fenv.h> functions are not supported on platforms that do not have hardware floating-point.
#else

#ifdef __cplusplus
extern "C" {
#endif

/*
    A collection of functions designed to provide access to the floating
    point environment for numerical programming. It is compliant with
    the floating-point requirements in C99.
            
    The file <fenv.h> declares many functions in support of numerical
    programming. Programs that test flags or run under
    non-default modes must do so under the effect of an enabling
    "fenv_access" pragma:

    #pragma STDC FENV_ACCESS on
 
    Note that prior to iPhone OS 2.0, these interfaces did nothing.
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



typedef struct {
    union
    {
        struct
        {
            unsigned int            __fpscr;    
            unsigned int            __reserved0;
            unsigned int            __reserved1;
            unsigned int            __reserved2;
        };
#if defined( __GNUC__ )
        struct
        {
            unsigned int        __fpscr_cmp_n : 1;
            unsigned int        __fpscr_cmp_z : 1;
            unsigned int        __fpscr_cmp_c : 1;
            unsigned int        __fpscr_cmp_v : 1;
            unsigned int        __fpscr_do_not_modify_1 : 2;            /* Should be zero */
            unsigned int        __fpscr_default_nan_mode : 1;
            unsigned int        __fpscr_flush_to_zero : 1;
            unsigned int        __fpscr_rounding_mode : 2;
            unsigned int        __fpscr_stride : 2;
            unsigned int        __fpscr_do_not_modify_2 : 1;            /* Should be zero */
            unsigned int        __fpscr_len : 3 ;
            unsigned int        __fpscr_trap_enable_subnormal : 1 ;     /* Note: we run under "Run Fast Mode". Setting this bit has undefined results. */
            unsigned int        __fpscr_do_not_modfify_3 : 2;           /* Should be zero */
            unsigned int        __fpscr_trap_enable_inexact : 1;        /* Note: we run under "Run Fast Mode". Setting this bit has undefined results. */
            unsigned int        __fpscr_trap_enable_underflow : 1;      /* Note: we run under "Run Fast Mode". Setting this bit has undefined results. */
            unsigned int        __fpscr_trap_enable_overflow : 1;       /* Note: we run under "Run Fast Mode". Setting this bit has undefined results. */
            unsigned int        __fpscr_trap_enable_div_by_zero : 1;    /* Note: we run under "Run Fast Mode". Setting this bit has undefined results. */
            unsigned int        __fpscr_trap_enable_invalid : 1;        /* Note: we run under "Run Fast Mode". Setting this bit has undefined results. */
            unsigned int        __fpscr_fp_state_flag_subnormal : 1;
            unsigned int        __fpscr_do_not_modify_4 : 2;            /* Should be zero */
            unsigned int        __fpscr_fp_state_flag_inexact : 1;
            unsigned int        __fpscr_fp_state_flag_underflow : 1;
            unsigned int        __fpscr_fp_state_flag_overflow : 1;
            unsigned int        __fpscr_fp_state_flag_div_by_zero : 1;
            unsigned int        __fpscr_fp_state_flag_invalid : 1;
        } __attribute((packed));
#endif
    };
} fenv_t;

typedef unsigned short fexcept_t;

/*    Definitions of floating-point exception macros                          */
#define FE_INEXACT          0x0010
#define FE_UNDERFLOW        0x0008
#define FE_OVERFLOW         0x0004
#define FE_DIVBYZERO        0x0002
#define FE_INVALID          0x0001
#define FE_ALL_EXCEPT       0x001F

/*    Definitions of rounding direction macros                                */
#define FE_TONEAREST        0x00000000
#define FE_UPWARD           0x00400000
#define FE_DOWNWARD         0x00800000
#define FE_TOWARDZERO       0x00C00000

/* default environment object        */
extern const fenv_t _FE_DFL_ENV;
#define FE_DFL_ENV &_FE_DFL_ENV          /* pointer to default environment    */


/*******************************************************************************
*     The following functions provide high level access to the exception flags.*  
*     The "int" input argument can be constructed by bitwise ORs of the        *
*     exception macros: for example: FE_OVERFLOW | FE_INEXACT.                 *
*******************************************************************************/

/*******************************************************************************
*     The function "feclearexcept" clears the supported floating point         *
*     exceptions represented by its argument.                                  *
*******************************************************************************/

extern int  feclearexcept(int /*excepts*/);


/*******************************************************************************
*    The function "fegetexceptflag" stores a implementation-defined            *
*    representation of the states of the floating-point status flags indicated *
*    by its integer argument excepts in the object pointed to by the argument, * 
*    flagp.                                                                    *
*******************************************************************************/

extern int  fegetexceptflag(fexcept_t * /*flagp*/, int /*excepts*/);


/*******************************************************************************
*     The function "feraiseexcept" raises the supported floating-point         *
*     exceptions represented by its argument. The order in which these         *
*     floating-point exceptions are raised is unspecified.                     *
*******************************************************************************/

extern int  feraiseexcept(int /*excepts*/);


/*******************************************************************************
*     The function "fesetexceptflag" sets or clears the floating point status  *
*     flags indicated by the argument excepts to the states stored in the      *
*     object pointed to by flagp. The value of the *flagp shall have been set  *
*     by a previous call to fegetexceptflag whose second argument represented  *
*     at least those floating-point exceptions represented by the argument     *
*     excepts. This function does not raise floating-point exceptions; it just *
*     sets the state of the flags.                                             *
*******************************************************************************/

extern int  fesetexceptflag(const fexcept_t * /*flagp*/, int /*excepts*/);


/*******************************************************************************
*     The function "fetestexcept" determines which of the specified subset of  *
*     the floating-point exception flags are currently set.  The excepts       *
*     argument specifies the floating-point status flags to be queried. This   *
*     function returns the value of the bitwise OR of the floating-point       *
*     exception macros corresponding to the currently set floating-point       *
*     exceptions included in excepts.                                          *
*                                                                              *
*******************************************************************************/

extern int  fetestexcept(int /*excepts*/);


/*******************************************************************************
*     The following functions provide control of rounding direction modes.     *
*******************************************************************************/

/*******************************************************************************
*     The function "fegetround" returns the value of the rounding direction    *
*     macro which represents the current rounding direction, or a negative     *
*     if there is no such rounding direction macro or the current rounding     *
*     direction is not determinable.                                           *
*******************************************************************************/

extern int  fegetround(void);


/*******************************************************************************
*     The function "fesetround" establishes the rounding direction represented *
*     by its argument "round". If the argument is not equal to the value of a  *
*     rounding direction macro, the rounding direction is not changed.  It     *
*     returns zero if and only if the argument is equal to a rounding          *
*     direction macro.                                                         *
*******************************************************************************/

extern int  fesetround(int /*round*/);


/*******************************************************************************
*    The following functions manage the floating-point environment, exception  *
*    flags and dynamic modes, as one entity.                                   *
*******************************************************************************/

/*******************************************************************************
*    The fegetenv function stores the current floating-point enviornment in    *
*    the object pointed to by envp.                                            *
*******************************************************************************/
extern int  fegetenv(fenv_t * /*envp*/);

/*******************************************************************************
*    The feholdexcept function saves the current floating-point environment in *
*    the object pointed to by envp, clears the floating-point status flags,    *
*    and then installs a non-stop (continue on floating-point exceptions)      *
*    mode, if available, for all floating-point exceptions. The feholdexcept   *
*    function returns zero if and only if non-stop floating-point exceptions   *
*    handling was successfully installed.                                      *
*******************************************************************************/
extern int   feholdexcept(fenv_t * /*envp*/);

/*******************************************************************************
*    The fesetnv function establishes the floating-point environment           *
*    represented by the object pointed to by envp. The argument envp shall     *
*    point to an object set by a call to fegetenv or feholdexcept, or equal to *
*    a floating-point environment macro -- we define only *FE_DFL_ENV and      *
*    FE_DISABLE_SSE_DENORMS_ENV -- to be C99 standard compliant and portable   *
*    to other architectures. Note that fesetnv merely installs the state of    *
*    the floating-point status flags represented through its argument, and     *
*    does not raise these floating-point exceptions.                           *
*******************************************************************************/
extern int  fesetenv(const fenv_t * /*envp*/);

/*******************************************************************************
*    The feupdateenv function saves the currently raised floating-point        *
*    exceptions in its automatic storage, installs the floating-point          *
*    environment represented by the object pointed to by envp, and then raises *
*    the saved floating-point exceptions. The argument envp shall point to an  *
*    object set by a call to feholdexcept or fegetenv or equal a               *
*    floating-point environment macro.                                         *
*******************************************************************************/
extern int  feupdateenv(const fenv_t * /*envp*/);

#ifdef __cplusplus
}
#endif

#endif // HARDWARE FP

#endif /* __FENV__ */

