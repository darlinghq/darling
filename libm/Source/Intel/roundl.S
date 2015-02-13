/*
 *  roundl.s
 *
 *		by Ian Ollmann
 *
 *  Copyright (c) 2007 Apple Inc. All rights reserved.
 *
 *	Implementation for C99 round, lround and llround functions for __i386__ and __x86_64__.
 */

#include "machine/asm.h"

#define LOCAL_STACK_SIZE	12
#include "abi.h"

.literal8
zero:           .long   0,          0x80000000      // { 0.0f, -0.0f }
one:            .long   0x3f800000, 0xbf800000      // { 1.0f, -1.0f }
large:          .long   0x5f000000, 0xdf000000      // { 0x1.0p63, -0x1.0p63 }

.literal16
explicitBit:    .quad   0x8000000000000000,     0
roundMask62:    .quad   0xFFFFFFFFFFFFFFFE,     0

.text
#if defined( __x86_64__ )
ENTRY( roundl )
    movzwq  8+FRAME_SIZE( STACKP ),     %rdx    // sign + biased exponent
    movq    FRAME_SIZE( STACKP ),       %rax    // mantissa
    fldt    FRAME_SIZE( STACKP )                // {x}
    movq    %rdx,                       %r8     // sign + biased exponent
    andq    $0x7fff,                    %rdx    // exponent + bias
    shrq    $15,                        %r8     // x < 0 ? 1 : 0
    subq    $0x3ffe,                    %rdx    // push |x| < 0.5 negative
    cmp     $(63),                      %rdx    // if( |x| >= 0x1.0p62 || |x| < 0.5 || isnan(x) )
    jae     1f                                  //      goto 1
  
//  |x| >= 0.5 and conversion does not overflow.
    movq    $63,                        %rcx    // 63
    subq    %rdx,                       %rcx    // 63-(exponent+1)
    leaq    large(%rip),                %r9     // address of large array
    fadds   (%r9, %r8, 4)                       // { x + (x < 0 ? -0x1.0p63 : 0x1.0p63) }       set inexact as necessary
    shrq    %cl,                        %rax    // shift units bit into 2's position
    fstp    %st(0)                              // { }
    addq    $1,                         %rax    // round away from zero
    shrq    $1,                         %rax    // shift units bit to 1's position

    // find new exponent
    bsrq    %rax,                       %r9     // position of leading set bit. rax is never zero.
    movq    $0x3fff,                    %rdx    // bias
    movq    $63,                        %rcx    // 63
    addq    %r9,                        %rdx    // biased exponent
    subq    %r9,                        %rcx    // 63 - position of leading set bit
    movw    %dx,        8+FRAME_SIZE( STACKP )  // write out new exponent
    
    // shift significand into position
    shlq    %cl,                        %rax    // shift leading bit to higest position
    movq    %rax,       FRAME_SIZE( STACKP )    // write mantissa

    // get sign
    fldt    FRAME_SIZE( STACKP )                // { |result| }
    leaq    one( %rip ),                %rax    // address of one array
    fmuls   (%rax, %r8, 4 )                     // { result }       multiply by +1 or -1 according to sign of original result
    ret
    
//  |x| >= 0x1.0p62 || |x| < 0.5 || isnan(x)
1:  je      3f
    jg      2f

//  |x| < 0.5
    fistpl  FRAME_SIZE( STACKP )                // { } set inexact if x != 0
    leaq    zero( %rip),                %rax    // address of zero array
    flds    (%rax, %r8, 4 )                     // load result
2:  ret
  
//  0x1.0p62 <= |x| < 0x1.0p63
3:  leaq    large(%rip),                %r9     // address of large array
    fadds   (%r9, %r8, 4)                       // { x + (x < 0 ? -0x1.0p63 : 0x1.0p63) }       set inexact as necessary
    fstp    %st(0)                              // { }
    addq    $1,                         %rax    // add 0.5 to significand
    jz      4f                                  // handle overflow
    
    andq    roundMask62(%rip),          %rax    // prune fractional bits
    movq    %rax,                       FRAME_SIZE( STACKP )    // write to mantissa
    fldt    FRAME_SIZE( STACKP )                // load result
    ret

// result is +- 0x1.0p63
4:  flds    (%r9, %r8, 4)                       // load result
    ret
    
    

#else
ENTRY( roundl )
    movzwl  8+FRAME_SIZE( STACKP ),     %edx
    movq    FRAME_SIZE( STACKP ),       %xmm0
    fldt    FRAME_SIZE( STACKP )
    calll   0f
0:  popl    %ecx
    movl    %edx,                       %eax    // sign + biased exponent
    andl    $0x7fff,                    %edx    // biased exponent
    shrl    $15,                        %eax    // signof( x )
    subl    $0x3ffe,                    %edx    // push |x| < 0.5 negative
    cmp     $63,                        %edx    // if( |x| >= 0x1.0p62 || |x| < 0.5 || isnan(x) )
    jae     1f                                  //      goto 1

//  |x| >= 0.5 and conversion does not overflow.
    subl    $63,                        %edx    // (exponent+1) - 63
    fadds   (large-0b)(%ecx, %eax, 4)           // set inexact if necessary
    negl    %edx                                // 63 - (exponent+1)
    fstp    %st(0)                              // {}
    movd    %edx,                       %xmm1   // 63 - (exponent+1)
    psrlq   %xmm1,                      %xmm0   // move 0.5 bit to units position
    pcmpeqb %xmm1,                      %xmm1   // -1
    psubq   %xmm1,                      %xmm0   // add 1
    psrlq   $1,                         %xmm0   // move 1's bit to units position
    movq    %xmm0,                      FRAME_SIZE( STACKP )    // write out
    
    fildll  FRAME_SIZE( STACKP )                // { |result| }
    fmuls   (one-0b)(%ecx, %eax, 4)             // { result }
    ret


//  |x| >= 0x1.0p62 || |x| < 0.5 || isnan(x)
1:  je      3f
    jg      2f

//  |x| < 0.5
    fistpl  FRAME_SIZE( STACKP )                // { } set inexact if x != 0
    flds    (zero-0b)(%ecx, %eax, 4 )                     // load result
2:  ret

//  0x1.0p62 <= |x| < 0x1.0p63
3:  fadds   (large-0b)(%ecx, %eax, 4)           // { x + (x < 0 ? -0x1.0p63 : 0x1.0p63) }       set inexact as necessary
    fstp    %st(0)                              // { }
    movdqa  %xmm0,                      %xmm2   // significand
    pcmpeqb %xmm1,                      %xmm1   // -1LL
    psubq   %xmm1,                      %xmm0   // add 0.5 to significand
    pxor    %xmm0,                      %xmm2   // set leading bit if leading bit changed (overflow)
    movmskpd    %xmm2,                  %edx
    test    $1,                         %edx
    jnz     4f
    
    pand    (roundMask62-0b)(%ecx),     %xmm0    // prune fractional bits
    movq    %xmm0,                      FRAME_SIZE( STACKP )    // write to mantissa
    fldt    FRAME_SIZE( STACKP )                // load result
    ret

// result is +- 0x1.0p63
4:  flds    (large-0b)(%ecx, %eax, 4)           // load result
    ret


#endif
