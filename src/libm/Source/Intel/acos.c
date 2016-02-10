/*	This is an implementation of acos.  It is written in standard C except:
   
		double is expected be an IEEE 754 double-precision implementation.
		
		"volatile" is used to attempt to force certain floating-point
		operations to occur at run time (to generate exceptions that might not
		be generated if the operations are performed at compile time).

		long double is used for extended precision if it has at least 64 bits
		of precision.  Otherwise, double is used with some cumbersome routines
		to provide multiple precision arithmetic, resulting in precise and
		accurate but slow code.  Other techniques, such as dividing the work
		into smaller intervals, were tried briefly but did not yield
		satisfactory solutions in the time allowed.

	If the long double issues are worked out, it should be good enough to serve
	as the libm acos with tolerable performance.
*/


// Include math.h to ensure we match the declarations.
#include <math.h>

// Include float.h so we can check the characteristics of the long double type.
#include <float.h>
#if 64 <= LDBL_MANT_DIG
	#define	UseLongDouble	1
#endif


/*	Declare certain constants volatile to force the compiler to access them
	when we reference them.  This in turn forces arithmetic operations on them
	to be performed at run time (or as if at run time).  We use such operations
	to generate invalid or inexact.
*/
static volatile const double Infinity = INFINITY;
static volatile const double Tiny = 0x1p-1022;


#if defined __STDC__ && 199901L <= __STDC_VERSION__ && !defined __GNUC__
	// GCC does not currently support FENV_ACCESS.  Maybe someday.
	#pragma STDC FENV_ACCESS ON
#endif


#if !UseLongDouble


	/*	If we are not using long double arithmetic, define some subroutines
		to provided extended precision using double arithmetic.

		These routines are derived from CR-LIBM:  A Library of correctly
		rounded elementary functions in double-precision by Catherine
		Daramy-Loirat, David Defour, Florent de Dinechin, Matthieu Gallet,
		Nicolas Gast, Jean-Michel Muller.

		The digits in the names indicate the precisions of the arguments and
		results.  For example, Add112 adds two numbers expressed with one
		double each and returns a number expressed with two doubles.
	*/


	// double2 represents a number equal to d0 + d1, with |d1| <= 1/2 ULP(d0).
	typedef struct { double d0, d1; } double2;


	// Return a + b given |b| < |a|.
	static inline double2 Add112RightSmaller(double a, double b)
	{
		double d0 = a + b, d1 = b - (d0 - a);
		return (double2) { d0, d1 };
	}


	// Return a * b, given |a|, |b| < 2**970.
	static inline double2 Mul112(double a, double b)
	{
		static const double c = 0x1p27 + 1;

		double
			ap = a * c,     bp = b * c,
			a0 = (a-ap)+ap, b0 = (b-bp)+bp,
			a1 = a - a0,    b1 = b - b0,
			d0 = a * b,
			d1 = a0*b0 - d0 + a0*b1 + a1*b0 + a1*b1;
		return (double2) { d0, d1 };
	}


	// Return a + b with relative error below 2**-103 given |b| < |a|.
	static inline double2 Add212RightSmaller(double2 a, double b)
	{
		double
			c0 = a.d0 + b,
			c1 = a.d0 - c0 + b + a.d1,
			d0 = c0 + c1,
			d1 = c0 - d0 + c1;
		return (double2) { d0, d1 };
	}


	/*	Return a - b with relative error below 2**-103 and then rounded to a
		double given |b| < |a|.
	*/
	static inline double Sub211RightSmaller(double2 a, double b)
	{
		double
			c0 = a.d0 - b,
			c1 = a.d0 - c0 - b + a.d1,
			d0 = c0 + c1;
		return d0;
	}


	/*	Return a - b with relative error below 2**-103 and then rounded to
		double given |b| < |a|.
	*/
	static inline double Sub221RightSmaller(double2 a, double2 b)
	{
		double
			c0 = a.d0 - b.d0,
			c1 = a.d0 - c0 - b.d0 - b.d1 + a.d1,
			d0 = c0 + c1;
		return d0;
	}


	/*	Return approximately a * b - 1 given |a|, |b| < 2**970 and a * b is
		very near 1.
	*/
	static inline double Mul121Special(double a, double2 b)
	{
		static const double c = 0x1p27 + 1;

		double
			ap = a * c,     bp = b.d0 * c,
			a0 = (a-ap)+ap, b0 = (b.d0-bp)+bp,
			a1 = a - a0,    b1 = b.d0 - b0,
			m1 = a0*b0 - 1 + a0*b1 + a1*b0 + a1*b1 + a*b.d1;
		return m1;
	}


	// Return approximately a * b given |a|, |b| < 2**970.
	static inline double Mul221(double2 a, double2 b)
	{
		static const double c = 0x1p27 + 1;

		double
			ap = a.d0 * c,     bp = b.d0 * c,
			a0 = (a.d0-ap)+ap, b0 = (b.d0-bp)+bp,
			a1 = a.d0 - a0,    b1 = b.d0 - b0,
			m0 = a.d0 * b.d0,
			m1 = a0*b0 - m0 + a0*b1 + a1*b0 + a1*b1 + (a.d0*b.d1 + a.d1*b.d0),
			d0 = m0 + m1;
		return d0;
	}


