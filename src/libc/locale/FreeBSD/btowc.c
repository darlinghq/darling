/*-
 * Copyright (c) 2002, 2003 Tim J. Robbins.
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
__FBSDID("$FreeBSD: src/lib/libc/locale/btowc.c,v 1.4 2004/05/12 14:26:54 tjr Exp $");

#include "xlocale_private.h"

#include <stdio.h>
#include <wchar.h>
#include "mblocal.h"

wint_t
btowc_l(int c, locale_t loc)
{
	static const mbstate_t initial;
	mbstate_t mbs = initial;
	char cc;
	wchar_t wc;

	NORMALIZE_LOCALE(loc);
	if (c == EOF)
		return (WEOF);
	/*
	 * We expect mbrtowc() to return 0 or 1, hence the check for n > 1
	 * which detects error return values as well as "impossible" byte
	 * counts.
	 */
	cc = (char)c;
	if (loc->__lc_ctype->__mbrtowc(&wc, &cc, 1, &mbs, loc) > 1)
		return (WEOF);
	return (wc);
}

wint_t
btowc(int c)
{
	return btowc_l(c, __current_locale());
}
