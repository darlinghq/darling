
/*
 *  lroundl.s
 *
 *      by Ian Ollmann
 *
 *  Apple Inc. Copyright (c) 2007.  All rights reserved.
 *
 */
 
#include "abi.h"
#include <machine/asm.h>

.align 2
.literal4
two63:          .long           0x5f000000
mtwo63:         .long           0xdf000000
one:            .long           1
inf:            .long           0x7f800000

.align 3
.literal8
cutoff32:        .double     2147483647.5            // 2**31-0.5
mcutoff32:       .double    -2147483648.5            // 2**31-0.5


.align 4
.literal16
cutoff:         .quad           0xffffffffffffffff, 0x403d 
sign:           .quad           0x0, 0xffffffffffffffff

.text
#if defined( __x86_64__ )

ENTRY( lroundl )
ENTRY( llroundl )
    movswl  8+FRAME_SIZE( STACKP ), %edx
    andl    $0x7fff,                %edx        // exponent of x
    movq    FRAME_SIZE( STACKP ),   %rax
    subl    $0x3ffe,                %edx        // push exponents less than -1 negative
    fldt    FRAME_SIZE( STACKP )                // { x }
    cmpl    $(63+1),                %edx        // if( |x| >= 0x1.0p63 || |x| < 0.5 || isnan(x) )
    jae     1f                                  //      goto 1
    
    // 0.5 <= |x| < 0x1.0p63
    fldt    cutoff( %rip )                      // { 0x1.0p63 - 0.5, x }
    fucomip %st(1),         %st(0)              // { x }
    je      3f
    
    //Shift the significand right so that units bit is at units + 1 position
    movl    $63,                    %ecx        
    subl    %edx,                   %ecx        // 63 - (exponent+1)
    shrq    %cl,                    %rax        // shift so that the units bit is at the +1 position
    movq    %rax,                   %rdx        // set aside a copy
    shrq    $1,                     %rax        // finish the shift with shift right by 1 bit -- we need to do 64-bit shifts here at times and not possible with ISA
    andq    $1,                     %rdx        // isolate the leading fractional bit
    addq    %rdx,                   %rax        // round the result up.

    //fix sign
    movswq  8+FRAME_SIZE( STACKP ), %rdx        // read the sign + exponent
    sarq    $16,                    %rdx        // remove exponent
    xorq    %rdx,                   %rax        // flip the sign of the result
    subq    %rdx,                   %rax        // correct for 2's complement

    // set inexact as necessary
    fabs                                        // { |x| }
    fadds   two63(%rip)                         // { |x| + 0x1.0p63 }   set inexact as necessary
    fstp    %st(0)                              // throw away numerical result.

    ret

//  |x| >= 0x1.0p63 || |x| < 0.5 || isnan(x)
1:  jge     2f

    // |x| < 0.5
    xorq    %rax,                   %rax
    fistpl  FRAME_SIZE( STACKP )                // set inexact  as necessary
    ret

//  |x| >= 0x1.0p63 || isnan(x)
2:  movswq  8+FRAME_SIZE( STACKP ), %rdx
    flds    mtwo63( %rip )
    fucomip %st(1), %st(0)
    je      4f
    fistpl  FRAME_SIZE( STACKP )                // set invalid
    shrq    $63,                    %rdx
    subq    $1,                     %rdx
    movq    $0x8000000000000000,    %rax
    xorq    %rdx,                   %rax
    ret


//  0x1.0p63 - 0.5, positive overflow
3:  fistpl  FRAME_SIZE( STACKP )                // set invalid
    movq    $0x7fffffffffffffff,    %rax
    ret
  
//  -0x1.0p63 or nan
4:  jp      5f
    fstp    %st(0)
    movq    $0x8000000000000000,    %rax
    ret    

//  nan
5:  fistpl  FRAME_SIZE( STACKP )                // set invalid
    movq    $0x8000000000000000,    %rax
    ret

#else

ENTRY( lroundl )
    movswl  8+FRAME_SIZE( STACKP ), %edx
    andl    $0x7fff,                %edx        // exponent of x
    movl    4+FRAME_SIZE( STACKP ), %eax
    subl    $0x3ffe,                %edx        // push exponents less than -1 negative
    fldt    FRAME_SIZE( STACKP )                // { x }
    cmpl    $(31+1),                %edx        // if( |x| >= 0x1.0p31 || |x| < 0.5 || isnan(x) )
    jae     1f                                  //      goto 1

    //
    call    0f
0:  popl    %ecx
    fldl    (cutoff32-0b)(%ecx)
    fucomip %st(1),     %st(0)
    jbe     3f
    
    // set inexact
    fabs
    fadds   (two63-0b)(%ecx)
    fstp    %st(0)

    // round
    movl    $31,                    %ecx
    subl    %edx,                   %ecx
    shrl    %cl,                    %eax
    movl    %eax,                   %edx
    shrl    $1,                     %eax
    andl    $1,                     %edx
    addl    %edx,                   %eax
    
    // fix sign
    movswl  8+FRAME_SIZE( STACKP ), %edx
    sarl    $16,                    %edx
    xorl    %edx,                   %eax
    subl    %edx,                   %eax
    ret

