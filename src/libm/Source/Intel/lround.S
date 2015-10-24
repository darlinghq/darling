
/*
 *	lround.s
 *
 *		by Ian Ollmann
 *
 *	Copyright (c) 2007,  Apple Inc.  All Rights Reserved.
 *
 *	Implementation of C99 lround and llround functions for i386 and x86_64.
 */
 
#include <machine/asm.h>
#include "abi.h"

.literal8
half:           .quad       0x3fe0000000000000      // 0.5
mtwo63:         .quad       0xc3e0000000000000      // -2**63
mtwo31:         .quad       0xc1e0000000000000      // -2**31
two52:          .quad       0x4330000000000000      // 2**52
twom32:         .quad       0x3df0000000000000      // 2**-32
implicit:       .quad       0x8000000000000000
cutoff:         .double     2147483647.5            // 2**31-0.5
mcutoff:        .double    -2147483648.5            // 2**31-0.5

.text
#if defined( __x86_64__ )
ENTRY( lround )
ENTRY( llround )
    movd    %xmm0,                  %rax
    movq    %rax,                   %rdx
    shrq    $52,                    %rax
    andq    $0x7ff,                 %rax    // exponent + bias
    subq    $0x3fe,                 %rax    // push exponent < -1 to negative
    cmpq    $64,                    %rax    // if( |x| >= 0x1.0p63 || |x| < 0.5 || isnan(x) )
    jae     1f                              //      goto 1
    
    // 0.5 <= |x| < 0x1.0p63
    shlq    $11,                    %rdx    // put most significant bit in leading position
    orq     implicit(%rip),         %rdx    // make implicit bit explicit

    // shift value right so that the integer bit is at position 1
    movq    $63,                    %rcx
    subq    %rax,                   %rcx    // 63 - (exponent+1)
    shrq    %cl,                    %rdx
    
    addq    $1,                     %rdx    // round away from zero
    shrq    $1,                     %rdx    // move unit bit to correct position
    
    // Fix sign
    movd    %xmm0,                  %rax
    sarq    $63,                    %rax
    movq    %rax,                   %rcx
    xorq    %rdx,                   %rax
    subq    %rcx,                   %rax
    
    // set inexact as necessary
    cvttsd2si %xmm0,                %rdx
    ret
  
//  |x| >= 0x1.0p63 || |x| < 0.5 || isnan(x) 
1:  jge         2f

    // |x| < 0.5
    cvttsd2si   %xmm0,              %rax    // set invalid (as necessary), prepare 0x8000000000000000
    ret
    
    // |x| >= 0x1.0p63 || isnan(x) 
2:  ucomisd     %xmm0,              %xmm0    
    cvttsd2si   %xmm0,              %rax    // set invalid (as necessary), prepare 0x8000000000000000
    jp          3f
    negq        %rdx
    sarq        $63,                %rdx
    xorq        %rdx,               %rax    
3:  ret
    
        
#else  /* __i386__ */

ENTRY( llround )
    movl    4+FRAME_SIZE( STACKP ), %eax    // x.hi
    movsd   FRAME_SIZE( STACKP ),   %xmm0   // x
    andl    $0x7fffffff,            %eax    // |x|.hi
    subl    $0x3fe00000,            %eax    // push exponent - 1 to negative
    call    0f
0:  popl    %ecx
    cmpl    $((63+1)<<20),          %eax    // if( |x| >= 0x1.0p63 || |x| < 0.5 || isnan(x) )
    jae     1f                              //      goto 1
  
    // 0.5 <= |x| < 0x1.0p63  
    movq    (implicit-0b)(%ecx),    %xmm1
    movapd  %xmm0,                  %xmm7
    shrl    $20,                    %eax    // move exponent+1 to units position
    psllq   $11,                    %xmm0   // put the most significant bit in the leading position
    movl    $63,                    %edx    
    orpd    %xmm1,                  %xmm0   // make implicit bit explicit
    subl    %eax,                   %edx    // subtract exponent+1 from 63
    movd    %edx,                   %xmm2   // move to xmm
    psrlq   %xmm2,                  %xmm0   // shift units bit to units+1 position
    pcmpeqb %xmm3,                  %xmm3   // -1LL
    psubq   %xmm3,                  %xmm0   // round away from zero
    psrlq   $1,                     %xmm0   // shift units bit to units position
    
    //fix sign
    movsd   (two52-0b)(%ecx ),      %xmm4   // 2**52
    xorpd   %xmm6,                  %xmm6   // 0
   cmpnltsd %xmm7,                  %xmm6   // x < 0 ? -1LL : 0
    pxor    %xmm6,                  %xmm0
    psubq   %xmm6,                  %xmm0

    // move result to GPR
    movd    %xmm0,                  %eax
    psrlq   $32,                    %xmm0
    movd    %xmm0,                  %edx
        
    //set inexact
    andnpd  %xmm7,                  %xmm1   // |x|
    minsd   %xmm4,                  %xmm1   //  min( |x|, 0x1.0p52 ) -- avoid spurious inexact for |x| > 0x1.0p52
    addsd   %xmm4,                  %xmm1   //  add 0x1.0p52, set inexact
        
    ret

