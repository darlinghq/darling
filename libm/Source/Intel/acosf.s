/*	acosf.s -- acosf for standard math library.

	Written by Eric Postpischil, July 2007.
*/


	.literal8

// Define the points where our algorithm changes.
nPoint:	.double	-.62000000476837158
pPoint:	.double	+.62000000476837158

// Define miscellaneous constants.
nOne:	.double	-1
pOne:	.double	+1
HalfPi:	.double	 1.5707963267948966192313217
Pi:		.double	 3.1415926535897932384626433

// Define a coefficient for center polynomial (used for x in [-.62, +.62]).
C2:		.double	 -0.7009458946846486978642201e-1

// Define coefficients for tail polynomial (used for x outside [-.62, +.62]).
pT2:	.double	  0.1475254770791043897245664e-2
nT2:	.double	 -0.1475254770791043897245664e-2

// Define coefficients for reciprocal-square-root refinement.
S0:		.double	 4.999999720603234088021257968
S1:		.double	-3.333333463718495862077843807


	.const
	.align	4

/*	Define some coefficients for center polynomial (used for x in [-.62,
	+.62]).  These are stored in pairs at aligned addresses for use in SIMD
	instructions.
*/
C1:		.double	+1.5714407739207253912688773, -1.5375796594506042231041545
C0:		.double	+1.2521077245762256716713145, +1.8992998261460658570880250

/*	Define some coefficients for positive tail polynomial (used for x in (+.62,
 	1)).
*/
T0:		.double	+14.7303701447222424478789710, +27.0927484663960477135094919
T1:		.double	 +2.7185038560132749878882294,  -8.6073428141752546318770571


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


// Define various symbols.

#define	BaseP		r0		// Base address for position-independent addressing.

#define	p			%xmm0	// Must be in %xmm0 for return on x86_64.
#define	x			%xmm1
#define	x1			%xmm2
#define	w			x1
#define	pa			%xmm3
#define	e			%xmm4
#define	rss			%xmm5

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


/*	float acosf(float x).

	Notes:

		Citations in parentheses below indicate the source of a requirement.

		"C" stands for ISO/IEC 9899:TC2.

		The Open Group specification (IEEE Std 1003.1, 2004 edition) adds no
		requirements since it defers to C and requires errno behavior only if
		we choose to support it by arranging for "math_errhandling &
		MATH_ERRNO" to be non-zero, which we do not.

	Return value:

		For arcosine of 1, return +0 (C F.9.1.1).

		For 1 < |x| (including infinity), return NaN (C F.9.1.1).

		For a NaN, return the same NaN (C F.9 11 and 13).  (If the NaN is a
		signalling NaN, we return the "same" NaN quieted.)

		Otherwise:

			If the rounding mode is round-to-nearest, return arccosine(x)
			faithfully rounded.

			Return a value in [0, pi] (C 7.12.4.1 3).  Note that this prohibits
			returning a correctly rounded value for acosf(-1), since pi rounded
			to a float lies outside that interval.
		
			Not implemented:  In other rounding modes, return arccosine(x)
			possibly with slightly worse error, not necessarily honoring the
			rounding mode (Ali Sazegari narrowing C F.9 10).

	Exceptions:

		May or may not raise inexact, even if the result is exact (C F.9 8).

		Raise invalid if the input is a signalling NaN (C 5.2.4.2.2 3, in spite
		of C 4.2.1)  or 1 < |x| (including infinity) (C F.9.1.1) but not if the
		input is a quiet NaN (C F.9 11).

		May not raise exceptions otherwise (C F.9 9).

	Properties:

		Monotonic, proven by exhaustive testing.

		Exhaustive testing proved this routine returns faithfully rounded
		results.  Since the rsqrtss instruction is specified to return a value
		in an interval, tests were performed using each possible result,
		showing that a valid result will be obtained regardless of which
		value rsqrtss provides.
*/
	.align	5
