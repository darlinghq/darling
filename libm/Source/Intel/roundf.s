/* Single-precision roundf, reimplemented using integer operations
 * for improved performance, especially on in-order machines.
 *
 * Steve Canon, March 2009.
 */

#if defined __i386__

.text
.align 4
.globl _roundf
_roundf:
    mov		  4(%esp),			%eax	// load the input, x
	movss	  4(%esp),			%xmm0
    and			$0x7f800000,    %eax	// |x|
	mov			$23,			%ecx
	mov			$0x1,			%edx
	sar			%cl,			%eax	// exponent(x) + 1
	
	sub			$0x7e,			%eax	// if |x| < 1.0f. goto 2
	jbe			2f
	
	sub			%eax,			%ecx	// 23 - exponent(x) - 1
	js			1f						// return x if |x| >= 0x1.0p23
	
	shl			%cl,			%edx
	mov			$0xfffffffe,	%eax
	shl			%cl,			%eax
	add			%edx,		  4(%esp)	// add 0.5 (ish -- non-integral bits are garbage)
	and			%eax,		  4(%esp)	// truncate
	cvttps2dq	%xmm0,			%xmm0	// raise inexact
1:	flds	  4(%esp)
	ret
.align 4
2:	je			3f						// if |x| >= 0.5f, goto 3
	andl		$0x80000000,  4(%esp)	// copysign(0.0, x)
	cvttps2dq	%xmm0,			%xmm0	// raise inexact
	flds	  4(%esp)
	ret
.align 4
3:	addl		$0x00800000,  4(%esp)
	andl		$0xff800000,  4(%esp)	// copysign(1.0, x)
	cvttps2dq	%xmm0,			%xmm0	// raise inexact
	flds	  4(%esp)
	ret
	
#elif defined __x86_64__

.const
.align 4
mzero:	.long	0x80000000
.align 4
expbit: .long	0x00800000
.align 4
expmask:.long	0xff800000

.text
.align 4
.globl _roundf
_roundf:
	movd		%xmm0,			%eax
    and			$0x7f800000,    %eax	// |x|
	mov			$23,			%ecx
	mov			$0x1,			%edx
	sar			%cl,			%eax	// exponent(x) + 1
	
	sub			$0x7e,			%eax	// if |x| < 1.0f. goto 2
	jbe			2f
	
	sub			%eax,			%ecx	// 23 - exponent(x) - 1
	js			1f						// return x if |x| >= 0x1.0p23
	
	shl			%cl,			%edx
	mov			$0xfffffffe,	%eax
	shl			%cl,			%eax
	movd		%edx,			%xmm2
	movd		%eax,			%xmm3
	cvttps2dq	%xmm0,			%xmm1	// raise inexact
	paddd		%xmm2,			%xmm0	// add 0.5 (ish -- non-integral bits are garbage)
	pand		%xmm3,			%xmm0	// truncate
1:	ret
.align 4
2:	je			3f						// if |x| >= 0.5f, goto 3
	cvttps2dq	%xmm0,			%xmm1	// raise inexact
	andps		mzero(%rip),	%xmm0	// copysign(0.0, x)
	ret
.align 4
3:	cvttps2dq	%xmm0,			%xmm1	// raise inexact
	paddd		expbit(%rip),	%xmm0
	pand		expmask(%rip),	%xmm0	// copysign(1.0, x)
	ret

#else
	#error unknown arch
#endif
