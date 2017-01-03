/*	sinfcosf.s -- sinf and cosf for standard math library.

	Written by Eric Postpischil, June 2007.
*/


#define	SignificandBits	24
#define	ExponentBits	 8

	.literal8

	.align	3

Half:	.double	.5


	.const

	.align	4

/*	Coefficients to calculate x*(x**4+c0*x**2+c1) * c4*(x**4+c2*x**2+c3).

	c0 and c2 are paired for loading into an XMM register, and so are c1 and c3.
*/
// Two binades, fractions of pi, minimize ULP error, refactored:
C02:	.double		-4.1647719829166299, -3.6493923712207774
C13:	.double		15.485466333375713, 2.6522034727867632
C4:		.double		0.076492480587904088

/*	ExponentFold specifies how many bits of the exponent will not be used  to
	look up entries in the table.  This must match the ExponentFold used to
	generate the table.

	The behavior for infinite inputs relies on the table contents -- if the
	table entries looked up for an infinite input are m0 and m1, then
	m0*infinity + m1*infinity should generate a NaN.  This happens of either m0
	or m1 is a zero or they have opposite signs.  If this were not true,
	additional arrangements would have to be made for infinite inputs.
*/
#define	ExponentFold	1

// Include the reduction table.  See sinfcosfMakeTable.c for more information.
#include "sinfcosfTable.s"


// Rename the general registers (just to make it easier to keep track of them).
#if defined __i386__
	#define	r0	%eax
	#define	r1	%ecx
	#define	r2	%edx
	#define	r3	%ebx
	#define	r4	%esp
	#define	r5	%ebp
	#define	r6	%esi
	#define	r7	%edi
#elif defined __x86_64__
	#define	r0	%rax
	#define	r1	%rcx
	#define	r2	%rdx
	#define	r3	%rbx
	#define	r4	%rsp
	#define	r5	%rbp
	#define	r6	%rsi
	#define	r7	%rdi
#else
	#error "Unknown architecture."
#endif


	.text


// Define symbols common to sinf and cosf.
#define	BaseP		r0		// Base address for position-independent addressing.
#define	xInteger	r1		// The input x in an integer register.

#define	p			%xmm0	// Must be in %xmm0 for return on x86_64.
#define	x			%xmm1
#define	ki			%xmm2
#define	k			%xmm3
#define	m1			%xmm4
#define	pa			m1

#if defined __i386__

	// Define location of argument x.
	#define	Argx			4(%esp)

	// Define how to address data.  BaseP must contain the address of label 0.
	#define	Address(label)	label-0b(BaseP)

#elif defined __x86_64__

	// Define location of argument x.
	#define	Argx			%xmm0

	// Define how to address data.
	#define	Address(label)	label(%rip)

#endif


/*	float sinf(float x).

	Notes:

		Citations in parentheses below indicate the source of a requirement.

		"C" stands for ISO/IEC 9899:TC2.

		The Open Group specification (IEEE Std 1003.1, 2004 edition) adds no
		requirements since it defers to C and requires errno behavior only if
		we choose to support it by arranging for "math_errhandling &
		MATH_ERRNO" to be non-zero, which we do not.

	Return value:

		For +/- zero, return zero with same sign (C F.9 12 and F.9.1.6).

		For +/- infinity, return a NaN (C F.9.1.6).

		For a NaN, return the same NaN (C F.9 11 and 13).

		Otherwise:

			If the rounding mode is round-to-nearest, return sine(x) faithfully
			rounded.
		  
			Not currently implemented:  In other rounding modes, return sine(x)
			possibly with slightly worse error, not necessarily honoring the
			rounding mode (Ali Sazegari narrowing C F.9 10).

		All results are in [-1, 1].  This is corollary to faithful rounding.

	Exceptions:

		Raise underflow for a denormal result (C F.9 7 and Draft Standard for
		Floating-Point Arithmetic P754 Draft 1.2.5 9.5).  If the input is the
		smallest normal, underflow may or may not be raised.  This is stricter
		than the older 754 standard.

		May or may not raise inexact, even if the result is exact (C F.9 8).

		Raise invalid if the input is a signalling NaN (C 5.2.4.2.2 3, in spite
		of C 4.2.1) or an infinity (C F.9.1.6) but not if the input is a quiet
		NaN (C F.9 11).

		May not raise exceptions otherwise (C F.9 9).

	Properties:

		Desired to be monotonic.  Not yet proven!

		Exhaustive testing proved this routine returns faithfully rounded
		results.
*/
	.align	5
	.globl _sinf