#if !defined DevelopmentInstrumentation
	// This is the regular name used in the deployed implementation.
		.globl _acosf
	_acosf:
#else
	// This is the name used for a special test version of the routine.
		.globl _acosfInstrumented
	_acosfInstrumented:
#endif

	cvtss2sd	Argx, x				// Convert x to double precision.

	#if defined __i386__

		// Get address of 0 in BaseP.
			call	0f				// Push program counter onto stack.
		0:
			pop		BaseP			// Get program counter.

	#endif

/*	We use different algorithms for different parts of the domain.  There
	is a "negative tail" from -1 to nPoint, a center from nPoint to pPoint,
	and a "positive tail" from pPoint to +1.  Here, we compare and branch
	to the appropriate code.

	There are also special cases:  NaNs, x < -1, and 1 < x.  These are weeded
	out in PositiveTail or NegativeTail.
*/

	ucomisd		Address(pPoint), x
	ja			PositiveTail

	ucomisd		Address(nPoint), x
	jb			NegativeTail


/*	Here we have nPoint <= x <= pPoint.  This is handled with a simple
	evaluation of a polynomial that approximates arccosine.

	The polynomial has been arranged into the form:

		pi/2 - x + x *
				c2   * (x**4 + c01 * x**2 + c00)
			*   x**2 * (x**4 + c11 * x**2 * c10)).

	The coefficients c00 and c10 are stored in a pair at C0, and c01 and c11
	are stored at C1.  c2 is at C2.

	The two quartic factors are evaluated in SIMD registers.  For brevity, some
	comments below describe only one element of a register.  The other is
	analagous.
*/
	movsd		x, x1				// Save a copy of x for later.
	mulsd		x, x				// Form x**2.
	movapd		Address(C1), p		// Get first coefficient pair.
	unpcklpd	x, x				// Duplicate in SIMD register.
	addpd		x, p				// Form x**2 + c1.
	mulpd		x, p				// Form x**4 + c1 * x**2.
	movlpd		Address(C2), x		// Put c2 in low element, with x**2 in high.
	mulsd		x1, x				// Multiply by x.
	addpd		Address(C0), p		// Form x**4 + c1 * x**2 + c0.
	mulpd		x, p				// Multiply by c2*x in low and x**2 in high.
	movhlps		p, pa				// Get high element.
	subsd		Address(HalfPi), x1	// Form x - pi/2.
	mulsd		pa, p				// Multiply two factors.
	subsd		x1, p				// Form pi/2 - x + x * c2 * ...

// Return the double-precision number currently in p.
ReturnDoubleInp:
	cvtsd2ss	p, p				// Convert result to single precision.

// Return the single-precision number currently in p.
ReturnSingleInp:

	#if defined __i386__
		movss		p, Argx			// Shuttle result through memory.
			// This uses the argument area for scratch space, which is allowed.
		flds		Argx			// Return input on floating-point stack.
	#else
		// On x86_64, the return value is now in p, which is %xmm0.
	#endif

	ret


// Handle pPoint < x.
PositiveTail:

	movsd		Address(pOne), w	// Get +1 for math and comparison.
	ucomisd		w, x				// Compare x to +1.
	jae			InputIsPositiveSpecial

