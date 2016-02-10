/*
 *  fenv.c
 *  cLibm
 *
 *  Created by Ian Ollmann on 7/15/07.
 *  Copyright 2007 Apple Inc. All rights reserved.
 *
 *	A partial implementation for Intel is provided here for testing purposes. 
 *	You'll need to provide a full implementation for your platform of choice.
 *
 */

#if defined(ARMLIBM_FENV_SUPPORT)

#include "fenv.h"
#include <stdint.h>
#include "required_arithmetic.h"

#define FE_ALL_RND  ( FE_TONEAREST | FE_TOWARDZERO | FE_UPWARD | FE_DOWNWARD )

#if defined( __GNUC__ )
	#define ALWAYS_INLINE __attribute__ ((__always_inline__))
#else
	#define ALWAYS_INLINE
#endif

#ifndef __arm__
	#error	This file is for use on ARM + VFP and later
#endif


#define GET_FPSCR()			({ uint32_t _fpscr; __asm__ __volatile__( "fmrx	%0, fpscr" : "=r" (_fpscr) ); /* return */ _fpscr; })
#define SET_FPSCR(_fpscr)	__asm__ __volatile__( "fmxr	fpscr, %0" : : "r" (_fpscr)  )


const fenv_t _FE_DFL_ENV = {{{ 0, 0, 0, 0 }}};
// Worker functions for making sure that much of the work is done in a uniform way

// For each bit set in excepts, change the floating point state to match the correspinding bit in *flagp
static inline int _fesetexceptflag_private(const fexcept_t *flagp, int excepts ) ALWAYS_INLINE;
static inline int _fesetexceptflag_private(const fexcept_t *flagp, int excepts )
{
	uint32_t fpscr = GET_FPSCR();
	
	excepts &=  FE_ALL_EXCEPT | 0x8000;
	
	fpscr &= ~excepts;
	fpscr |= *flagp & excepts;
	
	SET_FPSCR( fpscr );
	
	return 0;
}

// For each bit set in excepts, copy the corresponding bit from the floating point state into *flagp
//	All other bits shall be set to zero
static inline int _fegetexceptflag_private(fexcept_t *flagp, int excepts);
static inline int _fegetexceptflag_private(fexcept_t *flagp, int excepts)
{
	uint32_t fpscr = GET_FPSCR();
	
	*flagp = fpscr & excepts;
	
	return 0;
}

                                                    

/*******************************************************************************
*     The function "feclearexcept" clears the supported floating point         *
*     exceptions represented by its argument.                                  *
*******************************************************************************/

int  feclearexcept(int excepts)
{
    fexcept_t zero = 0;
    return _fesetexceptflag_private( &zero, excepts );
}



/*******************************************************************************
*     The function "feraiseexcept" raises the supported floating-point         *
*     exceptions represented by its argument. The order in which these         *
*     floating-point exceptions are raised is unspecified.                     *
*******************************************************************************/

int  feraiseexcept(int excepts)
{
	int inexact_set = 0;

	if( excepts & FE_OVERFLOW )
	{
		required_add_float( 0x1.0p127f, 0x1.0p127f );
		inexact_set = 1;
	}
	if( excepts & FE_UNDERFLOW )
	{
		required_multiply_float( 0x1.0p-126f, 0x1.0p-126f );
		inexact_set = 1;
	}
	if( excepts & FE_INVALID )
		required_add_float( __builtin_inff(), -__builtin_inff() );
	if( excepts & FE_DIVBYZERO )
		required_divide_float( 1.0f, 0.0f );
	if( 0 != (excepts & FE_INEXACT) && 0 == inexact_set )
		required_add_float( 0x1.0p127f, 1.0f );

	return 0;
}






/*******************************************************************************
*     The function "fetestexcept" determines which of the specified subset of  *
*     the floating-point exception flags are currently set.  The excepts       *
*     argument specifies the floating-point status flags to be queried. This   *
*     function returns the value of the bitwise OR of the floating-point       *
*     exception macros corresponding to the currently set floating-point       *
*     exceptions included in excepts.                                          *
*                                                                              *
*******************************************************************************/

int  fetestexcept(int excepts )
{
	fexcept_t	t = 0;

	_fegetexceptflag_private( &t, excepts);

	return t;
}


/*******************************************************************************
*     The following functions provide control of rounding direction modes.     *
*******************************************************************************/

/*******************************************************************************
*     The function "fegetround" returns the value of the rounding direction    *
*     macro which represents the current rounding direction, or a negative     *
*     if there is no such rounding direction macro or the current rounding     *
*     direction is not determinable.                                           *
*******************************************************************************/

