
/*
 *  frexp.s
 *
 *      by Ian Ollmann
 *
 *  Copyright © 2007 Apple Inc. All Rights Reserved.
 */
 
#define LOCAL_STACK_SIZE	(32 - FRAME_SIZE)

#include "machine/asm.h"
#include "abi.h"

#if defined( __LP64__ )
    #define RESULT_P       %rdi
#else
    #define RESULT_P       %edx
#endif

ENTRY( frexpf )
    //Fetch the arguments
#if defined( __i386__ )
    movl    FRAME_SIZE( STACKP ),           %eax
    movl    4+FRAME_SIZE( STACKP ),         RESULT_P
#else
    movd    %xmm0,                          %eax
#endif

    movl    %eax,                           %ecx        //  x
    andl    $0x7f800000,                    %eax        //  biased exponent
    addl    $0x00800000,                    %eax        //  add one to exponent
    cmpl    $0x00800000,                    %eax        // if( isdenormal(x) || isinf(x) || isnan(x) )
    jle     1f                                          //      goto 1

    andl    $0x807fffff,                    %ecx        //  sign + significand
    sarl    $23,                            %eax        //  shift exponent to units position
    orl     $0x3f000000,                    %ecx        //  set return exponent to -1
    subl    $127,                           %eax        //  remove bias
    
    //return result
#if defined( __i386__ )
    movl    %ecx,                           FRAME_SIZE( STACKP )
    flds    FRAME_SIZE( STACKP )
#else
    movd    %ecx,                           %xmm0
#endif
    movl    %eax,                           (RESULT_P)  //  store exponent
    ret

//  0, denorm, nan, inf
1:  jl      3f                                          // NaN, inf goto 3

    // 0 or denor
    movl    %ecx,                           %eax        // x
    andl    $0x7fffffff,                    %ecx        // |x|
    jz      3f                                          // if the value is zero, goto 3

    // denormal
    bsrl    %ecx,                           %ecx       // count leading zeros
    subl    $23,                            %ecx       // correct for the backwards bit numbering scheme
    negl    %ecx                                       // fix sign
    shl     %cl,                            %eax       // move high bit to lowest exponent bit
    addl    $125,                           %ecx       // find exponent
    andl    $0x007fffff,                    %eax       // trim high bit
    negl    %ecx
    movl    %ecx,                           (RESULT_P) // write out exponent -- we need the register

#if defined( __i386__ )
    movl    FRAME_SIZE( STACKP ),           %ecx       // we lost the sign bit along the way. Get it back
#else
    movd    %xmm0,                          %ecx       // we lost the sign bit along the way. Get it back
#endif
    orl     $0x3f000000,                    %eax
    andl    $0x80000000,                    %ecx
    orl     %ecx,                           %eax

#if defined( __i386__ )
    movl    %eax,                           FRAME_SIZE( STACKP )
    flds    FRAME_SIZE(STACKP)
#else
    movd    %eax,                           %xmm0
#endif
    ret
    
3:  // 0, inf, NaN
#if defined( __i386__ )
    flds    FRAME_SIZE(STACKP)
#endif
    movl    $0,                             (RESULT_P)
    ret



ENTRY( frexp )
    SUBP    $LOCAL_STACK_SIZE,  STACKP
    
//Load exponent of argument and result pointer
#if defined( __i386__ )
    movl    4+FIRST_ARG_OFFSET(STACKP),     %eax
    movl    8+FIRST_ARG_OFFSET(STACKP),     RESULT_P
#else
    movd    %xmm0,                          %rax
    shrq    $32,                            %rax
#endif    
    
    //make a copy of mantissa top bits and sign
    movl    %eax,                           %ecx
    
    //expose the exponent
    andl    $0x7FF00000,                    %eax
    
    //extract sign + mantissa high bits
    andl    $0x800FFFFF,                    %ecx

    //add 1 to the exponent 
    addl    $0x00100000,                    %eax
    
    //or in -1 + bias as the new exponent for the mantissa
    orl     $0x3fe00000,                    %ecx

    //test to see if we are in an exceptional case.  (0x00800000 -> 0 or denormal, 0x80000000 -> NaN or Inf)
    cmpl    $0x00100000,                    %eax
    jle     1f
    
    //Merge high and low mantissa parts and move to return register
    movd    %ecx,                           %xmm1
#if defined( __i386__ )
    movd    FIRST_ARG_OFFSET(STACKP),       %xmm0
#endif
    punpckldq %xmm1,                         %xmm0
#if defined( __i386__ )
    movq    %xmm0,                          (STACKP)
    fldl    (STACKP)
#endif

    //move exponent to units precision, and store
    sarl    $20,                            %eax
    addl    $-1023,                         %eax            //remove bias
    movl    %eax,                           (RESULT_P)

    //exit
    ADDP    $LOCAL_STACK_SIZE,   STACKP
    ret

    //special case code
1:  je      3f                              //handle zeros and denormals in 3

2:  movl    $0,                             (RESULT_P)
  //Infs, zeros and NaNs -- return input value
#if defined( __i386__ )
    fldl    FIRST_ARG_OFFSET( STACKP )
#endif
    ADDP    $LOCAL_STACK_SIZE,   STACKP
    ret

