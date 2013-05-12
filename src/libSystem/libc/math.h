#ifndef LIBC_MATH_H
#define LIBC_MATH_H

extern "C" {

long int __fpclassifyd(double arg);
long int __isnormald(double x);
long int __isfinited(double x);
long int __isnand(double x);
long int __signbitd(double arg);

}

#endif

