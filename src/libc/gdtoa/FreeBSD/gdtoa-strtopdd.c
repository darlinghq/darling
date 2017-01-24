/****************************************************************

The author of this software is David M. Gay.

Copyright (C) 1998, 2000 by Lucent Technologies
All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name of Lucent or any of its entities
not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.

****************************************************************/

/* Please send bug reports to David M. Gay (dmg at acm dot org,
 * with " at " changed at "@" and " dot " changed to ".").	*/

#include "xlocale_private.h"

#include "gdtoaimp.h"

#ifdef __APPLE__
/*
 * IEEE specifies that the most significant (head) double is required to
 * be equal to the long double rounded to the nearest double, so that means
 * the tail double might be the opposite sign as the head.  We can do this
 * adding (long double)0 to the number, which will fix it up.
 */
#define	fixLDBL(x)	((x) += 0.L)
#endif /* __APPLE__ */

 int
#ifdef KR_headers
strtopdd(s, sp, dd, loc) CONST char *s; char **sp; double *dd; locale_t loc;
#else
strtopdd(CONST char *s, char **sp, double *dd, locale_t loc)
#endif
{
#ifdef Sudden_Underflow
	static CONST FPI fpi0 = { 106, 1-1023, 2046-1023-106+1, 1, 1 };
#else
	static CONST FPI fpi0 = { 106, 1-1023-53+1, 2046-1023-106+1, 1, 0 };
#endif
	ULong bits[4];
	Long exp;
	int i, j, rv;
	typedef union {
		double d[2];
		ULong L[4];
#ifdef __APPLE__
		long double ld;
#endif /* __APPLE__ */
		} U;
	U *u;
#ifdef Honor_FLT_ROUNDS
#include "gdtoa_fltrnds.h"
#else
#define fpi &fpi0
#endif

	rv = strtodg(s, sp, fpi, &exp, bits, loc);
	u = (U*)dd;
	switch(rv & STRTOG_Retmask) {
	  case STRTOG_NoNumber:
		u->d[0] = u->d[1] = 0.;
		return rv; // avoid setting sign

	  case STRTOG_Zero:
		u->d[0] = u->d[1] = 0.;
		break;

	  case STRTOG_Normal:
		u->L[_1] = (bits[1] >> 21 | bits[2] << 11) & 0xffffffffL;
		u->L[_0] = (bits[2] >> 21) | ((bits[3] << 11) & 0xfffff)
			  | ((exp + 0x3ff + 105) << 20);
		exp += 0x3ff + 52;
		if (bits[1] &= 0x1fffff) {
			i = hi0bits(bits[1]) - 11;
			if (i >= exp) {
				i = exp - 1;
				exp = 0;
				}
			else
				exp -= i;
			if (i > 0) {
				bits[1] = bits[1] << i | bits[0] >> (32-i);
				bits[0] = bits[0] << i & 0xffffffffL;
				}
			}
		else if (bits[0]) {
			i = hi0bits(bits[0]) + 21;
			if (i >= exp) {
				i = exp - 1;
				exp = 0;
				}
			else
				exp -= i;
			if (i < 32) {
				bits[1] = bits[0] >> (32 - i);
				bits[0] = bits[0] << i & 0xffffffffL;
				}
			else {
				bits[1] = bits[0] << (i - 32);
				bits[0] = 0;
				}
			}
		else {
			u->L[2] = u->L[3] = 0;
			break;
			}
		u->L[2+_1] = bits[0];
		u->L[2+_0] = (bits[1] & 0xfffff) | (exp << 20);
#ifdef __APPLE__
		fixLDBL(u->ld);
#endif /* __APPLE__ */
		break;

	  case STRTOG_Denormal:
		if (bits[3])
			goto nearly_normal;
		if (bits[2])
			goto partly_normal;
		if (bits[1] & 0xffe00000)
			goto hardly_normal;
		/* completely denormal */
		u->L[2] = u->L[3] = 0;
		u->L[_1] = bits[0];
		u->L[_0] = bits[1];
		break;

	  nearly_normal:
		i = hi0bits(bits[3]) - 11;	/* i >= 12 */
		j = 32 - i;
		u->L[_0] = ((bits[3] << i | bits[2] >> j) & 0xfffff)
			| ((65 - i) << 20);
		u->L[_1] = (bits[2] << i | bits[1] >> j) & 0xffffffffL;
		u->L[2+_0] = bits[1] & ((1L << j) - 1);
		u->L[2+_1] = bits[0];
#ifdef __APPLE__
		fixLDBL(u->ld);
#endif /* __APPLE__ */
		break;

	  partly_normal:
		i = hi0bits(bits[2]) - 11;
		if (i < 0) {
			j = -i;
			i += 32;
			u->L[_0] = (bits[2] >> j & 0xfffff) | (33 + j) << 20;
			u->L[_1] = ((bits[2] << i) | (bits[1] >> j)) & 0xffffffffL;
			u->L[2+_0] = bits[1] & ((1L << j) - 1);
			u->L[2+_1] = bits[0];
#ifdef __APPLE__
			fixLDBL(u->ld);
#endif /* __APPLE__ */
			break;
			}
		if (i == 0) {
			u->L[_0] = (bits[2] & 0xfffff) | (33 << 20);
			u->L[_1] = bits[1];
			u->L[2+_0] = 0;
			u->L[2+_1] = bits[0];
#ifdef __APPLE__
			fixLDBL(u->ld);
#endif /* __APPLE__ */
			break;
			}
		j = 32 - i;
		u->L[_0] = (((bits[2] << i) | (bits[1] >> j)) & 0xfffff)
				| ((j + 1) << 20);
		u->L[_1] = (bits[1] << i | bits[0] >> j) & 0xffffffffL;
		u->L[2+_0] = 0;
		u->L[2+_1] = bits[0] & ((1L << j) - 1);
#ifdef __APPLE__
		fixLDBL(u->ld);
#endif /* __APPLE__ */
		break;

	  hardly_normal:
		j = 11 - hi0bits(bits[1]);
		i = 32 - j;
		u->L[_0] = (bits[1] >> j & 0xfffff) | ((j + 1) << 20);
		u->L[_1] = (bits[1] << i | bits[0] >> j) & 0xffffffffL;
		u->L[2+_0] = 0;
		u->L[2+_1] = bits[0] & ((1L << j) - 1);
#ifdef __APPLE__
		fixLDBL(u->ld);
#endif /* __APPLE__ */
		break;

	  case STRTOG_Infinite:
#ifdef __APPLE__
		u->L[_0] = 0x7ff00000;
		u->L[_1] = u->L[2+_0] = u->L[2+_1] = 0;
#else /* __APPLE__ */
		u->L[_0] = u->L[2+_0] = 0x7ff00000;
		u->L[_1] = u->L[2+_1] = 0;
#endif /* __APPLE__ */
		break;

	  case STRTOG_NaN:
#ifdef __APPLE__
		u->L[0] = d_QNAN0;
		u->L[1] = d_QNAN1;
		u->L[2] = u->L[3] = 0;
#else /* __APPLE__ */
		u->L[0] = u->L[2] = d_QNAN0;
		u->L[1] = u->L[3] = d_QNAN1;
#endif /* __APPLE__ */
		break;
#ifdef __APPLE__
	 case STRTOG_NaNbits:
		u->L[0] = d_QNAN0 | ((bits[2] >> 20 | bits[3] << 12) & 0xfffff);
		u->L[1] = d_QNAN1 | bits[1] >> 20 | bits[2] << 12;
		u->L[2] = u->L[3] = 0;
#endif /* __APPLE__ */
	  }
	if (rv & STRTOG_Neg) {
		u->L[  _0] |= 0x80000000L;
#ifdef __APPLE__
		u->L[2+_0] ^= 0x80000000L;
#else /* __APPLE__ */
		u->L[2+_0] |= 0x80000000L;
#endif /* __APPLE__ */
		}
	return rv;
	}
