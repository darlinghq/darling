/*	This is an implementation of asin.  It is written in standard C except:
   
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
	to generate exceptions such as invalid or inexact.
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


	/*	Return a + b with relative error below 2**-103 and then rounded to
		double given |b| < |a|.
	*/
	static inline double Add221RightSmaller(double2 a, double2 b)
	{
		double
			c0 = a.d0 + b.d0,
			c1 = a.d0 - c0 + b.d0 + b.d1 + a.d1,
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


	// Return a * b with relative error below 2**-102 given |a|, |b| < 2**970.
	static inline double2 Mul222(double2 a, double2 b)
	{
		static const double c = 0x1p27 + 1;

		double
			ap = a.d0 * c,     bp = b.d0 * c,
			a0 = (a.d0-ap)+ap, b0 = (b.d0-bp)+bp,
			a1 = a.d0 - a0,    b1 = b.d0 - b0,
			m0 = a.d0 * b.d0,
			m1 = a0*b0 - m0 + a0*b1 + a1*b0 + a1*b1 + (a.d0*b.d1 + a.d1*b.d0),
			d0 = m0 + m1,
			d1 = m0 - d0 + m1;
		return (double2) { d0, d1 };
	}


#endif	// !UseLongDouble


/*	double asin(double x).

	(This routine appears below, following the Tail subroutine.)

	Notes:

		Citations in parentheses below indicate the source of a requirement.

		"C" stands for ISO/IEC 9899:TC2.

		The Open Group specification (IEEE Std 1003.1, 2004 edition) adds no
		requirements since it defers to C and requires errno behavior only if
		we choose to support it by arranging for "math_errhandling &
		MATH_ERRNO" to be non-zero, which we do not.

	Return value:

		For arcsine of +/- zero, return zero with same sign (C F.9 12 and
		F.9.1.2).

		For 1 < |x| (including infinity), return NaN (C F.9.1.2).

		For a NaN, return the same NaN (C F.9 11 and 13).  (If the NaN is a
		signalling NaN, we return the "same" NaN quieted.)

		Otherwise:

			If the rounding mode is round-to-nearest, return arcsine(x)
			faithfully rounded.  This is not proven but seems likely.
			Generally, the largest source of errors is the evaluation of the
			polynomial using double precision.  Some analysis might bound this
			and prove faithful rounding.  The largest observed error is .678
			ULP.

			Return a value in [-pi/2, +pi/2] (C 7.12.4.2 3).
		
			Not implemented:  In other rounding modes, return arcsine(x)
			possibly with slightly worse error, not necessarily honoring
			the rounding mode (Ali Sazegari narrowing C F.9 10).

	Exceptions:

		Raise underflow for a denormal result (C F.9 7 and Draft Standard for
		Floating-Point Arithmetic P754 Draft 1.2.5 9.5).  If the input is the
		smallest normal, underflow may or may not be raised.  This is stricter
		than the older 754 standard.

		May or may not raise inexact, even if the result is exact (C F.9 8).

		Raise invalid if the input is a signalling NaN (C 5.2.4.2.2 3, in spite
		of C 4.2.1)  or 1 < |x| (including infinity) (C F.9.1.2) but not if the
		input is a quiet NaN (C F.9 11).

		May not raise exceptions otherwise (C F.9 9).

	Properties:

		Not proven:  Monotonic.
*/


// Return arcsine(x) given |x| <= .5, with the same properties as asin.
static double Center(double x)
{
	if (-0x1.7137449123ef5p-26 <= x && x <= +0x1.7137449123ef5p-26)
		return -0x1p-1022 < x && x < +0x1p-1022
			// Generate underflow and inexact and return x.
			?  x - x*x
			// Generate inexact and return x.
			: x * (Tiny + 1);

	static const double p03 =  0.1666666666666558995379880;
	static const double p05 =  0.0750000000029696112392353;
	static const double p07 =  0.0446428568582815922683933;
	static const double p09 =  0.0303819580081956423799529;
	static const double p11 =  0.0223717830666671020710108;
	static const double p13 =  0.0173593516996479249428647;
	static const double p15 =  0.0138885410156894774969889;
	static const double p17 =  0.0121483892822292648695383;
	static const double p19 =  0.0066153165197009078340075;
	static const double p21 =  0.0192942786775238654913582;
	static const double p23 = -0.0158620440988475212803145;
	static const double p25 =  0.0316658385792867081040808;

	// Square x.
	double x2 = x * x;

	return ((((((((((((
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
		+ p03) * x2 * x + x;
}


// Return arcsine(x) given .5 < x, with the same properties as asin.
static double Tail(double x)
{
	if (1 <= x)
		return 1 == x
			// If x is 1, generate inexact and return Pi/2 rounded down.
			?  0x3.243f6a8885a308d313198a2e03707344ap-1 + Tiny
			// If x is outside the domain, generate invalid and return NaN.
			: Infinity - Infinity;

	static const double p01 =  0.2145993335526539017488949;
	static const double p02 = -0.0890259194305537131666744;
	static const double p03 =  0.0506659694457588602631748;
	static const double p04 = -0.0331919619444009606270380;
	static const double p05 =  0.0229883479552557203133368;
	static const double p06 = -0.0156746038587246716524035;
	static const double p07 =  0.0097868293573384001221447;
	static const double p08 = -0.0052049731575223952626203;
	static const double p09 =  0.0021912255981979442677477;
	static const double p10 = -0.0006702485124770180942917;
	static const double p11 =  0.0001307564187657962919394;
	static const double p12 = -0.0000121189820098929624806;

	double polynomial = ((((((((((((
		+ p12) * x
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
		static const long double HalfPi = 0x3.243f6a8885a308d313198a2ep-1L;
		static const long double p00 = -1.5707961988153774692344105L;

		return HalfPi + sqrtl(1-x) * (polynomial + p00);
	#else	// #if UseLongDouble
		static const double2
			HalfPi = { 0x1.921fb54442d18p+0, 0x1.1a62633145c07p-54 },
			p00 = { -0x1.921fb31e97d96p0, +0x1.eab77149ad27cp-54 };

		// Estimate 1 / sqrt(1-x).
		double e = 1 / sqrt(1-x);

		double2 ex = Mul112(e, 1-x);		// e * x.
		double e2x = Mul121Special(e, ex);	// e**2 * x - 1.

		// Set t0 to an improved approximation of sqrt(1-x) with Newton-Raphson.
		double2 t0 = Add212RightSmaller(ex, ex.d0 * -.5 * e2x);

		// Calculate pi/2 + sqrt(1-x) * p(x).
		return Add221RightSmaller(HalfPi, Mul222(
			t0,
			Add212RightSmaller(p00, polynomial)));
	#endif	// #if UseLongDouble
}


// See documentation above.
double asin(double x)
{
	if (x < -.5)
		return -Tail(-x);
	else if (x <= .5)
		return Center(x);
	else
		return +Tail(+x);
}
