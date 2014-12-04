//  double rint(double x)
//
//  returns the argument rounded to an integral value using the prevailing
//  rounding mode.  rintf is allowed to raise the "inexact" flag if the result
//  differs from the argument (C99 7.12.9.4).
//
//  -- Stephen Canon, January 2010

.globl _rint

#if defined __i386__

.text
.align 4
_rint:
    fldl        4(%esp)
    frndint
    ret
    
#elif defined __x86_64__

.literal8
absmask:    .quad 0x7fffffffffffffff
twop52:     .quad 0x4330000000000000

.text
.align 4
_rint:
    movd        %xmm0,          %rcx
    mov         %rcx,           %rdx
    and         absmask(%rip),  %rcx    // |x|
    xor         %rcx,           %rdx    // signbit(x)
    movd        %rdx,           %xmm1
    cmp         twop52(%rip),   %rcx    // if |x| >= 0x1.0p52 or isnan(x)
    jae         L_xIsIntegral           //    just return x
    
    cvtsd2si    %xmm0,          %rax
    cvtsi2sd    %rax,           %xmm0
    orpd        %xmm1,          %xmm0   // restore signbit
L_xIsIntegral:
    ret

#else
    #error "this implementation only supports 32- and 64-bit intel."
#endif
