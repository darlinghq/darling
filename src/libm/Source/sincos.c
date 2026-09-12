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

struct __float2 __sincospif_stret(float v)
{
	if (isnan(v) || isinf(v)) {
		struct __float2 rv = { (float)NAN, (float)NAN };
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
	return __sincosf_stret((float)(v * M_PI));
}

struct __double2 __sincospi_stret(double v)
{
	if (isnan(v) || isinf(v)) {
		struct __double2 rv = { NAN, NAN };
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
	return __sincos_stret(v * M_PI);
}
