/*	sincostan.c -- sin, cos, and tan for standard math library.

	These are preliminary versions of sin, cos, and tan written to provide
	argument reduction using the correct value of pi, rather than pi rounded to
	66-bits as used by the Intel trigonometric instructions.  Versions to
	enhance accuracy and speed may follow.
*/


#include <math.h>


// Describe the destination floating-point type.
#define	SignificandBits	53
#define	ExponentBits	11


/*	Define a structure for accessing the internal components of a double.

	s.exponent is the raw exponent field.

	s.significand2 is the least two significant bits of the significand field.

	The other fields are unused.
*/
typedef union
{
	double d;
	struct
	{
		#if defined __BIG_ENDIAN__
			unsigned int sign         : 1;
			unsigned int exponent     : ExponentBits;
			unsigned int significand0 : SignificandBits-1-32;
			unsigned int significand1 : 30;
			unsigned int significand2 :  2;
		#else	// defined __BIG_ENDIAN__
			unsigned int significand2 :  2;
			unsigned int significand1 : 30;
			unsigned int significand0 : SignificandBits-1-32;
			unsigned int exponent     : ExponentBits;
			unsigned int sign         : 1;
		#endif	// defined __BIG_ENDIAN__
	} s;
} Double;


/*	Each of the following numbers is a strict upper bound on some technique
	for calculating sin, cos, or tan.  That is, the technique is known to apply
	when |x| < bound, where x is the function argument.  (ReduceFull also
	requires that \x| be greater than some amount around 0x1p-622, to avoid
	underflow or loss of accuracy.)

	If |x| < BoundDenormal, x is denormal or zero.

	If |x| < BoundTrivialSin, x is the correctly rounded sine of x.

	If |x| < BoundTrivialCos, 1 is the correctly rounded cosine of x.

	If |x| < BoundTrivialTan, x is the correctly rounded tangent of x.

	If |x| < BoundPolynomial, sinp(x) or cosp(x) may be used for sin(x) or
	cos(x), with no argument reduction.

	If |x| < BoundMedium, ReduceMedium may be used to reduce x.

	If |x| < BoundFull, ReduceFull may be used to reduce x.

	Otherwise, x is an infinity or a NaN.
*/
#define	BoundDenormal	0x1p-1022
#define	BoundTrivialSin	0x1.7137449123ef7p-26
#define	BoundTrivialCos	0x1.6A09E667F3BCDp-27
#define	BoundTrivialTan	0x1.250BFE1B082F5p-26
#define	BoundPolynomial	0x3.243f6a8885a31p-2
#define	BoundMedium		0x1.000013be57a40p19
#define	BoundFull		INFINITY


/*	Below, we define methods to test whether a function argument, x, is within
	bounds suitable for a techique for implementing the function.  Several
	methods are defined, using natural floating-point comparisons or using
	integer comparisons on 32 or 64 bits of the floating-point representation.
	Implementations may choose whichever method is best for performance.

	To use, first prepare the key used for checking bounds:

		BoundKeyType xk = Key(x);

	That is done once at the start of a routine.  Then, as many times as
	desired, bounds are checked:

		if (Within(xk, bound))
			...

	The "bound" must be one of the names defined above, such as BoundMedium.

	If Within(xk, bound) evaluates to true, then |x| < bound.  The converse is
	not necessarily true; if Within(xk, bound) is false, |x| might be less than
	bound.  This is because some implementations of Within do not check all
	bits of xk and bound.  They are intended only as fast screens that get the
	majority of values.  However, if floor(log[2](|x|)) < floor(log[2](bound)),
	Within(xk, bound) is guaranteed to be true -- its evaluation includes at
	least all of the floating-point exponent.
*/
#if defined __i386__	// (Add any other desired architectures.)

	/*	This method uses integer comparisons with the most significant 32 bits
		of the IEEE-754 representation of the floating-point number.
	*/

	#include <inttypes.h>

	typedef uint32_t BoundKeyType;

	typedef union
	{
		double d;
		#if defined __BIG_ENDIAN__
			struct { uint32_t key, notkey; } s;
		#else
			struct { uint32_t notkey, key; } s;
		#endif
	} DoubleForBound;

	// Prepare constants containing the bounds in our desired form.
	#define	DefineBound(bound)	\
		static const DoubleForBound Double##bound = { bound };
	DefineBound(BoundDenormal)
	DefineBound(BoundTrivialSin)
	DefineBound(BoundTrivialCos)
	DefineBound(BoundTrivialTan)
	DefineBound(BoundPolynomial)
	DefineBound(BoundMedium)
	DefineBound(BoundFull)

	// Get the key for |x|.
	static BoundKeyType Key(double x)
	{
		const DoubleForBound X = { x };
		BoundKeyType t = X.s.key;
		return t & 0x7fffffff;
	}

	#define	Within(xk, bound)	((xk) < Double##bound.s.key)