int  fegetround(void)
{
	int32_t	fpscr = GET_FPSCR();
	
	return fpscr & FE_ALL_RND;
}


/*******************************************************************************
*     The function "fesetround" establishes the rounding direction represented *
*     by its argument "round". If the argument is not equal to the value of a  *
*     rounding direction macro, the rounding direction is not changed.  It     *
*     returns zero if and only if the argument is equal to a rounding          *
*     direction macro.                                                         *
*******************************************************************************/

int  fesetround(int round )
{
	if( (round & FE_ALL_RND) != round )
		return round;

	int32_t fpscr = GET_FPSCR();

	fpscr &= ~FE_ALL_RND;
	fpscr |= round & FE_ALL_RND;
	 
	SET_FPSCR( fpscr );
	
	return 0;
}


/*******************************************************************************
*    The following functions manage the floating-point environment, exception  *
*    flags and dynamic modes, as one entity.                                   *
*******************************************************************************/

/*******************************************************************************
*    The fegetenv function stores the current floating-point enviornment in    *
*    the object pointed to by envp.                                            *
*******************************************************************************/
int  fegetenv(fenv_t *envp)
{
	envp->__fpscr = GET_FPSCR();
	envp->__reserved0 = 0;
	envp->__reserved1 = 0;
	envp->__reserved2 = 0;

	return 0;
}

/*******************************************************************************
*    The feholdexcept function saves the current floating-point environment in *
*    the object pointed to by envp, clears the floating-point status flags,    *
*    and then installs a non-stop (continue on floating-point exceptions)      *
*    mode, if available, for all floating-point exceptions. The feholdexcept   *
*    function returns zero if and only if non-stop floating-point exceptions   *
*    handling was successfully installed.                                      *
*******************************************************************************/
int   feholdexcept(fenv_t *envp)
{
	uint32_t fpscr = GET_FPSCR();

	envp->__fpscr = fpscr;
	envp->__reserved0 = 0;
	envp->__reserved1 = 0;
	envp->__reserved2 = 0;

	fpscr &=  ~( FE_ALL_EXCEPT | (FE_ALL_EXCEPT << 8) );

	SET_FPSCR( fpscr );

	return 0;
}


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
*******************************************************************************/
int  fesetenv(const fenv_t *envp)
{
	SET_FPSCR( envp->__fpscr );

	return 0;
}



/*******************************************************************************
*    The feupdateenv function saves the currently raised floating-point        *
*    exceptions in its automatic storage, installs the floating-point          *
*    environment represented by the object pointed to by envp, and then raises *
*    the saved floating-point exceptions. The argument envp shall point to an  *
*    object set by a call to feholdexcept or fegetenv or equal a               *
*    floating-point environment macro.                                         *
*                                                                              *
*******************************************************************************/
int  feupdateenv(const fenv_t *envp)
{
 	uint32_t oldenv = GET_FPSCR();
	
	SET_FPSCR( envp->__fpscr );
	
	int inexact_set = 0;

	if( oldenv & FE_OVERFLOW )
	{
		required_add_float( 0x1.0p127f, 0x1.0p127f );
		inexact_set = 1;
	}
	if( oldenv & FE_UNDERFLOW )
	{
		required_multiply_float( 0x1.0p-126f, 0x1.0p-126f );
		inexact_set = 1;
	}
	if( oldenv & FE_INVALID )
		required_add_float( __builtin_inff(), -__builtin_inff() );
	if( oldenv & FE_DIVBYZERO )
		required_divide_float( 1.0f, 0.0f );
	if( 0 != (oldenv & FE_INEXACT) && 0 == inexact_set )
		required_add_float( 0x1.0p127f, 1.0f );

	return 0;
}


/*******************************************************************************
*    The function "fegetexceptflag" stores a implementation-defined            *
*    representation of the states of the floating-point status flags indicated *
*    by its integer argument excepts in the object pointed to by the argument, * 
*    flagp.                                                                    *
*******************************************************************************/

int  fegetexceptflag(fexcept_t *flagp, int excepts)
{
	return _fegetexceptflag_private( flagp, excepts );
}
      
/*******************************************************************************
*     The function "fesetexceptflag" sets or clears the floating point status  *
*     flags indicated by the argument excepts to the states stored in the      *
*     object pointed to by flagp. The value of the *flagp shall have been set  *
*     by a previous call to fegetexceptflag whose second argument represented  *
*     at least those floating-point exceptions represented by the argument     *
*     excepts. This function does not raise floating-point exceptions; it just *
*     sets the state of the flags.                                             *
*******************************************************************************/

int  fesetexceptflag(const fexcept_t *flagp, int excepts )
{
	return _fesetexceptflag_private( flagp, excepts );
}

#endif


