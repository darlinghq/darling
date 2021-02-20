/****************************************************************

The author of this software is David M. Gay.

Copyright (C) 2000 by Lucent Technologies
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

#include "gdtoaimp.h"
#include <fpmath.h>

#ifndef __APPLE__
 static void
#ifdef KR_headers
L_shift(x, x1, i) ULong *x; ULong *x1; int i;
#else
L_shift(ULong *x, ULong *x1, int i)
#endif
{
	int j;

	i = 8 - i;
	i <<= 2;
	j = ULbits - i;
	do {
		*x |= x[1] << j;
		x[1] >>= i;
		} while(++x < x1);
	}
#endif /* !__APPLE__ */

 int
#ifdef KR_headers
hexnan(sp, fpi, x0)
	CONST char **sp; CONST FPI *fpi; ULong *x0;
#else
hexnan( CONST char **sp, CONST FPI *fpi, ULong *x0)
#endif
{
#ifdef __APPLE__
	int nbits, len;
	char *cp;
#else /* !__APPLE__ */
	ULong c, h, *x, *x1, *xe;
#endif /* __APPLE__ */
	CONST char *s;
#ifndef __APPLE__
	int havedig, hd0, i, nbits;
#endif /* !__APPLE__ */

#ifdef __APPLE__
	if (sp == NULL || *sp == NULL || **sp != '(')
		return STRTOG_NaN;
#else /* !__APPLE__ */
	if (!hexdig['0'])
		hexdig_init_D2A();
	nbits = fpi->nbits;
	x = x0 + (nbits >> kshift);
	if (nbits & kmask)
		x++;
	*--x = 0;
	x1 = xe = x;
	havedig = hd0 = i = 0;
#endif /* __APPLE__ */
	s = *sp;
#ifdef __APPLE__
	if ((cp = strchr(s + 1, ')')) == NULL) {
		return STRTOG_NaN;
		}
	else {
		len = cp - (s + 1);
		cp = alloca(len + 1);
		if (!cp)
#else /* !__APPLE__ */
	/* allow optional initial 0x or 0X */
	while((c = *(CONST unsigned char*)(s+1)) && c <= ' ')
		++s;
	if (s[1] == '0' && (s[2] == 'x' || s[2] == 'X')
	 && *(CONST unsigned char*)(s+3) > ' ')
		s += 2;
	while((c = *(CONST unsigned char*)++s)) {
		if (!(h = hexdig[c])) {
			if (c <= ' ') {
				if (hd0 < havedig) {
					if (x < x1 && i < 8)
						L_shift(x, x1, i);
					if (x <= x0) {
						i = 8;
						continue;
						}
					hd0 = havedig;
					*--x = 0;
					x1 = x;
					i = 0;
					}
				while(*(CONST unsigned char*)(s+1) <= ' ')
					++s;
				if (s[1] == '0' && (s[2] == 'x' || s[2] == 'X')
				 && *(CONST unsigned char*)(s+3) > ' ')
					s += 2;
				continue;
				}
			if (/*(*/ c == ')' && havedig) {
				*sp = s + 1;
				break;
				}
#ifndef GDTOA_NON_PEDANTIC_NANCHECK
			do {
				if (/*(*/ c == ')') {
					*sp = s + 1;
					break;
					}
				} while((c = *++s));
#endif
#endif /* __APPLE__ */
			return STRTOG_NaN;
#ifdef __APPLE__
		strlcpy(cp, s + 1, len + 1);
		*sp += len + 2;
#else /* !__APPLE__ */
			}
		havedig++;
		if (++i > 8) {
			if (x <= x0)
				continue;
			i = 1;
			*--x = 0;
			}
		*x = (*x << 4) | (h & 0xf);
#endif /* __APPLE__ */
		}
#ifdef __APPLE__
	nbits = fpi->nbits;
	/* a hack */
	if (nbits == 52) {	/* double */
		union IEEEd2bits u;
		u.d = nan(cp);
		x0[1] = u.bits.manh;
		x0[0] = u.bits.manl;
#else /* !__APPLE__ */
	if (!havedig)
		return STRTOG_NaN;
	if (x < x1 && i < 8)
		L_shift(x, x1, i);
	if (x > x0) {
		x1 = x0;
		do *x1++ = *x++;
			while(x <= xe);
		do *x1++ = 0;
			while(x1 <= xe);
#endif /* __APPLE__ */
		}
#ifdef __APPLE__
	else if (nbits < 52) {	/* float */
		union IEEEf2bits u;
		u.f = nanf(cp);
		x0[0] = u.bits.man;
#else /* !__APPLE__ */
	else {
		/* truncate high-order word if necessary */
		if ( (i = nbits & (ULbits-1)) !=0)
			*xe &= ((ULong)0xffffffff) >> (ULbits - i);
#endif /* __APPLE__ */
		}
#ifdef __APPLE__
	else {			/* long double */
		union IEEEl2bits u;
		u.e = nanl(cp);
#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) || defined(__arm64__)
		x0[1] = (ULong)u.bits.manh;
		x0[0] = (ULong)u.bits.manl;
#else
#error unsupported architecture
#endif
#else /* !__APPLE__ */
	for(x1 = xe;; --x1) {
		if (*x1 != 0)
			break;
		if (x1 == x0) {
			*x1 = 1;
			break;
			}
#endif /* __APPLE__ */
		}
	return STRTOG_NaNbits;
	}
