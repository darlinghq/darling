/*	atanf.s -- atanf for standard math library.

	Written by Eric Postpischil, July 2007.
*/


	.literal8

// Define the points where our algorithm changes.
nPoint0:	.double	-1
pPoint0:	.double	+1
nPoint1:	.double	-16777216
pPoint1:	.double	+16777216

// Define miscellaneous constants.
nHalfPi:	.double	-1.5707963267948966192313217
One:		.double	 1
pHalfPi:	.double	+1.5707963267948966192313217

// Define a coefficient for center polynomial (used for x in [-1, +1]).
C2:			.double	 0.0029352921857004596570518


	.const
	.align	4

/*	Define some coefficients for center polynomial (used for x in [-1, +1]).
	These are stored in pairs at aligned addresses for use in SIMD
	instructions.
*/
C01:		.double	 2.2971562298314633761676433,  0.0207432003007420961489920
C00:		.double	 2.4449692297316409651126041,  3.7888879287802702842997915
C11:		.double	-2.9466967515109826289085300, -4.9721072376211623038916292
C10:		.double	 5.4728447324456990092824269,  6.7197076223592378022736307

// Define coefficients for the tail polynomial (used for x outside [-1, +1]).
T01:	.double	 0.9193672354545696501477531, -0.0052035944094405570566862
T00:	.double	 0.3992772987220534996563340,  0.2521268658714555740707959
T11:	.double	-0.5273186542866779494437308, -0.7201738803584184183894208
T10:	.double	 0.1730466268612773143731748,  0.1408679409162453515360961


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

#define	p0			%xmm0	// Must be in %xmm0 for return on x86_64.
#define	x			%xmm1
#define	p1			%xmm2
#define	x1			%xmm3
#define	v0			%xmm4
#define	t0			%xmm5

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


/*	float atanf(float x).

	Notes:

		Citations in parentheses below indicate the source of a requirement.

		"C" stands for ISO/IEC 9899:TC2.

		The Open Group specification (IEEE Std 1003.1, 2004 edition) adds no
		requirements since it defers to C and requires errno behavior only if
		we choose to support it by arranging for "math_errhandling &
		MATH_ERRNO" to be non-zero, which we do not.

	Return value:

		For arctangent of +/- zero, return zero with same sign (C F.9 12 and
		F.9.1.3).

		For arctangent of +/- infinity, return +/- pi/2 (C F.9.1.3).

		For a NaN, return the same NaN (C F.9 11 and 13).  (If the NaN is a
		signalling NaN, we return the "same" NaN quieted.)

		Otherwise:

			If the rounding mode is round-to-nearest, return arctangent(x)
			faithfully rounded.

			Return a value in [-pi/2, +pi/2] (C 7.12.4.3 3).  Note that this
			prohibits returning correctly rounded values for atanf of large
			positive or negative arguments, since pi/2 rounded to a float lies
			outside that interval.
		
			Not implemented:  In other rounding modes, return arctangent(x)
			possibly with slightly worse error, not necessarily honoring the
			rounding mode (Ali Sazegari narrowing C F.9 10).

	Exceptions:

		Raise underflow for a denormal result (C F.9 7 and Draft Standard for
		Floating-Point Arithmetic P754 Draft 1.2.5 9.5).  If the input is the
		smallest normal, underflow may or may not be raised.  This is stricter
		than the older 754 standard.

		May or may not raise inexact, even if the result is exact (C F.9 8).

		Raise invalid if the input is a signalling NaN (C 5.2.4.2.2 3, in spite
		of C 4.2.1)  but not if the input is a quiet NaN (C F.9 11).

		May not raise exceptions otherwise (C F.9 9).

	Properties:

		Monotonic.

		Exhaustive testing proved this routine returns faithfully rounded
		results.
*/
	.align	5
#if !defined DevelopmentInstrumentation
	// This is the regular name used in the deployed implementation.
		.globl _atanf
	_atanf:
#else
	// This is the name used for a special test version of the routine.
		.globl _atanfInstrumented
	_atanfInstrumented:
#endif

	cvtss2sd	Argx, x				// Convert x to double precision.

	#if defined __i386__

		// Get address of 0 in BaseP.
			call	0f				// Push program counter onto stack.
		0:
			pop		BaseP			// Get program counter.

	#endif

/*	We use different algorithms for different parts of the domain.  There
	is a "negative tail" from -infinity to nPoint0, a center from nPoint0 to
	pPoint0, and a "positive tail" from pPoint0 to +infinity.  Here, we compare
	and branch to the appropriate code.

	NaNs are weeded out in PositiveTail or NegativeTail.
*/

	ucomisd		Address(pPoint0), x
	ja			PositiveTail

	ucomisd		Address(nPoint0), x
	jb			NegativeTail


/*	Here we have nPoint0 <= x <= pPoint0.  This is handled with a simple
	evaluation of a polynomial that approximates arctangent.

	The polynomial has been arranged into the form:

		x * c2
			* ((x**4 + c01 * x**2 + c00))
			* ((x**4 + c11 * x**2 + c10))
			* ((x**4 + c21 * x**2 + c20))
			* ((x**4 + c31 * x**2 + c30))

	The coefficients are stored in pairs, with c01 and c21 at C01, c00 and c20
	at C00, c11 and c31 at C11, and c10 and c30 at C10.  c2 is at C2.

	The quartic factors are evaluated in SIMD registers.  For brevity, some
	comments below describe only one element of a register.  The other is
	analagous.
*/
	movlhps		x, x1				// Save a copy of x for later.
	mulsd		x, x				// Form x**2.
