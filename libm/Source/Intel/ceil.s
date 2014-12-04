/* double ceil(double )
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
.globl _ceil
_ceil:
	mov		  8(%esp),			%ecx
 	fldl      4(%esp)
	cmp			$0x43300000,	%ecx	// if x is negative or x > 0x1.0p53
	jae			2f						//    goto 2
	
	fld			%st(0)
	fistpll	  4(%esp)
	fildll    4(%esp)					// rint(x)
	fucomi      %st(1),			%st(0)
	fstp        %st(1)
	jae			1f						// if rint(x) >= x, return rint(x)
	fld1
	faddp		%st(0),			%st(1)	// otherwise, return rint(x) + 1.0
1:	ret

2:	and			$0x7fffffff,	%ecx
	cmp			$0x43300000,	%ecx	// if |x| >= 0x1.0p53
	jae			3f
	
	fisttpll  4(%esp)
	fldz
	fildll	  4(%esp)					// ceil(x), up to sign of zero
	fucomi		%st(1),			%st(0)	// if result == 0
	fstp		%st(1)					//
	jne			3f						//
	fchs								// patch up sign bit
3:	ret

#else // i386, no SSE3

.text
.align 4
.globl _ceil
_ceil:
	mov		  8(%esp),			%ecx
 	fldl      4(%esp)
	cmp			$0x43300000,	%ecx	// if x is negative or x > 0x1.0p53
	jae			2f						//    goto 2
				
	fld         %st(0)	
	fistpll	  4(%esp)
	fildll	  4(%esp)					// rint(x)
	fucomi      %st(1),			%st(0)
	fstp        %st(1)
	jae			1f						// if rint(x) >= x, return rint(x)
	fld1
	faddp								// otherwise, return rint(x) + 1.0
1:	ret

2:	and			$0x7fffffff,	%ecx
	cmp			$0x43300000,	%ecx	// if |x| >= 0x1.0p53
	fld			%st(0)
	jae			4f
	
	fistpll   4(%esp)
	fldz
	fildll	  4(%esp)					// rint(x)
	fucomi		%st(2),			%st(0)
	fstp		%st(2)
	jae			3f						// if rint(x) < x, add one
	fld1
	faddp		%st(2),			%st(0)
3:	fucomi		%st(1),			%st(0)
	jne			4f						// if ceil(x) == 0, patch up sign bit
	fchs
	fstp		%st(1)
	ret
	
4:	fstp		%st(0)
	ret


#endif // SSE3

#else // x86_64

.const
.align 4
one:	.quad	0x3ff0000000000000
absmask:.quad   0x7fffffffffffffff
thresh: .quad	0x4330000000000000

.text
.align 4
.globl _ceil
_ceil:
	movd		%xmm0,			%rcx
	andq		absmask(%rip),	%rcx	// |x|
	cmpq		thresh(%rip),	%rcx	// if |x| >= 0x1.0p52 or isnan(x)
	movsd		absmask(%rip),	%xmm2	//
	jae			1f						//    early out, returning x.

	cvttsd2si   %xmm0,			%rax
	andnpd		%xmm0,			%xmm2	// signbit(x)
	cvtsi2sd    %rax,			%xmm1	// trunc(x), except for sign of zero
	cmplesd		%xmm1,			%xmm0
	orpd		%xmm2,			%xmm1	// trunc(x)
	andnpd		one(%rip),		%xmm0	// x <= trunc(x) ? 0.0 : 1.0
	orpd		%xmm2,			%xmm0	// x <= trunc(x) ? copysign(0.0, x) : 1.0
	addsd		%xmm1,			%xmm0	// ceil(x)
1:  ret

#endif
