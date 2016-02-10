/*
 *  xmm_fma.c
 *  xmmLibm
 *
 *  Created by Ian Ollmann on 8/8/05.
 *  Copyright 2005 Apple Computer Inc. All rights reserved.
 *
 */


#include "xmmLibm_prefix.h"

#include <math.h>

//For any rounding mode, we can calculate A + B exactly as a head to tail result as:
//
//  Rhi = A + B     (A has a larger magnitude than B)
//  Rlo = B - (Rhi - A);
//  
//  Rhi is rounded to the current rounding mode
//  Rlo represents the next 53+ bits of precision

//returns carry bits that don't fit into A
static inline long double extended_accum( long double *A, long double B ) ALWAYS_INLINE;
static inline long double extended_accum( long double *A, long double B ) 
{
        // assume B >~ *A, B needs to be larger or in the same binade.
    long double r = *A + B;
    long double carry = *A - ( r - B );
    *A = r;
    return carry;
}

// Set the LSB of a long double encoding. Turns +/- inf to nan, and +/- 0 to Dmin. 
static inline long double jam_stickyl(long double x) ALWAYS_INLINE;
static inline long double jam_stickyl(long double x) {
        union{
                long double ld;
                xUInt64 xu;
                struct{ uint64_t mantissa; int16_t sexp; } parts;
        } ux = {x}, uy, mask = {0.};
        mask.parts.mantissa = 0x1ULL;
        uy.xu = _mm_or_si128(ux.xu, mask.xu);
        return uy.ld;
}


// Compute next down unless value is already odd.
// This is useful for reflecting the contribution of "negative sticky". 
static inline long double jam_nstickyl(long double x) ALWAYS_INLINE;
static inline long double jam_nstickyl(long double x) {
        union{
                long double ld;
                xUInt64 xu;
                struct{ uint64_t mantissa; int16_t sexp; } parts;
        } ux = {x}, uy, mask = {0.}, exp_borrow = {0x1p-16382L};
        // exp_borrow.parts = {0x8000000000000000ULL, 0x0001}
        mask.parts.mantissa = 0x1ULL;
        const xUInt64 is_odd = _mm_and_si128(ux.xu, mask.xu);
        xUInt64 new_sig = _mm_sub_epi64(ux.xu, _mm_xor_si128(is_odd, mask.xu));

        //Deal with exact powers of 2.
        if(0x1 & _mm_movemask_pd((xDouble)new_sig)) {
                /* If the fraction was nonzero, i.e., the significand was 0x8000000000000000,
                   (recall that 80-bit long double has an explicit integer bit)
                   we could safely subtract one without needing to borrow from the exponent.
                 */
                ;
        } else {
                /* If we are here, we subtracted one from an all zero fraction
                   This means we needed to borrow one from the exponent. 
                   We also need to restore the explicit integer bit in the 80-bit format.
                */
                new_sig = _mm_sub_epi64(new_sig, exp_borrow.xu);
        }
        uy.xu = new_sig;

        return uy.ld;
}

