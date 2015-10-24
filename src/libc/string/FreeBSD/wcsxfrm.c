/*-
 * Copyright (c) 1995 Alex Tatmanjants <alex@elvisti.kiev.ua>
 *		at Electronni Visti IA, Kiev, Ukraine.
 *			All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#if 0
__FBSDID("FreeBSD: src/lib/libc/string/strxfrm.c,v 1.15 2002/09/06 11:24:06 tjr Exp ");
#endif
__FBSDID("$FreeBSD: src/lib/libc/string/wcsxfrm.c,v 1.3 2004/04/07 09:47:56 tjr Exp $");

#include "xlocale_private.h"

#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <errno.h>
#include "collate.h"

#define WCS_XFRM_OFFSET	1

size_t
wcsxfrm_l(wchar_t * __restrict dest, const wchar_t * __restrict src, size_t len,
    locale_t loc)
{
	size_t slen;
	wchar_t *xf[2];
	int sverrno;

	if (*src == L'\0') {
		if (len != 0)
			*dest = L'\0';
		return (0);
	}

	NORMALIZE_LOCALE(loc);
	if (loc->__collate_load_error) {
		slen = wcslen(src);
		if (len > 0) {
			if (slen < len)
				wcscpy(dest, src);
			else {
				wcsncpy(dest, src, len - 1);
				dest[len - 1] = L'\0';
			}
		}
		return (slen);
	}

	__collate_xfrm(src, xf, loc);

	slen = wcslen(xf[0]);
	if (xf[1])
		slen += wcslen(xf[1]) + 1;
	if (len > 0) {
		wchar_t *w = xf[0];
		while (len > 1) {
			if (!*w)
				break;
			*dest++ = *w++ + WCS_XFRM_OFFSET;
			len--;
		}
		if ((w = xf[1]) != NULL) {
			if (len > 1)
				*dest++ = WCS_XFRM_OFFSET;
			while (len > 1) {
				if (!*w)
					break;
				*dest++ = *w++ + WCS_XFRM_OFFSET;
				len--;
			}
		}
		*dest = 0;
 	}
	sverrno = errno;
	free(xf[0]);
	free(xf[1]);
	errno = sverrno;
 
	return (slen);
}

size_t
wcsxfrm(wchar_t * __restrict dest, const wchar_t * __restrict src, size_t len)
{
	return wcsxfrm_l(dest, src, len, __current_locale());
}