//zeros and denormals
3:  
#if defined( __i386__ )
	fldl    FIRST_ARG_OFFSET( STACKP )       // { 0 or denormal }
    fldz                                    //  { 0, 0 or denormal }
    fucomip %st(1), %st(0)                  //  { 0 or denormal }
    fstpt   (STACKP)                        //  save out in 80-bit format (normalizes denorms), and empty stack
    je      2b

    movsxw  8(STACKP),      %eax            //  load sign + exponent
    movl    %eax,           %ecx            //  copy to ecx
    andl    $0x7fff,        %eax            //  extract exponent
    andl    $0x8000,        %ecx            //  extract sign
    addl    $-16382,        %eax            //  subtract bias from exponent
    orl     $0x3ffe,        %ecx            //  copy sign bit to a new exponent of -1
    movl    %eax,           (RESULT_P)      //  store our exponent to the result pointer
    movw    %cx,            8(STACKP)       //  write the other exponent back to the 80-bit FP value
    fldt    (STACKP)                        //  load the manittsa in

    ADDP    $LOCAL_STACK_SIZE,   STACKP
    ret

#else
	xorpd	%xmm1,			%xmm1
	ucomisd	%xmm0,			%xmm1
	je		2b
	
	// read value from xmm
	movd	%xmm0,			%rax			
	movq	%rax,			%rcx			//set aside sign for later
	
	//take absolute value
	shlq	$1,				%rax
	shrq	$1,				%rax
	
	//save sign bit for later
	xorq	%rax,			%rcx			
	
	//read |x| as integer -- multiplies denormal by 2**(1022+52) to get normal number
	cvtsi2sd	%rax,		%xmm1
	movd		%xmm1,		%rax

	//prepare an exponent mask
	movq	$0x7ff,			%rdx
	shlq	$52,			%rdx			// 0x7ff0000000000000
	
	//extract exponent and mantissa
	andq	%rax,			%rdx			// exponent
	xorq	%rdx,			%rax			// mantissa
	
	//reduce exponent to units precision
	shrq	$52,			%rdx
	
	//or in sign bit to mantissa
	orq		%rcx,			%rax

	//prepare 0.5
	movq	$0x3fe,			%rcx
	shlq	$52,			%rcx			// 0x3fe0000000000000
	
	//subtract out the bias in the exponent
	subq	$(1022+52+1022),	%rdx
	
	//set mantissa exponent to 2**-1
	orq		%rcx,			%rax
	
	//write out the exponent
	movl	%edx,			(RESULT_P)
	
	//move result to destination register
	movd	%rax,			%xmm0
	
    ADDP    $LOCAL_STACK_SIZE,   STACKP
	ret
#endif

    

ENTRY( frexpl )
    SUBP    $LOCAL_STACK_SIZE,  STACKP
    
    //Load the sign + exponent
    movsxw  8+FIRST_ARG_OFFSET(STACKP),     %eax
    movl    %eax,                           %ecx

    //Load result pointer if necessary
#if defined( __i386__ )
    movl    SECOND_ARG_OFFSET(STACKP),      RESULT_P
#endif
    //Copy value to stack
    fldt    FIRST_ARG_OFFSET(STACKP)
    fstpt   (STACKP)

    //extract exponent
    andl    $0x7fff,                        %eax
    addl    $1,                             %eax
    cmpw    $1,                             %ax
    jle     1f                              //special case code for zero, nan, denormal, inf
    addl    $-16383,                        %eax
    movl    %eax,                           (RESULT_P)

    //prepare new exponent for mantissa
    andl    $0x8000,                        %ecx
    orl     $0x3ffe,                        %ecx
    movw    %cx,                            8(STACKP)
    fldt    (STACKP)
    
    //exit
    ADDP    $LOCAL_STACK_SIZE,   STACKP
    ret

1:  fldt    FIRST_ARG_OFFSET(STACKP)
    je      3f                  //denormals and zeros are handled elsewhere

2:  movl    $0,                             (RESULT_P)
    ADDP    $LOCAL_STACK_SIZE,   STACKP
    ret

//handle zero case
3:  fldz
    fucomip %st(1), %st(0)
    je      2b                  
    fstp    %st(0)

    //we have a denormal. Load the mantissa as an integer, and convert to normal floating point number.
    fildll  (STACKP)                            // {|f| * 2**(16382+63)}    //note: fails for malformed normals that have 0 exponent, but leading bit set. However, since this is a stack copy of the data, we naively hope that the hardware took care of that.
    fld     %st(0)                              // {|f| * 2**(16382+63), |f| * 2**(16382+63)}
    fchs                                        // {-|f| * 2**(16382+63), |f| * 2**(16382+63)}
    fcmovb  %st(1),         %st(0)              // { f * 2**(16382+63), |f| * 2**(16382+63) }
    fstpt   (STACKP)                            // { |f| * 2**(16382+63) }
    fstp    %st(0)                              // {}
    
//repeat frexpl with new bias
    //Load the sign + exponent
    movsxw  8(STACKP),                      %eax
    movl    %eax,                           %ecx
    
    //extract exponent
    andl    $0x7fff,                        %eax
    addl    $(-16383-16382-63+1),           %eax
    movl    %eax,                           (RESULT_P)
    
    //prepare new exponent for mantissa
    andl    $0x8000,                        %ecx
    orl     $0x3ffe,                        %ecx
    movw    %cx,                            8(STACKP)
    fldt    (STACKP)

    //exit
    ADDP    $LOCAL_STACK_SIZE,   STACKP
    ret
