//  long double nearbyintl(long double x)
//
//  returns the argument rounded to an integral value using the prevailing
//  rounding mode, and without raising the inexact flag (C99 7.12.9.4).
//
//  Original version due to Ian Ollmann, 2007
//
//  Edits by Steve Canon January 2010:
//      rearrange branches to match nearbyint[f]
//      minimal tune-up to get sign of zero right
 
#include <machine/asm.h>
#include "abi.h"

ENTRY( nearbyintl )
    SUBP    $(32-FRAME_SIZE),       STACKP
    
    //write the current rounding mode to the stack.  Using the mxcsr instead cost an extra 20 cycles.
    fnstcw  16(STACKP) 

    fldz                                        // { 0 }
    movq    32(STACKP),             %xmm0       //Load the mantissa
    fldt    32(STACKP)                          // { x, 0 }
    pxor    %xmm1,                  %xmm1       // set our default truncation mask to 0
    xorl    %ecx,                   %ecx
    movw    40(STACKP),             %cx         //Load the signed exponent
    movl    %ecx,                   %edx        // save signed exponent
    movw    %cx,                    8(STACKP)   // write out exponent of truncated value
    andl    $0x8000,                %edx        // sign
    andl    $0x7fff,                %ecx        // biased exponent
    shll    $16,                    %edx        // sign << 16
    addl    $(16384-62),            %ecx        // add 16384-62 to exponent. This forces Inf, NaN, numbers >= 2**63 to be negative
    cmpw    $(16383+16384-62),      %cx         // values with exponents less than 16384-62 are fractional numbers only
    jl      L_xIsLargeOrSmall                   // branch for |x| < 1.0L, |x| >= 0x1.0p63L, NaN

    //Now we know we have a number 1.0 <= |x| < 2**63

    fstp    %st(1)                              // { x }
    //create a mask of non-fractional bits to calculate trunc(x)
    subl    $(16383+16384-62+63),   %ecx        // unbias the exponent, undo our magic above
    negl                            %ecx        // bits to shift -1ULL left to create a truncation mask
    pcmpeqb %xmm1,                  %xmm1       // -1ULL
    movd    %ecx,                   %xmm7       // bits to shift -1ULL left to create a truncation mask
    psllq   %xmm7,                  %xmm1       // left shift -1ULL by number of fractional bits in x
    
    //Round. We get here in one of two ways:
    //      fall through from above:    xmm2 has a truncation mask   
    //      jmp from |x| < 1.0:         xmm2 holds the sign of x (stored as +-0.0f), which rounds x to zero of the correct sign in this step
L_reentryForSmallX:
    pand    %xmm0,                  %xmm1       // trunc(mantissa)
    movq    %xmm1,                  (STACKP)    
    fldt    (STACKP)                            // { trunc(x), x }
    fucomi  %st(1), %st(0)                      // trunc(x) == x ?
    je      L_truncate                          // if the value was an integer, we are done, so return x (satisfies x==0, small integer cases )

    //find our what our rounding mode is
    movl    16(STACKP),             %ecx
    andl    $0xC00,                 %ecx        // isolate the RC field
    subl    $0x400,                 %ecx        // move one of the possible values negative
    cmpw    $0x400,                 %cx         // test the RC value
    
    // Rounding modes in eax:
    //      -0x400          Round to nearest even
    //      0               Round to -Inf
    //      0x400           Round to +inf
    //      0x800           Round to zero
    
    jg      L_truncate                          //Round to zero mode.               Note: signed compare here
    je      L_roundUp                           //Round to +Inf, Go there
    jb      L_roundDown                         //Round to -Inf, Go there.          Note: unsigned compare here
    
    //Round to nearest even is the fall through path, because it is most common
    fsubr   %st(0), %st(1)                      // { trunc(x), fract }

    orl     $0x3f000000,            %edx        // copysign( 0.5f, x )
    movl    %edx,                   16(STACKP)
    flds    16(STACKP)                          // { copysign( 0.5L, x ), trunc(x), fract }
    
    //check for fract == copysign( 0.5f, x )

    fld     %st(2)                              // { fract, copysign( 0.5L, x ), trunc(x), fract }
    fabs                                        // { |fract|, copysign( 0.5L, x ), trunc(x), fract } 
    fld     %st(1)                              // { copysign( 0.5L, x ), |fract|, copysign( 0.5L, x ), trunc(x), fract }
    fabs                                        // { 0.5L, |fract|, copysign( 0.5L, x ), trunc(x), fract }
    fucomip %st(1), %st(0)                      // { |fract|, copysign( 0.5L, x ), trunc(x), fract }        
    fstp    %st(0)                              // { copysign( 0.5L, x ), trunc(x), fract } 
    je      L_exactHalfway                      // |fract| == 0.5, figure out which way to round in the comfort and safety of a separate code branch

    fldz                                        // { 0.0L, copysign( 0.5L, x ), trunc(x), fract } 
    fcmovb  %st(1), %st(0)                      // { 0.5L < |fract| ? copysign( 0.5L, x ) : 0.0L, copysign( 0.5L, x ), trunc(x), fract }
    fstp    %st(1)                              // { 0.5L < |fract| ? copysign( 0.5L, x ) : 0.0L, trunc(x), fract }
    fadd    %st(0), %st(0)                      // { 0.5L < |fract| ? copysign( 1.0L, x ) : 0.0L, trunc(x), fract }
    faddp                                       // { correctly rounded x, fract }
    fstp    %st(1)
    fabs                                        // { |result| }
    cmp     $0,     %edx
    jge     1f
    fchs                                        // flip the sign if x is negative
