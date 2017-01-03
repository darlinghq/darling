/*	tanf.s -- tanf for standard math library.

	Written by Eric Postpischil, June 2007.
*/


#define	SignificandBits	24
#define	ExponentBits	 8

	.literal8

	.align	3

NegativeOne:
	.double	-1


	.const

	.align	4

/*	Coefficients to calculate:

		x * (x**4 + c02*x**2 + c00) * (x**4 + c12*x**2 + c10)
		  * (x**4 + c22*x**2 + c20) * (       c32*x**2 + c30)

	c02 and c12 are stored as a pair at the location below labeled c02,
	c00 and c10 are at c00, c22 and c23 at c22, and c20 and c30 at c20.

	This polynomial was designed to cover two binades (supporting an
	exponent fold of one bit) for tanf with its period (pi) partitioned into
	two intervals.  The expanded polynomial was factored to create the form
	above convenient for calculating with SIMD.

	This was minimized for ULP error.
*/
c02:	.double	-0.34431297545117363561, -1.53503935362160698123
c00:	.double	+0.75337415977492632831, +0.92783629599735333058
c22:	.double	+0.91301179133375347365, +5.27860261872543679865
c20:	.double	+0.58807104225499649417, +3.82127246397188494330


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

// Include the reduction table.  See tanfMakeTable.c for more information.
#include "tanfTable.s"


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


// Define symbols for tanf.
#define	BaseP		r0		// Base address for position-independent addressing.
#define	xInteger	r1		// The input x in an integer register.
#define	ki			r1		// x*m0 rounded to an integer.

#define	p0			%xmm0	// Must be in %xmm0 for return on x86_64.
#define	p2			%xmm1
#define	x			%xmm2
#define	k			%xmm3
#define	m1			%xmm4
#define	x1 			%xmm5
#define	Numerator	%xmm6

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


/*	float tanf(float x).

	Notes:

		Citations in parentheses below indicate the source of a requirement.

		"C" stands for ISO/IEC 9899:TC2.

		The Open Group specification (IEEE Std 1003.1, 2004 edition) adds no
		requirements since it defers to C and requires errno behavior only if
		we choose to support it by arranging for "math_errhandling &
		MATH_ERRNO" to be non-zero, which we do not.

	Return value:

		For +/- zero, return zero with same sign (C F.9 12 and F.9.1.7).

		For +/- infinity, return a NaN (C F.9.1.7).

		For a NaN, return the same NaN (C F.9 11 and 13).

		Otherwise:

			If the rounding mode is round-to-nearest, return tangent(x)
			faithfully rounded.
		  
			Not currently implemented:  In other rounding modes, return
			tangent(x) possibly with slightly worse error, not necessarily
			honoring the rounding mode (Ali Sazegari narrowing C F.9 10).

	Exceptions:

		Raise underflow for a denormal result (C F.9 7 and Draft Standard for
		Floating-Point Arithmetic P754 Draft 1.2.5 9.5).  If the input is the
		smallest normal, underflow may or may not be raised.  This is stricter
		than the older 754 standard.

		May or may not raise inexact, even if the result is exact (C F.9 8).

		Raise invalid if the input is a signalling NaN (C 5.2.4.2.2 3, in spite
		of C F.2.1) or an infinity (C F.9.1.7) but not if the input is a quiet
		NaN (C F.9 11).

		May not raise exceptions otherwise (C F.9 9).

	Properties:

		Desired to be monotonic.  Not yet proven!

		Exhaustive testing proved this routine returns faithfully rounded
		results.
*/
	.align	5
	.globl _tanf
_tanf:

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

	movsd		Address(NegativeOne), Numerator

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

	ucomisd		x, x				// Test for Nan.
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
			1)/u*2**a, where u is the ULP of 2**e, p is the period (pi), and
			2**a is the number of intervals we divide the period into (we use
			two, so a is one).

			Observe that when 2**e <= x, x/u is an integer, and x/u <
			2**(24+ExponentFold).

			Then for some integer k, x*m = x*(u/p % 1)/u*2**a = x*(u/p -
			k)/u*2**a = x/p*2**a - x/u*k*2**a.  x/u*k*2**a is an integer, so
			x*m is congruent to x/p*2**a modulo 1.

			So tangent(x*m*p*2**-a) equals tangent(x).

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
			approximates tangent(p*x*2**-a) has been engineered to be good in
			this interval.
		*/

	movapd		Address(c02), p0	// Get c02 and c12.
	movapd		Address(c22), p2	// Get c22 and c32.

	cvtsd2si	x, ki				// Round to integer.
		// This requires round-to-nearest mode in the MXCSR.
	cvtsi2sd	ki, k				// Convert to floating-point.
	movhlps		x, m1				// Get x*m1.
	subsd		k, x				// Subtract integer, leaving fraction.
	addsd		m1, x				// Get x*m0%1 + x*m1.

/*	Below this point, references in the comments to "x" refer to the
	result of the reduction, not to the original input argument.
*/
	unpcklpd	x, x				// Duplicate x for SIMD work.
	test		$1, ki				// Is 2**0 bit on?
	movsd		x, x1				// Save a copy of x for later.
	mulpd		x, x				// Get x**2.
#define	x2	x	// Use the name x2 to emphasize we have x**2.
	addpd		x2, p0				// Get x**2+c02 and x**2+c12.
	mulpd		x2, p0				// Get x**4+c02*x**2 and x**4+c12*x**2.
	addpd		Address(c00), p0	// Finish first pair of terms.
	addsd		x2, p2				// Get x**2+c22 and c32.
	mulpd		x2, p2				// Get x**4+c22*x**2 and c32*x**2.
	addpd		Address(c20), p2	// Finish second pair of terms.
	mulpd		p2, p0				// Multiply parallel pairs.
	mulsd		x1, p0				// Multiply isolated x term.
	movhlps		p0, p2				// Get high factor.
	mulsd		p2, p0				// Combine parts from SIMD work.
	jne			2f					// Branch if bit was on.

// ki was even, tangent is in p0.

	cvtsd2ss	p0, p0				// Convert result to single precision.
	#if defined __i386__
		movss		p0, Argx		// Shuttle result through memory.
			// This uses the argument area for scratch space, which is allowed.
		flds		Argx			// Return result on floating-point stack.
	#else
		// On x86_64, the return value is now in p0, which is %xmm0.
	#endif

	ret

// ki was odd, tangent is -1/p0.
2:
	divsd		p0, Numerator
	cvtsd2ss	Numerator, p0		// Convert result to single precision.

ReturnSingle:
	#if defined __i386__
		movss		p0, Argx		// Shuttle result through memory.
			// This uses the argument area for scratch space, which is allowed.
		flds		Argx			// Return result on floating-point stack.
	#else
		// On x86_64, the return value is now in p0, which is %xmm0.
	#endif

	ret


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
 

/*	The raw exponent field is zero, so the input is a zero or a denormal.  We
	need to handle this separately to get -0 right, and it is faster.
*/
ExponentIsZero:
	.literal8
Finagle:	.double	1.00000001
	.text
	mulsd		Address(Finagle), x	// Make non-zero results inexact.
	cvtsd2ss	x, p0				// Return x.
	jmp			ReturnSingle