#endif	// !UseLongDouble


/*	double acos(double x).

	(This routine appears below, following some subroutines.)

	Notes:

		Citations in parentheses below indicate the source of a requirement.

		"C" stands for ISO/IEC 9899:TC2.

		The Open Group specification (IEEE Std 1003.1, 2004 edition) adds no
		requirements since it defers to C and requires errno behavior only if
		we choose to support it by arranging for "math_errhandling &
		MATH_ERRNO" to be non-zero, which we do not.

	Return value:

		For arccosine of 1, return +0 (C F.9.1.1).

		For 1 < |x| (including infinity), return NaN (C F.9.1.1).

		For a NaN, return the same NaN (C F.9 11 and 13).  (If the NaN is a
		signalling NaN, we return the "same" NaN quieted.)

		Otherwise:

			If the rounding mode is round-to-nearest, return arccosine(x)
			faithfully rounded.  This is not proven but seems likely.
			Generally, the largest source of errors is the evaluation of the
			polynomial using double precision.  Some analysis might bound this
			and prove faithful rounding.  The largest observed error is .922
			ULP.

			Return a value in [0, pi] (C 7.12.4.1 3).
		
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

		Not yet proven:  Monotonic.
*/


// Return arccosine(x) given |x| <= .4, with the same properties as acos.
static double Center(double x)
{
	static const double
		HalfPi = 0x3.243f6a8885a308d313198a2e03707344ap-1;

	/*	If x is small, generate inexact and return pi/2.  We must do this
		for very small x to avoid underflow when x is squared.
	*/
	if (-0x1.8d313198a2e03p-53 <= x && x <= +0x1.8d313198a2e03p-53)
		return HalfPi + Tiny;

	static const double p03 = + .1666666666666251331848183;
	static const double p05 = + .7500000000967090522908427e-1;
	static const double p07 = + .4464285630020156622713320e-1;
	static const double p09 = + .3038198238851575770651788e-1;
	static const double p11 = + .2237115216935265224962544e-1;
	static const double p13 = + .1736953298172084894468665e-1;
	static const double p15 = + .1378527665685754961528021e-1;
	static const double p17 = + .1277870997666947910124296e-1;
	static const double p19 = + .4673473145155259234911049e-2;
	static const double p21 = + .1951350766744288383625404e-1;

	// Square x.
	double x2 = x * x;

	return HalfPi - (((((((((((
		+ p21) * x2
		+ p19) * x2
		+ p17) * x2
		+ p15) * x2
		+ p13) * x2
		+ p11) * x2
		+ p09) * x2
		+ p07) * x2
		+ p05) * x2
		+ p03) * x2 * x + x);
}


// Return arccosine(x) given .4 <= |x| <= .6, with the same properties as acos.
static double Gap(double x)
{
	static const double p03 = + .1666666544260252354339083;
	static const double p05 = + .7500058936188719422797382e-1;
	static const double p07 = + .4462999611462664666589096e-1;
	static const double p09 = + .3054999576148835435598555e-1;
	static const double p11 = + .2090953485621966528477317e-1;
	static const double p13 = + .2626916834046217573905021e-1;
	static const double p15 = - .2496419961469848084029243e-1;
	static const double p17 = + .1336320190979444903198404;
	static const double p19 = - .2609082745402891409913617;
	static const double p21 = + .4154485118940996442799104;
	static const double p23 = - .3718481677216955169129325;
	static const double p25 = + .1791132167840254903934055;

	// Square x.
	double x2 = x * x;

	double poly = ((((((((((((
		+ p25) * x2
		+ p23) * x2
		+ p21) * x2
		+ p19) * x2
		+ p17) * x2
		+ p15) * x2
		+ p13) * x2
		+ p11) * x2
		+ p09) * x2
		+ p07) * x2
		+ p05) * x2
		+ p03) * x2 * x;

	#if UseLongDouble
		static const long double
			HalfPi = 0x3.243f6a8885a308d313198a2e03707344ap-1L;
		return HalfPi - (poly + (long double) x);
	#else	// #if UseLongDouble
		static const double2
			HalfPi = { 0x1.921fb54442d18p+0, 0x1.1a62633145c07p-54 };
		return Sub221RightSmaller(HalfPi, Add112RightSmaller(x, poly));
	#endif	// #if UseLongDouble
}


