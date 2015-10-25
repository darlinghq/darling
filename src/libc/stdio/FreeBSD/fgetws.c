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
__FBSDID("$FreeBSD: src/lib/libc/stdio/fgetws.c,v 1.8 2009/11/25 04:45:45 wollman Exp $");

#include "xlocale_private.h"

#include "namespace.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "local.h"
#include "mblocal.h"

wchar_t *
fgetws_l(wchar_t * __restrict ws, int n, FILE * __restrict fp, locale_t loc)
{
	wchar_t *wsp;
	size_t nconv;
	const char *src;
	unsigned char *nl;
	struct __xlocale_st_runelocale *rl;
	size_t (*__mbsnrtowcs)(wchar_t * __restrict, const char ** __restrict, size_t, size_t, __darwin_mbstate_t * __restrict, locale_t);

	NORMALIZE_LOCALE(loc);
	rl = loc->__lc_ctype;
	__mbsnrtowcs = rl->__mbsnrtowcs;
	FLOCKFILE(fp);
	ORIENT(fp, 1);

	if (n <= 0) {
		errno = EINVAL;
		goto error;
	}

	if (fp->_r <= 0 && __srefill(fp))
		/* EOF */
		goto error;
	wsp = ws;
	do {
		src = (const char *)fp->_p;
		nl = memchr(fp->_p, '\n', fp->_r);
		nconv = __mbsnrtowcs(wsp, &src,
		    nl != NULL ? (nl - fp->_p + 1) : fp->_r,
		    n - 1, &fp->_mbstate, loc);
		if (nconv == (size_t)-1)
			/* Conversion error */
			goto error;
		if (src == NULL) {
			/*
			 * We hit a null byte. Increment the character count,
			 * since mbsnrtowcs()'s return value doesn't include
			 * the terminating null, then resume conversion
			 * after the null.
			 */
			nconv++;
			src = memchr(fp->_p, '\0', fp->_r);
			src++;
		}
		fp->_r -= (unsigned char *)src - fp->_p;
		fp->_p = (unsigned char *)src;
		n -= nconv;
		wsp += nconv;
	} while (wsp[-1] != L'\n' && n > 1 && (fp->_r > 0 ||
	    __srefill(fp) == 0));
	if (wsp == ws)
		/* EOF */
		goto error;
	if (!rl->__mbsinit(&fp->_mbstate, loc))
		/* Incomplete character */
		goto error;
	*wsp = L'\0';
	FUNLOCKFILE(fp);

	return (ws);

error:
	FUNLOCKFILE(fp);
	return (NULL);
}

wchar_t *
fgetws(wchar_t * __restrict ws, int n, FILE * __restrict fp)
{
	return fgetws_l(ws, n, fp, __current_locale());
}
