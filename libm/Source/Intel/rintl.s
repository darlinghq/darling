//  long double rintl(long double x)
//
//  returns the argument rounded to an integral value using the prevailing
//  rounding mode.  rintf is allowed to raise the "inexact" flag if the result
//  differs from the argument (C99 7.12.9.4).
//
//  -- Stephen Canon, January 2010

#if defined __i386__

    #define     input       4(%esp)
    #define     exponent    12(%esp)
    
#elif defined __x86_64__

    #define     input       8(%rsp)
    #define     exponent    16(%rsp)
    
#else
    #error "this implementation only supports 32- and 64-bit intel."
#endif
    
.text
.align 4
.globl _rintl
_rintl:
    movswl      exponent,       %eax
    fldt        input
    mov         %eax,           %edx
    and         $0x7fff,        %eax    // biased exponent of x
    and         $0x80000000,    %edx    // signbit of x
    sub         $16383,         %eax    // unbiased exponent of x
    cmp         $63,            %eax    // if |x| < 1.0 or |x| >= 0x1.0p63 or isnan(x)
    jae         1f
    
    or          $0x5f000000,    %edx    // copysignf( 0x1.0p63f, x )
    mov         %edx,           input
    flds        input
    fadd        %st(0),         %st(1)  // x + copysign(0x1.0p63f, x)
    fsubrp                              // rintl(x)
    ret
    
1:  jge         2f
    fistps      input
    or          $0x3f800000,    %edx    // copysignf(1.0f,x)
    filds       input                   // rintl(x), up to sign of zero
    mov         %edx,           input
    fabs
    fmuls       input                   // restore sign of zero
    
2:  ret
