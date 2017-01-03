//  float nearbyintf(float x)
//
//  returns the argument rounded to an integral value using the prevailing
//  rounding mode, and without raising the inexact flag (C99 7.12.9.4).
//
//  -- Stephen Canon, January 2010

#include <System/i386/cpu_capabilities.h>

.text
.align 4
.globl _nearbyint
_nearbyint:

// Fast path if SSE4.1 is available ------------------------------------------

#if defined __i386__
    movsd      4(%esp),         %xmm0
    mov         _COMM_PAGE_CPU_CAPABILITIES,    %eax
    test        $(kHasSSE4_1),  %eax
    jz          L_noSSE4_1
    roundsd     $0xc,  %xmm0,   %xmm0
    movsd       %xmm0,         4(%esp)
    fldl       4(%esp)
    ret
#elif defined __x86_64__
    movabs      _COMM_PAGE_CPU_CAPABILITIES,    %eax
    test        $(kHasSSE4_1),  %eax
    jz          L_noSSE4_1
    roundsd     $0xc,  %xmm0,   %xmm0
    ret
#else
#error "This implementations supports i386 and x86_64 only"
#endif

// Fallback path if SSE4.1 is not available ----------------------------------
//
// FPSCR rounding control bits 11:10
//
//  00  --  Round to nearest even
//  01  --  Round to -Inf
//  10  --  Round to +Inf
//  11  --  Round to zero
//  
// This path is adapted from Ian Ollmann's original implementation

L_noSSE4_1:
#if defined __i386__
    mov        8(%esp),         %ecx    // high word of input
    sub         $4,             %esp    // make space for the fp control word
    #define     fpcw            (%esp)
    #define     floatval       8(%esp)  // address of argument to nearbyintf
#else
    movd        %xmm0,          %rcx
    shr         $32,            %rcx    // high word of input
    #define     fpcw          -4(%rsp)  // space for fp control word in red zone
#endif
    fnstcw      fpcw
    
    mov         %ecx,           %edx
    pcmpeqb     %xmm1,          %xmm1
    and         $0x7fffffff,    %ecx    // |x|
    psllq       $63,            %xmm1   // -0.0
    xor         %ecx,           %edx    // signbit of x
    movapd      %xmm1,          %xmm2
    sub         $0x3ff00000,    %ecx    // high word of |x| - exponent bias
    andpd       %xmm0,          %xmm2   // signbit of x
    cmp         $0x03400000,    %ecx    // if |x| < 1.0 or |x| >= 0x1.0p52 or isnan(x)
    jae         L_edgeCases             //    branch
    
    shr         $20,            %ecx    // exponent of x, in [0, 51]
    sub         $52,            %ecx
    neg         %ecx                    // number of fractional bits in x
    pcmpeqb     %xmm1,          %xmm1
    movd        %ecx,           %xmm7
    psllq       %xmm7,          %xmm1   // mask covering integral bits of x
    
L_reentryForSmallX:
    andpd       %xmm0,          %xmm1   // trunc(x)
    ucomisd     %xmm1,          %xmm0   // if x == trunc(x)
    je          L_roundToZero           //    return trunc(x)
    
    mov         fpcw,           %ecx
    and         $0xc00,         %ecx    // current rounding mode encoding
    sub         $0x400,         %ecx
    cmp         $0x400,         %ecx
    jg          L_roundToZero           // ecx = 0x800
    je          L_roundUp               // ecx = 0x400
    jb          L_roundDown             // ecx = 0x000
    
L_roundToNearest:                       // fall into default rounding path
    subsd       %xmm1,          %xmm0   // fractional part of x
    pcmpeqb     %xmm3,          %xmm3
    psllq       $55,            %xmm3
    psrlq       $2,             %xmm3   // 0.5
    xorpd       %xmm2,          %xmm0   // |fract(x)|
    ucomisd     %xmm3,          %xmm0   // if |fractional part| == 0.5
    je          L_exactHalfway          //    branch to handle exact halfway case
    
    cmpltsd     %xmm3,          %xmm0   // |fract(x)| < 0.5 ? -1 : 0
    addsd       %xmm3,          %xmm3   // 1.0
    orpd        %xmm2,          %xmm3   // copysign(1.0,x)
    andnpd      %xmm3,          %xmm0   // |fract(x)| < 0.5 ? 0.0 : copysign(1.0,x)
    addsd       %xmm1,          %xmm0   // correctly rounded result
    orpd        %xmm2,          %xmm0   // restore sign of zero
#if defined( __i386__ )
    movsd       %xmm0,          floatval
    fldl        floatval
    add         $4,             %esp
