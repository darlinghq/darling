#include "math.h"
#include <cmath>

long int __fpclassifyd(double arg)
{
	return std::fpclassify(arg);
}

long int __isnormald(double x)
{
	return std::isnormal(x);
}

long int __isfinited(double x)
{
	return std::isfinite(x);
}

long int __isnand(double x)
{
	return std::isnan(x);
}

long int __signbitd(double arg)
{
	return std::signbit(arg);
}

