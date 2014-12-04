/*	atan2f.s -- atan2f for standard math library.

	Written by Eric Postpischil, July 2007.
*/


	.literal8

// Define miscellaneous constants.

Threshold:	.double	2.384185791015625e-07		// 2**-22.
nZero:		.double	-0

pPi1v4:		.double	+.7853981633974483096156608	// 1/4 pi.
nPi1v4:		.double	-.7853981633974483096156608
pPi1v2:		.double	+1.570796326794896619231322	// 1/2 pi.
nPi1v2:		.double	-1.570796326794896619231322
pPi3v4:		.double	+2.356194490192344928846982	// 3/4 pi.
nPi3v4:		.double	-2.356194490192344928846982

/*	Define values near +/- pi that yield +/- pi rounded toward zero when
	converted to single precision.  This allows us to generate inexact and
	return the desired values for atan2f on and near the negative side of the x
	axis.
*/
AlmostpPi:	.double	+3.1415924

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

// This needs to be 16-byte aligned because it is used in an orpd instruction.
	.align	4
pPi:		.double	+3.141592653589793238462643	// pi.


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

#define	y			%xmm0	// Must be in %xmm0 for return on x86_64.
#define	x			%xmm1
#define	p0			%xmm2
#define	x1			%xmm3
#define	t0			%xmm4
#define	Base		%xmm5
#define	p1			%xmm6

#if defined __i386__

	// Define locations of arguments.
	#define	Argy			4(%esp)
	#define	Argx			8(%esp)

	// Define how to address data.  BaseP must contain the address of label 0.
	#define	Address(label)	label-0b(BaseP)

#elif defined __x86_64__

	// Define locations of arguments.
	#define	Argx			%xmm1
	#define	Argy			%xmm0

	// Define how to address data.
	#define	Address(label)	label(%rip)

#endif


/*	float atan2f(float x).

	Notes:

		This routine has not been proven to be correct.  See the notes in the
		accompanying C version regarding potential proof.  The polynomial it
		uses was proven to provide faithfully rounded results in atanf.  atan2f
		introduces additional error performing the division and additional
		points used in the domain of the polynomial.

		Citations in parentheses below indicate the source of a requirement.

		"C" stands for ISO/IEC 9899:TC2.

		The Open Group specification (IEEE Std 1003.1, 2004 edition) adds no
		requirements since it defers to C and requires errno behavior only if
		we choose to support it by arranging for "math_errhandling &
		MATH_ERRNO" to be non-zero, which we do not.

	Return value for atan2f(y, x) (C F.9.1 12 and C F.9.1.4):

		y			x			atan2f(y, x)

		-infinity	-infinity	-3*pi/4.
					< 0			-2*pi/4.
					-0			-2*pi/4.
					+0			-2*pi/4.
					> 0			-2*pi/4.
					+infinity	-1*pi/4.

		< 0			-infinity	-4*pi/4.
					< 0			arctangent(y/x) in [-4*pi/4, -2*pi/4].
					-0			-2*pi/4.
					+0			-2*pi/4.
					> 0			arctangent(y/x) in [-2*pi/4, -0*pi/4].
					+infinity	-0*pi/4.

		-0			-infinity	-4*pi/4.
					< 0			-4*pi/4.
					-0			-4*pi/4.
					+0			-0*pi/4.
					> 0			-0*pi/4.
					+infinity	-0*pi/4.

		+0			-infinity	+4*pi/4.
					< 0			+4*pi/4.
					-0			+4*pi/4.
					+0			+0*pi/4.
					> 0			+0*pi/4.
					+infinity	+0*pi/4.

		> 0			-infinity	+4*pi/4.
					< 0			arctangent(y/x) in [+2*pi/4, +4*pi/4].
					-0			+2*pi/4.
					+0			+2*pi/4.
					> 0			arctangent(y/x) in [+0*pi/4, +2*pi/4].
					+infinity	+0*pi/4.

		+infinity	-infinity	+3*pi/4.
					< 0			+2*pi/4.
					-0			+2*pi/4.
					+0			+2*pi/4.
					> 0			+2*pi/4.
					+infinity	+1*pi/4.

		If either input is a NaN, return one of the NaNs in the input.  (C F.9
		11 and 13).  (If the NaN is a signalling NaN, we return the "same" NaN
		quieted.)

		Otherwise:

			If the rounding mode is round-to-nearest, return arctangent(x)
			faithfully rounded.

			Return a value in [-pi, +pi] (C 7.12.4.4 3).  Note that this
			prohibits returning correctly rounded values for -pi and +pi, since
			pi rounded to a float lies outside that interval.
		
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

		We desire this routine to be monotonic, but that has not
		been proven.  (For atan2f, monotonicity would mean, if (x0, y0) and
		(x1, y1) are in the same quadrant, then y0/x0 <= y1/x1 implies
		atan2f(y0, x0) <= atan2f(y1, x1).)
*/
	.align	5
	.globl _atan2f
