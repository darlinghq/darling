/*
 * Adapted from original written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 *	by Ian Ollmann, Apple Computer 2006
 */

.text
.align 4
.globl _fmodf
_fmodf:
#if defined __x86_64__
    #define xf        -4(%rsp)
    #define yf        -8(%rsp)
    movss   %xmm1,      yf
    movss   %xmm0,      xf
#elif defined __i386__
    #define xf         4(%esp)
    #define yf         8(%esp)
#else
    #error "This implementation supports only 32- and 64-bit intel."
#endif
    flds    yf
    flds    xf
1:  fprem
    fnstsw  %ax
    btw     $10,        %ax
    jc      1b
    fstp    %st(1)
#if defined __x86_64__
    fstps   xf
    movss   xf,         %xmm0
#endif
    ret
    
.text
.align 4
.globl _fmod
_fmod:
#if defined __x86_64__
    #define x         -8(%rsp)
    #define y        -16(%rsp)
    movsd   %xmm1,      y
    movsd   %xmm0,      x
#elif defined __i386__
    #define x          4(%esp)
    #define y         12(%esp)
#else
    #error "This implementation supports only 32- and 64-bit intel."
#endif
    fldl    y
    fldl    x
1:  fprem
    fnstsw  %ax
    btw     $10,        %ax
    jc      1b
    fstp    %st(1)
#if defined __x86_64__
    fstpl   x
    movsd   x,          %xmm0
#endif
    ret
    
.text
.align 4
.globl _fmodl
_fmodl:
#if defined __x86_64__
    #define xl         8(%rsp)
    #define yl        24(%rsp)
#elif defined __i386__
    #define xl         4(%esp)
    #define yl        20(%esp)
#else
    #error "This implementation supports only 32- and 64-bit intel."
#endif
	fldt	yl
	fldt	xl
1:	fprem
	fstsw	%ax
	btw		$10,%ax
	jc		1b
	fstp	%st(1)
	ret

