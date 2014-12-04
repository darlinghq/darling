/*
 *  nextafterl.s
 *  LibmV5
 *
 *  Written by Ian Ollmann on 11/9/07.
 *  Copyright 2006 Apple Computer. All rights reserved.
 *
 */

#include <machine/asm.h>
#include "abi.h"

#if defined( __x86_64__ )
.align 3
.literal8
signBit:        .quad   0x8000000000000000
nonSignBit:     .quad   0x7fffffffffffffff
#endif

.align 4
.const

tiny:   .quad   0x8000000000000000, 0x1         //0x1.0p-16382
big:    .quad   0x8000000000000000, 0x7ffe      //0x1.0p16383
min:    .quad   0x1, 0x0, 0x1, 0x8000           //{ 0x1.0p-16445, -0x1.0p-16445 }

.text
ENTRY(nexttowardl)
ENTRY( nextafterl )
    fldt    FRAME_SIZE( STACKP )        //  { x }
    fldt    16+FRAME_SIZE( STACKP )     //  { y, x }
    xor     AX_P,       AX_P            //  0
    fucomip %st(1),     %st(0)          //  { x }       if( x == y || isnan(x) || isnan(y) )
    setbe   %al                         //  x >= y ? 1 : 0
    je      3f                          //                      goto 2

    fldz                                //  { 0, x }
    xor     DX_P,       DX_P            //  0
    fucomip %st(1),     %st(0)          //  { x }       if( x == 0 )
    setb    %dl                         //  0 < x ? 1 : 0
    je      2f                          //                      goto 1

    xor     DX_P,       AX_P            //  (x >= y) != (0 < x) ? 1 : 0
    add     AX_P,       AX_P            //  (x >= y) != (0 < x) ? 2 : 0

#if defined( __i386__ )
    movq    FRAME_SIZE( STACKP ),       %xmm0   // load x significand
    xorpd   %xmm1,                      %xmm1   //  { 0, 0 }
    cmpeqsd %xmm1,                      %xmm1   //  { 0, -1LL }
    movdqa  %xmm0,                      %xmm2   // old significand
    movd    %eax,                       %xmm3   // (x >= y) != (0 < x) ? 2 : 0
    paddq   %xmm1,                      %xmm0   // subtract 1 from significand
    psllq   $63,                        %xmm1   // 0x8000000000000000ULL
    paddq   %xmm3,                      %xmm0   // add back either 2 or 0 depending on the direction we are moving the exponent

    //At this point, the significand might have overflowed or it might have underflowed.
    //If it overflowed, the leading bit was just lost due to modulo overflow. The rest of the bits
    //are zero (since we added 1), so all we need to do is set the leading bit again.  If it
    //underflowed, the bits are 0x7fffffffffffffff (since we subtracted 1) and all we need to do
    //is set the leading bit again. We still need to patch up the exponent, and we might have just
    //created a denormal. We will deal with that later.
    pxor    %xmm0,                      %xmm2   // set leading bit to 1 if leading bit changed (we have overflow/underflow)
    pand    %xmm1,                      %xmm2   // zero the non-leading bits
    por     %xmm2,                      %xmm0   // set leading bit again in the significand if it became unset
    movmskpd %xmm2,                     %edx    // put carry bit into edx
    movq    %xmm0,        FRAME_SIZE(STACKP)    // write out new significand
#else
    movq    FRAME_SIZE( STACKP ),       %rcx    // load significand of x
    movq    %rcx,                       %rdx    // old significand of x
    subq    $1,                         %rcx    // subtract 1 from significand of x
    addq    %rax,                       %rcx    // add back in either 0 or 2 depending on direction of movement
    xorq    %rcx,                       %rdx    // set leading bit to 1 if leading bit changed (we have overflow/underflow)
    andq    signBit(%rip),              %rdx    // zero all the other bits
    orq     %rdx,                       %rcx    // restore leading bit in the significand if it changed
    shrq    $63,                        %rdx    // move carry bit to low bit
    movq    %rcx,         FRAME_SIZE(STACKP)    // write out new significand
#endif

    // fix exponent
    test    $1,                         %edx    // Did we change it?
    movzwl  8+FRAME_SIZE( STACKP ),     %edx    // Load the exponent
    jnz     1f
    
    // check for overflow/underflow
0:  andl    $0x7fff,                    %edx
    subl    $1,                         %edx
    cmpl    $0x7ffe,                    %edx
    jae     5f
    
    fldt    FRAME_SIZE( STACKP )
    fstp    %st(1)
    ret

    //fix exponent
1:  subl    $1,                         %eax    // 
    addl    %eax,                       %edx    //  add back either 2 or 0 depending on direction of motion.
    movw    %dx,        8+FRAME_SIZE( STACKP )
    jmp     0b


2:  // x == 0, x != y, return copysignl( 0x1.0p-16445, y )
    movzwl  24+FRAME_SIZE( STACKP ),    %eax    // sign + exponent of y
    and     $0x8000,                    AX_P    // sign of y
    shr     $11,                        AX_P    // y is negative ? 16 : 0
    fstp    %st(0)                              //  {}
#if defined( __x86_64__ )
    leaq    min(%rip),                  %rcx
    fldt    (%rcx, %rax, 1 )                    //  { copysign( 0x1.0p-16445, y ) }
    fldt    tiny( %rip )                        //  { 0x1.0p-16382, copysign( 0x1.0p-16445, y ) }
#else
    call    0f
0:  popl    %ecx
    fldt    (min-0b)(%ecx, %eax, 1 )            //  { copysign( 0x1.0p-16445, y ) }
    fldt    (tiny-0b)( %ecx )                   //  { 0x1.0p-16382, copysign( 0x1.0p-16445, y ) }
#endif  
    fmulp   %st(0), %st(0)                      //  { copysign( 0x1.0p-16445, y ) }         set underflow, inexact
    ret


    // x == y || isnan(x) || isnan(y)
3:  fldt    16+FRAME_SIZE( STACKP )     // { y, x }
    jp      4f                          //  if( isnan(x) || isnan(y) )  goto 3
    fstp    %st(1)                      // { y }    return y
    ret

    // nan
4:  faddp                               //  return x + y
    ret
  
// overflow or underflow
5:  je      6f                          // result is infinite, goto 6

    //underflow
    fstp    %st(0)                      // get rid of x
#if defined( __x86_64__ )
    fldt    tiny( %rip )                //  { 0x1.0p-16382 }
    andq    nonSignBit(%rip),           %rcx
    movq    %rcx,           FRAME_SIZE( STACKP )
#else
    call    0f
0:  popl    %ecx
    fldt    (tiny-0b)( %ecx )           //  { 0x1.0p-16382 }
    pandn   %xmm0,                      %xmm2   // unset the leading bit
    movq    %xmm2,        FRAME_SIZE(STACKP)    // write out denorm significand
#endif  
    fmulp   %st(0), %st(0)              // set underflow
    fldt    FRAME_SIZE( STACKP )
    ret    
    

// infinite
6:  faddp   %st(0), %st(0)              // {}       add x to itself and throw away the result -- sets overflow and inexact
    fldt    FRAME_SIZE( STACKP )
    ret
