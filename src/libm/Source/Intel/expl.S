
/*
 *  expl.s 
 *
 *      by Ian Ollmann
 *
 *  C99 expl for Mac OS X for Intel
 */

#include "abi.h"
#include "machine/asm.h"
 
.literal4
inf:         .long   0x7f800000
minf:        .long   0xff800000

.const
.align 4
maxldbl:     .quad   0xffffffffffffffff, 0x7ffe
minldbl:     .quad   0x8000000000000000, 0x0001
ln2_hi:      .quad   0xB17217F7D1D00000, 0x3ffe
ln2_lo:      .quad   0x8654361C4C67FC0D, 0xbFCE


#if defined( __i386__ )
    #define RELATIVE_ADDR( _a )     ((_a) - 0b)(%ecx)
#else
    #define RELATIVE_ADDR( _a )     _a(%rip)
#endif

.text 
ENTRY( expl )
    fldt        FRAME_SIZE( STACKP )                    // { x }
    movswl      8+FRAME_SIZE( STACKP ), %eax            // sign + exponent
    movl        %eax,                   %edx            // sign + exponent
    andl        $0x7fff,                %eax            // exponent
    subl        $(16383-64),            %eax            // subtract out most of the bias -- push expoents less than -64 negative     
    cmpl        $(64+14),               %eax            // if( exponent < -64 ||  exponent > 14 || isnan(x)      
    ja          2f                                      //      goto 2
    
    fldl2e                                              // { log2(e), x }
    andl        $0x80000000,            %edx            // signof (x)
    fmul        %st(1),     %st(0)                      // { x * log2(e), x }
    orl         $0x3f000000,            %edx            // copysign( 0.5f, x )
    movl        %edx,                   FRAME_SIZE(STACKP)
    fadds       FRAME_SIZE( STACKP )                    //  { x * log2(e) + copysign( 0.5, x ), x }
    
    //truncate the top of stack
#if defined( __SSE3__ )
    fisttpl     FRAME_SIZE(STACKP)                      // { x }
    movl        FRAME_SIZE(STACKP),     %edx
    fildl       FRAME_SIZE(STACKP)                      // { round(2(x * log2(e)), x }
#else
    fstpl       FRAME_SIZE(STACKP)                      // { x }
    cvtsd2si    FRAME_SIZE(STACKP),     %edx
    movl        %edx,                   FRAME_SIZE(STACKP)
    fildl       FRAME_SIZE(STACKP)                      // { round(2(x * log2(e)), x }
#endif

    // PIC
#if defined( __i386__ )
    call        0f
0:  popl        %ecx
#endif

    // reduce x to the fractional part
    fldt        RELATIVE_ADDR(ln2_hi)                   // { ln(2) hi part, round(2(x * log2(e)), x }
    fmul        %st(1),                 %st(0)          // { ln(2) hi part * round(2(x * log2(e)), round(2(x * log2(e)), x }
    fsubrp      %st(0),                 %st(2)          // { round(2(x * log2(e)), x - log2(e) hi part * round(2(x * log2(e)) }
    fldt        RELATIVE_ADDR(ln2_lo)                   // { ln2
    fmul        %st(1),                 %st(0)          // { round(2(x * log2(e)) * ln(2) lo part, round(2(x * log2(e)), x - log2(e) hi part * round(2(x * log2(e)) }
    fsubrp      %st(0),                 %st(2)          // { round(2(x * log2(e)), reduced x }
    fxch                                                // { reduced x,  round(2(x * log2(e)) }

    // multiply by log2(e)
    fldl2e                                              // { log2(e), x - ln(2) hi part * round(2(x * log2(e))  - round(2(x * log2(e)) * ln(2) lo part }
    fmulp                                               // { log2(e) * reduced x, round(2(x * log2(e)) }

    // calculate 2**(log2(e) * reduced x )
    f2xm1                                               // { 2**f - 1, round(2(x * log2(e)) }
    fld1                                                // { 1, 2**f - 1, round(2(x * log2(e)) }
    faddp                                               // { 2**f, round(2(x * log2(e)) }

    // scale results
    fscale                                              // { result, , round(2(x * log2(e)) }
    fstp        %st(1)

    // The scale may fail to set underflow. Check to see if we need to do that
    fldt        RELATIVE_ADDR( minldbl )                // { 0x1.0p-16382, result }
    fucomip     %st(1),                 %st(0)          // { 0x1.0p-16382, result }
    ja          1f

    // Nope! We're done
    ret

1:  // set underflow
    fldt        RELATIVE_ADDR( minldbl )                // { 0x1.0p-16382, result }
    fmul        %st(0),     %st(0)
    fstp        %st(0)
    ret
    
//0x1.p-64 > |x| || 0x1.p14 < |x| || isnan(x) 
2:  jg          3f

    // 0x1.p-64 > |x|
    fld1                                                // { 1, x }
    faddp                                               // { 1 + x }
    ret

  
//  0x1.p14 < |x| || isnan(x) 
3:
#if defined( __i386__ )
    call        0f
0:  popl        %ecx
#endif
    cmpl        $0,                     %edx            // if ( x < 0 ) 
    jl          4f                                      //      goto 4

    // 0x1.p14 < x || isnan(x) 
    fldt        RELATIVE_ADDR(maxldbl)                  // { LDBL_MAX, x }
    fmulp                                               // { x * LDBL_MAX }     set overflow, Inf   or quiet nan
    ret

    
    // -0x1.p14 > x || isnan(x)
4:  flds        RELATIVE_ADDR( minf )                   // { -inf, x }      set overflow, Inf  or quiet nan
    fucomip     %st(1), %st(0)                          // { x }
    je          6f

    // set underflow, return 0
