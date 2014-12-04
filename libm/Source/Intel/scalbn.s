//  double scalbn( double x, int n );
//  double ldexp( double x, int n );
//  double scalb( double x, double n );
//  double scalbln( double x, long int n );
//
//  Returns x * 2^n computed efficiently, without undue over/underflow.
//  Adapted from Ian Ollmann's scalbn implementation of 2005.
//  
//  -- Stephen Canon, January 2010

.globl _scalbn
.globl _ldexp
.globl _scalb
.globl _scalbln

#if defined __x86_64__

.literal8
threek:  .quad 0x40a7700000000000
mthreek: .quad 0xc0a7700000000000

    #define     floatval    -24(%rsp)
    #define     n           %edi
    #define     ln          %rdi
    #define     clamphi     threek(%rip)
    #define     clamplo     mthreek(%rip)
  
.text
.align 4
_scalb:
    movsd       %xmm0,      floatval
    pcmpeqd     %xmm0,      %xmm0
    fldl        floatval                // load x on x87
    psllq       $63,        %xmm0       // conjure  long dbl mantissa of 2^n
    ucomisd     %xmm1,      %xmm1       // if isnan(n)
    jp          L_nIsNaN                //    branch to return n
    
    minsd       clamphi,    %xmm1       // clamp n to [-3000, 3000]
    maxsd       clamplo,    %xmm1
    cvtsd2si    %xmm1,      n           // convert clamped n to integer
    jmp         L_common                // and jump to the shared scalbn path
    
L_nIsNaN:
    fstp        %st(0)
    movapd      %xmm1,      %xmm0       // since n is a NaN, we just return n
    ret

.align 4
_scalbln:
    movsd       %xmm0,      floatval
    pcmpeqd     %xmm0,      %xmm0
    fldl        floatval                // load x on x87
    psllq       $63,        %xmm0       // conjure  long dbl mantissa of 2^n
    mov         $3000,      %rdx
    cmp         %rdx,       ln          // if n > 3000
    cmovg       %rdx,       ln          //    n == 3000
    neg         %rdx
    cmp         %rdx,       ln          // if n < -3000
    cmovl       %rdx,       ln          //    n == -3000
    jmp         L_common

.align 4
_scalbn:
_ldexp:
    movsd       %xmm0,      floatval
    fldl        floatval                // load x on x87
    pcmpeqd     %xmm0,      %xmm0
    psllq       $63,        %xmm0       // conjure  long dbl mantissa of 2^n
    mov         $3000,      %edx
    cmp         %edx,       n           // if n > 3000
    cmovg       %edx,       n           //    n == 3000
    neg         %edx
    cmp         %edx,       n           // if n < -3000
    cmovl       %edx,       n           //    n == -3000
    
L_common:    
    add         $0x3fff,    n           // long dbl exponent field of 2^n
    pinsrw      $4, n,      %xmm0       // insert exponent to make 2^n
    movdqa      %xmm0,      floatval    
    fldt        floatval
    fmulp                               // x * 2^n
    fstpl       floatval                // round to double
    movsd       floatval,   %xmm0
    ret
    
#elif defined __i386__

.literal8
threek:  .quad 0x40a7700000000000
mthreek: .quad 0xc0a7700000000000

    #define     floatval    4(%esp)
    #define     exponent    12(%esp)
    #define     n           %eax
    #define     nw          %ax
    #define     clamphi     (threek-0b)(%ecx)
    #define     clamplo     (mthreek-0b)(%ecx)

.text
.align 4
_scalb:
    call        0f    
0:  pop         %ecx                    // conjure pic info
    movsd       12(%esp),   %xmm1       // load n
    fldl        floatval                // load x on x87
    ucomisd     %xmm1,      %xmm1       // if isnan(n)
    jp          L_nIsNaN                //    branch to return n
    
    fld1
    fstpt       floatval                // conjure 1.0L
    minsd       clamphi,    %xmm1       // clamp n to [-3000, 3000]
    maxsd       clamplo,    %xmm1
    cvtsd2si    %xmm1,      n           // convert clamped n to integer
    jmp         L_common                // and jump to the shared scalbn path
    
L_nIsNaN:
    fstp        %st(0)
    fldl        12(%esp)                // n is nan, return n
    ret

.align 4
_scalbln:
_scalbn:
_ldexp:
    mov         12(%esp),   n           // load n
    fldl        floatval                // load x on x87
    fld1
    fstpt       floatval                // conjure 1.0L
    mov         $3000,      %edx
    cmp         %edx,       n           // if n > 3000
    cmovg       %edx,       n           //    n == 3000
    mov         $-3000,     %edx
    cmp         %edx,       n           // if n < -3000
    cmovl       %edx,       n           //    n == -3000
    
L_common:
    add         nw,         exponent    // 2^n in memory
    fldt        floatval
    fmulp                               // x * 2^n
    fstpl       floatval                // round to double
    fldl        floatval
    ret
    
#else
    #error "This implementation supports 32- and 64-bit Intel only"
#endif