#elif defined __x86_64__	// (Add any other desired architectures.)

	/*	This method uses integer comparisons with all 64 bits of the IEEE-754
		representation of the floating-point number.
	*/

	#include <inttypes.h>

	typedef uint64_t BoundKeyType;

	typedef union
	{
		double d;
		uint64_t key;
	} DoubleForBound;

	// Prepare constants containing the bounds in our desired form.
	#define	DefineBound(bound)	\
		static const DoubleForBound Double##bound = { bound };
	DefineBound(BoundDenormal)
	DefineBound(BoundTrivialSin)
	DefineBound(BoundTrivialCos)
	DefineBound(BoundTrivialTan)
	DefineBound(BoundPolynomial)
	DefineBound(BoundMedium)
	DefineBound(BoundFull)

	// Get the key for |x|.
	static BoundKeyType Key(double x)
	{
		const DoubleForBound X = { x };
		BoundKeyType t = X.key;
		return t & 0x7fffffffffffffff;
	}

	#define	Within(xk, bound)	((xk) < Double##bound.key)

#else

	// If no other method is selected, use regular floating-point comparison.

	typedef double BoundKeyType;

	#define	DefineBound(bound)

	// Get the key for |x|.
	static BoundKeyType Key(double x)
	{
		#if defined __i386__ || defined __x86_64__
			// On Intel, just clear the sign bit.
			double xk = -0.;
			__asm__("andnpd %[x], %[xk]" : [xk] "+x" (xk) : [x] "x" (x));
			return xk;
		#elif 1
			return x < 0 ? -x : +x;
		#else
			return fabs(x);
		#endif
	}

	#define	Within(xk, bound)	((xk) < bound)

#endif


/*	Several argument-reduction routines follow.  Each has this specification:

		static void Reduce<Name>(double *xp, int *a, double x).

	Input:
	
		x is a number to be reduced modulo pi/2.  Each routine has
		qualifications on what values it supports for x.  Generally, x must not
		be a NaN or infinity.

	Output:

		*xp is set to a residue of x modulo pi/2.  Mostly, -pi/4 <= x <= +pi/4,
		but *xp may be outside this interval by as much as 3.24128e-11.

		*a is set to the arc of the circle x is in.  0 <= *a < 4.

		On output, x is approximately k * 2*pi + *a * pi/2 + *xp for some
		integer k.  This holds even if x is outside [-pi/4, pi/4]; *a matches
		*xp.

	This means all the reduction routines are returning *xp in radians.
	ReduceFull goes through steps in which it has period-fraction units (0 to 1
	corresponding to 0 to pi/2 radians), so the restoration of radians is
	forced and introduces additional rounding error and execution time.  We
	might want to do something about that.

	Since *xp is a single double, it is insufficient to represent the residue
	precisely enough to compute a faithfully rounded function.  For that, we
	will have to return extended precision, perhaps a long double or perhaps
	two doubles.
*/


