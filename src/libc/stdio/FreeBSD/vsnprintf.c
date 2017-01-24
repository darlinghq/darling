/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
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
static char sccsid[] = "@(#)vsnprintf.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdio/vsnprintf.c,v 1.24 2008/04/17 22:17:54 jhb Exp $");

#include "xlocale_private.h"

#include <limits.h>
#include <stdio.h>
#include "local.h"

__private_extern__ int
_vsnprintf(printf_comp_t __restrict pc, printf_domain_t __restrict domain, char * __restrict str, size_t n, locale_t __restrict loc, const char * __restrict fmt, __va_list ap)
{
	size_t on;
	int ret;
	char dummy[2];
	FILE f;
	struct __sFILEX ext;
	f._extra = &ext;
	INITEXTRA(&f);

	on = n;
	if (n != 0)
		n--;
#if defined(__i386__)
	/* <rdar://problem/16329527> don't corrupt the output buffer at all if the size underflowed */
	if (n > INT_MAX)
		on = n = 0;
#else
	if (n > INT_MAX)
		n = INT_MAX;
#endif

	/* Stdio internals do not deal correctly with zero length buffer */
	if (n == 0) {
		if (on > 0)
	  		*str = '\0';
		str = dummy;
		n = 1;
	}
	f._file = -1;
	f._flags = __SWR | __SSTR;
	f._bf._base = f._p = (unsigned char *)str;
	f._bf._size = f._w = n;
	f._orientation = 0;
	memset(&f._mbstate, 0, sizeof(mbstate_t));
	ret = __v2printf(pc, domain, &f, loc, fmt, ap);
	if (on > 0)
		*f._p = '\0';
	return (ret);
}
int
vsnprintf_l(char * __restrict str, size_t n, locale_t __restrict loc, const char * __restrict fmt,
    __va_list ap)
{
	return _vsnprintf(XPRINTF_PLAIN, NULL, str, n, loc, fmt, ap);
}

int
vsnprintf(char * __restrict str, size_t n, const char * __restrict fmt,
    __va_list ap)
{
	return _vsnprintf(XPRINTF_PLAIN, NULL, str, n, __current_locale(), fmt, ap);
}
