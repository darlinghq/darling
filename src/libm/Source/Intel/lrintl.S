
/*
 * lrintl.s
 *
 *      by Ian Ollmann
 *
 *  Copyright Apple Inc. 2007.
 *
 *  C99 implementation of lrintl and llrintl
 */
 
#include <machine/asm.h>
#include "abi.h"

#if ! defined( __LP64__ )

ENTRY( lrintl )
    fldt    FRAME_SIZE( STACKP )                // { x }
    movswl  8+FRAME_SIZE( STACKP),      %eax    // Load signed exponent
    movl    %eax,                       %ecx    // set aside sign
    andl    $0x7fff,                    %eax    // remove sign
    cmpl    $0x401d,                    %eax    // if( |x| > 0x1.0p30 || isnan(x) )
    jae     2f                                  //      goto 2

    // |x| < 0x1.0p30 || isnan(x)
    fistpl  FRAME_SIZE( STACKP )
1:  movl    FRAME_SIZE( STACKP ),       %eax    // load result
    ret
    
    //|x| >= 0x1.0p30 || isnan(x)
2:  fistl   FRAME_SIZE( STACKP )
    fucomip %st(0),     %st(0)                  // if( isnan(x) )
    jp      1b                                  //      goto 1

    // |x| >= 0x1.0p30. If sign != old sign, xor with -1 
    movl    FRAME_SIZE( STACKP ),       %eax    // load result
    xorl    %eax,                       %ecx    // check to see if the sign changed
    sarl    $31,                        %ecx    // sign changed ? -1 : 0
    xorl    %ecx,                       %eax    // if the sign changed, xor with -1  (flips 0x80000000 to 0x7fffffff)
    ret

#define MOVSW   movswl

#else
#define MOVSW   movswq

ENTRY( lrintl )
#endif
ENTRY( llrintl )
    MOVSW   8+FRAME_SIZE( STACKP),  AX_P        // load signed exponent
    fldt    FRAME_SIZE( STACKP )                //  { x }
    mov     AX_P,                   CX_P        // signed exponent
    and     $0x7fff,                AX_P        // remove sign
    cmp     $0x403d,                AX_P        // if( exponent + bias >= 16383+62 )
    jae     2f                                  //      goto 1

    // |x| < 0x1.0p62. Nothing can go wrong. Return the long long.
1:  fistpll FRAME_SIZE( STACKP )                //  store out x as 64-bit int with appropriate rounding

#if defined( __LP64__ )
    movq    FRAME_SIZE( STACKP ),   %rax
#else
    movl    FRAME_SIZE( STACKP ),   %eax
    movl    4+FRAME_SIZE( STACKP ), %edx
#endif
    ret

    // |x| >= 2**62 || isnan(x) -- might be invalid
2:  fucomi  %st(0), %st(0)                      // if( x != x )
    jp      1b
    
    fistpll FRAME_SIZE( STACKP )                //  store out x as 64-bit int with appropriate rounding

    // Load data in. If the sign changed, xor with -1
#if defined( __LP64__ )
    movq    FRAME_SIZE( STACKP ),   %rax
    xorq    %rax,                   %rcx
    sarq    $63,                    %rcx
    xorq    %rcx,                   %rax
#else
    movl    FRAME_SIZE( STACKP ),   %eax
    movl    4+FRAME_SIZE( STACKP ), %edx
    xorl    %edx,                   %ecx
    sarl    $31,                    %ecx
    xorl    %ecx,                   %eax
    xorl    %ecx,                   %edx
#endif
    ret
    


