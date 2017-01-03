//  long double hypotl(long double x, long double y)
//  long double cabsl(long double complex z)
//
//  Returns sqrt(x*x + y*y), computed without spurious overflow or underflow.
//  This implementation does not attempt to deliver sub-ulp accurate results.
//
//  -- Stephen Canon, August 2009


//  Check that we are being built for Intel -----------------------------------

#if !defined __i386__ && !defined __x86_64__
#error This implementation is for i386 and x86_64 only
#endif

//  Macros to unify x86_64 and i386 source ------------------------------------

#ifdef __i386__
    #define FIRSTARG     4(%esp)
    #define SECONDARG   20(%esp)
#else
    #define FIRSTARG     8(%rsp)
    #define SECONDARG   24(%rsp)
#endif

//  Entry point ---------------------------------------------------------------

.text
.globl _hypotl
.globl _cabsl
.align 4
_hypotl:
_cabsl:
    
//  Check argument scaling ----------------------------------------------------
//    
//  Do a fast integer check on the exponent fields of both x and y.  If the
//  biased exponent of either is outside the range [0x2000,0x5ffe], jump to
//  L_beCareful, where we will do more careful checks for special cases and
//  guard against undue overflow or underflow.
//
//  The biased exponent range [0x2000,0x5ffe] corresponds to unbiased exponents
//  from -0x1fff to 0x1fff; numbers with exponents outside of this range will
//  underflow or overflow when they are squared, preventing use of the naive
//  algorithm.

    movl      8+FIRSTARG,       %ecx
    andl        $0x7fff,        %ecx
    movl        %ecx,           %eax
    movl      8+SECONDARG,      %edx  
    subl        $0x2000,        %ecx
    andl        $0x7fff,        %edx
    cmpl        $0x3ffe,        %ecx  
    ja          L_beCareful
    
    movl        %edx,           %ecx
    subl        $0x2000,        %ecx
    cmpl        $0x3ffe,        %ecx
    ja          L_beCareful
    
//  Naive computation ---------------------------------------------------------
//
//  At this point we know that both x and y are squareable without risk of
//  overflow or underflow, so we do the naive computation:
//
//    return sqrtl(x*x + y*y)
                                        
    fldt        FIRSTARG                //  x
    fmul        %st(0),         %st(0)  // x^2
    fldt        SECONDARG               //  y  x^2
    fmul        %st(0),         %st(0)  // y^2 x^2
    faddp                               // x^2+y^2
    fsqrt                               // result
    ret
    
//  Edge case returns ---------------------------------------------------------
//
//  These are called from L_beCareful (below) to generate the correct return
//  value when one of the arguments is zero, infinity, or NaN.  The column
//  of comments to the right contains the state of the x87 stack after the
//  instruction that is on the same line, or at the time of the jump in the
//  case of labels.
//
//  hypot and cabs are a little bit special in that if one argument is infinity
//  and the other NaN, the C99 return value is infinity, not NaN.  Put simply,
//  if either argument is infinity, we return infinity.  Otherwise if either
//  argument is zero or NaN, we return |x|+|y|.

L_yIsInfOrNaN:                          // inf |y| |x|
    jp          L_yIsNaN                // inf |y| |x|
    fstp        %st(1)                  // inf |x|
    fstp        %st(1)                  // inf
    ret
L_xIsInfOrNaN:                          // |y| |x|
    jp          L_xIsNaN                // |y| |x|
    fstp        %st(0)                  // |x|
    ret
L_yIsNaN:                               // inf |y| |x|
    fucomip     %st(2),         %st(0)  // |y| |x|
    jz          L_xIsInfOrNaN           // |y| |x|
L_xIsNaN:                               // |y| |x|
    faddp                               // |y|+|x|
    ret
L_yIsZero:                              // 0.0 |y| |x|
    fstp        %st(0)                  // |y| |x|
L_xIsZero:                              // |y| |x|
    faddp                               // |y|+|x|
    ret

//  Detect special cases ------------------------------------------------------
//
//  Here we know that one or both arguments are either edge cases or are
//  outside the range of numbers that can be squared without overflow or
//  underflow.  Begin by loading both arguments on the x87 stack, and checking
//  to see if either is zero, infinity, or NaN:

#ifdef __i386__
    #define PIC         call 0f; 0: pop %ecx;
    #define INFINITY    L_infinity-0b(%ecx)
    #define TINY        L_tiny-0b(%ecx)
#else
    #define PIC         // nothing
    #define INFINITY    L_infinity(%rip)
    #define TINY        L_tiny(%rip)
#endif

L_beCareful:
    PIC                                 // x87 stack
    fldt        FIRSTARG                //  x
    fabs                                // |x|
    fldt        SECONDARG               //  y  |x|
    fabs                                // |y| |x|
    
    fldt        INFINITY                // inf |y| |x|
    fucomi      %st(1),         %st(0)  // inf |y| |x|
    jz          L_yIsInfOrNaN           // inf |y| |x|
    fucomip     %st(2),         %st(0)  // |y| |x|
    jz          L_xIsInfOrNaN           // |y| |x|
    
    fldz                                // 0.0 |y| |x|
    fcomi       %st(1),         %st(0)  // 0.0 |y| |x|
    jz          L_yIsZero               // 0.0 |y| |x|
    fcomip      %st(2),         %st(0)  // |y| |x|
    jz          L_xIsZero               // |y| |x|
       
