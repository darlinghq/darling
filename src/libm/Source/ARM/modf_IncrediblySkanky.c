#if defined(__SOFTFP__)

#include <math.h>
#warning Temporary softfloat implementation of modf, performance is expected to be terrible.

double modf(double x, double *iptr) {
	*iptr = trunc(x);
	double tmp = isinf(x) ? 0.0 : x - *iptr;
	return __builtin_copysign(tmp, x);
}

#endif
