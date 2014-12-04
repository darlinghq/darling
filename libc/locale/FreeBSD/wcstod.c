/*-
 * Copyright (c) 2002 Tim J. Robbins
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
__FBSDID("$FreeBSD: src/lib/libc/locale/wcstod.c,v 1.4 2004/04/07 09:47:56 tjr Exp $");

#include "xlocale_private.h"

#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>
#include <_simple.h>

/*
 * __wcs_end_offset calculates the offset to the end within the wide character
 * string, assuming numbers and letters are single bytes in multibyte
 * representation, get the actual decimal string for localeconv_l.  If the
 * decimal point was within the string, compensate for the fact that the
 * (possible more than one byte) decimal point one takes one wide character.
 */
__private_extern__ size_t
__wcs_end_offset(const char * __restrict buf, const char * __restrict end, locale_t loc)
{
	const char *decimalpoint = localeconv_l(loc)->decimal_point;
	size_t n = end - buf;
	char *p;

	if ((p = strnstr(buf, decimalpoint, n)) != NULL)
		n -= strlen(decimalpoint) - 1;
	return n;
}

/*
 * Convert a string to a double-precision number.
 *
 * This is the wide-character counterpart of strtod(). So that we do not
 * have to duplicate the code of strtod() here, we convert the supplied
 * wide character string to multibyte and call strtod() on the result.
 * This assumes that the multibyte encoding is compatible with ASCII
 * for at least the digits and letters.  The radix character can be more
 * than one byte.
 */

double
wcstod_l(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr,
    locale_t loc)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	double val;
	char *buf, *end;
	size_t len;
	locale_t ctype;
	_SIMPLE_STRING b;
	char mb[MB_CUR_MAX + 1];
	const wchar_t *nptr0 = nptr;
	const wchar_t *first;

	NORMALIZE_LOCALE(loc);
	ctype = __numeric_ctype(loc);

	while (iswspace_l(*nptr, ctype))
		nptr++;

	if ((b = _simple_salloc()) == NULL)
		return (0.0);

	first = nptr;
	mbs = initial;
	while (*nptr && (len = wcrtomb_l(mb, *nptr, &mbs, ctype)) != (size_t)-1) {
		mb[len] = 0;
		if (_simple_sappend(b, mb) < 0) { /* no memory */
			_simple_sfree(b);
			return (0.0);
		}
		nptr++;
	}

	/* Let strtod() do most of the work for us. */
	buf = _simple_string(b);
	val = strtod_l(buf, &end, loc);

	/*
	 * We only know where the number ended in the _multibyte_
	 * representation of the string. If the caller wants to know
	 * where it ended, count multibyte characters to find the
	 * corresponding position in the wide char string.
	 */
	if (endptr != NULL)
		/* XXX Assume each wide char is one byte. */
		*endptr = (end == buf) ? (wchar_t *)nptr0 : ((wchar_t *)first + __wcs_end_offset(buf, end, loc));

	_simple_sfree(b);

	return (val);
}

double
wcstod(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr)
{
	return wcstod_l(nptr, endptr, __current_locale());
}