1:  ADDP    $(32-FRAME_SIZE),       STACKP
    ret
    
L_exactHalfway:                                 // { copysign( 0.5L, x ), trunc(x), fract }
    addl    $0x20000000,            %edx        // copysign( 0x1.0p63f, x )
    movl    %edx,                   16(STACKP)  // copysign( 0x1.0p63f, x )
    flds    16(STACKP)                          // { copysign( 0x1.0p63L, x ), copysign( 0.5L, x ), trunc(x), fract }
    fadd    %st(2), %st(0)                      // { trunc(x) + copysign( 0x1.0p63L, x ), copysign( 0.5L, x ), trunc(x), fract }
    fstpt   (STACKP)                            // { copysign( 0.5L, x ), trunc(x), fract }
    fstp    %st(0)                              // { trunc(x), fract }
    fxch                                        // { fract, trunc(x) }
    fadd    %st(0), %st(0)                      // { copysign( 1.0L, x ), trunc(x) }
    movl    $1,                     %eax        //
    fldz                                        // { 0, copysign( 1.0L, x ), trunc(x) }
    andl    (STACKP),               %eax        // (mantissa & 1ULL) == 0
    fcmovne %st(1), %st(0)                      // { rounding value, copysign( 1.0L, x ), trunc(x) }
    fstp    %st(1)                              // { rounding value, trunc(x) }
    faddp                                       // { result }
    fabs                                        // { |result| }
    cmp     $0,     %edx
    jge     1f
    fchs                                        // flip the sign if x is negative
1:  ADDP    $(32-FRAME_SIZE),       STACKP
    ret

L_truncate:
    fstp    %st(1)
    ADDP    $(32-FRAME_SIZE),       STACKP
    ret                           
    
L_roundUp:                                      // { trunc(x), x }
    fsubr   %st(0), %st(1)                      // { trunc(x), fract }
    fldz                                        // { 0.0L, trunc(x), fract }
    fucomi  %st(2), %st(0)                      // { 0.0L, trunc(x), fract }      0.0L < fract ?
    fld1                                        // { 1.0L, 0.0L, trunc(x), fract }
    fcmovnbe %st(1), %st(0)                     // { 0.0L or 1.0L, 0.0L, trunc(x), fract }
    fstp    %st(3)                              // { 0.0L, trunc(x), 0.0L or 1.0L }
    fstp    %st(0)                              // { trunc(x), 0.0L or 1.0L }
    fadd    %st(1), %st(0)                      // { correctly rounded x, 0.0L or 1.0L }
    fstp    %st(1)                              // { correctly rounded x }
    fabs                                        // { |result| }
    cmp     $0,     %edx
    jge     1f
    fchs                                        // flip the sign if x is negative
1:  ADDP    $(32-FRAME_SIZE),       STACKP
    ret

L_roundDown:                                    // { trunc(x), x }
    fsubr   %st(0), %st(1)                      // { trunc(x), fract }
    fldz                                        // { 0.0L, trunc(x), fract }
    fucomi  %st(2), %st(0)                      // { 0.0L, trunc(x), fract }      0.0L > fract ?
    fld1                                        // { 1.0L, 0.0L, trunc(x), fract }
    fcmovb  %st(1), %st(0)                      // { 0.0L or 1.0L, 0.0L, trunc(x), fract }
    fstp    %st(3)                              // { 0.0L, trunc(x), 0.0L or 1.0L }
    fstp    %st(0)                              // { trunc(x), 0.0L or 1.0L }
    fsub    %st(1), %st(0)                      // { correctly rounded x, 0.0L or 1.0L }
    fstp    %st(1)                              // { correctly rounded x }
    fabs                                        // { |result| }
    cmp     $0,     %edx
    jge     1f
    fchs                                        // flip the sign if x is negative
1:  ADDP    $(32-FRAME_SIZE),       STACKP
    ret

L_xIsLargeOrSmall:
    jae     L_truncate                          // NaN, |x| >= 2**63 we are done

    // |x| < 1.0f skip to rounding
    fstp    %st(1)                              // { x }
    andw    $0x8000,    8(STACKP)               // zero exponent of truncated value, but preserve sign
    jmp     L_reentryForSmallX     
