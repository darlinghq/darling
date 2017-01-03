/* double round( double )
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
.globl _round
_round:
	movl	  8(%esp),			%ecx
	cmpl		$0x43300000,	%ecx
	fldl	  4(%esp)
	jae			2f

	fld			%st(0)					// { x, x }
	TRUNCATE							// { trunc(x), x }
	fxch								// { x, trunc(x) }
	fsub		%st(1),			%st(0)	// { frac(x), x }
	fadd		%st(0),			%st(0)	// { 2*frac(x), x }
	fld1								// { 1.0, 2*frac(x), x }
	fucomi		%st(1),			%st(0)
	fstp		%st(1)					// { 1.0, trunc(x) }
	ja			1f
	fadd		%st(0),			%st(1)	// { 1.0, trunc(x) + 1.0 }
1:	fstp		%st(0)					// { round(x) }
	ret
	
2:	andl		$0x7fffffff,	%ecx
	cmpl		$0x43300000,	%ecx
	jge			4f
	
	fabs
	fld			%st(0)
	TRUNCATE							// { trunc(x), x }
	fxch								// { x, trunc(x) }
	fsub		%st(1),			%st(0)	// { frac(x), x }
	fadd		%st(0),			%st(0)	// { 2*frac(x), x }
	fld1								// { 1.0, 2*frac(x), x }
	fucomi		%st(1),			%st(0)
	fstp		%st(1)					// { 1.0, trunc(x) }
	ja			3f
	fadd		%st(0),			%st(1)	// { 1.0, trunc(x) + 1.0 }
3:	fstp		%st(0)					// { round(x) }
	fchs
4:	ret

#else //x86_64

.const
.align 4
one:	.quad	0x3ff0000000000000
absmask:.quad   0x7fffffffffffffff
half:	.quad	0x3fe0000000000000
thresh: .quad	0x4330000000000000

.text
.align 4
.globl _round
_round:
	movd		%xmm0,			%rcx
	andq		absmask(%rip),	%rcx
	movsd		absmask(%rip),	%xmm2
	cmpq		thresh(%rip),	%rcx
	jae			1f
	
	cvttsd2si	%xmm0,			%rax
	andnpd		%xmm0,			%xmm2	// signbit(x)
	movsd		half(%rip),		%xmm3
	cvtsi2sd	%rax,			%xmm1	// trunc(x)
	subsd		%xmm1,			%xmm0	// frac(x)
	orpd		%xmm2,			%xmm1
	xorpd		%xmm2,			%xmm0	// |frac(x)|
	cmpltpd		%xmm3,			%xmm0	// (|frac(x)| < 0.5) ?
	andnpd		one(%rip),		%xmm0	// (|frac(x)| < 0.5) ? 0.0 : 1.0
	orpd		%xmm2,			%xmm0
	addsd		%xmm1,			%xmm0	// round(x)
1:	ret

#endif
