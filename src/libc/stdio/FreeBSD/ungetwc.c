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
__FBSDID("$FreeBSD: src/lib/libc/stdio/ungetwc.c,v 1.11 2008/04/17 22:17:54 jhb Exp $");

#include "xlocale_private.h"

#include "namespace.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "local.h"
#include "mblocal.h"

/*
 * Non-MT-safe version.
 */
wint_t
__ungetwc(wint_t wc, FILE *fp, locale_t loc)
{
	char buf[MB_LEN_MAX];
	size_t len;

	if (wc == WEOF)
		return (WEOF);
	if ((len = loc->__lc_ctype->__wcrtomb(buf, wc, &fp->_mbstate, loc)) == (size_t)-1) {
		fp->_flags |= __SERR;
		return (WEOF);
	}
	while (len-- != 0)
		if (__ungetc((unsigned char)buf[len], fp) == EOF)
			return (WEOF);

	return (wc);
}

/*
 * MT-safe version.
 */
wint_t
ungetwc(wint_t wc, FILE *fp)
{
	wint_t r;

	FLOCKFILE(fp);
	ORIENT(fp, 1);
	r = __ungetwc(wc, fp, __current_locale());
	FUNLOCKFILE(fp);

	return (r);
}

wint_t
ungetwc_l(wint_t wc, FILE *fp, locale_t loc)
{
	wint_t r;

	NORMALIZE_LOCALE(loc);
	FLOCKFILE(fp);
	ORIENT(fp, 1);
	r = __ungetwc(wc, fp, loc);
	FUNLOCKFILE(fp);

	return (r);
}