// Return arccosine(x) given +.6 < x, with the same properties as acos.
static double pTail(double x)
{
	if (1 <= x)
		return 1 == x
			// If x is 1, return zero.
			? 0
			// If x is outside the domain, generate invalid and return NaN.
			: Infinity - Infinity;

	static const double p01 = - .2145900291823555067724496;
	static const double p02 = + .8895931658903454714161991e-1;
	static const double p03 = - .5037781062999805015401690e-1;
	static const double p04 = + .3235271184788013959507217e-1;
	static const double p05 = - .2125492340970560944012545e-1;
	static const double p06 = + .1307107321829037349021838e-1;
	static const double p07 = - .6921689208385164161272068e-2;
	static const double p08 = + .2912114685670939037614086e-2;
	static const double p09 = - .8899459104279910976564839e-3;
	static const double p10 = + .1730883544880830573920551e-3;
	static const double p11 = - .1594866672026418356538789e-4;

	double t0 = (((((((((((
		+ p11) * x
		+ p10) * x
		+ p09) * x
		+ p08) * x
		+ p07) * x
		+ p06) * x
		+ p05) * x
		+ p04) * x
		+ p03) * x
		+ p02) * x
		+ p01) * x;

	#if UseLongDouble
		static const long double p00 = +1.5707956046853235350824205L;
		return sqrtl(1-x) * (t0 + p00);
	#else	// #if UseLongDouble
		static const double2
			p00 = { 0x1.921fa926d2f24p0, +0x1.b4a23d0ecbb40p-59 };
			/*	p00.d1 might not be needed.  However, omitting it brings the
				sampled error to .872 ULP.  We would need to prove this is okay.
			*/

		// Estimate square root to double precision.
		double e = 1 / sqrt(1-x);

		// Refine estimate using Newton-Raphson.
		double2 ex = Mul112(e, 1-x);
		double e2x = Mul121Special(e, ex);
		double2 t1 = Add212RightSmaller(ex, ex.d0 * -.5 * e2x);

		// Return sqrt(1-x) * (t0 + p00).
		return Mul221(t1, Add212RightSmaller(p00, t0));
	#endif	// #if UseLongDouble
}


// Return arccosine(x) given x < -.6, with the same properties as acos.
static double nTail(double x)
{
	if (x <= -1)
		return -1 == x
			// If x is -1, generate inexact and return pi rounded toward zero.
			? 0x3.243f6a8885a308d313198a2e03707344ap0 + Tiny
			// If x is outside the domain, generate invalid and return NaN.
			: Infinity - Infinity;

	static const double p00 = +1.5707956513160834076561054;
	static const double p01 = + .2145907003920708442108238;
	static const double p02 = + .8896369437915166409934895e-1;
	static const double p03 = + .5039488847935731213671556e-1;
	static const double p04 = + .3239698582040400391437898e-1;
	static const double p05 = + .2133501549935443220662813e-1;
	static const double p06 = + .1317423797769298396461497e-1;
	static const double p07 = + .7016307696008088925432394e-2;
	static const double p08 = + .2972670140131377611481662e-2;
	static const double p09 = + .9157019394367251664320071e-3;
	static const double p10 = + .1796407754831532447333023e-3;
	static const double p11 = + .1670402962434266380655447e-4;

	double poly = sqrt(1+x) * ((((((((((((
		+ p11) * x
		+ p10) * x
		+ p09) * x
		+ p08) * x
		+ p07) * x
		+ p06) * x
		+ p05) * x
		+ p04) * x
		+ p03) * x
		+ p02) * x
		+ p01) * x
		+ p00);

	#if UseLongDouble
		static const long double Pi = 0x3.243f6a8885a308d313198a2e03707344ap0L;
		return Pi - poly;
	#else	// #if UseLongDouble
		static const double2
			Pi = { 0x1.921fb54442d18p+1, 0x1.1a62633145c07p-53 };
		return Sub211RightSmaller(Pi, poly);
	#endif	// #if UseLongDouble
}


// See documentation above.
double acos(double x)
{
	if (x < -.4)
		if (x < -.6)
			return nTail(x);
		else
			return Gap(x);
	else if (x <= +.4)
		return Center(x);
	else
		if (x <= +.6)
			return Gap(x);
		else
			return pTail(x);
}
