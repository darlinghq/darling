/*-
 * Copyright (c) 2002, 2003 Tim J. Robbins
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
__FBSDID("$FreeBSD: src/lib/libc/locale/wcstof.c,v 1.3 2004/04/07 09:47:56 tjr Exp $");

#include "xlocale_private.h"

#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>
#include <_simple.h>

/*
 * See wcstod() for comments as to the logic used.
 */

extern size_t __wcs_end_offset(const char * __restrict buf, const char * __restrict end, locale_t loc);

float
wcstof_l(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr,
    locale_t loc)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	float val;
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

	buf = _simple_string(b);
	val = strtof_l(buf, &end, loc);

	if (endptr != NULL)
		*endptr = (end == buf) ? (wchar_t *)nptr0 : ((wchar_t *)first + __wcs_end_offset(buf, end, loc));

	_simple_sfree(b);

	return (val);
}

float
wcstof(const wchar_t * __restrict nptr, wchar_t ** __restrict endptr)
{
	return wcstof_l(nptr, endptr, __current_locale());
}
