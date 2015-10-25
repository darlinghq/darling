// float fmaf(float a, float b, float c)
//
// The fmaf function returns a*b + c computed without intermediate rounding, and
// sets the flags appropriately.  IEEE-754 leaves undefined whether or not an
// implementation should set the invalid flag for fmaf(0,inf,NaN).  We choose
// not to do so in this implementation.
//
// This implementation is intended to be fully IEEE-754 compliant.  It is
// designed to produce the correctly rounded result in all rounding modes,
// and sets all flags appropriately.  It is intended to be relatively easily
// ported across architectures and compilers; the one issue to be aware of
// is the use of a union for type-punning double and uint64_t.  Some compilers
// will produce incorrect codegen for this due to overambitious type-based
// aliasing optimizations.  Make sure to check the generated code with your
// compiler.
//
// Stephen Canon, January 2010

#include <stdint.h>
#include "math.h"

float fmaf(float a, float b, float c) {
    
    // Detect if c is NaN to prevent fmaf(0, inf, NaN) from setting invalid.
    // If a or b is NaN, falling through the default path generates the correct
    // qNaN result.
    
    if (isnan(c)) return c + c;
    
    // Naïve computation of a*b + c in double.  Rounding this back to single
    // gives the correct result in all but one-in-a-billion cases.
    
    double product = (double)a*b;
    double resultRoundedToDouble = product + c;
    
    // Interlude -- possible incorrect double-rounding scenarios when an
    // infinitely precise result is rounded first to double, then to single:
    //
    // In round to nearest:
    // --------------------
    //                        bits 0 - 23   bits 24 - 52   bits 53 -
    //                result     .....0      1000...00        ...1...
    //        (double)result     .....0      1000...00
    // (float)(double)result     .....0
    //         (float)result     .....1
    //
    //                        bits 0 - 23   bits 24 - 52   bits 53 -
    //                result     .....1      0111...11        1...1..
    //        (double)result     .....1      1000...00
    // (float)(double)result     .....0
    //         (float)result     .....1
    //
    // In the directed rounding modes, double rounding does not occur.
    //
    // In order to protect against double rounding, we check if
    // resultRoundedToDouble is an exact halfway value for rounding to single.
    // If it isn't, then rounding a second time (to single) gives the correct
    // result.
    
    union {double d; uint64_t x;} bitPattern = {.d = resultRoundedToDouble};
    uint32_t roundingBits = (uint32_t)bitPattern.x & UINT32_C(0x1fffffff);
    
    if (roundingBits != UINT32_C(0x10000000))
        return (float)resultRoundedToDouble;
    
    // Here we know that we were in an exact halfway case, so we need to find
    // the low part of the sum and massage its information into the low part
    // of the double precision result.
    //
    // We calculate the low part of the sum as follows: let "big" be the larger
    // (in magnitude) of "product" and "c", and let "small" be the smaller.
    // We already know resultRoundedToDouble, big, and small in the following
    // equation:
    //
    //     resultRoundedToDouble + lowPart = big + small
    //     
    // so we can compute "lowPart" as:
    //
    //      lowPart = (big - resultRoundedToDouble) + small
    //
    // This cannot generate spurious NaN/invalid because if resultRoundedToDouble
    // were inf, we wouldn't have matched as a halfway case, and wouldn't be on
    // this codepath.
    //
    // Note also that "lowPart" will not be exact in some circumstances in
    // directed rounding modes.  This is OK, because we are only going to use
    // it to twiddle the low bit of an exact-halfway case for round-to-nearest;
    // if we are in a directed rounding mode, this does not effect the final
    // result.  It may be an inexact operation, but the inexact flag is already
    // raised at this point if that is the case.
    
    double lowPart;
    if (__builtin_fabs(product) >= __builtin_fabs(c))
        lowPart = (product - resultRoundedToDouble) + c;
    else
        lowPart = (c - resultRoundedToDouble) + product;
    
    // If the lowPart is non-zero, and has the same sign as the result,
    // add one to the bit representation of the double-precision result
    // to act as sticky. This cannot spuriously raise inexact, because
    // inexact is already (correctly) raised if lowPart is non-zero.  The
    // product cannot over/underflow because the inputs originally came from
    // single precision, so the exponents are well controlled.
    
    if (lowPart*resultRoundedToDouble > 0.0) {
        bitPattern.x += 1;
        resultRoundedToDouble = bitPattern.d;
    }
    
    // If instead the signs are opposite, subtract one.
    
    if (lowPart*resultRoundedToDouble < 0.0) {
        bitPattern.x -= 1;
        resultRoundedToDouble = bitPattern.d;
    }
    
    // If the low part is exactly zero, then this really is an exact halfway
    // case for rounding to single, and no action is necessary.
    
    // Now that we have massaged in a sticky bit for the low part, we can round
    // to single-precision and get the correct final result.
    
    return (float)resultRoundedToDouble;
}