#define	x2	x	// Define name describing current register contents.
	movlhps		x2, x2				// Duplicate x**2.
	movapd		Address(C11), p1
	addpd		x2, p1				// Form x**2 + c11.
	mulpd		x2, p1				// Form x**4 + c11 * x**2.
	addpd		Address(C10), p1	// Form x**4 + c11 * x**2 + c10.
	movapd		Address(C01), p0	// Get first coefficients.
	movlpd		Address(C2), x1		// Put c2 in one element, with x in other.
	addpd		x2, p0				// Form x**2 + c01.
	mulpd		x2, p0				// Form x**4 + c01 * x**2.
	addpd		Address(C00), p0	// Form x**4 + c01 * x**2 + c00.
	mulpd		p1, p0				// Combine factors.
	mulpd		x1, p0				// Multiply by x and c2.
	movhlps		p0, p1				// Get high element.
	mulsd		p1, p0				// Finish combining factors.
#undef x2

// Return the double-precision number currently in p0.
ReturnDoubleInp0:
	cvtsd2ss	p0, p0				// Convert result to single precision.

// Return the single-precision number currently in p0.
ReturnSingleInp0:

	#if defined __i386__
		movss		p0, Argx		// Shuttle result through memory.
			// This uses the argument area for scratch space, which is allowed.
		flds		Argx			// Return input on floating-point stack.
	#else
		// On x86_64, the return value is now in p0, which is %xmm0.
	#endif

	ret


// Handle pPoint0 < x.
PositiveTail:

	ucomisd		Address(pPoint1), x		// Compare x to algorithm change point.
	jae			InputIsPositiveSpecial

	movsd		Address(pHalfPi), p0	// Prepare +pi/2.

CommonTail:

/*	Here we have pPoint0 < x < pPoint1.  The algorithm here is inspired by the
	identity (for positive x) arctangent(x) = +pi/2 - arctangent(1/x).  This
	is approximated by evaluating:

		+pi/2 -
			* ((x**4 + t01 * x**2 + t00))
			* ((x**4 + t11 * x**2 + t10))
			* ((x**4 + t21 * x**2 + t20))
			* ((x**4 + t31 * x**2 + t30))
			* (1/x) * (1/x)**16.

	The quartic factors are evaluated in SIMD registers.  For brevity, some
	comments below describe only one element of a register.  The other is
	analagous.
*/

	movsd		Address(One), v0	// Get constant.
	divsd		x, v0				// Form 1/x, which we will call r.
		/*	I tried rcpss, but using it requires a precision conversion, two
			adds, and three multiplies.  divsd performance is not bad on Core2, 
			and I was not able to get the rcpss version to run as fast.
		*/
	mulsd		x, x				// Form x**2.
#define	x2	x	// Define name describing current register contents.
	movlhps		v0, v0				// Copy finished reciprocal to high element.
	movlhps		x2, x2				// Duplicate x**2.
	mulsd		v0, v0				// Form r**2.
	mulsd		v0, v0				// Form r**4.
	mulsd		v0, v0				// Form r**8.
	movapd		Address(T11), p1	// Get first coefficients.
	mulsd		v0, v0				// Form r**16.

	movapd		Address(T01), t0	// Get first coefficients.
	addpd		x2, p1				// Form x**2 + t11.
	mulpd		x2, p1				// Form x**4 + t11 * x**2.
	addpd		Address(T10), p1	// Form x**4 + t11 * x**2 + t10.
	addpd		x2, t0				// Form x**2 + t01.
	mulpd		x2, t0				// Form x**4 + t01 * x**2.
	addpd		Address(T00), t0	// Form x**4 + t01 * x**2 + t00.
	mulpd		p1, t0				// Combine factors.
	mulpd		v0, t0				// Multiply by r**16 and r.
	movhlps		t0, p1				// Get high element.
	mulsd		p1, t0				// Finish combining factors.
	subsd		t0, p0				// Subtract from pi/2.
#undef x2

	jmp			ReturnDoubleInp0


// Handle x < nPoint0.
NegativeTail:

	ucomisd		Address(nPoint1), x		// Compare x to algorithm change point.
	jbe			InputIsNegativeSpecial

	movsd		Address(nHalfPi), p0	// Prepare -pi/2.

	jmp			CommonTail


	.literal8
/*	Define values near +/- pi/2 that yield +/- pi/2 rounded toward zero when
	converted to single precision.  This allows us to generate inexact and
	return the desired values for atanf of big positive and negative numbers.
*/
AlmostpHalfPi:	.double	+1.5707962
AlmostnHalfPi:	.double	-1.5707962


	.text

/*	Here we handle inputs greater or equal to pPoint1, including infinity,
	but not including NaNs.
*/
InputIsPositiveSpecial:

	/*	The input is a big positive number.  Return +pi/2 rounded toward zero
		and generate an inexact exception.
	*/
	cvtsd2ss	Address(AlmostpHalfPi), p0
	jmp			ReturnSingleInp0


/*	Here we handle inputs less than or equal to -1, including -infinity and
	NaNs.  The condition code must be set as if by using ucomisd to compare the
	input in the "source operand" to nPoint1 in the "destination operand".
*/
InputIsNegativeSpecial:

	jp			InputIsNaN

	/*	The input is a big negative number.  Return -pi/2 rounded toward zero
		and generate an inexact exception.
	*/
	cvtsd2ss	Address(AlmostnHalfPi), p0
	jmp			ReturnSingleInp0


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
