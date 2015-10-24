// float sqrtf(float)
//
// Portable implementation of sqrtf( ) with correct IEEE-754 default rounding.
//
// Assumes that integer and float have the same endianness on the target
// platform.
//
// Stephen Canon, July 2010

#include <math.h>

#if defined __SOFTFP__
#include <stdint.h>
#include <limits.h>

typedef float fp_t;
typedef uint32_t rep_t;
static const int significandBits = 23;
#define REP_C UINT32_C

static inline int rep_clz(rep_t a) {
    return __builtin_clz(a);
}

static inline rep_t toRep(fp_t x) {
    const union { rep_t i; fp_t f; } rep = { .f = x };
    return rep.i;
}

static inline fp_t fromRep(rep_t x) {
    const union { rep_t i; fp_t f; } rep = { .i = x };
    return rep.f;
}

static inline uint32_t mulhi(uint32_t a, uint32_t b) {
    return (uint64_t)a*b >> 32;
}

fp_t sqrtf(fp_t x) {
    
    // Various constants parametrized by the type of x:
    static const int typeWidth = sizeof(rep_t) * CHAR_BIT;
    static const int exponentBits = typeWidth - significandBits - 1;
    static const int exponentBias = (1 << (exponentBits - 1)) - 1;
    static const rep_t minNormal = REP_C(1) << significandBits;
    static const rep_t significandMask = minNormal - 1;
    static const rep_t signBit = REP_C(1) << (typeWidth - 1);
    static const rep_t absMask = signBit - 1;
    static const rep_t infRep = absMask ^ significandMask;
    static const rep_t qnan = infRep | REP_C(1) << (significandBits - 1);
    
    // Extract the various important bits of x
    const rep_t xRep = toRep(x);
    rep_t significand = xRep & significandMask;
    int exponent = (xRep >> significandBits) - exponentBias;
    
    // Using an unsigned integer compare, we can detect all of the special
    // cases with a single branch: zero, denormal, negative, infinity, or NaN.
    if (xRep - minNormal >= infRep - minNormal) {
        const rep_t xAbs = xRep & absMask;
        // sqrt(+/- 0) = +/- 0
        if (xAbs == 0) return x;
        // sqrt(NaN) = qNaN
        if (xAbs > infRep) return fromRep(qnan | xRep);
        // sqrt(negative) = qNaN
        if (xRep > signBit) return fromRep(qnan);
        // sqrt(infinity) = infinity
        if (xRep == infRep) return x;
        
        // normalize denormals and fall back into the mainline
        const int shift = rep_clz(significand) - rep_clz(minNormal);
        significand <<= shift;
        exponent += 1 - shift;
    }
    
    // Insert the implicit bit of the significand.  If x was denormal, then
    // this bit was already set by the normalization process, but it won't hurt
    // to set it twice.
    significand |= minNormal;
    
    // Halve the exponent to get the exponent of the result, and transform the
    // significand into a Q30 fixed-point xQ30 in the range [1,4) -- if the
    // exponent of x is odd, then xQ30 is in [2,4); if it is even, then xQ30
    // is in [1,2).
    const int resultExponent = exponent >> 1;
    uint32_t xQ30 = significand << (7 + (exponent & 1));
    
    // Q32 linear approximation to the reciprocal square root of xQ30.  This
    // approximation is good to a bit more than 3.5 bits:
    //
    //     1/sqrt(a) ~ 1.1033542890963095 - a/6
    const uint32_t oneSixthQ34 = UINT32_C(0xaaaaaaaa);
    uint32_t recipQ32 = UINT32_C(0x1a756d3b) - mulhi(oneSixthQ34, xQ30);
    
    // Newton-Raphson iterations to improve our reciprocal:
    const uint32_t threeQ30 = UINT32_C(0xc0000000);
    uint32_t residualQ30 = mulhi(xQ30, mulhi(recipQ32, recipQ32));
    recipQ32 = mulhi(recipQ32, threeQ30 - residualQ30) << 1;
    residualQ30 = mulhi(xQ30, mulhi(recipQ32, recipQ32));
    recipQ32 = mulhi(recipQ32, threeQ30 - residualQ30) << 1;
    residualQ30 = mulhi(xQ30, mulhi(recipQ32, recipQ32));
    recipQ32 = mulhi(recipQ32, threeQ30 - residualQ30) << 1;
    residualQ30 = mulhi(xQ30, mulhi(recipQ32, recipQ32));
    recipQ32 = mulhi(recipQ32, threeQ30 - residualQ30) << 1;
    
    // recipQ32 now holds an approximate 1/sqrt(x).  Multiply by x to get an
    // initial sqrt(x) in Q23.  From the construction of this estimate, we know
    // that it is either the correctly rounded significand of the result or one
    // less than the correctly rounded significand (the -2 guarantees that we
    // fall on the correct side of the actual square root).
    rep_t result = (mulhi(recipQ32, xQ30) - 2) >> 7;
    
    // Compute the residual x - result*result to decide if the result needs to
    // be rounded up.
    rep_t residual = (xQ30 << 16) - result*result;
    result += residual > result;
    
    // Clear the implicit bit of result:
    result &= significandMask;
    // Insert the exponent:
    result |= (rep_t)(resultExponent + exponentBias) << significandBits;
    return fromRep(result);    
}

#else // __SOFTFP__

float sqrtf(float x) {
	return __builtin_sqrtf(x);
}

#endif // __SOFTFP__
