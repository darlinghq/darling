/*
 *  fenv.c
 *  xmmLibm
 *
 *  Created by iano on 6/21/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "xmmLibm_prefix.h"

#include "fenv.h"
#include <xmmintrin.h>

#define DEFAULT_CONTROL 0x037f
#define DEFAULT_STATUS  0x0
#define DEFAULT_RESERVED "\0\0\0\0\0\0\0"

#define GET_FSW()      ({ unsigned short _result; asm volatile ("fnstsw %0" : "=m" (_result)::"memory"); /*return*/ _result; })
#define GET_FCW()      ({ unsigned short _result; asm volatile ("fnstcw %0" : "=m" (_result)::"memory"); /*return*/ _result; })
#define SET_FCW(_a)    { unsigned short _aa = _a; asm volatile ("fldcw %0" : :"m" (_aa)); }

typedef struct {
    unsigned short __control;
    unsigned short __reserved1;
    unsigned short __status;
    unsigned short __reserved2;
    unsigned int __private3;
    unsigned int __private4;
    unsigned int __private5;
    unsigned int __private6;
    unsigned int __private7;
} __fpustate_t;

#define FE_ALL_RND  ( FE_TONEAREST | FE_TOWARDZERO | FE_UPWARD | FE_DOWNWARD )

static inline int _fesetexceptflag(const fexcept_t *flagp, int excepts ) ALWAYS_INLINE;
static inline int _fesetexceptflag(const fexcept_t *flagp, int excepts )
{
    int state;
    __fpustate_t currfpu;
    unsigned int mxcsr;
    unsigned int exceptMask = excepts & FE_ALL_EXCEPT;
    unsigned int andMask = ~exceptMask;                     // clear just the bits indicated
    unsigned int orMask =  *flagp & exceptMask;             // latch the specified bits
    
    //read the state
    mxcsr = _mm_getcsr();                                   //read the MXCSR state
    asm volatile ("fnstenv %0" : "=m" (currfpu) );          //read x87 state

    //fix up the MXCSR state
    mxcsr &= andMask;
    mxcsr |= orMask;

    //fix up the x87 state
    state = currfpu.__status;
    state &= andMask; 	   
    state |= orMask; 
    currfpu.__status = state; 

    //store the state
    asm volatile ("ldmxcsr %0 ; fldenv %1" : : "m" (mxcsr), "m" (currfpu));
	return 0;
}

static inline int _fegetexceptflag(fexcept_t *flagp, int excepts);
static inline int _fegetexceptflag(fexcept_t *flagp, int excepts)
{
    fexcept_t fsw = GET_FSW();              //get the x87 status word
    unsigned int mxcsr = _mm_getcsr();      //get the mxcsr
    fexcept_t result = mxcsr | fsw;

    result &= excepts & FE_ALL_EXCEPT;
    
    *flagp = result;
	return 0;
}


#if defined( BUILDING_FOR_CARBONCORE_LEGACY )

const fenv_t _FE_DFL_ENV                        = { DEFAULT_CONTROL, 
                                                    DEFAULT_STATUS, 
													DEFAULT_MXCSR, 
                                                    DEFAULT_RESERVED};
                                                    
const fenv_t _FE_DFL_DISABLE_SSE_DENORMS_ENV    = { DEFAULT_CONTROL, 
                                                    DEFAULT_STATUS, 
													DEFAULT_MXCSR | 0x8040, 
                                                    DEFAULT_RESERVED };


/*******************************************************************************
*     The function "feclearexcept" clears the supported floating point         *
*     exceptions represented by its argument.                                  *
*******************************************************************************/

int  feclearexcept(int excepts)
{
    fexcept_t zero = 0;
    return _fesetexceptflag( &zero, excepts );
}



/*******************************************************************************
*     The function "feraiseexcept" raises the supported floating-point         *
*     exceptions represented by its argument. The order in which these         *
*     floating-point exceptions are raised is unspecified.                     *
*******************************************************************************/

int  feraiseexcept(int excepts)
{
    fexcept_t t = excepts;
    
    int err = _fesetexceptflag ( &t, excepts );
    asm volatile ("fwait" :"=X" (t)::"memory"); 			// and raise the exception(s)
	return err;
}






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