//  |x| >= 0x1.0p63 || |x| < 0.5 || isnan(x)     
1:  jge     2f

    // |x| < 0.5
    cvttsd2si   %xmm0,              %eax
    xorl        %edx,               %edx
    ret

2:  xorl    %eax,                   %eax
    ucomisd (mtwo63-0b)(%ecx),      %xmm0   // special case for -0x1.0p63, nan
    je      3f

    //overflow
    xorpd   %xmm1,                  %xmm1
    cmpltsd %xmm0,                  %xmm1
    movd    %xmm1,                  %eax
    cvttsd2si %xmm0,                %edx
    xorl    %eax,                   %edx
    ret
    
// special case for -0x1.0p63
3:  jp      4f                             //nans end up here, so get rid of them
    movl    $0x80000000,            %edx
    xorl    %eax,                   %eax
    ret

//nan
4:  cvttsd2si %xmm0,                %edx
    ret

    
ENTRY( lround )
    movl    4+FRAME_SIZE( STACKP ), %eax    // x.hi
    movsd   FRAME_SIZE( STACKP ),   %xmm0   // x
    andl    $0x7fffffff,            %eax    // |x|.hi
    subl    $0x3fe00000,            %eax    // push exponent - 1 to negative
    call    0f
0:  popl    %ecx
    movq    (implicit-0b)(%ecx),    %xmm1
    cmpl    $((31+1)<<20),          %eax    // if( |x| >= 0x1.0p63 || |x| < 0.5 || isnan(x) )
    jae     1f                              //      goto 1

    // weed out positive overflow cases
    ucomisd (cutoff-0b)(%ecx),      %xmm0   // if( x >= 0x1.0p31-0.5 )
    jae     3f
  
    // 0.5 <= |x| < 0x1.0p31  
    movapd  %xmm0,                  %xmm7
    shrl    $20,                    %eax    // move exponent+1 to units position
    psllq   $11,                    %xmm0   // put the most significant bit in the leading position
    movl    $63,                    %edx    
    orpd    %xmm1,                  %xmm0   // make implicit bit explicit
    subl    %eax,                   %edx    // subtract exponent+1 from 63
    movd    %edx,                   %xmm2   // move to xmm
    psrlq   %xmm2,                  %xmm0   // shift units bit to units+1 position
    pcmpeqb %xmm3,                  %xmm3   // -1LL
    psubq   %xmm3,                  %xmm0   // round away from zero
    psrlq   $1,                     %xmm0   // shift units bit to units position
    
    //fix sign
    movl    4+FRAME_SIZE( STACKP ), %edx
    movd    %xmm0,                  %eax
    sarl    $31,                    %edx
    xorl    %edx,                   %eax
    subl    %edx,                   %eax
        
    //set inexact
    andnpd  %xmm7,                  %xmm1   // |x|
    addsd   (two52-0b)(%ecx ),      %xmm1   // |x| += 0x1.0p52, set inexact
    ret

//  |x| >= 0x1.0p31 || |x| < 0.5 || isnan(x) 
1:  andnpd  %xmm0,                  %xmm1   // |x|
    ucomisd (mcutoff-0b)(%ecx),     %xmm0   // special case for -0x1.0p31, nan
    jbe     2f

    // weed out positive overflow cases
    ucomisd (cutoff-0b)(%ecx),      %xmm0   // if( x >= 0x1.0p31-0.5 )
    jae     3f

    // -0x1.0p63-0.5 < x <= -0x1.063 || |x| < 0.5
    cvttsd2si %xmm0,                %eax
    ret
    
// negative overflow cases and nan
2:  pcmpeqb %xmm0,                  %xmm0   // nan
    cvttsd2si %xmm0,                %eax    // set invalid
    ret

// positive overflow cases
3:  movl    $0x7fffffff,            %eax    // result is 0x7fffffff
    pcmpeqb %xmm0,                  %xmm0   // nan
    cvttsd2si %xmm0,                %edx    // set invalid
    ret

#endif
