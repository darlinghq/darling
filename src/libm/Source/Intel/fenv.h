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

#if defined( __ppc__ ) || defined( __ppc64__ )
    #error  Wrong arch. This is Intel only.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
    A collection of functions designed to provide access to the floating
    point environment for numerical programming. It is compliant with
    the floating-point requirements in C99.
    
    Earlier versions of fenv.h were merely "modeled after" C9X. Many of the functions 
    that formerly returned ints now return void to be standard compliant.
    
    Note: There are actually two physical floating point environments on x86. There
    is the one described by the x87 floating point control and status words, which applies 
    primarily to calculations done with long double on MacOS X for Intel. There is the 
    MXCSR which applies primarily to calculations done with scalar float, scalar double 
    and SSE/SSE2/SSE3. The high level interface, which uses FE_ macros as int arguments 
    to configure the fexcep_t, returns and works with values that represents the logical 
    OR of these two sets of flags or masks. That is, if a flag or mask is set in either 
    environment, it will be set in fexcept_t when the state is read. Likewise, setting 
    the mask using a fexcep_t will set that mask on both environments. For this reason, 
    changing the value of the MXCSR state or floating point control/status word state on 
    your own will make the results of the functions declared in this header undefined. 
    See below for details about how and when. Exception: you may change the FZ, DAZ, DE 
    and DM bits in the MXCSR independent of this interface and retain defined behavior, 
    so long as you do not change the other bits. It is suggested that developers who wish 
    this level of control access the bits in the fenv_t directly. They are direct copies 
    of the hardware special purpose registers of similar name. Please consult appropriate 
    Intel documentation for the processor about the meaning of various bits in each register. 
        
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
typedef struct {
    unsigned short  __control;      /* A direct copy of the floaing point control word */
    unsigned short  __status;       /* A direct copy of the floaing point status word */
    unsigned int    __mxcsr;        /* A direct copy of the MXCSR */
    char            __reserved[8];  /* Reserved for future expansion. */   
} fenv_t;

typedef unsigned short fexcept_t;

/*    Definitions of floating-point exception macros                          */
#define FE_INEXACT          0x0020
#define FE_UNDERFLOW        0x0010
#define FE_OVERFLOW         0x0008
#define FE_DIVBYZERO        0x0004
#define FE_INVALID          0x0001
#define FE_ALL_EXCEPT       0x003D

/*    Definitions of rounding direction macros                                */
#define FE_TONEAREST        0x0000
#define FE_DOWNWARD         0x0400
#define FE_UPWARD           0x0800
#define FE_TOWARDZERO       0x0C00

/* default environment object        */
extern const fenv_t _FE_DFL_ENV;
#define FE_DFL_ENV &_FE_DFL_ENV          /* pointer to default environment    */

/*******************************************************************************
* A environment object that sets to defualt settings and in addition sets the  *
* FZ and DAZ bits in the MXCSR, which causes flush-to-zero behavior of         *
* denormals. When using this environment, denormals encountered by XMM based   *
* calculation (which normally should be all single and double precision scalar *
* floating point calculations, and all SSE/SSE2/SSE3 computation) will be      *
* treated as zero. Calculation results that are denormals will also be         *
* truncated to zero. This calculation mode is not IEEE-754 compliant, but may  *
* prevent lengthy stalls that occur in code that encounters denormals. It is   *
* suggested that you do not use this mode unless you have established that     *
* denormals are causing trouble for your code. Please use wisely.              *
*                                                                              *
* CAUTION: The math library currently is not architected to do the right       *
* thing in the face of DAZ + FZ mode.  For example, ceil( +denormal) returns   *
* +denormal rather than 1.0 in some versions of MacOS X. In some circumstances *
* this may lead to unexpected application behavior. Use at your own risk.      *
*                                                                              *
* It is not possible to disable denorm stalls on calculation using the x87 FPU.*
*******************************************************************************/
extern const fenv_t _FE_DFL_DISABLE_SSE_DENORMS_ENV;
#define FE_DFL_DISABLE_SSE_DENORMS_ENV  &_FE_DFL_DISABLE_SSE_DENORMS_ENV

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
*     On MacOS X for Intel, the result is the value of union of the            *
*     corresponding result from the x87 and SSE floating point states.         *
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
*                                                                              *
*    On MacOS X for Intel you may test and set the bits in *envp yourself,     *
*    provided that you conditionalize the code appropriately to preserve       *
*    portability and you follow the various strictures and suggestions         *
*    provided by Intel in appropriate processor documentation. Please be aware *
*    that because there are two hardware locations for setting and reading     *
*    floating point environment, this function (and others like it) are not    *
*    atomic -- that is, for a brief period of time during the function call    *
*    your new environment will have been applied to one but not both of the    *
*    floating point engines (x87 and SSE). In addition, the behavior of some   *
*    higher level interfaces (fegetround) is undefined if the x87 and SSE      *
*    floating point units rounding modes are configured differently.  Please   *
*    use common sense.                                                         *
*******************************************************************************/
extern int  fesetenv(const fenv_t * /*envp*/);

/*******************************************************************************
*    The feupdateenv function saves the currently raised floating-point        *
*    exceptions in its automatic storage, installs the floating-point          *
*    environment represented by the object pointed to by envp, and then raises *
*    the saved floating-point exceptions. The argument envp shall point to an  *
*    object set by a call to feholdexcept or fegetenv or equal a               *
*    floating-point environment macro.                                         *
*                                                                              *
*    Please see the description of feholdexcept for additional ways to create  *
*    a fenv_t object, which are valid only for MacOS X for Intel.              *
*******************************************************************************/
extern int  feupdateenv(const fenv_t * /*envp*/);


#ifdef __cplusplus
}
#endif

#endif /* __FENV__ */

