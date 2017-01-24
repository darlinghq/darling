/*-
 * Copyright (c) 2002-2004 Tim J. Robbins
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/param.h>
__FBSDID("$FreeBSD: src/lib/libc/locale/utf8.c,v 1.16 2007/10/15 09:51:30 ache Exp $");

#include "xlocale_private.h"

#include <errno.h>
#include <limits.h>
#include <runetype.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "mblocal.h"

/*
 * 10952550: detect ill-formed UTF-8
 * Unicode 6.0, section D92, mandates specific byte sequences for well-
 * formed UTF-8.  UTF-8 sequences are now limited to 4 bytes, while the
 * FreeBSD code originally handled up to 6.  Illegal surrogate code point
 * sequences are now detected.  And while "non-shortest forms" were detected,
 * this only happened after completing the sequence.  Now, all ill-formed
 * sequences are detected at the earliest point.
 *
 *          Table 3-7.  Well-Formed UTF-8 Byte Sequences
 *
 *      Code Points         1st      2nd      3rd      4th Byte
 *    U+0000..U+007F       00..7F
 *    U+0080..U+07FF       C2..DF   80..BF
 *    U+0800..U+0FFF       E0       A0..BF   80..BF
 *    U+1000..U+CFFF       E1..EC   80..BF   80..BF
 *    U+D000..U+D7FF       ED       80..9F   80..BF
 *    U+E000..U+FFFF       EE..EF   80..BF   80..BF
 *    U+10000..U+3FFFF     F0       90..BF   80..BF   80..BF
 *    U+40000..U+FFFFF     F1..F3   80..BF   80..BF   80..BF
 *    U+100000..U+10FFFF   F4       80..8F   80..BF   80..BF
 *
 * Note that while any 3rd and 4th byte can be in the range 80..BF, the
 * second byte is often limited to a smaller range.
 */

typedef struct {
	unsigned char lowerbound;
	unsigned char upperbound;
} SecondByte;
static SecondByte sb_00_00 = {0x00, 0x00};
static SecondByte sb_80_8F = {0x80, 0x8F};
static SecondByte sb_80_9F = {0x80, 0x9F};
static SecondByte sb_80_BF = {0x80, 0xBF};
static SecondByte sb_90_BF = {0x90, 0xBF};
static SecondByte sb_A0_BF = {0xA0, 0xBF};

#define UTF8_MB_CUR_MAX		4

static size_t	_UTF8_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict, locale_t);
static int	_UTF8_mbsinit(const mbstate_t *, locale_t);
static size_t	_UTF8_mbsnrtowcs(wchar_t * __restrict,
		    const char ** __restrict, size_t, size_t,
		    mbstate_t * __restrict, locale_t);
static size_t	_UTF8_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict, locale_t);
static size_t	_UTF8_wcsnrtombs(char * __restrict, const wchar_t ** __restrict,
		    size_t, size_t, mbstate_t * __restrict, locale_t);

typedef struct {
	wchar_t	ch;
	int	want;
	SecondByte sb;
} _UTF8State;

int
_UTF8_init(struct __xlocale_st_runelocale *xrl)
{

	xrl->__mbrtowc = _UTF8_mbrtowc;
	xrl->__wcrtomb = _UTF8_wcrtomb;
	xrl->__mbsinit = _UTF8_mbsinit;
	xrl->__mbsnrtowcs = _UTF8_mbsnrtowcs;
	xrl->__wcsnrtombs = _UTF8_wcsnrtombs;
	xrl->__mb_cur_max = UTF8_MB_CUR_MAX;
	/*
	 * UCS-4 encoding used as the internal representation, so
	 * slots 0x0080-0x00FF are occuped and must be excluded
	 * from the single byte ctype by setting the limit.
	 */
	xrl->__mb_sb_limit = 128;

	return (0);
}

static int
_UTF8_mbsinit(const mbstate_t *ps, locale_t loc)
{

	return (ps == NULL || ((const _UTF8State *)ps)->want == 0);
}