/*	Here we have pPoint < x < 1.  The algorithm here is inspired by the
	identity arccosine(x) = 2 * arcsine(sqrt((1-x)/2)).  Replacing arcsine with
	an approximating polynomial would give an odd polynomial in sqrt((1-x)/2),
	which is the same as sqrt(1-x) multiplied by some polynomial in x.  So we
	have:

		arccosine(x) ~= sqrt(1-x) * t(x).

	Unfortunately, the square-root instruction (rsqrtss) takes too long, so we
	use the faster reciprocal-square-root-estimate instruction instead and
	refine its estimate.  Given an estimate e from the rsqrtss instruction, the
	square root of 1-x is very nearly, e * (1-x) * s(e**2 * (1-x)).  Let e2a
	be e**2 * (1-x), so sqrt(1-x) is nearly e * a * s(e2a).  The leading
	coefficient of s, cl, has been removed (by dividing s by it) and multiplied
	into the polynomial p above.  That leaves:

		sqrt(1-x) / cl ~= e * a * s(e2a)/cl.

	s(e2a)/cl is e2a**2 + s1 * e2a + s0, where s1 and s0 have been stored at
	labels S1 and S0, above.

	t(x) has been arranged into the form:

			t2
		*	(x**2 + t01 * x + t00)
		*	(x**2 + t11 * x + t10).

	The two quadratic factors are evaluated in SIMD registers.  For brevity,
	some comments below describe only one element of a register.  The other is
	analagous.

	So, our job here is to evaluate:

		a = 1-x.

		e = estimate of 1/sqrt(a).

		e2a = e*e*a.

		acosf(x) ~= e * a
			*	(e2a**2 + s1 * e2a + s0)
			*	t2
			*	(x**2 + t01 * x + t00)
			*	(x**2 + t11 * x + t10).
*/

	subsd		x, w				// Form 1-x.
	cvtsd2ss	w, e				// Convert to single precision for rsqrtss.
	movapd		Address(T1), p		// Start asinf polynomial.
	unpcklpd	x, x				// Duplicate x.
	addpd		x, p				// Form x + t1.
	#if !defined DevelopmentInstrumentation
		// This is the regular code used in the deployed implementation.
		rsqrtss		e, e			// Estimate 1/sqrt(1-x).
	#else
		/*	This instruction uses an estimate of 1/sqrt(1-x) passed by the
			caller rather than the rsqrtss instruction.  This allows us to test
			the implementation with all values that rsqrtss might return.
		*/
		movss	8(%esp), e			// Use caller's estimate of 1/sqrt(1-x).
	#endif
	cvtss2sd	e, e				// Convert to double precision.
	mulpd		x, p				// Form x**2 + t1*x.
	mulsd		e, w				// Form e    * (1-x).
	mulsd		w, e				// Form e**2 * (1-x).
		// "e" in comments refers to the initial estimate from rsqrtss.
	movhpd		Address(pT2), w		// Copy coefficient into high element.
	addpd		Address(T0), p		// Form x**2 + t1*x + t0.
	movsd		Address(S1), rss	// Form s1.
	addsd		e, rss				// Form e**2 * (1-x) + s1.
	mulpd		w, p				// Form e * (1-x) * p(x), split.
	mulsd		e, rss				// Form e2a**2 + s1 * e2a.
	movhlps		p, pa				// Separate high element.
	addsd		Address(S0), rss	// Form e2a**2 + s1 * e2a + s0.
	mulsd		pa, p				// Finish e * (1-x) * p(x).
	mulsd		rss, p				// Form e * (1-x) / sqrt(1-x) * p(x).

	jmp			ReturnDoubleInp


// Handle x < nPoint.
NegativeTail:

	movsd		Address(pOne), w	// Get +1 for math.
	ucomisd		Address(nOne), x	// Compare x to -1.
	jbe			InputIsNegativeSpecial

