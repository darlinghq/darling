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
__FBSDID("$FreeBSD: src/lib/libc/stdio/fputws.c,v 1.8 2009/01/15 18:53:52 rdivacky Exp $");

#include "xlocale_private.h"

#include "namespace.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <wchar.h>
#include "un-namespace.h"
#include "fvwrite.h"
#include "libc_private.h"
#include "local.h"
#include "mblocal.h"

int
fputws_l(const wchar_t * __restrict ws, FILE * __restrict fp, locale_t loc)
{
	size_t nbytes;
	char buf[BUFSIZ];
	struct __suio uio;
	struct __siov iov;
	const wchar_t *wsp = ws;
	size_t (*__wcsnrtombs)(char * __restrict, const wchar_t ** __restrict,
	    size_t, size_t, mbstate_t * __restrict, locale_t);

	NORMALIZE_LOCALE(loc);
	__wcsnrtombs = loc->__lc_ctype->__wcsnrtombs;
	FLOCKFILE(fp);
	ORIENT(fp, 1);
	if (prepwrite(fp) != 0)
		goto error;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	iov.iov_base = buf;
	do {
		nbytes = __wcsnrtombs(buf, &wsp, SIZE_T_MAX, sizeof(buf),
		    &fp->_mbstate, loc);
		if (nbytes == (size_t)-1)
			goto error;
		iov.iov_len = uio.uio_resid = nbytes;
		if (__sfvwrite(fp, &uio) != 0)
			goto error;
	} while (wsp != NULL);
	FUNLOCKFILE(fp);
	return (0);

error:
	FUNLOCKFILE(fp);
	return (-1);
}

int
fputws(const wchar_t * __restrict ws, FILE * __restrict fp)
{
	return fputws_l(ws, fp, __current_locale());
}
