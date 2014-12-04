//  long double scalbnl( long double x, int n );
//  long double ldexpl( long double x, int n );
//  long double scalblnl( long double x, long int n );
//
//  Returns x * 2^n computed efficiently, without undue over/underflow.
//  
//  -- Stephen Canon, January 2010


.text
.globl _scalblnl
.globl _scalbnl
.globl _ldexpl

#if defined __x86_64__

    #define     floatval    -24(%rsp)
    #define     exponent    -16(%rsp)
    #define     n           %edi
    #define     ln          %rdi
    #define     nw          %di

.align 4
_scalblnl:
    fldt        8(%rsp)                 // load x
    fld1
    fstpt       floatval                // conjure 1.0L
    
    cmp         $16382,     ln          // if n is so large that 2^n is not
    jg          L_bigPositiveN          // representable as a long double,
    cmp         $-16382,    ln          // branch to handle the scaling
    jl          L_bigNegativeN          // carefully.
    jmp         L_scaleX
    
.align 4
_scalbnl:
_ldexpl:  
    fldt        8(%rsp)                 // load x
    
#elif defined __i386__

    #define     floatval    4(%esp)
    #define     exponent    12(%esp)
    #define     n           %eax
    #define     nw          %ax
    
.align 4
_scalblnl:
_scalbnl:
_ldexpl:
    mov       20(%esp),     n           // load n
    fldt        floatval                // load x
    
#else
    #error "This implementation supports 32- and 64-bit Intel only"
#endif

    fld1
    fstpt       floatval                // conjure 1.0L
    
    cmp         $16382,     n           // if n is so large that 2^n is not
    jg          L_bigPositiveN          // representable as a long double,
    cmp         $-16382,    n           // branch to handle the scaling
    jl          L_bigNegativeN          // carefully.

L_scaleX:    
    add         nw,         exponent    // 2^n
    fldt        floatval
    fmulp                               // x * 2^n
    ret
    
L_bigPositiveN:
    addw        $16382,     exponent
    fldt        floatval                // 2^16382
    mov         $(3*16382), %edx
    cmp         %edx,       n
    cmovg       %edx,       n           // clamp n to 3 * 16382
0:  fmul        %st(0),     %st(1)      // do {
    sub         $16382,     n           //  x *= 2^16382
    cmp         $16382,     n           //  n -= 16382
    jg          0b                      // } (while n > 16382)
    fstp        %st(0)
    movw        $0x3fff,    exponent
    jmp         L_scaleX
    
L_bigNegativeN:
    addw        $-16382,    exponent
    fldt        floatval                // 2^-16382
    mov         $(-3*16382),%edx
    cmp         %edx,       n
    cmovl       %edx,       n           // clamp n to -3 * 16382
0:  fmul        %st(0),     %st(1)      // do {
    add         $16382,     n           //  x *= 2^-16382
    cmp         $-16382,    n           //  n += 16382
    jl          0b                      // } (while n < -16382)
    fstp        %st(0)
    movw        $0x3fff,    exponent
    jmp         L_scaleX
