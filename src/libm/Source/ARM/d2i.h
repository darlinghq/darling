
/*
 *	d2i.h
 *
 *		by Ian Ollmann
 *
 *	C utility routines for rounding double to integer, with round to nearest like behavior.
 *	We do not specify here which way ties round.  Functions are rounding mode independent, except that
 *	the value +-0x1.fffffffffffffp-2 may round away from zero depending on rounding mode.
 *	
 *	The C entry points f2i and d2i assume the following:
 *
 *		All values are in the range: |x| <= 0x1.0p31 - 1
 *		NaN does not occur
 *		x is not large. See comments under Flags_Behavior for more.
 *
 *	For double -> int64 conversions, please see <rdar://problems/5316332&5316398>
 */

#ifndef D2I_H
#define D2I_H

#ifndef __arm__
	#error header valid for __arm__ only
#endif

#include <stdint.h>

#ifndef FLAGS_BEHAVIOR
#define FLAGS_BEHAVIOR
typedef enum
{
	kCorrectFlags = -1,					// sets inexact when and only when it should.  Returns the wrong answer if |x| >= 0x1.0p31.
	kLazyFlags =     0,					// I don't care whether inexact is set or not. Returns the wrong answer if |x| >= 0x1.0p31.
	kNoFlags =		 1					// never sets flags.                           Returns the wrong answer if |x| >= 0x1.0p31.
}Flags_Behavior;
#endif

static inline int32_t	d2i_cheesy( double x, Flags_Behavior behavior ) __attribute__ ((always_inline));
static inline int32_t	d2i_cheesy( double x, Flags_Behavior behavior )
{
	double rounded;
	int32_t	result;
	union{ double d; uint64_t u; }u;

	// switch should be optimized away
	switch( behavior )
	{
		case	kCorrectFlags:

			result = (int32_t) x;
			rounded = (float) result;
			if( x != rounded )
			{
#if defined( AVOID_PIC )				
				u.d = x;
				u.u = (u.u & 0x8000000000000000ULL) | 0x3fe0000000000000ULL;		// copysign( 0.5, x )
				result = (int32_t) (x + u.d);
#else
				{
					static const double	half[2] = { 0.5, -0.5 };
					result = (int32_t)( x + half[ x < 0.0 ] );
				}
#endif
			}
			return result;

		case	kLazyFlags:
#if defined( AVOID_PIC )				
			u.d = x;
			u.u = (u.u & 0x8000000000000000ULL) | 0x3fe0000000000000ULL;		// copysign( 0.5, x )
			result = (int32_t) (x + u.d);
#else
			{
				static const double	half[2] = { 0.5, -0.5 };
				result = (int32_t)( x + half[ x < 0.0 ] );
			}
#endif
			return result;

		case	kNoFlags:
			{
				u.d = x;
				uint64_t fabsx = u.u & 0x7fffffffffffffffULL;
				int32_t exp = (int32_t)(fabsx >> 52);
				result = 0;

				if( exp >= 1022U )
				{ // a lookup table can be used to make this part faster, see powf implementation.
					int32_t leftShift = exp - (1023+52);
					int32_t rightShift = -leftShift;
					int32_t mask = leftShift >> 31;
					int32_t signX = (int32_t)((u.u ^ fabsx) >> 32);
					leftShift &= ~mask;
					rightShift &= mask;
					signX >>= 31;
	
					// convert float to fixed point
					int64_t r = ( u.u & 0x000fffffffffffffULL) | 0x0010000000000000ULL;

					// round
					r += 0x0008000000000000ULL >> (exp - 1022);

					// correct position of fixed point
					r >>= rightShift;
					r <<= leftShift;

					result = (int32_t) r;
					
					//fix sign
					result ^= signX;
					result -= signX;
				}
				return result;
			}
			break;
	}
	
	return 0;
}

// Compliant implementation of C99 lrint
static inline long lrint_private( double x ) __attribute__ ((always_inline));
static inline long lrint_private( double x )
{
	register float temp;
	register long result;

	__asm__ __volatile__ ( "ftosid %0, %P1" : "=w" (temp) : "w" (x) );
	__asm__ __volatile__ ( "fmrs %0, %1" : "=r" (result) : "w" (temp) );

	return result;
}

#endif	/* D2I_H */