1:  jge     2f

    // |x| < 0.5
    xorl    %eax,                   %eax
    fistpl  FRAME_SIZE( STACKP )                // set inexact  as necessary
    ret

2:  movswl  8+FRAME_SIZE( STACKP),  %edx
    call    0f
0:  popl    %ecx
    fldl    ( mcutoff32-0b)(%ecx)
    fucomip %st(1),         %st(0)
    jae     4f
    fldl    ( cutoff32-0b )(%ecx)
    fucomip %st(1),         %st(0)
    jbe     3f

    // non overflowing result
    shrl    $31,                    %edx
    subl    $1,                     %edx
    movl    $0x80000000,            %eax
    xorl    %edx,                   %eax

    //set inexact
    fabs
    fadds   (two63-0b)(%ecx)
    fstp    %st(0)
    ret
    
//  positive overflow
3:  jp      5f
    fistps  FRAME_SIZE( STACKP )
    movl    $0x7fffffff,            %eax
    ret
    
// negative overflow
4:  fistps  FRAME_SIZE( STACKP )
    movl    $0x80000000,            %eax
    ret

// nan
5:  fistpl  FRAME_SIZE( STACKP )
    movl    $0x80000000,            %eax
    ret
    

ENTRY( llroundl )
    movswl  8+FRAME_SIZE( STACKP ), %edx
    andl    $0x7fff,                %edx        // exponent of x
    movq    FRAME_SIZE( STACKP ),   %xmm0
    subl    $0x3ffe,                %edx        // push exponents less than -1 negative
    fldt    FRAME_SIZE( STACKP )                // { x }
    cmpl    $(63+1),                %edx        // if( |x| >= 0x1.0p63 || |x| < 0.5 || isnan(x) )
    jae     1f                                  //      goto 1
    
    call    0f
0:  popl    %ecx
    
    // 0.5 <= |x| < 0x1.0p63
    fldt    (cutoff-0b)( %ecx )                 // { 0x1.0p63 - 0.5, x }
    fucomip %st(1),         %st(0)              // { x }
    je      3f
    
    //Shift the significand right so that units bit is at units + 1 position
    movl    $63,                    %eax    
    movd    (one-0b)(%ecx),         %xmm2       // 1
    subl    %edx,                   %eax        // 63 - (exponent+1)
    movd    %eax,                   %xmm1      
    psrlq   %xmm1,                  %xmm0       // shift so that the units bit is at the +1 position
    movq    %xmm0,                  %xmm1       // set aside a copy
    psrlq   $1,                     %xmm0       // finish the shift with shift right by 1 bit -- we need to do 64-bit shifts here at times and not possible with ISA
    pand    %xmm2,                  %xmm1       // isolate the leading fractional bit
    paddq   %xmm1,                  %xmm0       // round the result up.

    // set inexact as necessary
    fabs                                        // { |x| }
    fadds   (two63-0b)(%ecx)                         // { |x| + 0x1.0p63 }   set inexact as necessary
    fstp    %st(0)                              // throw away numerical result.

    //fix sign
    movswl  8+FRAME_SIZE( STACKP ), %eax        // read the sign + exponent
    shrl    $31,                    %eax        // remove exponent
    movq    (sign-0b)(%ecx, %eax,8), %xmm1
    pxor    %xmm1,                  %xmm0
    psubq   %xmm1,                  %xmm0
    movd    %xmm0,                  %eax
    psrlq   $32,                    %xmm0
    movd    %xmm0,                  %edx

    ret

//  |x| >= 0x1.0p63 || |x| < 0.5 || isnan(x)
1:  jge     2f

    // |x| < 0.5
    xorl    %eax,                   %eax
    xorl    %edx,                   %edx
    fistpl  FRAME_SIZE( STACKP )                // set inexact  as necessary
    ret

//  |x| >= 0x1.0p63 || isnan(x)
2:  movswl  8+FRAME_SIZE( STACKP ), %eax
    call    0f
0:  popl    %ecx
    flds    (mtwo63-0b)( %ecx )
    fucomip %st(1), %st(0)
    je      4f
    fistpl  FRAME_SIZE( STACKP )                // set invalid
    shrl    $31,                    %eax
    subl    $1,                     %eax
    movl    $0x80000000,            %edx
    xorl    %eax,                   %edx
    ret


//  0x1.0p63 - 0.5, positive overflow
3:  fistpl  FRAME_SIZE( STACKP )                // set invalid
    movl    $-1,                    %eax
    movl    $0x7fffffff,            %edx
    ret
  
//  -0x1.0p63 or nan
4:  jp      5f
    fstp    %st(0)
    movl    $0x80000000,            %edx
    xorl    %eax,                   %eax
    ret    

//  nan
5:  fistpl  FRAME_SIZE( STACKP )                // set invalid
    movl    $0x80000000,            %edx
    xorl    %eax,                   %eax
    ret



#endif