_sinf:

	// Put x into an integer register.
	#if defined __i386__
		mov		Argx, xInteger		// Put x into xInteger.
	#elif defined __x86_64__
		movd	Argx, xInteger		// Put x into xInteger.
	#endif

	cvtss2sd	Argx, x				// Convert x to double precision.

	#if defined __i386__

		// Get address of 0 in BaseP.
			call	0f				// Push program counter onto stack.
		0:
			pop		BaseP			// Get program counter.

	#elif defined __x86_64__

		// Get address of reduction table in BaseP.
			lea		ReductionTable(%rip), BaseP

	#endif

	unpcklpd	x, x				// Duplicate x.

	/*	Shift:

			SignificandBits-1 bits right to remove significand.

			ExponentFold bits right to remove unused bits of exponent.

			4 bits left to multiply by size of a table entry.
	*/
	shr		$SignificandBits-1+ExponentFold-4, xInteger

	// Clear bits other than those we are using from exponent.
	and		$(1<<ExponentBits-ExponentFold)-1 << 4, xInteger
	je		ExponentIsZero			// Branch so we can get -0 right.

	ucomisd		x, x				// Test for NaN.
	jpe			HandleNaN

	// Get table entry containing m0 and m1 and calculate x*m0 and x*m1.
	#if defined __i386__
		mulpd	ReductionTable-0b(BaseP, xInteger), x
	#elif defined __x86_64__
		mulpd	(BaseP, xInteger), x
	#endif
		/*	Let the exponent of x be E, that is, E is the largest integer such
			that 2**E <= x.  We use e to index the table, where e is E with the
			low ExponentFold bits set to zero.  For example, if ExponentFold is
			1, e = E & ~1.

			The table entry indexed by e represents a number m that is (u/p %
			1)/u*2**a, where u is the ULP of 2**e, p is the period (2*pi), and
			2**a is the number of intervals we divide the period into (we use
			two, so a is one).

			Observe that when 2**e <= x, x/u is an integer, and x/u <
			2**(24+ExponentFold).

			Then for some integer k, x*m = x*(u/p % 1)/u*2**a = x*(u/p -
			k)/u*2**a = x/p*2**a - x/u*k*2**a.  x/u*k*2**a is an integer, so
			x*m is congruent to x/p*2**a modulo 1.

			So sin(x*m*p*2**-a) equals sin(x).

			However, we do not have m exactly.  The table contains two numbers
			for 2**e, m0 and m1.  m0 has the first 29 bits of m, and m1 has the
			next 53 bits.

			Since x has 23 bits and m0 has 29 bits, their multiplication in
			double format is exact.  From that product, we subtract the nearest
			integer, which changes nothing modulo 1 but reduces the magnitude.
			Then we add x*m1, yielding a precise but inexact approximation of
			x*m0 % 1 + x*m1.

			The magnitude of x*m0 % 1 is at most 1/2, and x*m1 may make the sum
			slightly larger.

			Recall that m is (u/p % 1)/u*2**a, so m's magnitude is at most
			.5/u*2**a.  m0 contains 29 bits of this, so m1's magnitude is at
			most 2**-29*.5/u*2**a.  With an exponent fold of 1 bit, x/u is less
			than 2**25, so |x*m1| <= |x*2**-29*.5/u*2**a| <
			2**25**2**-29*.5*2**a = 2**(a-5).

			Therefore, |x*m0 % 1 + x*m1| < .5 + 2**(a-5).  The polynomial that
			approximates sin(p*x*2**-a) has been engineered to be good in this
			interval.
		*/

	cvtpd2dq	x, ki				// Round to integer.
		// This requires round-to-nearest mode in the MXCSR.
	cvtdq2pd	ki, k				// Convert to floating-point.
	movhlps		x, m1				// Get x*m1.
	movapd		Address(C02), p		// Get c0 and c2.
	psllq		$63, ki				// Move low bit to high bit.
	subsd		k, x				// Subtract integer, leaving fraction.
	addsd		m1, x				// Get x*m0%1 + x*m1.
	xorpd		x, ki				// Negate x if low bit of ki was set.
	mulsd		x, x				// Square x.
	unpcklpd	x, x				// Duplicate x**2.
	movhpd		Address(C4), ki		// Get sign*x and c4.
	addpd		x, p				// Make x**2+c0 and x**2+c2.
	mulpd		x, p				// Make x**4+c0*x**2 and x**4+c2*x**2.
	addpd		Address(C13), p		// Make x**4+c0*x**2+c1 and x**4+c2*x**2+c3.
	mulpd		ki, p				// Make sign*x*(x**4+c0*x**2+c1) and
									// c4*(x**4+c2*x**2+c3).
	movhlps		p, pa				// Get c4*(x**4+c2*x**2+c3).
	mulsd		pa, p				// Multiply final two factors.
	cvtsd2ss	p, p				// Convert result to single precision.

