//  float scalbnf( float x, int n );
//  float ldexpf( float x, int n );
//  float scalblnf( float x, long int n );
//
//  Returns x * 2^n computed efficiently, without undue over/underflow.
//  
//  -- Stephen Canon, January 2010


.text
.globl _scalblnf
.globl _scalbnf
.globl _ldexpf

#if defined __x86_64__

    #define     n           %edi
    #define     ln          %rdi
    
.align 4
_scalblnf:
    cvtss2sd    %xmm0,      %xmm0       // (double)x
    mov         $300,       %rdx
    cmp         %rdx,       ln          // if n > 300
    cmovg       %rdx,       ln          //    n == 300
    neg         %rdx
    cmp         %rdx,       ln          // if n < -300
    cmovl       %rdx,       ln          //    n == -300
    jmp         L_common
    
.align 4
_scalbnf:
_ldexpf:
    cvtss2sd    %xmm0,      %xmm0       // (double)x
    mov         $300,       %edx
    cmp         %edx,       n           // if n > 300
    cmovg       %edx,       n           //    n == 300
    neg         %edx
    cmp         %edx,       n           // if n < -300
    cmovl       %edx,       n           //    n == -300
L_common:
    add         $0x3ff,     n           //
    shl         $52,        ln          // (double)2^n
    movd        ln,         %xmm1
    mulsd       %xmm1,      %xmm0       // (double)(x * 2^n)
    cvtsd2ss    %xmm0,      %xmm0       // x * 2^n
    ret

#elif defined __i386__

    #define     n           %eax
    
.align 4
_scalblnf:
_scalbnf:
_ldexpf:
    cvtss2sd    4(%esp),    %xmm0       // (double)x
    mov         8(%esp),    n
    mov         $300,       %edx
    cmp         %edx,       n           // if n > 300
    cmovg       %edx,       n           //    n == 300
    neg         %edx
    cmp         %edx,       n           // if n < -300
    cmovl       %edx,       n           //    n == -300
    add         $0x3ff,     n           //
    movd        n,          %xmm1
    psllq       $52,        %xmm1       // (double)2^n
    mulsd       %xmm1,      %xmm0       // (double)(x * 2^n)
    cvtsd2ss    %xmm0,      %xmm0       // x * 2^n
    movss       %xmm0,      4(%esp)
    flds        4(%esp)
    ret
    
#else
    #error "This implementation supports 32- and 64-bit Intel only"
#endif
