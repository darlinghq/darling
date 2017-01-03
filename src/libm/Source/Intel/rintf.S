//  float rintf(float x)
//
//  returns the argument rounded to an integral value using the prevailing
//  rounding mode.  rintf is allowed to raise the "inexact" flag if the result
//  differs from the argument (C99 7.12.9.4).
//
//  -- Stephen Canon, January 2010

.text
.align 4
.globl _rintf
_rintf:
#if defined __i386__
    movl        4(%esp),        %ecx
    movss       4(%esp),        %xmm0
#else
    movd        %xmm0,          %ecx
#endif
    mov         %ecx,           %edx
    and         $0x7fffffff,    %ecx    // |x|
    xor         %ecx,           %edx    // signbit(x)
    movd        %edx,           %xmm1
    cmp         $0x4b000000,    %ecx    // if |x| >= 2^23 or isnan(x)
    jae         L_xIsIntegral           //    then the result is just x
    
    cvtps2dq    %xmm0,          %xmm0
    cvtdq2ps    %xmm0,          %xmm0
    orps        %xmm1,          %xmm0   // restore signbit
    
#if defined __i386__
    movss       %xmm0,          4(%esp)
L_xIsIntegral:
    flds        4(%esp)
    ret
#else
L_xIsIntegral:
    ret
#endif
