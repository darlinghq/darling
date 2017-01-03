//  double hypot(double x, double y)
//  double cabs(double complex z)
//
//  Returns sqrt(x*x + y*y), computed without spurious overflow or underflow.
//
//  -- Stephen Canon, August 2009

#if !defined __i386__ && !defined __x86_64__
#error This implementation is for i386 and x86_64 only
#endif

.text
.globl _hypot
.globl _cabs

#if defined __i386__

#define ABSHI $0x7fffffff
#define INFHI $0x7ff00000

//  Entry point --------------------------------------------------------

.align 4
_cabs:
_hypot:
    mov       8(%esp),      %eax
    mov      16(%esp),      %edx
    mov         ABSHI,      %ecx
    
//  Detect special cases -----------------------------------------------
    
    and         %ecx,       %eax    // |x|
    jz          L_xHiIsZero         // 
L_returnFromXHiIsZero:
    and         %ecx,       %edx    // |y|
    jz          L_yHiIsZero         //
L_returnFromYHiIsZero:
    cmp         INFHI,      %eax    // we need to detect if either x or y is
    jae         L_xIsSpecial        // an infinity and return +infinity if so;
    cmp         INFHI,      %edx    // this is because hypot(inf, NaN) needs
    jae         L_returnAbsY        // to return +inf, not NaN.
    
//  Mainline computation -----------------------------------------------

    fldl      4(%esp)               // x87 stack: { x }
    fmul        %st(0),     %st(0)  //            { x*x }
    fldl     12(%esp)               //            { y x*x }
    fmul        %st(0),     %st(0)  //            { y*y x*x }
    faddp                           //            { y*y+x*x }
    fsqrt                           //            { sqrt(y*y + x*x) }
    fstpl     4(%esp)               // round to double
    fldl      4(%esp)
    ret

//  Special case handling ----------------------------------------------

L_xHiIsZero:
    cmp         $0,       4(%esp)   // is the low word of x zero?
    jnz     L_returnFromXHiIsZero   // if not, jump back to mainline
L_returnAbsY:
    and         %ecx,    16(%esp)
    fldl     12(%esp)
    ret

L_yHiIsZero:
    cmp         $0,      12(%esp)   // is the low word of y zero?
    jnz     L_returnFromYHiIsZero   // if not, jump back to mainline
L_returnAbsX:
    and         %ecx,     8(%esp)
    fldl      4(%esp)
    ret
    
L_xIsSpecial:
    cmp         INFHI,      %edx    // check if y is infinity
    jnz         L_returnAbsX
    cmp         $0,      12(%esp)
    jz          L_returnAbsY
    jmp         L_returnAbsX
    
#else // defined __i386__

#define ABSMASK $0x7fffffffffffffff
#define INFINITY $0x7ff0000000000000

//  Entry point --------------------------------------------------------

.align 4
_cabs:
_hypot:
    movd        %xmm0,      %rax
    movd        %xmm1,      %rdx
    mov         ABSMASK,    %rcx
    
//  Detect special cases -----------------------------------------------
    
    and         %rcx,       %rax    // |x|
    jz          L_returnAbsY        // 
    and         %rcx,       %rdx    // |y|
    jz          L_returnAbsX        //
    mov         INFINITY,   %rdi    //
    
    movsd       %xmm0,   -8(%rsp)   // store x to the red zone
    movsd       %xmm1,  -16(%rsp)   // store y to the red zone
    
    cmp         %rdi,       %rax    // we need to detect if either x or y is
    jae         L_xIsSpecial        // an infinity and return +infinity if so;
    cmp         %rdi,       %rdx    // this is because hypot(inf, NaN) needs
    jae         L_returnAbsY        // to return +inf, not NaN.
    
//  Mainline computation -----------------------------------------------
    
    fldl     -8(%rsp)               // x87 stack: { x }
    fmul        %st(0),     %st(0)  //            { x*x }
    fldl    -16(%rsp)               //            { y x*x }
    fmul        %st(0),     %st(0)  //            { y*y x*x }
    faddp                           //            { y*y+x*x }
    fsqrt                           //            { sqrt(y*y + x*x) }
    fstpl    -8(%rsp)               // round result to double
    movsd    -8(%rsp),      %xmm0   // and load in xmm register to return
    ret

//  Special case handling ----------------------------------------------

L_xIsSpecial:                       // x is either inf or NaN
    cmp         %rdi,       %rdx
    jz          L_returnAbsY        // if y is infinity, return |y|
L_returnAbsX:
    movd        %rax,       %xmm0   // otherwise, return |x|
    ret
L_returnAbsY:
    and         %rcx,       %rdx
    movd        %rdx,       %xmm0   // return |y|
    ret
    
#endif // defined __i386__
