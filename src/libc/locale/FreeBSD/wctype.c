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
__FBSDID("$FreeBSD: src/lib/libc/locale/wctype.c,v 1.4 2008/03/17 18:22:23 antoine Exp $");

#include "xlocale_private.h"

#include <ctype.h>
#include <string.h>
#include <wctype.h>
#include <limits.h>

wctype_t
wctype_l(const char *property, locale_t loc)
{
	_RuneLocale *rl;
	static const struct {
		const char	*name;
		wctype_t	 mask;
	} props[] = {
		{ "alnum",	_CTYPE_A|_CTYPE_D },
		{ "alpha",	_CTYPE_A },
		{ "blank",	_CTYPE_B },
		{ "cntrl",	_CTYPE_C },
		{ "digit",	_CTYPE_D },
		{ "graph",	_CTYPE_G },
		{ "lower",	_CTYPE_L },
		{ "print",	_CTYPE_R },
		{ "punct",	_CTYPE_P },
		{ "space",	_CTYPE_S },
		{ "upper",	_CTYPE_U },
		{ "xdigit",	_CTYPE_X },
		{ "ideogram",	_CTYPE_I },	/* BSD extension */
		{ "special",	_CTYPE_T },	/* BSD extension */
		{ "phonogram",	_CTYPE_Q },	/* BSD extension */
		{ "rune",	0xFFFFFF00L },	/* BSD extension */
		{ NULL,		0UL },		/* Default */
	};
	int i;

	i = 0;
	while (props[i].name != NULL && strcmp(props[i].name, property) != 0)
		i++;

	if (props[i].mask)
		return (props[i].mask);

	NORMALIZE_LOCALE(loc);
	rl = &loc->__lc_ctype->_CurrentRuneLocale;
	if ((i = rl->__ncharclasses) > 0) {
		_RuneCharClass *rp;
		for (rp = rl->__charclasses; i-- > 0; rp++) {
			if (strncmp(rp->__name, property, CHARCLASS_NAME_MAX) == 0)
				return (rp->__mask);
		}
	}
	return 0;
}

wctype_t
wctype(const char *property)
{
	return wctype_l(property, __current_locale());
}