static size_t
_UTF8_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps, locale_t loc)
{
	_UTF8State *us;
	int ch, i, mask, want;
	wchar_t wch;
	SecondByte sb;

	us = (_UTF8State *)ps;

	if (us->want < 0 || us->want > UTF8_MB_CUR_MAX) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL) {
		s = "";
		n = 1;
		pwc = NULL;
	}

	if (n == 0)
		/* Incomplete multibyte sequence */
		return ((size_t)-2);

	if (us->want == 0 && ((ch = (unsigned char)*s) & ~0x7f) == 0) {
		/* Fast path for plain ASCII characters. */
		if (pwc != NULL)
			*pwc = ch;
		return (ch != '\0' ? 1 : 0);
	}

	if (us->want == 0) {
		/*
		 * Determine the number of octets that make up this character
		 * from the first octet, and a mask that extracts the
		 * interesting bits of the first octet. We already know
		 * the character is at least two bytes long.
		 *
		 * We detect if the first byte is illegal, and set sb to
		 * the legal range of the second byte.
		 */
		ch = (unsigned char)*s;
		if ((ch & 0x80) == 0) {
			mask = 0x7f;
			want = 1;
			sb = sb_00_00;
		} else if ((ch & 0xe0) == 0xc0) {
			if (ch < 0xc2) goto malformed;
			mask = 0x1f;
			want = 2;
			sb = sb_80_BF;
		} else if ((ch & 0xf0) == 0xe0) {
			mask = 0x0f;
			want = 3;
			switch (ch) {
			case 0xe0:
				sb = sb_A0_BF;
				break;
			case 0xed:
				sb = sb_80_9F;
				break;
			default:
				sb = sb_80_BF;
				break;
			}
		} else if ((ch & 0xf8) == 0xf0) {
			if (ch > 0xf4) goto malformed;
			mask = 0x07;
			want = 4;
			switch (ch) {
			case 0xf0:
				sb = sb_90_BF;
				break;
			case 0xf4:
				sb = sb_80_8F;
				break;
			default:
				sb = sb_80_BF;
				break;
			}
		} else {
malformed:
			/*
			 * Malformed input; input is not UTF-8.
			 */
			errno = EILSEQ;
			return ((size_t)-1);
		}
	} else {
		want = us->want;
		sb = us->sb;
	}

	/*
	 * Decode the octet sequence representing the character in chunks
	 * of 6 bits, most significant first.
	 */
	if (us->want == 0)
		wch = (unsigned char)*s++ & mask;
	else
		wch = us->ch;
	for (i = (us->want == 0) ? 1 : 0; i < MIN(want, n); i++) {
		if (sb.lowerbound) {
			if ((unsigned char)*s < sb.lowerbound ||
			   (unsigned char)*s > sb.upperbound) goto malformed;
			sb = sb_00_00;
		} else if ((*s & 0xc0) != 0x80) goto malformed;
		wch <<= 6;
		wch |= *s++ & 0x3f;
	}
	if (i < want) {
		/* Incomplete multibyte sequence. */
		us->want = want - i;
		us->sb = sb;
		us->ch = wch;
		return ((size_t)-2);
	}
	if (pwc != NULL)
		*pwc = wch;
	us->want = 0;
	return (wch == L'\0' ? 0 : want);
}

