/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Donn Seeley at UUNET Technologies, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)vsscanf.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdio/vsscanf.c,v 1.14 2008/04/17 22:17:54 jhb Exp $");

#include "xlocale_private.h"

#include <stdio.h>
#include <string.h>
#include "local.h"

static int
eofread(void *, char *, int);

/* ARGSUSED */
static int
eofread(cookie, buf, len)
	void *cookie;
	char *buf;
	int len;
{

	return (0);
}

int
vsscanf_l(str, loc, fmt, ap)
	const char * __restrict str;
	locale_t loc;
	const char * __restrict fmt;
	__va_list ap;
{
	FILE f;
	struct __sFILEX ext;
	f._extra = &ext;
	INITEXTRA(&f);

	f._file = -1;
	f._flags = __SRD;
	f._bf._base = f._p = (unsigned char *)str;
	f._bf._size = f._r = strlen(str);
	f._read = eofread;
	f._ub._base = NULL;
	f._lb._base = NULL;
	f._orientation = 0;
	memset(&f._mbstate, 0, sizeof(mbstate_t));
	return (__svfscanf_l(&f, loc, fmt, ap));
}

int
vsscanf(str, fmt, ap)
	const char * __restrict str;
	const char * __restrict fmt;
	__va_list ap;
{
	return vsscanf_l(str, __current_locale(), fmt, ap);
}