int  fetestexcept(int excepts )
{
    fexcept_t fsw = GET_FSW();              //get the x87 status word
    unsigned int mxcsr = _mm_getcsr();      //get the mxcsr
    unsigned int exceptMask = excepts & FE_ALL_EXCEPT;

    mxcsr |= fsw;
    mxcsr &= exceptMask;
    
    return mxcsr;
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
    int fcw = GET_FCW();

    return (fcw & FE_ALL_RND);
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
    if ((round & ~FE_ALL_RND))
        return 1;
    else
    {
        unsigned short fcw = GET_FCW();
        int mxcsr = _mm_getcsr();
        
        fcw &= ~FE_ALL_RND;
        fcw |= round;
        mxcsr &= ~( FE_ALL_RND << 3 );
        mxcsr |= round << 3;
    
        _mm_setcsr( mxcsr );
        SET_FCW( fcw );

        return 0;
    }
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
    __fpustate_t currfpu;
    int mxcsr = _mm_getcsr();
    
    asm volatile ("fnstenv %0" : "=m" (currfpu) :: "memory");
    
    envp->__control = currfpu.__control;
    envp->__status = currfpu.__status;
    envp->__mxcsr = mxcsr;
	((int*) envp->__reserved)[0] = 0;
	((int*) envp->__reserved)[1] = 0;
	   
	// fnstenv masks floating-point exceptions.  We restore the state here
	// in case any exceptions were originally unmasked.
	asm volatile ("fldenv %0" : : "m" (currfpu));
	
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
    __fpustate_t currfpu;
    int mxcsr;
    
    mxcsr = _mm_getcsr();
	asm volatile ("fnstenv %0" : "=m" (*&currfpu) :: "memory");
    
    envp->__control = currfpu.__control;
    envp->__status = currfpu.__status;
    envp->__mxcsr = mxcsr;
	((int*) envp->__reserved)[0] = 0;
	((int*) envp->__reserved)[1] = 0;
	
    currfpu.__control |= FE_ALL_EXCEPT; // FPU shall handle all exceptions
	currfpu.__status &= ~FE_ALL_EXCEPT;
    mxcsr |= FE_ALL_EXCEPT << 7;  // left shifted because control mask is <<7 of the flags
	mxcsr &= ~FE_ALL_EXCEPT;

    asm volatile ("ldmxcsr %0 ; fldenv %1" : : "m" (*&mxcsr), "m" (*&currfpu));
    
    return 0;
}

#define MXCSR_PLUS_FZ_DAZ	( DEFAULT_MXCSR | 0x8040 ) 


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
int  fesetenv(const fenv_t *envp)
{
    __fpustate_t currfpu;
    asm volatile ("fnstenv %0" : "=m" (currfpu));
    
    currfpu.__control = envp->__control;
    currfpu.__status = envp->__status;
    
    asm volatile ("ldmxcsr %0 ; fldenv %1" : : "m" (envp->__mxcsr), "m" (currfpu));
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
*    Please see the description of feholdexcept for additional ways to create  *
*    a fenv_t object, which are valid only for MacOS X for Intel.              *
*******************************************************************************/
int  feupdateenv(const fenv_t *envp)
{
    __fpustate_t currfpu;
    asm volatile ("fnstenv %0" : "=m" (currfpu));
    
    currfpu.__control = envp->__control;
    currfpu.__status = envp->__status;
    
    asm volatile ("ldmxcsr %0 ; fldenv %1; fwait " : : "m" (envp->__mxcsr), "m" (currfpu));
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

#else

/*******************************************************************************
*    The function "fegetexceptflag" stores a implementation-defined            *
*    representation of the states of the floating-point status flags indicated *
*    by its integer argument excepts in the object pointed to by the argument, * 
*    flagp.                                                                    *
*******************************************************************************/

int  fegetexceptflag(fexcept_t *flagp, int excepts)
{
	return _fegetexceptflag( flagp, excepts );
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
	return _fesetexceptflag( flagp, excepts );
}

int __fegetfltrounds( void );
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

#endif