_atan2f:

	cvtss2sd	Argy, y					// Convert to double precision.
	cvtss2sd	Argx, x

	#if defined __i386__

		// Get address of 0 in BaseP.
			call	0f					// Push program counter onto stack.
		0:
			pop		BaseP				// Get program counter.

	#endif

#define	nx	t0
	movsd		Address(nZero), nx
	xorpd		x, nx					// Negate x.

	ucomisd		x, y
	jae			yGEx					// If we jump, y >= x.
	je			Unordered				// If we jump, an operand is a NaN.
										// If we fall through, y < x.

	ucomisd		y, nx
	jae			yLTx_and_nxGEy			// If we jump, y < x and -x >= y.
										// If we fall through, -x < y < x.

	// Return atand(y/x).
	divsd		x, y					// Form y/x.
	movsd		y, x					// Move to register used by Polynomial.
	movsd		Address(nZero), Base	// Set Base to -0.
		// This makes the return value -0 if y is -0 and x > 0.
	jmp			Polynomial				// Return 0 + arctangent(y/x).


yLTx_and_nxGEy:							// Here y < x && y <= -x.
	je			yLTx_and_nxEQy			// If we jump, y < x && -x == y.
										// If we fall through, y < x < -y.

	// Return -pi/2 - atand(x/y).
	movsd		Address(nZero), t0		// Get -0 for sign bit.
	divsd		y, x					// Form x/y.
	xorpd		t0, x					// Form -x/y.
	movsd		Address(nPi1v2), Base	// Set Base to -pi/2.
	jmp			Polynomial				// Return -pi/2 + arctangent(-x/y).


yLTx_and_nxEQy: 						// Here -x == y < x.
	// Return -1/4*pi with inexact exception.
	cvtsd2ss	Address(nPi1v4), y
	jmp			ReturnSingle


yGEx:									// Here y >= x.
	je			yEQx					// If we jump, y == x.
										// If we fall through, y > x.

	ucomisd		y, nx
#undef	nx
	jae			yGTx_and_nxGEy			// If we jump, y > x && -x >= y.
										// If we fall through, -y < x < y.

	// Return +pi/2 - atand(x/y).
	movsd		Address(nZero), t0		// Get -0 for sign bit.
	divsd		y, x					// Form x/y.
	movsd		Address(pPi1v2), Base	// Set Base to +pi/2.
	xorpd		t0, x					// Form -x/y.
	jmp			Polynomial				// Return +pi/2 + arctangent(-x/y).


yGTx_and_nxGEy:							// Here y > x && -x >= y.
	je			yGTx_and_nxEQy			// If we jump, y > x && -x == y.
										// If we fall through, x < y < -x.

	movsd		Address(nZero), Base	// Get mask for sign bit.
	movapd		Base, t0				// Copy mask.
	andpd		y, Base					// Extract sign bit of y.
	divsd		x, y					// Form y/x.
	andnpd		y, t0					// Take absolute value of quotient.
	comisd		Address(Threshold), t0	// Is quotient small?
	jbe			NearNegativeXAxis
	orpd		Address(pPi), Base		// Set Base to pi with y's sign.
	movsd		y, x					// Move to register used by Polynomial.
	jmp			Polynomial				// Return +/- pi + arctangent(y/x).


NearNegativeXAxis:
	// Return -pi or +pi, matching the sign of y, rounded toward zero.
	movsd		Address(AlmostpPi), p0
	xorpd		Base, p0
	jmp			ReturnDouble