static size_t
_UTF8_mbsnrtowcs(wchar_t * __restrict dst, const char ** __restrict src,
    size_t nms, size_t len, mbstate_t * __restrict ps, locale_t loc)
{
	_UTF8State *us;
	const char *s;
	size_t nchr;
	wchar_t wc;
	size_t nb;

	us = (_UTF8State *)ps;

	s = *src;
	nchr = 0;

	if (dst == NULL) {
		/*
		 * The fast path in the loop below is not safe if an ASCII
		 * character appears as anything but the first byte of a
		 * multibyte sequence. Check now to avoid doing it in the loop.
		 */
		if (nms > 0 && us->want > 0 && (signed char)*s > 0) {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		for (;;) {
			if (nms > 0 && (signed char)*s > 0)
				/*
				 * Fast path for plain ASCII characters
				 * excluding NUL.
				 */
				nb = 1;
			else if ((nb = _UTF8_mbrtowc(&wc, s, nms, ps, loc)) ==
			    (size_t)-1)
				/* Invalid sequence - mbrtowc() sets errno. */
				return ((size_t)-1);
			else if (nb == 0 || nb == (size_t)-2)
				return (nchr);
			s += nb;
			nms -= nb;
			nchr++;
		}
		/*NOTREACHED*/
	}

	/*
	 * The fast path in the loop below is not safe if an ASCII
	 * character appears as anything but the first byte of a
	 * multibyte sequence. Check now to avoid doing it in the loop.
	 */
	if (nms > 0 && len > 0 && us->want > 0 && (signed char)*s > 0) {
		errno = EILSEQ;
		return ((size_t)-1);
	}
	while (len-- > 0) {
		if (nms > 0 && (signed char)*s > 0) {
			/*
			 * Fast path for plain ASCII characters
			 * excluding NUL.
			 */
			*dst = (wchar_t)*s;
			nb = 1;
		} else if ((nb = _UTF8_mbrtowc(dst, s, nms, ps, loc)) ==
		    (size_t)-1) {
			*src = s;
			return ((size_t)-1);
		} else if (nb == (size_t)-2) {
			*src = s + nms;
			return (nchr);
		} else if (nb == 0) {
			*src = NULL;
			return (nchr);
		}
		s += nb;
		nms -= nb;
		nchr++;
		dst++;
	}
	*src = s;
	return (nchr);
}

static size_t
_UTF8_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps, locale_t loc)
{
	_UTF8State *us;
	unsigned char lead;
	int i, len;

	us = (_UTF8State *)ps;

	if (us->want != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);

	if ((wc & ~0x7f) == 0) {
		/* Fast path for plain ASCII characters. */
		*s = (char)wc;
		return (1);
	}

	/*
	 * Determine the number of octets needed to represent this character.
	 * We always output the shortest sequence possible. Also specify the
	 * first few bits of the first octet, which contains the information
	 * about the sequence length.
	 */
	if ((wc & ~0x7f) == 0) {
		lead = 0;
		len = 1;
	} else if ((wc & ~0x7ff) == 0) {
		lead = 0xc0;
		len = 2;
	} else if ((wc & ~0xffff) == 0) {
		if (wc >= 0xd800 && wc <= 0xdfff) goto illegal;
		lead = 0xe0;
		len = 3;
	} else if ((wc & ~0x1fffff) == 0) {
		if (wc > 0x10ffff) goto illegal;
		lead = 0xf0;
		len = 4;
	} else {
illegal:
		errno = EILSEQ;
		return ((size_t)-1);
	}

	/*
	 * Output the octets representing the character in chunks
	 * of 6 bits, least significant last. The first octet is
	 * a special case because it contains the sequence length
	 * information.
	 */
	for (i = len - 1; i > 0; i--) {
		s[i] = (wc & 0x3f) | 0x80;
		wc >>= 6;
	}
	*s = (wc & 0xff) | lead;

	return (len);
}

static size_t
_UTF8_wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps, locale_t loc)
{
	_UTF8State *us;
	char buf[MB_LEN_MAX];
	const wchar_t *s;
	size_t nbytes;
	size_t nb;

	us = (_UTF8State *)ps;

	if (us->want != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	s = *src;
	nbytes = 0;

	if (dst == NULL) {
		while (nwc-- > 0) {
			if (0 <= *s && *s < 0x80)
				/* Fast path for plain ASCII characters. */
				nb = 1;
			else if ((nb = _UTF8_wcrtomb(buf, *s, ps, loc)) ==
			    (size_t)-1)
				/* Invalid character - wcrtomb() sets errno. */
				return ((size_t)-1);
			if (*s == L'\0')
				return (nbytes + nb - 1);
			s++;
			nbytes += nb;
		}
		return (nbytes);
	}

	while (len > 0 && nwc-- > 0) {
		if (0 <= *s && *s < 0x80) {
			/* Fast path for plain ASCII characters. */
			nb = 1;
			*dst = *s;
		} else if (len > (size_t)UTF8_MB_CUR_MAX) {
			/* Enough space to translate in-place. */
			if ((nb = _UTF8_wcrtomb(dst, *s, ps, loc)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
		} else {
			/*
			 * May not be enough space; use temp. buffer.
			 */
			if ((nb = _UTF8_wcrtomb(buf, *s, ps, loc)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
			if (nb > (int)len)
				/* MB sequence for character won't fit. */
				break;
			memcpy(dst, buf, nb);
		}
		if (*s == L'\0') {
			*src = NULL;
			return (nbytes + nb - 1);
		}
		s++;
		dst += nb;
		len -= nb;
		nbytes += nb;
	}
	*src = s;
	return (nbytes);
}