ReturnSingle:
	#if defined __i386__
		movss		p, Argx			// Shuttle result through memory.
			// This uses the argument area for scratch space, which is allowed.
		flds		Argx			// Return input on floating-point stack.
	#else
		// On x86_64, the return value is now in p, which is %xmm0.
	#endif

	ret


/*	The raw exponent field is zero, so the input is a zero or a denormal.  We
	need to handle this separately to get -0 right, and it is faster.
*/
ExponentIsZero:
	.literal8
Finagle:	.double	1.00000001
	.text
	mulsd		Address(Finagle), x	// Make non-zero results inexact.
	cvtsd2ss	x, p				// Return x.
	jmp			ReturnSingle


// Handle a NaN input.
HandleNaN:

	#if defined __i386__
		flds		Argx			// Return result on floating-point stack.
	#else
		cvtsd2ss	x, %xmm0		// Return in %xmm0.
			/*	We cannot just return the input, because we must quiet a
				signalling NaN.
			*/
	#endif

	ret


/*	float cosf(float x).

	Notes:

		Citations in parentheses below indicate the source of a requirement.

		"C" stands for ISO/IEC 9899:TC2.

		The Open Group specification (IEEE Std 1003.1, 2004 edition) adds no
		requirements since it defers to C and requires errno behavior only if
		we choose to support it by arranging for "math_errhandling &
		MATH_ERRNO" to be non-zero, which we do not.

	Return value:

		For +/- infinity, return a NaN (C F.9.1.6).

		For a NaN, return the same NaN (C F.9 11 and 13).

		Otherwise:

			If the rounding mode is round-to-nearest, return cosine(x)
			faithfully rounded.
		  
			Not currently implemented:  In other rounding modes, return sine(x)
			possibly with slightly worse error, not necessarily honoring the
			rounding mode (Ali Sazegari narrowing C F.9 10).

		All results are in [-1, 1].  This is corollary to faithful rounding.

	Exceptions:

		Raise underflow for a denormal result (C F.9 7 and Draft Standard for
		Floating-Point Arithmetic P754 Draft 1.2.5 9.5).  If the input is the
		smallest normal, underflow may or may not be raised.  This is stricter
		than the older 754 standard.

		May or may not raise inexact, even if the result is exact (C F.9 8).

		Raise invalid if the input is a signalling NaN (C 5.2.4.2.2 3, in spite
		of C 4.2.1) or an infinity (C F.9.1.5) but not if the input is a quiet
		NaN (C F.9 11).

		May not raise exceptions otherwise (C F.9 9).

	Properties:

		Desired to be monotonic.  Not yet proven!

		Exhaustive testing proved this routine returns faithfully rounded
		results.
*/
	.align	5
	.globl _cosf
