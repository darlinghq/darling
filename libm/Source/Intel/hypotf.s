//  float hypotf(float x, float y)
//  float cabsf(float complex z)
//
//  Returns sqrt(x*x + y*y), computed without spurious overflow or underflow.
//
//  -- Stephen Canon, August 2009

#if !defined __i386__ && !defined __x86_64__
#error This implementation is for i386 and x86_64 only
#endif

#define ABSMASK $0x7fffffff
#define INFINITY $0x7f800000

.text
.globl _cabsf
.globl _hypotf

//  Entry points -------------------------------------------------------

#if defined __i386__
.align 4
_cabsf:                             // on i386, we can use the same code for
_hypotf:                            // hypotf and cabsf, because the arguments
    mov       4(%esp),      %eax    // come in at the same stack offsets
    mov       8(%esp),      %edx    //
    movss     4(%esp),      %xmm0   // real at esp + 4
    movss     8(%esp),      %xmm1   // imag at esp + 8
#else
.align 4                            // however, on x86_64, the registers used
_cabsf:                             // are different.  cabsf's arguments come
    pshufd  $0xfd,  %xmm0,  %xmm1   // in packed in xmm0.
.align 4
_hypotf:                            //
    movd        %xmm0,      %eax    // hypotf, on the other hand, gets x in
    movd        %xmm1,      %edx    // xmm0 and y in xmm1.
#endif
    mov         ABSMASK,    %ecx

//  Detect special cases -----------------------------------------------

    and         %ecx,       %eax    // |x|
    and         %ecx,       %edx    // |y|
    
    cmp         INFINITY,   %eax    // we need to detect if either x or y is
    jz          L_returnAbsX        // an infinity and return +infinity if so;
    cmp         INFINITY,   %edx    // this is because hypot(inf, NaN) needs
    jz          L_returnAbsY        // to return +inf, not NaN.
    
//  Mainline -----------------------------------------------------------

    cvtss2sd    %xmm0,      %xmm0   // Naive computation of sqrt(x*x + y*y)
    cvtss2sd    %xmm1,      %xmm1   // in double.  The extra exponent range
    mulsd       %xmm0,      %xmm0   // and precision protect against spurious
    mulsd       %xmm1,      %xmm1   // over/underflow and the double rounding
    addsd       %xmm1,      %xmm0   // is inconsequential, because hypot
    sqrtsd      %xmm0,      %xmm0   // does not need to be correctly rounded.
    cvtsd2ss    %xmm0,      %xmm0   // 
    
#ifdef __i386__
    movss       %xmm0,    4(%esp)
    flds      4(%esp)
#endif
    ret

//  Special case handling ----------------------------------------------

#ifdef __i386__
L_returnAbsY:
    mov         %edx,     4(%esp)
    flds      4(%esp)
    ret
L_returnAbsX:
    mov         %eax,     4(%esp)
    flds      4(%esp)
    ret
#else
L_returnAbsY:
    movd        %edx,       %xmm0
    ret
L_returnAbsX:
    movd        %eax,       %xmm0
    ret
#endif