/*	static void ReduceFull(double *xp, int *a, double x).

	Input:
	
		x is a number to be reduced modulo pi/2.  This routine requires 1 <=
		|x|.  (The actual limit is something smaller, perhaps around 0x1p-622,
		but 1 suffices.)

	Output:

		*xp is set to x modulo pi/2.

		*a is set to the arc of the circle x is in.  0 <= *a < 4.

		On output, x is approximately k * 2*pi + *a * pi/2 + *xp for some
		integer k.

	This routine was adapted from the _sin function in double_fns.h (which
	implements vvsin).
*/
static void ReduceFull(double *xp, int *a, double x)
{
	// The rows are 8-bit shifts of 27-bit windows on 2/pi * 0x1p400.
	static const double TwoOverPiWithOffset[4][45] =
	{
		{
			+0x0.0000000p+0, +0x1.45f3070p+399, -0x1.1b1bbe8p+372,
			-0x1.6b01ec8p+345, +0x1.5f47d50p+318, -0x1.6447e48p+291,
			-0x1.3ad4ce0p+263, +0x1.e21c820p+237, +0x1.fe51640p+208,
			-0x1.5085110p+182, +0x1.586dca0p+154, -0x1.c8e2df0p+129,
			+0x1.374b800p+102, +0x1.924bbb0p+74, -0x1.f62e6e0p+48,
			+0x1.cfe1df0p+20, -0x1.38d3b58p-6, -0x1.63045e0p-34,
			+0x1.1afa980p-63, -0x1.44bb7b0p-88, -0x1.6638fe0p-116,
			+0x1.ad17e00p-142, -0x1.bec66e0p-168, -0x1.4e33e58p-195,
			+0x1.9cfa4e0p-223, +0x1.08bf178p-249, -0x1.036be40p-279,
			+0x1.8ffc4c0p-303, -0x1.0fd3000p-339, -0x1.fc04340p-358,
			-0x1.dce94c0p-385, +0x1.4da3ee0p-413, -0x1.64c0980p-439,
			-0x1.b069ec8p-465, -0x1.1617380p-493, -0x1.32c3400p-521,
			-0x1.5d28ae0p-548, +0x1.eeb1fb0p-574, -0x1.a0e8500p-604,
			+0x1.e839cf8p-627, +0x1.e294a48p-654, +0x1.d4d7f68p-681,
			+0x1.fb11f90p-708, -0x1.517bd50p-735, +0x1.9823800p-767
		},
		{
			+0x0.0000000p+0, +0x1.45f3000p+399, +0x1.b727240p+377,
			-0x1.f56b020p+353, +0x1.3abe900p+325, -0x1.5964480p+299,
			+0x1.b6c52b0p+271, +0x1.93c4390p+244, +0x1.07f9480p+214,
			-0x1.38a8428p+191, -0x1.0ea7920p+162, -0x1.b7238c0p+135,
			+0x1.09374b8p+110, +0x1.924c000p+74, -0x1.15f62e8p+56,
			+0x1.21cfe20p+28, -0x1.0a71a70p+1, -0x1.acb1820p-25,
			-0x1.77dca08p-52, -0x1.68a25d8p-79, -0x1.ec598e0p-106,
			-0x1.fb29740p-133, -0x1.037d8d0p-161, +0x1.1d63980p-188,
			+0x1.a99cfa0p-215, +0x1.3908bf0p-241, +0x1.77bf250p-269,
			+0x1.d8ffc80p-299, -0x1.a000880p-322, +0x1.6603fc0p-350,
			-0x1.a1dce90p-377, -0x1.2fac970p-403, -0x1.2593040p-433,
			+0x1.9e4f960p-457, +0x1.36e9e90p-485, -0x1.c099620p-512,
			+0x1.7fa8b60p-538, -0x1.5b08a70p-565, -0x1.41a0e80p-596,
			-0x1.30be300p-622, -0x1.821d6b8p-646, +0x1.25d4d80p-673,
			-0x1.2813b80p-702, -0x1.ca8be00p-730, +0x1.580cc10p-754
		},
		{
			+0x0.0000000p+0, +0x1.4600000p+399, -0x1.9f246c0p+386,
			-0x1.bbead60p+360, -0x1.ec54100p+329, -0x1.c159648p+307,
			+0x1.c0db628p+280, +0x1.5993c40p+252, +0x1.c820ff0p+225,
			+0x1.458eaf0p+198, +0x1.ebbc560p+172, +0x1.b7246e0p+144,
			+0x1.d2126f0p+117, -0x1.a3ff370p+91, +0x1.2eea0a0p+64,
			-0x1.736f180p+37, -0x1.e214e40p+8, +0x1.62534e8p-17,
			-0x1.177dc80p-48, -0x1.0568a28p-71, +0x1.1213a68p-98,
			-0x1.c7eca60p-127, +0x1.7df9040p-154, +0x1.cc8eb20p-179,
			-0x1.9f2b318p-206, -0x1.6c6f780p-237, +0x1.f8bbdf8p-260,
			+0x1.283b200p-288, -0x1.da00080p-318, -0x1.fa67f00p-344,
			-0x1.0d0ee80p-372, +0x1.6b414e0p-397, -0x1.7049650p-423,
			+0x1.fb3c9f0p-450, +0x1.6136ea0p-477, -0x1.7381320p-505,
			-0x1.8680578p-530, +0x1.aea4f78p-557, -0x1.38141a0p-584,
			-0x1.d098600p-613, +0x1.ce7de28p-638, +0x1.4a4baa0p-666,
			-0x1.404a050p-692, +0x1.1f8d5d0p-720, +0x1.0ac0680p-749
		},
		{
			+0x0.0000000p+0, +0x1.8000000p+399, -0x1.d067c90p+396,
			-0x1.b1bbeb0p+368, +0x1.4fe13b0p+341, -0x1.05c1598p+315,
			+0x1.bb81b70p+287, -0x1.d6a66c0p+260, -0x1.de37df0p+233,
			-0x1.ae9c400p+200, -0x1.4214438p+180, -0x1.4f246e0p+153,
			+0x1.b8e9090p+126, +0x1.ba5c010p+99, -0x1.b6d1160p+72,
			+0x1.3a32440p+43, -0x1.80f10a0p+17, -0x1.c69dac8p-9,
			-0x1.8c11780p-36, +0x1.1afa978p-63, -0x1.12edec8p-90,
			+0x1.338e050p-117, -0x1.4ba0818p-144, -0x1.f633718p-171,
			+0x1.8e60d50p-198, -0x1.8c16c70p-225, +0x1.17e2f00p-254,
			-0x1.036be28p-279, +0x1.ff89800p-308, -0x1.0fd3400p-339,
			+0x1.fde5e00p-365, +0x1.18b5a10p-388, -0x1.64b8248p-414,
			-0x1.9302618p-441, -0x1.834f648p-468, -0x1.6173820p-497,
			+0x1.9a797f8p-522, +0x1.45aea50p-549, -0x1.14e0500p-578,
			-0x1.a0e84c0p-604, -0x1.7c63040p-631, -0x1.d6b5b40p-658,
			-0x1.59404a0p-684, -0x1.3b81cc0p-714, +0x1.7421580p-738
		}
	};

	Double X = { x };

	/*	Set ec to the unbiased exponent minus 33.  Why 33?  I do not know.
		This was in the vvsin code in double_fns.h.
	*/
	int ec = X.s.exponent - (1023+33);

	// Set k to ceiling(ec / 27) and m to residue.
	int k = (ec + 26) * (607*4) >> 16;
	int m = 27*k - ec;

	// offset is used to select a row in the reduction table.  See below.
	int offset = m >> 3;

	/*	The reduction table, TwoOverPiWithOffset, contains bits of 2/pi.
		First, all entries are scaled by 2**400 to avoid overflow/underflow
		issues.  Each entry contains 27 bits of 2/pi, except in the first two
		columns.  The first column contains zeroes because no reduction is done
		for numbers that are already small.  The entries in the second row
		start before (at a bit with higher value) the leading bit of 2/pi, so
		they contain some leading zeroes.  The sign bits form part of the 27
		bits represented.

		Each row forms a contiguous string of bits of 2/pi.  That is, adding
		all the entries with sufficient precision yields a single bit string
		representing 2/pi.  The rows differ in their starting points; row 0
		begins with the leading bit of 2/pi, and rows 1, 2, and 3 begin 8, 16,
		and 24 bits before that.
	*/

	/*	Scale x to avoid overflow in Dekker split.  This is compensated for
		in the entries in TwoOverPiWithOffset.
	*/
	x *= 0x1p-400;

	/*	Use Dekker's algorithm to split x into 26 bits and 27 bits.  This
		requires round-to-nearest mode.
	*/
	double xDekker = x * (0x1p27 + 1);
	double x0 = xDekker - (xDekker - x);
	double x1 = x - x0;

	// Get address of starting point in table.
	const double *p0 = &TwoOverPiWithOffset[offset][k];

	// Get table entries.
	const double fp0 = p0[0];
	const double fp1 = p0[1];
	const double fp2 = p0[2];
	const double fp3 = p0[3];

	// Get high bits of x * f, where f is the part of 2/pi we are using.
	const double f0 = x1 * fp0 + fp1 * x0;

	double f = x1 * fp1 + fp2 * x0;

	// Combine to do integer work.
	const double fi = f0 + f;

	static const double IntegerBias = 0x1.8p52;

	// Force the integer bits into a specific position.
	Double Fi = { fi + IntegerBias };
		/*	|fi| is less than 0x1p36, so fi + IntegerBias is well within
			[0x1p52, 0x1p53), so it has a known exponent, and the bits with
			weight 2 and 1 are the least significant bits in its significand.

			We know |fi| is less than 0x1p36 because it is the sum of x1 * fp0,
			fp1 * x0, and something small in f.  x0 has the same exponent as
			x.  Say 2**e <= |x| < 2**(e+1).  (That is the original x, before we
			scaled it by 0x1p-400.)  The table entry we look up for fp0 has
			magnitude less than 2**(400+27-27*ceiling((e-33)/27)), and fp1 is
			less, bounded by 2**(400-27*ceiling((e-33)/27)).  Including the
			scaling in x, fp1 * x0 is less than 2**(e+1-400) *
			2**(400-27*ceiling((e-33)/27)) <= 2**34.  Similarly, fp0 * x1 is
			less than 2**34, so their sum is less than 2**35.
		*/

	// Get the two least significant integer bits.
	*a = Fi.s.significand2;

	double fint = Fi.d - IntegerBias;

	const double fp4 = p0[4];
	const double fp5 = p0[5];
	const double fp6 = p0[6];

	f = f0 - fint + f;
	f += x1 * fp2 + fp3 * x0;
	f += x1 * fp3 + fp4 * x0;
	f += x1 * fp4 + fp5 * x0;
	f += x1 * fp5 + fp6 * x0;

	// Convert to radians by multiplying by pi/2.
	*xp = f * 0x3.243F6A8885A3p-1;
}


