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
__FBSDID("$FreeBSD: src/lib/libc/stdio/fgetwln.c,v 1.2 2004/08/06 17:00:09 tjr Exp $");

#include "namespace.h"
#include <stdio.h>
#include <wchar.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "local.h"

wchar_t *
fgetwln_l(FILE * __restrict fp, size_t *lenp, locale_t loc)
{
	wint_t wc;
	size_t len;

	FLOCKFILE(fp);
	ORIENT(fp, 1);

	len = 0;
	while ((wc = __fgetwc(fp, loc)) != WEOF) {
#define	GROW	512
		if (len * sizeof(wchar_t) >= fp->_lb._size &&
		    __slbexpand(fp, (len + GROW) * sizeof(wchar_t)))
			goto error;
		*((wchar_t *)fp->_lb._base + len++) = wc;
		if (wc == L'\n')
			break;
	}
	if (len == 0)
		goto error;

	FUNLOCKFILE(fp);
	*lenp = len;
	return ((wchar_t *)fp->_lb._base);

error:
	FUNLOCKFILE(fp);
	*lenp = 0;
	return (NULL);
}

wchar_t *
fgetwln(FILE * __restrict fp, size_t *lenp)
{
	return fgetwln_l(fp, lenp, __current_locale());
}
