/*-
 * Copyright (c) 2002-2004 Tim J. Robbins.
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/locale/wcsnrtombs.c,v 1.3 2005/02/12 08:45:12 stefanf Exp $");

#include "xlocale_private.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "mblocal.h"

size_t
wcsnrtombs_l(char * __restrict dst, const wchar_t ** __restrict src, size_t nwc,
    size_t len, mbstate_t * __restrict ps, locale_t loc)
{
	NORMALIZE_LOCALE(loc);
	if (ps == NULL)
		ps = &loc->__mbs_wcsnrtombs;
	return (loc->__lc_ctype->__wcsnrtombs(dst, src, nwc, len, ps, loc));
}

size_t
wcsnrtombs(char * __restrict dst, const wchar_t ** __restrict src, size_t nwc,
    size_t len, mbstate_t * __restrict ps)
{
	return wcsnrtombs_l(dst, src, nwc, len, ps, __current_locale());
}

__private_extern__ size_t
__wcsnrtombs_std(char * __restrict dst, const wchar_t ** __restrict src,
    size_t nwc, size_t len, mbstate_t * __restrict ps, locale_t loc)
{
	mbstate_t mbsbak;
	char buf[MB_LEN_MAX];
	const wchar_t *s;
	size_t nbytes;
	size_t nb;
	struct __xlocale_st_runelocale *runeLocale = loc->__lc_ctype;
	size_t (*__wcrtomb)(char * __restrict, wchar_t, mbstate_t * __restrict, locale_t) = runeLocale->__wcrtomb;
	int mb_cur_max = runeLocale->__mb_cur_max;

	s = *src;
	nbytes = 0;

	if (dst == NULL) {
		while (nwc-- > 0) {
			if ((nb = __wcrtomb(buf, *s, ps, loc)) == (size_t)-1)
				/* Invalid character - wcrtomb() sets errno. */
				return ((size_t)-1);
			else if (*s == L'\0')
				return (nbytes + nb - 1);
			s++;
			nbytes += nb;
		}
		return (nbytes);
	}

	while (len > 0 && nwc-- > 0) {
		if (len > (size_t)mb_cur_max) {
			/* Enough space to translate in-place. */
			if ((nb = __wcrtomb(dst, *s, ps, loc)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
		} else {
			/*
			 * May not be enough space; use temp. buffer.
			 *
			 * We need to save a copy of the conversion state
			 * here so we can restore it if the multibyte
			 * character is too long for the buffer.
			 */
			mbsbak = *ps;
			if ((nb = __wcrtomb(buf, *s, ps, loc)) == (size_t)-1) {
				*src = s;
				return ((size_t)-1);
			}
			if (nb > (int)len) {
				/* MB sequence for character won't fit. */
				*ps = mbsbak;
				break;
			}
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