/*	static void ReduceMedium(double *xp, int *a, double x).

	Input:
	
		x is a number to be reduced modulo pi/2.  This routine requires |x| <=
		X.  X is described below.

	Output:

		*xp is set to x modulo pi/2.

		*a is set to the arc of the circle x is in.  0 <= *a < 4.

		On output, x is approximately k * 2*pi + *a * pi/2 + *xp for some
		integer k.

	Nomenclature:

		p is the period, pi/2.

		X is the maximum value x may have.  X is 0x1.000013be57a3fp19.

		InversePeriod is 1/p, rounded to the nearest double, ties to even.

		n is x * InversePeriod, rounded to the nearest double, ties to even,
		and then rounded to the nearest integer, ties to even.

		N is the maximum value n may have.  N is X * InversePeriod, rounded to
		the nearest double, ties to even, and then rounded to the nearest
		integer, ties to even.  N is 333773.

	Notes:

		In comments in this routine, unquoted expressions are mathematical and
		quoted expressions are floating-point.  Thus, "a + b" refers to the
		floating-point operation of addition, including rounding.

		"n * Period[0]" is exact for all |n| <= N.  It is inexact for n = N+1,
		so this is the source of the bound on x.  X is the greatest value for
		which |n| <= N.  (It is happenstance that Period[0] limits x; if the
		period were different, its bits might result in Period[1] causing the
		limit.  n has some other roles in the errors in this routine that might
		limit x when used with other periods or different precisions.)

		The difference between InversePeriod and 1/p can cause n to differ from
		the ideal value when x is near a multiple of p.  The result is that
		instead of producing a result inside the target interval [-p/2, p/2], a
		result slightly outside the interval is produced.  The result may be as
		much as 3.24128e-11 outside the interval.  The polynomial and the
		polynomial evaluation must be satisfactory over this extended interval.
*/
static void ReduceMedium(double *xp, int *a, double x)
{
	static const double InversePeriod =
		2 / 0x3.243f6a8885a308d313198a2e03707344ap0;

	/*	Period is an extended-precision representation of the reduction period.
		Each element except the last has the property that multiplication by
		any integer with magnitude at most N is exact.
	*/
	static const double Period[] = {
			+0x1.921FB54440000p-0,
			+0x1.68C234C4C0000p-39,
			+0x1.98A2E03707345p-77
		};

	// Estimate x / p and round to nearest integer.
	double n = x * InversePeriod + 0x1.8p52 - 0x1.8p52;

	// Record which arc of the circle x lies in.
	*a = (int) n & 3;

	// This is exact, per design of Period.
	double np0 = n * Period[0];

	// This is exact, per design of Period.
	double np1 = n * Period[1];

	/*	The error here is at most 1/2 ULP(n * Period[2]), which is
		1/2 ULP(n * +0x1.98A2E03707345p-77) = ULP(n * +0x1.98A2E03707345p-78).

		n <= N = 333773, so an absolute bound on that is ULP(333773 *
		+0x1.98A2E03707345p-78) = 0x1p-111, but we need a bound dependent on n
		to partition a proof below.

		This is called error (a).
	*/
	double np2 = n * Period[2];

	/*	Set x00 to x - n * Period[0].  np0 is very near x, so this is exact,
		per David Goldberg, _What Every Computer Scientist Should Know About
		Floating-Point Arithmetic_, Theorem 11.

		The nomenclature used here is:

			x0 is x - n * (Period[0]),
			x1 is x - n * (Period[0] + Period[1]), and
			x2 is x - n * (Period[0] + Period[1] + Period[2]).

			x00 is the first and only "word" of an extended precision
			representation of x0.

			x10 is the first "word" of x1; it has the most significant bits.
			x11 is the next "word" of x1; it has the following bits.
	*/
	double x00 = x - np0;

	/*	Subtract n * Period[1] from x0 to produce x1.  These operations yield
		an exact result, per Knuth, _The Art of Computer Programming_ 2, second
		edition, page 221, section 4.2.2, Theorem C, in the sense that x10 +
		x11 will exactly equal x0 - n * Period[1].  The first statement puts
		the high bits in x10, and the second statement determines what was
		rounded away in x10 and puts it in x11.

		Knuth assumes |np1| <= |x00|.  This is often the case, but perhaps
		cancellation has reduced the magnitude of x00.  Hoever, if |x00| <
		|np1|, this arithmetic is exact, because x00's least significant bit is
		at least as large as the ULP of Period[1].
	*/
	double x10 = x00 - np1;
	double x11 = x00 - x10 - np1;

	/*	This comment demonstrates the rounding error in the following
		statement, "t = x11 - np2", is tiny relative to x10.

		This is called error (b).

		Let LSB(x) be the weight of the least significant bit set in a
		floating-point number x.  E.g, LSB(1.25) is 1/4, although ULP(1.25) is
		2**-52.
		
		Here, x10 and x11 are each multiples of LSB(Period[1]).  If |x10| <
		2**53 LSB(Period[1]), then x11 is zero, because all the bits of x10 +
		x11 fit into x10.  In that case, "x11 - np2" is exact.

		Otherwise, 2**53 LSB(Period[1]) <= |x10|, so 2 LSB(Period[1]) <=
		ULP(x10).  LSB(Period[1]) = 0x1p-73, so 0x1p-72 <= ULP(x10).

		By construction of x10 and x11, |x11| <= 1/2 ULP(x10).
		
		|np2| <= |"n * Period[2]"| <= |n * Period[2] * (1+2**-53)| = 333733 *
		0x1.98A2E03707345p-77 * (1+2**-53) < 0x1.0426p-58.

		|"x11 - np2"| <= 2*max(|x11|, |np2|) <= 2*max(1/2 ULP(x10),
		0x1.0426p-58) = max(ULP(x10), 0x1.0426p-57).  The rounding error in
		"x11 - np2" is at most 2**-53 times this, so it is at most max(2**-53
		ULP(x10), 0x1p-110).  Since ULP(x10) is at least 0x1p-72, the rounding
		error is at most 0x1p-38 ULP(x10).
	*/
	double t = x11 - np2;

	/*	Add t to x1 to produce x2.  As before, we could use Knuth's technique
		to produce an exact sum, x20 + x21 = x10 + t.  For now, we only return
		x20.  When we want extended precision, we could return x21 as well.
		(If so, be careful to show that |t| <= |x10|, as required for Knuth's
		technique, or otherwise show the arithmetic is sufficiently accurate.)

		The difference between x20 and x10 + t is called error (c).  It is at
		most 1/2 ULP(x20).
	*/
	double x20 = x10 + t;
	// double x21 = x10 - x20 + t;

	*xp = x20;

	/*	Having subtracted n * (Period[0] + Period[1] + Period[2]) from x, we
		have an error caused by the difference between p and Period[0] +
		Period[1] + Period[2], which is less than 0x1.6fdb2p-131.  So the error
		is less than n * 0x1.6fdb2p-131.
	   
		n <= N = 333773, so an absolute bound on that is 333773 *
		0x1.6fdb2p-131 < 0x1.d45fp-113, but we need a bound dependent on n to
		partition a proof below.

		This is called error (d).

		Our reduction has four errors:

			(a), which is at most ULP(n * +0x1.98A2E03707345p-78) <= 0x1p-111.
			(b), which is at most 0x1p-38 ULP(x10).
			(c), which is at most 1/2 ULP(x20).
			(d), which is at most n * 0x1.6fdb2p-131 < 0x1.d45fp-113.

		From the discussion of (b), if x11 is not zero, then 0x1p-72 <=
		ULP(x10).  Then x10 dominates the result, because np2 is so small that
		subtracting it from x1 to produce x2 cannot reduce the exponent by more
		than one.  So the errors (a), (b), and (d) are tiny relative to x10,
		and the total error is very nearly (c), 1/2 ULP(x20).

		However, if x11 is zero, then (b) is zero, and we only need to consider
		(a), (c), and (d).  We partition this into two cases.

		Case 0:  |x| < 0x1p12.

			The double-precision floating-point number closest to a multiple of
			pi/2 in that interval is 0x1.6c6cbc45dc8dep6 (according to Maple
			code from Muller, _Elementary Functions_), and it is about
			0x1.6d61b58c99c43p-60 away from a multiple of pi/2.

			n is at most 0x1p12 * InversePi, rounded to a double, so n < 2608.
			Then:

				(a) is at most ULP(2608 * +0x1.98A2E03707345p-78) = 0x1p-119.
				(b) is zero.
				(c) is at most 1/2 ULP(x20).
				(d) is at most 2608 * 0x1.6fdb2p-131 < 0x1p-119.

			The final result might be as low as about 0x1.6d6p-60.  That has an
			ULP of 0x1p-113, so (a) and (d) are small relative to it, and the
			total error is nearly (c), 1/2 ULP(x20).

		Case 1:  0x1p12 <= |x|.

			We still have |x| <= 0x1.000013be57a3fp19.  The double-precision
			floating-point number closest to a multiple of pi/2 in that set is
			0x1.6c6cbc45dc8dep11 (Muller again), and it is about
			0x1.6d61b58c99c43p-55 away from a multiple of pi/2.

			Then:

				(a) is at most ULP(333773 * +0x1.98A2E03707345p-78) = 0x1p-111.
				(b) is zero.
				(c) is at most 1/2 ULP(x20).
				(d) is at most 333773 * 0x1.6fdb2p-131 < 0x1.d45fp-113.

			The final result might be as low as about 0x1.6d6p-55.  That has an
			ULP of 0x1p-108, so (a) and (d) are small relative to it, and the
			total error is nearly (c), 1/2 ULP(x20).

		Therefore, the total error is never much more than 1/2 ULP of the value
		returned in *xp.
	*/
}