double fma( double a, double b, double c )
{
	union{ double ld; uint64_t u;}ua = {a};
	union{ double ld; uint64_t u;}ub = {b};
	union{ double ld; uint64_t u;}uc = {c};

	int32_t sign = (ua.u ^ ub.u ^ uc.u)>>63; // 0 for true addition, 1 for true subtraction
    int32_t aexp = (ua.u>>52) & 0x7ff;
	int32_t bexp = (ub.u>>52) & 0x7ff;
	int32_t cexp = (uc.u>>52) & 0x7ff;
	int32_t pexp =  aexp + bexp - 0x3ff;
	
    if((aexp == 0x7ff)||(bexp == 0x7ff)||(cexp == 0x7ff)) {
        //deal with NaN
        if( a != a )    return a;
        if( b != b )    return b;
        if( c != c )    return c;
        //Have at least one inf and no NaNs at this point.
        //Need to deal with invalid cases for INF - INF.
        //Also, should avoid inadvertant overflow for big*big - INF.
        if(cexp < 0x7ff) { // If c is finite, then the product is the result.
                return a*b; // One of a or b is INF, so this will produce correct exceptions exactly
        }
        // c is infinite. 
        if((aexp < 0x7ff) && (0 < aexp)) {
                ua.u = (ua.u & 0x8000000000000000ULL) | 0x4000000000000000ULL; // copysign(2.0,a)
        }
        if((bexp < 0x7ff) && (0 < bexp)) {
                ub.u = (ub.u & 0x8000000000000000ULL) | 0x4000000000000000ULL; // copysign(2.0,b)
        }
        return (ua.ld*ub.ld)+c;
    }
    // Having ruled out INF and NaN, do a cheap test to see if any input might be zero
    if((aexp == 0x000)||(bexp == 0x000)||(cexp == 0x000)) {
            if(a==0.0 || b==0.0) {
                    // If at least one input is +-0 we can use native arithmetic to compute a*b+c.
                    // If a or b is zero, then we have a signed zero product that we add to c.
                    // Not that in this case, c may also be zero, but the addition will produce the correctly signed zero.
                    return ((a*b)+c);
            }
            if(c==0.0) {
                    // If c is zero and the infinitely precise product is non-zero, 
                    // we do not add c.
                    // The floating point multiply with produce the correct result including overflow and underflow.
                    // Note that if we tried to do (a*b)+c for this case relying on c being zero
                    // we would get the wrong sign for some underflow cases, e.g., tiny * -tiny + 0.0 should be -0.0.
                    return (a*b);
            }
    }

    static const xUInt64 mask26 = { 0xFFFFFFFFFC000000ULL, 0 };
    double ahi, bhi;

    //break a, b and c into high and low components
    //The high components have 26 bits of precision
    //The low components have 27 bits of precision
    xDouble xa = DOUBLE_2_XDOUBLE(a);
    xDouble xb = DOUBLE_2_XDOUBLE(b);

    xa = _mm_and_pd( xa, (xDouble) mask26 );
    xb = _mm_and_pd( xb, (xDouble) mask26 );
    
    ahi = XDOUBLE_2_DOUBLE( xa );
    bhi = XDOUBLE_2_DOUBLE( xb );

    //double precision doesn't have enough precision for the next part. 
    //so we abandond it and go to extended.
    ///
    //The problem is that the intermediate multiplication product needs to be infinitely
    //precise. While we can fix the mantissa part of the infinite precision problem,
    //we can't deal with the case where the product is outside the range of the
    //representable double precision values. Extended precision allows us to solve
    //that problem, since all double values and squares of double values are normalized
    //numbers in extended precision
    long double Ahi = ahi;
    long double Bhi = bhi;
    long double Alo = (long double) a - Ahi;
    long double Blo = (long double) b - Bhi;
    long double C = c;
    
    //The result of A * B is now exactly:
    //
    //  B1 + Ahi*Bhi + Alo*Bhi + Ahi*Blo + Alo*Blo 
    //  all of these intermediate terms have either 52 or 53 bits of precision and are exact
    long double AhiBhi = Ahi * Bhi;        //52 bits 
    long double AloBhi = Alo * Bhi;        //53 bits  
    long double AhiBlo = Ahi * Blo;        //53 bits  
    long double AloBlo = Alo * Blo;        //54 bits   
    
    //accumulate the results into two head/tail buffers. This is infinitely precise.
    //no effort is taken to make sure that a0 and a1 are actually head to tail
    long double a0, a1;
    a0 = AloBlo;
    a1 = extended_accum( &a0, (AhiBlo + AloBhi) );
    a1 += extended_accum( &a0, AhiBhi );

    //Add C. If C has greater magnitude than a0, we need to swap them
    if( __builtin_fabsl( C ) > __builtin_fabsl( a0 ) )
    {
        long double temp = C;
        C = a0;
        a0 = temp;
    } 

    if(cexp + 120 < pexp) {
            // Check to see if c < ulp(a*b).
            // If it is then c only contributes as a sticky bit. 

            /* Since we know that the product A*B fits in 106 bits
               and a0+a1 can accomodate 128 bits, 
               we just need to add something of the correct sign and
               less than ULP(A*B) but not too small.
               We construct this by multiplying a0 by +/- 2^-120.
            */ 
            long double ulp_prod_est = a0 * (sign? -0x1p-120L: 0x1p-120L);
            
            a1 += ulp_prod_est;
    } else {

            //this will probably overflow, but we have 128 bits of precision here, which should mean we are covered. 
            long double a2 = C;
            if(__builtin_fabsl(C) < __builtin_fabsl(a0)) {
                    a2 = a0;
                    a0 = C;
            }
            a1 += extended_accum( &a0, a2 );
    }

    //push overflow in a1 back into a0. This should give us the correctly rounded result
    a0 = extended_accum( &a1, a0 );
    if(a0 > 0.) a1 = (a1>0.)?jam_stickyl(a1):jam_nstickyl(a1);
    if(a0 < 0.) a1 = (a1<0.)?jam_stickyl(a1):jam_nstickyl(a1);
    return a1;
}

