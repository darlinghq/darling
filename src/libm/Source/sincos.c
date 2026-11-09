#include "math.h"

struct __float2 __sincosf_stret(float v)
{
	struct __float2 rv = {
		sinf(v), cosf(v)
	};
	return rv;
}

struct __double2 __sincos_stret(double v)
{
	struct __double2 rv = {
		sin(v), cos(v)
	};
	return rv;
}

// sin(pi*v) and cos(pi*v) for a finite v that is not an integer or a
// half-integer. sinpi and cospi have period 2, so v is first reduced exactly to
// a in [0, 1] (remainder() is exact), then folded to [0, 1/4] with the identities
// sinpi(1-a) = sinpi(a), cospi(1-a) = -cospi(a) and sinpi(1/2-b) = cospi(b).
// Each subtraction is exact, so the only rounding left is in pi*b and in sin/cos
// of a small argument, which keeps the relative error small even next to the
// zeros of sinpi and cospi.
static struct __double2 sincospi_reduced(double v)
{
	double r = remainder(v, 2.0);
	double a = fabs(r);
	double sign_sin = (r < 0) ? -1.0 : 1.0;
	double sign_cos = 1.0;
	double s, c;

	if (a > 0.5) {
		a = 1.0 - a;
		sign_cos = -1.0;
	}
	if (a <= 0.25) {
		s = sin(a * M_PI);
		c = cos(a * M_PI);
	} else {
		double b = 0.5 - a;
		s = cos(b * M_PI);
		c = sin(b * M_PI);
	}

	struct __double2 rv = { sign_sin * s, sign_cos * c };
	return rv;
}

struct __float2 __sincospif_stret(float v)
{
	if (isnan(v) || isinf(v)) {
		// NaN for both; v - v also raises FE_INVALID for an infinity
		struct __float2 rv = { v - v, v - v };
		return rv;
	}
	// For |v| >= 2^24, all representable floats are even integers
	if (fabsf(v) >= 16777216.0f) {
		struct __float2 rv = { copysignf(0.0f, v), 1.0f };
		return rv;
	}
	float intpart;
	float frac = modff(v, &intpart);
	if (frac == 0.0f) {
		long long n = (long long)fmodf(intpart, 2.0f);
		struct __float2 rv = {
			copysignf(0.0f, v),
			(n == 0) ? 1.0f : -1.0f
		};
		return rv;
	}
	if (fabsf(frac) == 0.5f) {
		long long n = (long long)fmodf(intpart * 2.0f + (frac > 0 ? 1.0f : -1.0f), 4.0f);
		if (n < 0) n += 4;
		struct __float2 rv = {
			(n == 1) ? 1.0f : -1.0f,
			0.0f
		};
		return rv;
	}
	// Computed in double precision and rounded once
	struct __double2 d = sincospi_reduced(v);
	struct __float2 rv = { (float)d.__sinval, (float)d.__cosval };
	return rv;
}

struct __double2 __sincospi_stret(double v)
{
	if (isnan(v) || isinf(v)) {
		// NaN for both; v - v also raises FE_INVALID for an infinity
		struct __double2 rv = { v - v, v - v };
		return rv;
	}
	// For |v| >= 2^53, all representable doubles are even integers
	if (fabs(v) >= 9007199254740992.0) {
		struct __double2 rv = { copysign(0.0, v), 1.0 };
		return rv;
	}
	double intpart;
	double frac = modf(v, &intpart);
	if (frac == 0.0) {
		long long n = (long long)fmod(intpart, 2.0);
		struct __double2 rv = {
			copysign(0.0, v),
			(n == 0) ? 1.0 : -1.0
		};
		return rv;
	}
	if (fabs(frac) == 0.5) {
		long long n = (long long)fmod(intpart * 2.0 + (frac > 0 ? 1.0 : -1.0), 4.0);
		if (n < 0) n += 4;
		struct __double2 rv = {
			(n == 1) ? 1.0 : -1.0,
			0.0
		};
		return rv;
	}
	return sincospi_reduced(v);
}