/*	static double sinp(double r)
	
	Return sine(r) using only a polynomial, no reduction.

	Input:
	
		r, with |r| < pi/4 + 3.24128e-11.

	Output:

		Approximately sine(r) is returned.
*/
static double sinp(double r)
{
	double rr = r * r;

	/*	Derived from Cephes Math Library Release 2.8:  June, 2000.  Maple's
		infnorm routine says this polynomial is within .0688 ULP of
		sine(r) for |r| < pi/4 + 3.24128e-11.
	*/
	return  r + r * rr * ((((((
		+ 1.58962301576546568060E-10) * rr
		- 2.50507477628578072866E-8 ) * rr
		+ 2.75573136213857245213E-6 ) * rr
		- 1.98412698295895385996E-4 ) * rr
		+ 8.33333333332211858878E-3 ) * rr
		- 1.66666666666666307295E-1 );
}


/*	static double cosp(double r)
	
	Return cosine(r) using only a polynomial, no reduction.

	Input:
	
		r, with |r| < pi/4 + 3.24128e-11.

	Output:

		Approximately cosine(r) is returned.
*/
static double cosp(double r)
{
	double rr = r * r;

	/*	Derived from Cephes Math Library Release 2.8:  June, 2000.  Maple's
		infnorm routine says this polynomial is within .00709 ULP of
		cosine(r) for |r| < pi/4 + 3.24128e-11.
	*/
	return (((((((
		- 1.13585365213876817300E-11) * rr
		+ 2.08757008419747316778E-9 ) * rr
		- 2.75573141792967388112E-7 ) * rr
		+ 2.48015872888517045348E-5 ) * rr
		- 1.38888888888730564116E-3 ) * rr
		+ 4.16666666666665929218E-2 ) * rr
		-  .5                       ) * rr
		+ 1;
}