5:  fldt        RELATIVE_ADDR( minldbl )                //  { LDBL_MIN, x }
    fmul        %st(0), %st(1)                          //  { LDBL_MIN, LDBL_MIN * x }
    fmul        %st(0), %st(1)                          //  { LDBL_MIN, LDBL_MIN * x }
    fchs                                                //  { -LDBL_MIN, LDBL_MIN * x }
    fmulp                                               //  { -LDBL_MIN * (LDBL_MIN * x) }
    ret
    
 
    // x == -inf  or nan 
6:  jp          5b

    // x == -inf
    fstp        %st(0)
    fldz
    ret
    
  


ENTRY( expl$fenv_access_off )
    fldt        FRAME_SIZE( STACKP )                    // { x }
    movswl      8+FRAME_SIZE( STACKP ), %eax            // sign + exponent
    movl        %eax,                   %edx            // sign + exponent
    andl        $0x7fff,                %eax            // exponent
    subl        $(16383-64),            %eax            // subtract out most of the bias -- push expoents less than -64 negative     
    cmpl        $(64+14),               %eax            // if( exponent < -64 ||  exponent > 14 || isnan(x)      
    ja          2f                                      //      goto 2
    
    fldl2e                                              // { log2(e), x }
    andl        $0x80000000,            %edx            // signof (x)
    fmul        %st(1),     %st(0)                      // { x * log2(e), x }
    orl         $0x3f000000,            %edx            // copysign( 0.5f, x )
    movl        %edx,                   FRAME_SIZE(STACKP)
    fadds       FRAME_SIZE( STACKP )                    //  { x * log2(e) + copysign( 0.5, x ), x }
    
    //truncate the top of stack
#if defined( __SSE3__ )
    fisttpl     FRAME_SIZE(STACKP)                      // { x }
    movl        FRAME_SIZE(STACKP),     %edx
    fildl       FRAME_SIZE(STACKP)                      // { round(2(x * log2(e)), x }
#else
    fstpl       FRAME_SIZE(STACKP)                      // { x }
    cvtsd2si    FRAME_SIZE(STACKP),     %edx
    movl        %edx,                   FRAME_SIZE(STACKP)
    fildl       FRAME_SIZE(STACKP)                      // { round(2(x * log2(e)), x }
#endif

    // PIC
#if defined( __i386__ )
    call        0f
0:  popl        %ecx
#endif

    // reduce x to the fractional part
    flds        RELATIVE_ADDR(ln2_hi)                   // { ln(2) hi part, round(2(x * log2(e)), x }
    fmul        %st(1),                 %st(0)          // { ln(2) hi part * round(2(x * log2(e)), round(2(x * log2(e)), x }
    fsubrp      %st(0),                 %st(2)          // { round(2(x * log2(e)), x - log2(e) hi part * round(2(x * log2(e)) }
    fldt        RELATIVE_ADDR(ln2_lo)                   // { ln2
    fmul        %st(1),                 %st(0)          // { round(2(x * log2(e)) * ln(2) lo part, round(2(x * log2(e)), x - log2(e) hi part * round(2(x * log2(e)) }
    fsubrp      %st(0),                 %st(2)          // { round(2(x * log2(e)), reduced x }
    fxch                                                // { reduced x,  round(2(x * log2(e)) }

    // multiply by log2(e)
    fldl2e                                              // { log2(e), x - ln(2) hi part * round(2(x * log2(e))  - round(2(x * log2(e)) * ln(2) lo part }
    fmulp                                               // { log2(e) * reduced x, round(2(x * log2(e)) }

    // calculate 2**(log2(e) * reduced x )
    f2xm1                                               // { 2**f - 1, round(2(x * log2(e)) }
    fld1                                                // { 1, 2**f - 1, round(2(x * log2(e)) }
    faddp                                               // { 2**f, round(2(x * log2(e)) }

    // scale results
    fscale                                              // { result, , round(2(x * log2(e)) }
    fstp        %st(1)
    ret

    
//0x1.p-64 > |x| || 0x1.p14 < |x| || isnan(x) 
2:  jg          3f

    // 0x1.p-64 > |x|
    fld1                                                // { 1, x }
    faddp                                               // { 1 + x }
    ret

  
//  0x1.p14 < |x| || isnan(x) 
3:
#if defined( __i386__ )
    call        0f
0:  popl        %ecx
#endif
    cmpl        $0,                     %edx            // if ( x < 0 ) 
    jl          4f                                      //      goto 4

    // 0x1.p14 < x || isnan(x) 
    fldt        RELATIVE_ADDR(maxldbl)                  // { LDBL_MAX, x }
    fmulp                                               // { x * LDBL_MAX }     set overflow, Inf   or quiet nan
    ret

    
    // -0x1.p14 > x || isnan(x)
4:  flds        RELATIVE_ADDR( minf )                   // { -inf, x }      set overflow, Inf  or quiet nan
    fucomip     %st(1), %st(0)                          // { x }
    je          6f

    // set underflow, return 0
5:  fldt        RELATIVE_ADDR( minldbl )                //  { LDBL_MIN, x }
    fmul        %st(0), %st(1)                          //  { LDBL_MIN, LDBL_MIN * x }
    fmul        %st(0), %st(1)                          //  { LDBL_MIN, LDBL_MIN * x }
    fchs                                                //  { -LDBL_MIN, LDBL_MIN * x }
    fmulp                                               //  { -LDBL_MIN * (LDBL_MIN * x) }
    ret
    
 
    // x == -inf  or nan 
6:  jp          5b

    // x == -inf
    fstp        %st(0)
    fldz
    ret
    

