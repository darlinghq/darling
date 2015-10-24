/*-
 * Copyright (c) 2002 Tim J. Robbins.
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
__FBSDID("$FreeBSD: src/lib/libc/locale/wctrans.c,v 1.3 2003/11/01 08:20:58 tjr Exp $");

#include "xlocale_private.h"

#include <errno.h>
#include <string.h>
#include <wctype.h>

enum {
	_WCT_ERROR	= 0,
	_WCT_TOLOWER	= 1,
	_WCT_TOUPPER	= 2
};

wint_t
towctrans_l(wint_t wc, wctrans_t desc, locale_t loc)
{

	NORMALIZE_LOCALE(loc);
	switch (desc) {
	case _WCT_TOLOWER:
		wc = towlower_l(wc, loc);
		break;
	case _WCT_TOUPPER:
		wc = towupper_l(wc, loc);
		break;
	case _WCT_ERROR:
	default:
		errno = EINVAL;
		break;
	}

	return (wc);
}

wint_t
towctrans(wint_t wc, wctrans_t desc)
{
	return towctrans_l(wc, desc, __current_locale());
}

wctrans_t
wctrans(const char *charclass)
{
	struct {
		const char	*name;
		wctrans_t	 trans;
	} ccls[] = {
		{ "tolower",	_WCT_TOLOWER },
		{ "toupper",	_WCT_TOUPPER },
		{ NULL,		_WCT_ERROR },		/* Default */
	};
	int i;

	i = 0;
	while (ccls[i].name != NULL && strcmp(ccls[i].name, charclass) != 0)
		i++;

	if (ccls[i].trans == _WCT_ERROR)
		errno = EINVAL;
	return (ccls[i].trans);
}

/*
 * The extended locale version just calls the regular version.  If there
 * is ever support for arbitrary per-locale translations, this need to
 * be modified.
 */
wctrans_t
wctrans_l(const char *charclass, locale_t loc)
{
	return wctrans(charclass);
}