/*	Here we have -1 < x < nPoint.  We use the same algorithm as in PositiveTail
	but adapted for -x.

	For brevity, some comments below describe only one element of a register.
	The other is analagous.

	Our job here is to evaluate:

		a = 1+x.

		e = estimate of 1/sqrt(a).

		e2a = e*e*a.

		acosf(x) ~= pi - e * a
			*	(e2a**2 + s1 * e2a + s0)
			*	t2
			*	(x**2 - t01 * x + t00)
			*	(x**2 - t11 * x + t10).

	Note that the signs of terms involving x are negated from those in
	PositiveTail.

	For convenience, the final two factors are negated:

			*	(-x**2 + t01 * x - t00)
			*	(-x**2 + t11 * x - t10).
*/

	addsd		x, w				// Form 1+x.
	cvtsd2ss	w, e				// Convert to single precision for rsqrtss.
	movapd		Address(T1), p		// Start asinf polynomial.
	unpcklpd	x, x				// Duplicate x.
	subpd		x, p				// Form -x + t1.
	#if !defined DevelopmentInstrumentation
		// This is the regular code used in the deployed implementation.
		rsqrtss		e, e			// Estimate 1/sqrt(1+x).
	#else
		/*	This instruction uses an estimate of 1/sqrt(1+x) passed by the
			caller rather than the rsqrtss instruction.  This allows us to test
			the implementation with all values that rsqrtss might return.
		*/
		movss	8(%esp), e			// Use caller's estimate of 1/sqrt(1+x).
	#endif
	cvtss2sd	e, e				// Convert to double precision.
	mulpd		x, p				// Form -x**2 + t1*x.
	mulsd		e, w				// Form e    * (1+x).
	mulsd		w, e				// Form e**2 * (1+x).
		// "e" in comments refers to the initial estimate from rsqrtss.
	movhpd		Address(nT2), w		// Copy coefficient into high element.
	subpd		Address(T0), p		// Form -x**2 + t1*x - t0.
	movsd		Address(S1), rss	// Form s1.
	addsd		e, rss				// Form e**2 * (1+x) + s1.
	mulpd		w, p				// Form e * (1+x) * p(x), split.
	mulsd		e, rss				// Form e2a**2 + s1 * e2a.
	movhlps		p, pa				// Separate high element.
	addsd		Address(S0), rss	// Form e2a**2 + s1 * e2a + s0.
	mulsd		pa, p				// Finish e * (1+x) * p(x).
	mulsd		rss, p				// Form e * (1+x) / sqrt(1+x) * p(x).
	addsd		Address(Pi), p		// Form pi - 2*asin(sqrt((1+x)/2)).

	jmp			ReturnDoubleInp


/*	Here we handle inputs greater than or equal to one, including infinity,
	but not including NaNs.  The condition code must be set to indicate equal
	(zero flag is one) iff the input is one.
*/
InputIsPositiveSpecial:
	je			InputIsPositiveOne
	jmp			SignalInvalid


/*	Here we handle inputs less than or equal to -1, including -infinity, and
	NaNs.  The condition code must be set as if by using ucomisd to compare
	the input in the "source operand" to -1 in the "destination operand".
*/
InputIsNegativeSpecial:
	jp			InputIsNaN
	je			InputIsNegativeOne

	// Continue into SignalInvalid.
//	jmp			SignalInvalid


/*	Here we handle inputs outside the domain of the function.  We subtract
	infinity from itself to generate the invalid signal and return a NaN.
*/

	.literal4
Infinity:	.long	0x7f800000

	.text
SignalInvalid:
	movss		Address(Infinity), p
	subss		p, p				// Generate invalid signal and NaN value.
	jmp			ReturnSingleInp


/*	Here we handle an input of +1 or -1.  arccosine(-1) is pi, which increases
	when rounded to single precision.  However, we are required to return
	results in [0, pi], so we return pi rounded down.
*/

	.literal8
/*	Define a value near pi that yields pi rounded down when converted to
	single precision.  This allows us to generate inexact and return the
	desired value for acosf(-1).
*/
AlmostPi:	.double	3.1415925

	.text
InputIsPositiveOne:
	xorps		p, p					// Return zero.
	jmp			ReturnSingleInp

InputIsNegativeOne:
	cvtsd2ss	Address(AlmostPi), p
	jmp			ReturnSingleInp


InputIsNaN:

	#if defined __i386__
		flds		Argx			// Return result on floating-point stack.
	#else
		cvtsd2ss	x, %xmm0		// Return in %xmm0.
			/*	We cannot just return the input, because we must quiet a
				signalling NaN.
			*/
	#endif

	ret