/*	double sin(double x).

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

			If the rounding mode is round-to-nearest, return sine(x) within a
			few ULP.  The maximum error of this routine is not precisely
			known.  The maximum error of the reduction might be around 3 ULP,
			although this is partly a guess.  The polynomials have small
			errors.  The polynomial evaluation might have an error under 1
			ULP.  So the worst error for this routine might be under 4 ULP.
		  
			Not currently implemented:  In other rounding modes, return sine(x)
			possibly with slightly worse error, not necessarily honoring the
			rounding mode (Ali Sazegari narrowing C F.9 10).

		All results are in [-1, 1].

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
*/
double sin(double x)
{
	// Get |x| in form used by the bound-check operations for a key.
	BoundKeyType xk = Key(x);

	double r;
	int j0;

	/*	Handle denormal numbers and zero here to generate underflow for
		denormals numbers and to avoid generating inexact for zero.
	*/
	if (Within(xk, BoundDenormal))
		return x * (1 - 0x1p-53);

	/*	For small numbers, we can return x for sine(x).  Unfortunately,
		adding this check slows down the other cases, which may be more
		frequent.
	*/
	else if (Within(xk, BoundTrivialSin))
	{
		/*	Make t0 volatile to force compiler to fetch it at run-time
			rather than optimizing away the multiplication.
		*/
		static volatile const double t0 = 1/3.;

		/*	Get t0 once.  If we wrote "t0*t0", the compiler would load it
			twice, since it is volatile.
		*/
		const double t1 = t0;

		/*	Perform a multiplication and pass it into an assembly construct to
			prevent the compiler from knowing we do not use the result and
			optimizing away the multiplication.
		*/
		__asm__("" : : "X" (t1*t1));

		// Return the floating-point number nearest sine(x), which is x.
		return x;
	}

	// If |x| is small, no reduction is necessary.
	else if (Within(xk, BoundPolynomial))
		r = x, j0 = 0;
		/*	The interval for this could be enlarged by examining the sine
			polynomial and figuring out how big r can get before the error is
			too large.
		*/

	// If |x| is medium, we can use a fast reduction routine.
	else if (Within(xk, BoundMedium))
		ReduceMedium(&r, &j0, x);

	// Otherwise, we need the full, slow reduction routine.
	else if (Within(xk, BoundFull))
		ReduceFull(&r, &j0, x);

	else
		return x-x;

	switch (j0)
	{
		default:
		case 0:	return +sinp(r);
		case 1:	return +cosp(r);
		case 2:	return -sinp(r);
		case 3:	return -cosp(r);
	}
}


