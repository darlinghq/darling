
/*
 *  modf.s
 *
 *      by Steve Canon (scanon)
 *
 *  Copyright (c) 2008 Apple Inc. All Rights Reserved.
 *
 *  Rewritten by Steve Canon in December '08 to fix
 *  behavior of small signaling nans and to get the
 *  sign of a zero fractional part correct.
 */

// double modf(double x, double *iptr);
//
// breaks x into integral and fractional parts, each of which has the same sign as the argument.
// the fractional part is returned, and iptr holds the integral part.
//
// Special Cases:
//
//		Input	Fractional Part		Integral Part
//		+-inf		+-0					+-inf
//		NaN			NaN					NaN

#if defined __i386__

.text
.align 4
.globl _modf
_modf:
	movl	   8(%esp),		%eax	// high word of x
	andl	   $0x7fffffff,	%eax	// high word of |x|
	subl	   $0x3ff00000,	%eax	// subtract off exponent bias
	
	movsd	   4(%esp),		%xmm0	// x
	pcmpeqb		%xmm3,		%xmm3
	psllq	   $63,			%xmm3
	movapd		%xmm3,		%xmm4	// set aside signbit mask for later use
	andpd		%xmm0,		%xmm3	// signbit(x)
	
	cmpl	   $0x03400000,	%eax	// compare to unbiased 2**52
	jae			1f
	
//	1.0 <= |x| < 2**52
	shrl	   $20,			%eax	// unbiased exponent of x
	movl	   $52,			%edx
	subl		%eax,		%edx
	movd		%edx,		%xmm2	// 52 - unbiased exponent of x
	pcmpeqb		%xmm1,		%xmm1
	psllq		%xmm2,		%xmm1	// mask for integral bits of x
	andpd		%xmm0,		%xmm1	// trunc(x)
	subsd		%xmm1,		%xmm0	// fractional part, except that sign of zero may be wrong
	andnpd		%xmm0,		%xmm4	// |fractional part|
	orpd		%xmm3,		%xmm4	// copysign(fractional part, x)
	
	movl	  12(%esp),		%ecx	// iptr
	movsd		%xmm4,	   4(%esp)
	movsd		%xmm1,	    (%ecx)	// store integral part to iptr
	fldl	   4(%esp)				// return fraction part on the x87 stack
	retl
	
1:	movl	  12(%esp),		%ecx	// iptr
	jge			2f

//	|x| < 1.0
	movsd		%xmm3,		(%ecx)	// *iptr = copysign(0.0, x)
	fldl	   4(%esp)				// fractional part = x
	retl
	
2:	ucomisd		%xmm0,		%xmm0	// check for NaN
	jp			3f

//	|x| >= 2**52
	movsd		%xmm3,	   4(%esp)	// fractional part = copysign(0.0, x)
3:	movsd		%xmm0,		(%ecx)	// *iptr = x
	fldl	   4(%esp)
	retl
	
#else // __x86_64__

.const
.align 4
absmask:
	.quad	0x7fffffffffffffff
one:
	.quad	0x3ff0000000000000

.text
.align 4
.globl _modf
_modf:
	movsd	absmask(%rip),	%xmm1
	andpd		%xmm0,		%xmm1	// |x|
	movd		%xmm1,		%rax
	xorpd		%xmm0,		%xmm1	// copysign(0.0, x)
	subq	one(%rip),		%rax	// subtract off exponent bias
	movq	   $52,			%rcx
	sarq		%cl,		%rax
	cmpq		%rcx,		%rax	// compare exponent of x to 52
	jae			1f
	
//	1.0 <= |x| < 2**52
	subq		%rax,		%rcx	// 52 - unbiased exponent of x
	pcmpeqb		%xmm2,		%xmm2
	movd		%rcx,		%xmm3
	psllq		%xmm3,		%xmm2	// mask for integral bits of x
	andpd		%xmm0,		%xmm2	// trunc(x)
	subsd		%xmm2,		%xmm0	// fractional part, except sign of zero may be wrong
	andpd	absmask(%rip),	%xmm0	// |fractional part|
	orpd		%xmm1,		%xmm0	// copysign(fractional part, x)
	movsd		%xmm2,		(%rdi)	// *iptr = trunc(x)
	retq
	
1:	jge			2f
	
//	|x| < 1.0
	movsd		%xmm1,		(%rdi)	// *iptr = copysign(0.0, x)
	retq
	
2:	ucomisd		%xmm0,		%xmm0	// check for NaN
	jp			3f
	
//	|x| >= 2**52
	movsd		%xmm0,		(%rdi)	// *iptr = x
	movapd		%xmm1,		%xmm0	// fractional part = copysign(0.0, x)
	retq
	
3:	movsd		%xmm0,		(%rdi)	// *iptr = x
	retq

#endif // __ARCH__