//  Detect argument scaling ---------------------------------------------------

    sub         %edx,           %eax
    jl          L_yIsLarger
    
    // We want to have the argument with larger magnitude on the top of the x87
    // stack; if x is larger in magnitude than y, we exchange them on the stack
    fxch
    
    // If exponent(x) - exponent(y) <= 32, we need to do a careful rescaling
    // computation.  Otherwise, we y only contributes rounding to the final
    // result, so we can use a simpler approach.
    cmp         $32,            %eax
    jle          L_scaledHypot
    
//  Scaled computation of |hi| + eps ------------------------------------------
//
//  At this point, we know that x and y are vastly different in scale, and the
//  smaller of the two contributes only to the rounding of the last bit of the
//  larger, which we call "hi".
//
//  Instead of doing a normal hypot computation, just add a small term to the
//  significand to force rounding.
//
//  We do this addition in the significand space; begin by splitting hi into
//  hisig and hiexp that satisfy:
//
//          hi = hisig * 2^hiexp   and   1.0 <= hisig < 2.0
//
//  Compute hisigRounded = hisig + tiny.
//
//  Rescale: result = hisigRounded * 2^hiexp
//
//  Note that if hi is denormal, this will cause a double rounding.  We don't
//  protect against this because (a) we're not trying for sub-ulp accuracy,
//  and (b) long double denormal are exceptionally rare.  If one wanted to do
//  the "really really right thing", a simple exponent check would allow one
//  to protect against this.

L_forceRounding:
    fstp        %st(1)                  // pop the smaller argument
    fxtract                             // extract hisig, hiexp
    fldt        TINY                    // load tiny
    faddp                               // hisig + tiny
    fscale                              // compute the final result and pop
    fstp        %st(1)                  // hiexp off the stack.
    ret
          
//  y >~ x --------------------------------------------------------------------

L_yIsLarger:
    // If exponent(x) - exponent(y) < -32, we don't need to be careful, because
    // x only affects the rounding of the last bit of |y|.  Otherwise, do a
    // careful scaled computation.
    cmp         $-32,           %eax
    jl          L_forceRounding
    
//  Scaled computation of sqrt(x^2 + y^2) -------------------------------------
//
//  At this point, we know that x and y are approximately equal in scale
//  (within 2^32 of eachother), that squaring them will result in overflow or
//  underflow, and that they are on the top of the floating-point stack -- in
//  particular, the larger of the two is in st(0), and the smaller is in st(1).
//  We'll call the larger one "hi" and the smaller one "lo".
//
//  In order to compute sqrt(x^2 + y^2) without undue overflow or underflow,
//  begin by splitting hi and lo into exponents and significands that satisfy:
//
//          hi = hisig * 2^hiexp   and   1.0 <= hisig < 2.0
//          lo = losig * 2^loexp   and   1.0 <= losig < 2.0
//
//  Because of the checks we have already done on the exponents, we know that:
// 
//          -32 <= (loexp - hiexp) <= 0
//
//  Let scaledlo be losig * 2^(loexp - hiexp), and note that the following hold:
//
//          0x1.0p-33 < scaledlo < 2.0   and   lo = scaledlo * 2^(hiexp)
//          
//  Now compute a scaled result using hisig and scaledlo:
//
//          scaledResult = sqrt(hisig^2 + scaledlo^2)
//
//  Note that because of the bounds we have on hisig and scaledlo, this
//  computation cannot cause overflow or underflow.  Next, apply the proper
//  scaling to get the final result, which may overflow, but such overflow
//  is justified:
//
//          result = scaledResult * 2^hiexp

L_scaledHypot:
    fxtract                         // Extract hisig and hiexp.
    
    fxch        %st(2)              // Move lo to the top of the stack and
    fxtract                         // extract losig and loexp.
    
    fxch                            // Move loexp to the top of the stack and
    fsub        %st(2),     %st(0)  // compute (loexp - hiexp).
    
    fxch                            // Move losig to the top of the stack and
    fscale                          // scale by (loexp - hiexp) to compute
    fstp        %st(1)              // scaledlo.
    
    fmul        %st(0),     %st(0)  // scaledlo^2.
    
    fxch        %st(2)              // move hisig to the top of the stack and
    fmul        %st(0),     %st(0)  // compute hisig^2.
    
    faddp       %st(0),     %st(2)  // compute hisig^2 + scaledlo^2, and move
    fxch                            // it to the top of the stack.  Then take
    fsqrt                           // the square root to get scaledResult.
    
    fscale                          // apply the scale hiexp to get the final
    fstp        %st(1)              // result and return.
    ret

//  Useful constants ----------------------------------------------------------

.literal16
.align 4
L_infinity:
.quad 0x8000000000000000, 0x0000000000007fff
L_tiny:
.quad 0x8000000000000000, 0x0000000000003f00