/*	double cos(double x).

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

			If the rounding mode is round-to-nearest, return cosine(x) within a
			few ULP.  The maximum error of this routine is not precisely
			known.  The maximum error of the reduction might be around 3 ULP,
			although this is partly a guess.  The polynomials have small
			errors.  The polynomial evaluation might have an error under 1
			ULP.  So the worst error for this routine might be under 4 ULP.
		  
			Not currently implemented:  In other rounding modes, return
			cosine(x) possibly with slightly worse error, not necessarily
			honoring the rounding mode (Ali Sazegari narrowing C F.9 10).

		All results are in [-1, 1].

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
*/
double cos(double x)
{
	// Get |x| in form used by the bound-check operations for a key.
	BoundKeyType xk = Key(x);

	double r;
	int j0;

	// Avoid generating inexact for zero.
	if (x == 0)
		return 1;

	/*	For small numbers, we can return 1 for cosine(x).  Unfortunately,
		adding this check slows down the other cases, which may be more
		frequent.  It greatly speeds up cases where |x| < 0x1p-492, by
		avoiding arithmetic with denormals.
	*/
	else if (Within(xk, BoundTrivialCos))
	{
		/*	Make t0 volatile to force compiler to fetch it at run-time
			rather than optimizing away the multiplication.
		*/
		static volatile const double t0 = 1/3.;

		/*	Get t0 once.  If we wrote "t0*t0", the compiler would load it
			twice, since it is volatile.
		*/
		const double t1 = t0;

		/*	Perform a multiplication and pass it into an assembly construct to
			prevent the compiler from knowing we do not use the result and
			optimizing away the multiplication.
		*/
		__asm__("" : : "X" (t1*t1));

		// Return the floating-point number nearest cosine(x), which is 1.
		return 1;
	}

	// If |x| is small, no reduction is necessary.
	else if (Within(xk, BoundPolynomial))
		r = x, j0 = 0;
		/*	The interval for this could be enlarged by examining the cosine
			polynomial and figuring out how big r can get before the error is
			too large.
		*/

	// If |x| is medium, we can use a fast reduction routine.
	else if (Within(xk, BoundMedium))
		ReduceMedium(&r, &j0, x);

	// Otherwise, we need the full, slow reduction routine.
	else if (Within(xk, BoundFull))
		ReduceFull(&r, &j0, x);

	else
		return x-x;

	switch (j0)
	{
		default:
		case 0:	return +cosp(r);
		case 1:	return -sinp(r);
		case 2:	return -cosp(r);
		case 3:	return +sinp(r);
	}
}


