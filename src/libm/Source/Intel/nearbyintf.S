//  float nearbyintf(float x)
//
//  returns the argument rounded to an integral value using the prevailing
//  rounding mode, and without raising the inexact flag (C99 7.12.9.4).
//
//  -- Stephen Canon, January 2010

#include <System/i386/cpu_capabilities.h>

.text
.align 4
.globl _nearbyintf
_nearbyintf:

// Fast path if SSE4.1 is available ------------------------------------------

#if defined __i386__
    movss      4(%esp),         %xmm0
    mov         _COMM_PAGE_CPU_CAPABILITIES,    %eax
    test        $(kHasSSE4_1),  %eax
    jz          L_noSSE4_1
    roundss     $0xc,  %xmm0,   %xmm0
    movss       %xmm0,         4(%esp)
    flds       4(%esp)
    ret
#elif defined __x86_64__
    movabs      _COMM_PAGE_CPU_CAPABILITIES,    %eax
    test        $(kHasSSE4_1),  %eax
    jz          L_noSSE4_1
    roundss     $0xc,  %xmm0,   %xmm0
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
    movd        %xmm0,          %ecx
#if defined __i386__
    sub         $4,             %esp    // make space for the fp control word
    #define     fpcw            (%esp)
    #define     floatval       8(%esp)  // address of argument to nearbyintf
#else
    #define     fpcw          -4(%rsp)  // space for fp control word in red zone
#endif
    fnstcw      fpcw
      
// Detect special cases and branch
    mov         %ecx,           %edx
    and         $0x7fffffff,    %ecx    // |x|
    xor         %ecx,           %edx    // signbit of x
    movd        %edx,           %xmm2
    sub         $0x3f800000,    %ecx    // |x| - exponent bias
    mov         $0x80000000,    %eax
    cmp         $0x0b800000,    %ecx    // if |x| < 1.0 or |x| >= 0x1.0p23f or isnan(x)
    jae         L_edgeCases             //    branch
    
    shr         $23,            %ecx    // exponent of x, in [0,22]
    add         $8,             %ecx
    sar         %cl,            %eax    // mask covering integral bits of x
    
L_reentryForSmallX:
    movd        %eax,           %xmm1
    andps       %xmm0,          %xmm1   // trunc(x)
    ucomiss     %xmm1,          %xmm0   // if x == trunc(x)
    je          L_roundToZero           //    return trunc(x)

    mov         fpcw,           %ecx
    and         $0xc00,         %ecx    // current rounding mode encoding
    sub         $0x400,         %ecx
    cmp         $0x400,         %ecx
    jg          L_roundToZero           // ecx = 0x800
    je          L_roundUp               // ecx = 0x400
    jb          L_roundDown             // ecx = 0x000

L_roundToNearest:                       // fall into default rounding path
    subss       %xmm1,          %xmm0   // fractional part of x
    movd        %xmm0,          %ecx
    or          $0x3f000000,    %edx    // copysignf(0.5f,x)
    xor         %edx,           %ecx    // frac(x) ^ copysignf(0.5f,x)
    jz          L_exactHalfway          // handle exact halfway cases separately
    
    sub         $0x00800000,    %ecx    // |frac(x)| > 0.5 ? negative : positive
    or          $0x00800000,    %edx
    sar         $31,            %ecx    // |frac(x)| > 0.5 ? -1 : 0
    and         %ecx,           %edx    // |frac(x)| > 0.5 ? copysignf(1.0f,x) : 0
    movd        %edx,           %xmm0
    addss       %xmm1,          %xmm0   // |frac(x)| > 0.5 ? trunc(x) + copysign(1.0f,x) : trunc(x)
    orps        %xmm2,          %xmm0   // apply signbit to get the right zero
#if defined( __i386__ )
    movss       %xmm0,          floatval
    flds        floatval
    add         $4,             %esp
#endif
    ret

L_exactHalfway:
    shl         $1,             %eax
    addss       %xmm0,          %xmm0   // copysignf(1.0f,x)
    or          $0x80000000,    %eax    // mask covering signbit and bits larger than 2^1
    movd        %eax,           %xmm3
    pandn       %xmm1,          %xmm3   // trunc(x) odd ? non-zero : zero
    pxor        %xmm4,          %xmm4
    pcmpeqd     %xmm4,          %xmm3   // trunc(x) odd ? 0 : -1
    andnps      %xmm0,          %xmm3   // trunc(x) odd ? copysignf(1.0f,x) : 0.0f
    addss       %xmm3,          %xmm1
    orps        %xmm2,          %xmm1   // apply signbit to result
L_roundToZero:
#if defined( __i386__ )
    movss       %xmm1,          floatval
    flds        floatval
    add         $4,             %esp
#else
    movaps      %xmm1,          %xmm0
#endif
    ret
    
L_roundUp:                              // we know that x is not integral
    psrad       $31,            %xmm0   // x < 0 ? -1 : 0
    pcmpeqb     %xmm3,          %xmm3   // -1
    psubd       %xmm3,          %xmm0   // x < 0 ? 0 : 1
    cvtdq2ps    %xmm0,          %xmm0   // x < 0 ? 0.0 : 1.0
    addss       %xmm1,          %xmm0   // x < 0 ? trunc(x) : trunc(x) + 1
    orps        %xmm2,          %xmm0   // apply signbit to get the right zero
#if defined( __i386__ )
    movss       %xmm0,          floatval
    flds        floatval
    add         $4,             %esp
#endif
    ret
    
L_roundDown:                            // we know that x is not integral
    psrad       $31,            %xmm0   // x < 0 ? -1 : 0
    cvtdq2ps    %xmm0,          %xmm0   // x < 0 ? -1 : 0
    addss       %xmm1,          %xmm0   // x < 0 ? trunc(x) - 1 : trunc(x)
#if defined( __i386__ )
    movss       %xmm0,          floatval
    flds        floatval
    add         $4,             %esp
#endif
    ret

L_edgeCases:                            // if |x| < 1.0, jump back to mainline
    jl          L_reentryForSmallX      // otherwise, we just return x
#if defined( __i386__ )
    movss       %xmm0,          floatval
    flds        floatval
    add         $4,             %esp
#endif
    ret