yGTx_and_nxEQy:							// Here x < y == -x.
	// Return +3/4*pi with inexact exception.
	cvtsd2ss	Address(pPi3v4), y
	jmp			ReturnSingle


yEQx:									// Here y == x.

	ucomisd		Address(nZero), y
	jae			yEQx_and_yGE0			// If we jump, y == x && y >= 0.
										// If we fall through, x == y < -x.

	// Return -3/4*pi with inexact exception.
	cvtsd2ss	Address(nPi3v4), y
	jmp			ReturnSingle


yEQx_and_yGE0:							// Here y == x && y >= 0.
	je			yEQx_and_yEQ0			// If we jump, y == x && y == 0.
										// If we fall through, -x < y == x.

	// Return +1/4*pi with inexact exception.
	cvtsd2ss	Address(pPi1v4), y
	jmp			ReturnSingle


yEQx_and_yEQ0:							// Here y == x == 0.

	/*	Return:
			x	y	atan2f(y, x)
			-0	-0	-pi, with inexact exception.
			-0	+0	+pi, with inexact exception.
			+0	-0	-0.
			+0	+0	+0.
	*/

	/*	We want to know if x is -0 or +0, but there is no direct test for that
		that puts the results in a vector register.  We do an arithmetic right
		shift to fill up the exponent bits with copies of the sign bit.  This
		produces a NaN or +0.  Then we test for "unordered", which yields all
		one bits if x was -0 and all zero bits if x was +0.
	*/
	psraw		$12, x
	cmpunordsd	x, x

	// Form (almost) pi if x was -0 and 0 if x was +0.
	movsd		Address(AlmostpPi), p0
	andpd		x, p0

	orpd		y, p0					// Apply the sign bit from y.
	jmp			ReturnDouble


Unordered:								// Here x or y is a NaN.
	addsd		x, y					// Return one of the NaNs, quieted.
	cvtsd2ss	y, y
	jmp			ReturnSingle


/*	This is the principal arctangent evaluation.  Previous code has prepared
	the Base and y registers, and we need to calculate Base + arctangent(y).
	The result is then converted to a single-precision number and returned.

	-1 <= y <= +1.  (Actually, -1 < y < +1.  The equalities were sidetracked
	during all the branching above, and division of two different
	single-precision numbers converted to double-precision never rounds to
	one.)

	There are some slight inefficiencies here, in that special cases could omit
	a few instructions -- sometimes the base is zero or y had to be negated to
	fit this common code.  So, if speed is all important, this routine might be
	speeded up a little by replicating this code.
*/
Polynomial:

/*	The polynomial that approximates arctangent has been arranged into the
 	form:

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
	movsd		x, x1				// Save a copy of x for later.
	movapd		Address(C11), p1
	mulsd		x, x				// Form x**2.
#define	x2	x	// Define name describing current register contents.
	movlhps		x2, x2				// Duplicate x**2.
	addpd		x2, p1				// Form x**2 + c11.
	mulpd		x2, p1				// Form x**4 + c11 * x**2.
	addpd		Address(C10), p1	// Form x**4 + c11 * x**2 + c10.
	movapd		Address(C01), p0	// Get first coefficients.
	addpd		x2, p0				// Form x**2 + c01.
	mulpd		x2, p0				// Form x**4 + c01 * x**2.
	movhpd		Address(C2), x1		// Put c2 in one element, with x in other.
	addpd		Address(C00), p0	// Form x**4 + c01 * x**2 + c00.
	mulpd		p1, p0				// Combine factors.
	mulpd		x1, p0				// Multiply by x and c2.
	movhlps		p0, p1				// Get high element.
	mulsd		p1, p0				// Finish combining factors.
#undef x2
	addsd		Base, p0

// Return the double-precision number currently in p0.
ReturnDouble:
	cvtsd2ss	p0, y				// Convert result to single precision.

// Return the single-precision number currently in p0.
ReturnSingle:

	#if defined __i386__
		movss		y, Argx			// Shuttle result through memory.
			// This uses the argument area for scratch space, which is allowed.
		flds		Argx			// Return input on floating-point stack.
	#else
		// On x86_64, the return value is now in y, which is %xmm0.
	#endif

	ret
