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
	return __sincosf_stret(v * M_PI);
}

struct __double2 __sincospi_stret(double v)
{
	return __sincos_stret(v * M_PI);
}