#endif
    ret
    
L_exactHalfway:
    addsd       %xmm3,          %xmm3   // 1.0
    pcmpeqb     %xmm4,          %xmm4
    psubq       %xmm4,          %xmm7
    psllq       %xmm7,          %xmm4   // mask covering signbit and bits larger than 2^1
    orpd        %xmm2,          %xmm3   // copysign(1.0, x)
    pandn       %xmm1,          %xmm4   // trunc(x) odd ? nonzero : zero
    xorpd       %xmm5,          %xmm5
    pcmpeqd     %xmm5,          %xmm4   // trunc(x) odd ? at least one dword is 0 : -1
    pshufd      $0xe1,  %xmm4,  %xmm5
    pand        %xmm5,          %xmm4   // trunc(x) odd ? 0 : -1
    andnpd      %xmm3,          %xmm4   // trunc(x) odd ? copysign(1.0,x), 0
    addsd       %xmm4,          %xmm1
    orpd        %xmm2,          %xmm1   // reapply sign of zero
    
L_roundToZero:
#if defined( __i386__ )
    movsd       %xmm1,          floatval
    fldl        floatval
    add         $4,             %esp
#else
    movapd      %xmm1,          %xmm0
#endif
    ret
    
L_roundUp:                              // we already know that x is not integral
    xorpd       %xmm3,          %xmm3
    cmpltsd     %xmm0,          %xmm3   // x > 0 ? -1 : 0
    cvtdq2pd    %xmm3,          %xmm3   // x > 0 ? -1.0 : 0.0
    subsd       %xmm3,          %xmm1   // x > 0 ? trunc(x) + 1.0 : trunc(x)
#if defined( __i386__ )
    movsd       %xmm1,          floatval
    fldl        floatval
    add         $4,             %esp
#else
    movapd      %xmm1,          %xmm0
#endif
    ret

L_roundDown:                            // we already know that x is not integral
    psrlq       $63,            %xmm0   // x < 0 ? 1 : 0
    cvtdq2pd    %xmm0,          %xmm3   // x < 0 ? 1.0 : 0.0
    subsd       %xmm3,          %xmm1   // x < 0 ? trunc(x) - 1.0 : trunc(x)
    pcmpeqb     %xmm0,          %xmm0
    psrlq       $1,             %xmm0   // 0x7fffffffffffffff
    orpd        %xmm2,          %xmm0   // x < 0 ? 0xffffffffffffffff : 0x7fffffffffffffff
    andpd       %xmm1,          %xmm0
#if defined( __i386__ )
    movsd       %xmm0,          floatval
    fldl        floatval
    add         $4,             %esp
#endif
    ret

L_edgeCases:                            // if |x| < 1.0, jump back to mainline
    xorpd       %xmm7,          %xmm7   // initialize data in xmm7
    jl          L_reentryForSmallX      // otherwise, we just return x
#if defined( __i386__ )
    movsd       %xmm0,          floatval
    fldl        floatval
    add         $4,             %esp
#endif
    ret

/*
7:  //Round to even nearest, half way case
    addsd   %xmm6,                  %xmm6       // 1.0
    pcmpeqb %xmm3,                  %xmm3       // -1ULL
    psubq   %xmm3,                  %xmm7       // add one to the truncation mask shift constant
    psllq   %xmm7,                  %xmm3       // prepare a new truncation mask with left edge past unit bit. (Works for 1.0 too, since least significant bit of exponent has odd parity)
    orpd    %xmm2,                  %xmm6       // copysign( 1.0, x )
    pandn   %xmm1,                  %xmm3       // if trunc(x) is odd, this bit will be non-zero (in the +-0.5, case we have +-0.5 here instead of just a bit set )
    xorpd   %xmm4,                  %xmm4       // 0.0
    pcmpeqd %xmm4,                  %xmm3       // check if each 32-bit chunk is equal to 0. Unforunately there is no 64-bit integer compare. Wed hit denorms here if we use double precision compare.
    pshufd  $0xE1, %xmm3,           %xmm4       // swap both chunks
    pand    %xmm4,                  %xmm3       // trunc(x) is odd ? 0 : -1U    (make sure the other chunk is also equal to 0)
    andnpd  %xmm6,                  %xmm3       // trunc(x) is odd ? copysign( 1.0, x ) : 0.0f
    addsd   %xmm3,                  %xmm1       // round

#if defined( __i386__ )
    movsd   %xmm1,                  (STACKP)
    fldl    (STACKP)
#else
    movsd   %xmm1,                  %xmm0
#endif    
    ADDP    $(16-FRAME_SIZE),       STACKP
    ret
    
    */
    