long double fmal( long double a, long double b, long double c )
{
	/*****************
	 Edge cases, from Ian's code.
	*****************/
	
	union{ long double ld; struct{ uint64_t mantissa; int16_t sexp; }parts; }ua = {a};
	union{ long double ld; struct{ uint64_t mantissa; int16_t sexp; }parts; }ub = {b};
	
	int16_t sign = (ua.parts.sexp ^ ub.parts.sexp) & 0x8000;
	int32_t aexp = (ua.parts.sexp & 0x7fff);
	int32_t bexp = (ub.parts.sexp & 0x7fff);
	int32_t exp  =  aexp + bexp - 16383;
	
	//deal with NaN
	if( a != a )    return a;
	if( b != b )    return b;
	if( c != c )    return c;
	
	// a = ° | b = °
	if ((aexp == 0x7fff) || (bexp == 0x7fff)) {				// We've already dealt with NaN, so this is only true 
																										// if one of a and b is an inf.
		if (( b == 0.0L ) || ( a == 0.0L)) return a*b;	// Return NaN if a = ±°, b = 0, c ­ NaN (or a = 0, b = °)
		if ( __builtin_fabsl(c) == __builtin_infl() ) {
			if ( sign & 0x8000 ) {
				if ( c > 0 )
					return c - __builtin_infl();							// Return NaN if a = ±°, c = -a
				else
					return c;																	// Return ±inf if a = c = ±°.
			} else {
				if ( c < 0 )
					return c + __builtin_infl();							// Return NaN if a = ±°, c = -a
				else
					return c;																	// Return ±inf if a = c = ±°.
			}
			if ( sign & 0x8000 ) return -__builtin_infl();
			else return __builtin_infl();
		}
	}
	
	// c = °
	if ( __builtin_fabsl(c) == __builtin_inf() ) return c; // a,b at this point are finite, c is infinite.

	
	if( exp < -16382 - 63 ) //sub denormal
		return c;
	
	/*****************
	 Computation of a*b + c. scanon, Jan 2007
	 
	 The whole game we're playing here only works in round-to-nearest.
	 *****************/
	
	long double ahi, alo, bhi, blo, phi, plo, xhi, xlo, yhi, ylo, tmp;
	
	//  split a,b into high and low parts.
	static const uint64_t split32_mask = 0xffffffff00000000ULL;
	ua.parts.mantissa = ua.parts.mantissa & split32_mask;
	ahi = ua.ld;								alo = a - ahi;
	ub.parts.mantissa = ub.parts.mantissa & split32_mask;
	bhi = ub.ld;								blo = b - bhi;
	
	// compute the product of a and b as a sum phi + plo.  This is exact.
	phi = a * b;
	
	// In case of overflow, stop here and return phi.  This will need to be changed
	// in order to have a fully C99 fmal.
	if (__builtin_fabsl(phi) == __builtin_infl()) {
		return phi; // We know that c != inf or nan, so phi is the correct result.
	}
	
	plo = (((ahi * bhi - phi) + ahi*blo) + alo*bhi) + alo*blo;
	
	// compute plo + c = xhi + xlo where (xhi,xlo) is head-tail.
	xhi = plo + c;
	if (__builtin_fabsl(plo) > __builtin_fabsl(c)) {
		tmp = xhi - plo;
		xlo = c - tmp;
	} else {
		tmp = xhi - c;
		xlo = plo - tmp;
	}
	
	// Special case: xlo == 0, hence return phi + xhi:
	if (xlo == 0.0L) return phi + xhi;
	
	// At this point we know that the meaningful bits of phi and xhi are entirely to the
	// left (larger) side of the meaningful bits of xlo, and that our result is 
	// round(phi + xhi + xlo).
	yhi = phi + xhi;
	if (__builtin_fabsl(phi) > __builtin_fabsl(xhi)) {
		tmp = yhi - phi;
		ylo = xhi - tmp;
	} else {
		tmp = yhi - xhi;
		ylo = phi - tmp;
	}
	
	// Handle the special case that one of yhi or ylo is zero.
	// If yhi == 0, then ylo is also zero, so yhi + xlo = xlo is the appropriate result.
	// If ylo == 0, then yhi + xlo is the appropriate result.
	if ((yhi == 0.0L) || (ylo == 0.0L)) return yhi + xlo;
	
	// Now we have that (in terms of meaningful bits)
	// yhi is strictly bigger than ylo is strictly bigger than xlo.
	// Additionally, our desired result is round(yhi + ylo + xlo).
	
	// The only way for xlo to affect rounding (in round-to-nearest) is for ylo to
	// be exactly half an ulp of yhi.  Test for the value of the mantissa of ylo;
	// this is not the same condition, but getting this wrong can't affect the rounding.
	union{ long double ld; struct{ uint64_t mantissa; int16_t sexp; }parts; } uylo = {ylo};
	if (uylo.parts.mantissa == 0x8000000000000000ULL) {
		return yhi + (ylo + copysignl(0.5L*ylo, xlo));
	}
	// In the other case, xlo has no affect on the final result, so just return yhi + ylo
	else {
		return yhi + ylo;
	}
	
	/*
	 Code that Ian wrote to do this in integer, never finished.
	 
	 //mantissa product
    //  hi(a.hi*b.hi)   lo(a.hi*b.hi)
    //                  hi(a.hi*b.lo)   lo(a.hi*b.lo)
    //                  hi(a.lo*b.hi)   lo(a.lo*b.hi)
    // +                                hi(a.lo*b.lo)   lo(a.lo*b.lo)
    // --------------------------------------------------------------

    uint32_t    ahi = ua.parts.mantissa >> 32;
    uint32_t    bhi = ub.parts.mantissa >> 32;
    uint32_t    alo = ua.parts.mantissa & 0xFFFFFFFFULL;
    uint32_t    blo = ub.parts.mantissa & 0xFFFFFFFFULL;

    uint64_t    templl, temphl, templh, temphh;
    xUInt64     l, h, a;
    
    templl = (uint64_t) alo * (uint64_t) blo;
    temphl = (uint64_t) ahi * (uint64_t) blo;
    templh = (uint64_t) alo * (uint64_t) bhi;
    temphh = (uint64_t) ahi * (uint64_t) bhi;

    l = _mm_cvtsi32_si128( (int32_t) templl );          templl >>= 32;

    temphl +=  templl;
    temphl +=  templh & 0xFFFFFFFFULL;
    h = _mm_cvtsi32_si128( (int32_t) temphl);           temphl >>= 32;
    a = _mm_unpacklo_epi32( l, h );
    

    temphh += templh >> 32;
    temphh += temphl;

    a = _mm_cvtsi32_si128( (int32_t) temphh );          temphh >>= 32;
    h = _mm_cvtsi32_si128( (int32_t) temphh);           
    a = _mm_unpacklo_epi32( a, h );
    l = _mm_unpacklo_epi64( l, a );
    
    union
    {
        xUInt64     v;
        uint64_t    u[2];
    }z = { l };

    long double lo = (long double) temphh.
	 */
}