_cosf:
	// This code is identical to _sinf except for the addition of Half.

	// Put x into an integer register.
	#if defined __i386__
		mov		Argx, xInteger		// Put x into xInteger.
	#elif defined __x86_64__
		movd	Argx, xInteger		// Put x into xInteger.
	#endif

	cvtss2sd	Argx, x				// Convert x to double precision.

	#if defined __i386__

		// Get address of 0 in BaseP.
			call	0f				// Push program counter onto stack.
		0:
			pop		BaseP			// Get program counter.

	#elif defined __x86_64__

		// Get address of reduction table in BaseP.
			lea		ReductionTable(%rip), BaseP

	#endif

	unpcklpd	x, x				// Duplicate x.

	/*	Shift:

			SignificandBits-1 bits right to remove significand.

			ExponentFold bits right to remove unused bits of exponent.

			4 bits left to multiply by size of a table entry.
	*/
	shr		$SignificandBits-1+ExponentFold-4, xInteger

	// Clear bits other than those we are using from exponent.
	and		$(1<<ExponentBits-ExponentFold)-1 << 4, xInteger

	ucomisd		x, x				// Test for NaN.
	jpe			HandleNaN

	// Get table entry containing m0 and m1 and calculate x*m0 and x*m1.
	#if defined __i386__
		mulpd	ReductionTable-0b+0*8(BaseP, xInteger), x
	#elif defined __x86_64__
		mulpd	(BaseP, xInteger), x
	#endif
		/*	Let the exponent of x be E, that is, E is the largest integer such
			that 2**E <= x.  We use e to index the table, where e is E with the
			low ExponentFold bits set to zero.  For example, if ExponentFold is
			1, e = E & ~1.

			The table entry indexed by e represents a number m that is (u/p %
			1)/u*2**a, where u is the ULP of 2**e, p is the period (2*pi), and
			2**a is the number of intervals we divide the period into (we use
			two, so a is one).

			Observe that when 2**e <= x, x/u is an integer, and x/u <
			2**(24+ExponentFold).

			Then for some integer k, x*m = x*(u/p % 1)/u*2**a = x*(u/p -
			k)/u*2**a = x/p*2**a - x/u*k*2**a.  x/u*k*2**a is an integer, so
			x*m is congruent to x/p*2**a modulo 1.

			So cos(x*m*p*2**-a) equals cos(x).

			However, we do not have m exactly.  The table contains two numbers
			for 2**e, m0 and m1.  m0 has the first 29 bits of m, and m1 has the
			next 53 bits.

			Since x has 23 bits and m0 has 29 bits, their multiplication in
			double format is exact.  From that product, we subtract the nearest
			integer, which changes nothing modulo 1 but reduces the magnitude.
			Then we add x*m1, yielding a precise but inexact approximation of
			x*m0 % 1 + x*m1.

			The magnitude of x*m0 % 1 is at most 1/2, and x*m1 may make the sum
			slightly larger.

			Recall that m is (u/p % 1)/u*2**a, so m's magnitude is at most
			.5/u*2**a.  m0 contains 29 bits of this, so m1's magnitude is at
			most 2**-29*.5/u*2**a.  With an exponent fold of 1 bit, x/u is less
			than 2**25, so |x*m1| <= |x*2**-29*.5/u*2**a| <
			2**25**2**-29*.5*2**a = 2**(a-5).

			Therefore, |x*m0 % 1 + x*m1| < .5 + 2**(a-5).  The polynomial that
			approximates sin(p*x*2**-a) has been engineered to be good in this
			interval.
		*/

	addsd		Address(Half), x
		// Add a quarter circle, then calculate sine.
	cvtpd2dq	x, ki				// Round to integer.
		// This requires round-to-nearest mode in the MXCSR.
	cvtdq2pd	ki, k				// Convert to floating-point.
	movhlps		x, m1				// Get x*m1.
	movapd		Address(C02), p		// Get c0 and c2.
	psllq		$63, ki				// Move low bit to high bit.
	subsd		k, x				// Subtract integer, leaving fraction.
	addsd		m1, x				// Get x*m0%1 + x*m1.
	xorpd		x, ki				// Negate x if low bit of ki was set.
	mulsd		x, x				// Square x.
	unpcklpd	x, x				// Duplicate x**2.
	movhpd		Address(C4), ki		// Get sign*x and c4.
	addpd		x, p				// Make x**2+c0 and x**2+c2.
	mulpd		x, p				// Make x**4+c0*x**2 and x**4+c2*x**2.
	addpd		Address(C13), p		// Make x**4+c0*x**2+c1 and x**4+c2*x**2+c3.
	mulpd		ki, p				// Make sign*x*(x**4+c0*x**2+c1) and
									// c4*(x**4+c2*x**2+c3).
	movhlps		p, pa				// Get c4*(x**4+c2*x**2+c3).
	mulsd		pa, p				// Multiply final two factors.
	cvtsd2ss	p, p				// Convert result to single precision.

	#if defined __i386__
		movss		p, Argx			// Shuttle result through memory.
			// This uses the argument area for scratch space, which is allowed.
		flds		Argx			// Return result on floating-point stack.
	#else
		// On x86_64, the return value is now in p, which is %xmm0.
	#endif

	ret