/*	double tan(double x).

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

			If the rounding mode is round-to-nearest, return tangent(x) within
			a few ULP.  The maximum error of this routine is not precisely
			known.  The maximum error of the reduction might be around 3 ULP,
			although this is partly a guess.  The polynomials have small
			errors.  The polynomial evaluation might have an error under 1
			ULP.  How the final division affects error has not been considered
			yet.  4.55 ULP has been observed.
		  
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
		of C 4.2.1) or an infinity (C F.9.1.7) but not if the input is a quiet
		NaN (C F.9 11).

		May not raise exceptions otherwise (C F.9 9).

	Properties:

		Desired to be monotonic.  Not yet proven!
*/
double tan(double x)
{
	// Get |x| in form used by the bound-check operations for a key.
	BoundKeyType xk = Key(x);

	double r;
	int j0;

	/*	Handle denormal numbers and zero here to generate underflow for
		denormals numbers and to avoid generating inexact for zero.
	*/
	if (Within(xk, BoundDenormal))
		return x * (1 - 0x1p-53);

	/*	For small numbers, we can return x for tangent(x).  Unfortunately,
		adding this check slows down the other cases, which may be more
		frequent.
	*/
	else if (Within(xk, BoundTrivialTan))
	{
		/*	Make t0 volatile to force compiler to fetch it at run-time
			rather than optimizing away the multiplication.
		*/
		static volatile const double t0 = 1/3.;

		/*	Get t0 once.  If we wrote "t0*t0", the compiler would load it
			twice, since it is volatile.
		*/
		const double t1 = t0;

		/*	Perform a multiplication and pass it into an assembly construct to
			prevent the compiler from knowing we do not use the result and
			optimizing it away.
		*/
		__asm__("" : : "X" (t1*t1));

		// Return the floating-point number nearest tangent(x), which is x.
		return x;
	}

	// If |x| is small, no reduction is necessary.
	else if (Within(xk, BoundPolynomial))
		r = x, j0 = 0;
		/*	The interval for this could be enlarged by examining the
			polynomials and figuring out how big r can get before the error is
			too large.
		*/

	// If |x| is medium, we can use a fast reduction routine.
	else if (Within(xk, BoundMedium))
		ReduceMedium(&r, &j0, x);

	// Otherwise, we need the full, slow reduction routine.
	else if (Within(xk, BoundFull))
		ReduceFull(&r, &j0, x);

	else
		return x-x;

	switch (j0)
	{
		default:
		case 0:	case 2: return + sinp(r) / cosp(r);
		case 1:	case 3: return - cosp(r) / sinp(r);
	}
}
