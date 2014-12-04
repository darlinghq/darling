
/*
 *	f2i.h
 *
 *		by Ian Ollmann
 *
 *	C utility routines for rounding float to integer, with round to nearest like behavior.
 *	We do not specify here which way ties round.  Functions are rounding mode independent, except that
 *	the value +-0x1.fffffep-2f may round away from zero depending on rounding mode.
 *	
 *	The C entry points f2i and d2i assume the following:
 *
 *		All values are in the range: |x| < 0x1.0p31
 *		NaN does not occur
 *		x is not large. See comments under Flags_Behavior for more.
 */

#ifndef F2I_H
#define F2I_H

#include <stdint.h>

#ifndef __arm__ 
	#error	This is a __arm__ specific header
#endif

#ifndef FLAGS_BEHAVIOR
#define FLAGS_BEHAVIOR
typedef enum
{
	kCorrectFlags = -1,					// sets inexact when and only when it should.  Returns the wrong answer if |x| >= 0x1.0p31.
	kLazyFlags =     0,					// I don't care whether inexact is set or not. Returns the wrong answer if |x| >= 0x1.0p23.
	kNoFlags =		 1					// never sets flags.                           Returns the wrong answer if |x| >= 0x1.0p31.
}Flags_Behavior;
#endif

static inline int32_t	f2i_cheesy( float x, Flags_Behavior behavior ) __attribute__ ((always_inline));
static inline int32_t	f2i_cheesy( float x, Flags_Behavior behavior )
{
	float rounded;
	int32_t	result;
	union{ float f; uint32_t u; }u;

	// switch should be optimized away
	switch( behavior )
	{
		case	kCorrectFlags:

			result = (int32_t) x;
			rounded = (float) result;
			if( x != rounded )
			{
#if defined( AVOID_PIC )				
				u.f = x;
				u.u = (u.u & 0x80000000U) | 0x3f000000U;		// copysign( 0.5, x )
				result = (int32_t) (x + u.f);
#else
				{
					static const float	half[2] = { 0.5f, -0.5f };
					result = (int32_t)( x + half[ x < 0.0f ] );
				}
#endif
			}
			return result;

		case	kLazyFlags:
#if defined( AVOID_PIC )				
			u.f = x;
			u.u = (u.u & 0x80000000U) | 0x3f000000U;		// copysign( 0.5, x )
			result = (int32_t) (x + u.f);
#else
			{
				static const float	half[2] = { 0.5f, -0.5f };
				result = (int32_t)( x + half[ x < 0.0f ] );
			}
#endif
			return result;

		case	kNoFlags:
			{
				u.f = x;
				uint32_t fabsx = u.u & 0x7fffffffU;
				int32_t exp = fabsx >> 23;
				result = 0;

				if( exp >= 126U )
				{ // a lookup table can be used to make this part faster, see powf implementation.
					int32_t leftShift = exp - (127+23);
					int32_t rightShift = -leftShift;
					int32_t mask = leftShift >> 31;
					int32_t signX = u.u ^ fabsx;
					leftShift &= ~mask;
					rightShift &= mask;
					signX >>= 31;
	
					// convert float to fixed point
					result = ( u.u & 0x007fffffU) | 0x00800000U;

					// round
					result += 0x00800000U >> (exp - 126);

					// correct position of fixed point
					result >>= rightShift;
					result <<= leftShift;
					
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

static inline long lrintf_private( float x ) __attribute__ ((always_inline));
static inline long lrintf_private( float x )
{
	register float temp;
	register long  result;

	__asm__ __volatile__ ( "ftosis %0, %1" : "=w" (temp) : "w" (x) );
	__asm__ __volatile__ ( "fmrs %0, %1" : "=r" (result) : "w" (temp) );

	return result;
}

#endif	/* F2I_H */
