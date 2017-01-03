/* double trunc( double )
 *
 * Reimplemented by Steve Canon, based on Ian Ollmann's implementations
 * tuned for increased performance on in-order machines (but faster on
 * out-of-order machines as well).
 *
 * Copyright 2009, Apple Inc.
 */

#ifdef __i386__

#ifdef __SSE3__
#define TRUNCATE						\
	fisttpll  4(%esp);					\
	fildll	  4(%esp)
#else
#define	TRUNCATE						\
	fnstcw	  4(%esp);					\
	movw	  4(%esp),			%dx;	\
	orw         $0xc00,		  4(%esp);	\
	fldcw	  4(%esp);					\
	frndint;							\
	movw        %dx,		  4(%esp);	\
	fldcw	  4(%esp)
#endif

.text
.align 4
.globl _trunc
_trunc:
	movl	  8(%esp),			%ecx
	cmpl		$0x43300000,	%ecx
	fldl	  4(%esp)
	jae			2f

	TRUNCATE
1:	ret
	
2:	andl		$0x7fffffff,	%ecx
	cmpl		$0x43300000,	%ecx
	jge			3f
	
	fabs
	TRUNCATE
	fchs
3:	ret

#else //x86_64

.const
.align 4
one:	.quad	0x3ff0000000000000
absmask:.quad   0x7fffffffffffffff
half:	.quad	0x3fe0000000000000
thresh: .quad	0x4330000000000000

.text
.align 4
.globl _trunc
_trunc:
	movd		%xmm0,			%rcx
	andq		absmask(%rip),	%rcx
	movsd		absmask(%rip),	%xmm2
	cmpq		thresh(%rip),	%rcx
	jae			1f
	
	cvttsd2si	%xmm0,			%rax
	andnpd		%xmm0,			%xmm2	// signbit(x)
	cvtsi2sd	%rax,			%xmm0	// trunc(x)
	orpd		%xmm2,			%xmm0
1:	ret

#endif
