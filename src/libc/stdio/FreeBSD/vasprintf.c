/*	$OpenBSD: vasprintf.c,v 1.4 1998/06/21 22:13:47 millert Exp $	*/

/*
 * Copyright (c) 1997 Todd C. Miller <Todd.Miller@courtesan.com>
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
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdio/vasprintf.c,v 1.19 2008/04/17 22:17:54 jhb Exp $");

#include "xlocale_private.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "local.h"

__private_extern__ int
_vasprintf(printf_comp_t __restrict pc, printf_domain_t __restrict domain, char ** __restrict str, locale_t __restrict loc, const char * __restrict fmt, __va_list ap)
{
	int ret;
	FILE f;
	struct __sFILEX ext;
	f._extra = &ext;
	INITEXTRA(&f);
	
	f._file = -1;
	f._flags = __SWR | __SSTR | __SALC;
	f._bf._base = f._p = (unsigned char *)malloc(128);
	if (f._bf._base == NULL) {
		*str = NULL;
		errno = ENOMEM;
		return (-1);
	}
	f._bf._size = f._w = 127;		/* Leave room for the NUL */
	f._orientation = 0;
	memset(&f._mbstate, 0, sizeof(mbstate_t));
	ret = __v2printf(pc, domain, &f, loc, fmt, ap);
	if (ret < 0) {
		free(f._bf._base);
		*str = NULL;
		errno = ENOMEM;
		return (-1);
	}
	*f._p = '\0';
	*str = (char *)f._bf._base;
	return (ret);
}

int
vasprintf_l(char ** __restrict str, locale_t __restrict loc, const char * __restrict fmt, __va_list ap)
{
	return _vasprintf(XPRINTF_PLAIN, NULL, str, loc, fmt, ap);
}

int
vasprintf(char ** __restrict str, const char * __restrict fmt, __va_list ap)
{
	return _vasprintf(XPRINTF_PLAIN, NULL, str, __current_locale(), fmt, ap);
}
