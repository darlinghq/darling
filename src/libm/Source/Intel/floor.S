/* double floor( double )
 *
 * Reimplemented by Steve Canon, based on Ian Ollmann's implementations
 * tuned for increased performance on in-order machines (but faster on
 * out-of-order machines as well).
 *
 * Copyright 2009, Apple Inc.
 */

#ifdef __i386__

#ifdef __SSE3__

.text
.align 4
.globl _floor
_floor:
	mov		  8(%esp),			%ecx
 	fldl      4(%esp)
	cmp			$0x43300000,	%ecx	// if x is negative or x > 0x1.0p53
	jae			1f						//    goto 1
	
	fisttpll  4(%esp)					// fast path if +0 <= x < 0x1.0p52
	fildll	  4(%esp)					// return trunc(x)
	ret
	
1:	and			$0x7fffffff,	%ecx
	sub			$1,				%ecx
	cmp			$0x43300000,	%ecx	// if |x| > 0x1.0p52 or isnan(x) or
	fld			%st(0)					// the high word of |x| is zero,
	jae			4f						//    goto 4
	
2:	fistpll	  4(%esp)
	fildll    4(%esp)					// rint(x)
	fucomi      %st(1),			%st(0)
	fstp        %st(1)
	jbe			3f						// if rint(x) <= x, return rint(x)
	fld1
	fsubrp      %st(0),			%st(1)	// otherwise, return rint(x) - 1.0
3:	ret

4:	jg			5f						// if |x| > 0x1.0p52 or isnan(x), return x
	testl		$0xffffffff,  4(%esp)	// if x is -denorm, go back through the
	jnz			2b						// main path, returning -1.0
5:	fstp		%st(1)					// otherwise, pop the extra copy of x, and
	ret									// return x.
	
#else // i386, no SSE3

.text
.align 4
.globl _floor
_floor:
	mov		  8(%esp),			%ecx
	and			$0x7fffffff,	%ecx	// high word of |x|
 	fldl      4(%esp)
	sub			$1,				%ecx
	cmp			$0x432fffff,	%ecx	// if |x| >= 0x1.0p52 or isnan(x) or iszero(x)
	fld         %st(0)					// or x is a denormal whose high 32 bits are
	jae			2f						// all zero, goto 2

0:	fistpll	  4(%esp)
	fildll    4(%esp)					// rint(x)
	fucomi      %st(1),			%st(0)
	fstp        %st(1)
	jbe         1f						// if rint(x) <= x, return rint(x)
	fld1
	fsubrp      %st(0),			%st(1)	// otherwise, return rint(x) - 1.0
1:	ret

2:	jg			3f						// if |x| >= 0x1.0p52 or isnan(x), goto 3
	testl		$0xffffffff,  4(%esp)	// if x is denorm (not zero)
	jnz			0b						//    jump back to the mainline
3:	fstp		%st(1)					// pop the extra copy of x off the stack
	ret									// and return x

#endif // SSE3
	
#else // x86_64

.const
.align 4
mone:	.quad	0xbff0000000000000
absmask:.quad   0x7fffffffffffffff
thresh: .quad	0x432fffffffffffff

.text
.align 4
.globl _floor
_floor:
	movd		%xmm0,			%rcx
	andq		absmask(%rip),	%rcx	// |x|
	subq		$1,				%rcx
	cmpq		thresh(%rip),	%rcx	// if |x| >= 0x1.0p52 or isnan(x) or iszero(x)
	jae			1f						//    early out, returning x.

	cvttsd2si   %xmm0,			%rax
	cvtsi2sd    %rax,			%xmm1	// trunc(x)
	cmpltsd		%xmm1,			%xmm0
	andpd		mone(%rip),		%xmm0	// x < trunc(x) ? -1.0 : 0.0
	addsd       %xmm1,			%xmm0	// floor(x)
1:  ret

#endif
