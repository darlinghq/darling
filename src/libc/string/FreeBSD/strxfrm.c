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
__FBSDID("$FreeBSD: src/lib/libc/string/strxfrm.c,v 1.17 2008/10/19 09:10:44 delphij Exp $");

#include "xlocale_private.h"

#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <errno.h>
#include "collate.h"

/*
 * In the non-POSIX case, we transform each character into a string of
 * characters representing the character's priority.  Since char is usually
 * signed, we are limited by 7 bits per byte.  To avoid zero, we need to add
 * XFRM_OFFSET, so we can't use a full 7 bits.  For simplicity, we choose 6
 * bits per byte.  We choose 4 bytes per character as a good compromise
 * between maximum coverage and minimum size.  This gives 24 bits, or 16M
 * priorities.  So we choose COLLATE_MAX_PRIORITY to be (2^24 - 1).  This
 * this can be increased if more is needed.
 */

#define	XFRM_BYTES	4
#define	XFRM_OFFSET	('0')	/* make all printable characters */
#define	XFRM_SHIFT	6
#define	XFRM_MASK	((1 << XFRM_SHIFT) - 1)

static void
xfrm(unsigned char *p, int pri)
{

	p[3] = (pri & XFRM_MASK) + XFRM_OFFSET;
	pri >>= XFRM_SHIFT;
	p[2] = (pri & XFRM_MASK) + XFRM_OFFSET;
	pri >>= XFRM_SHIFT;
	p[1] = (pri & XFRM_MASK) + XFRM_OFFSET;
	pri >>= XFRM_SHIFT;
	p[0] = (pri & XFRM_MASK) + XFRM_OFFSET;
}

size_t
strxfrm_l(char * __restrict dest, const char * __restrict src, size_t len,
    locale_t loc)
{
	size_t slen;
	wchar_t *wcs, *xf[2];
	int sverrno;

	if (!*src && dest) {
		if (len > 0)
			*dest = '\0';
		return 0;
	}

	NORMALIZE_LOCALE(loc);
	if (loc->__collate_load_error || (wcs = __collate_mbstowcs(src, loc)) == NULL)
		return strlcpy(dest, src, len);

	__collate_xfrm(wcs, xf, loc);

	slen = wcslen(xf[0]) * XFRM_BYTES;
	if (xf[1])
		slen += (wcslen(xf[1]) + 1) * XFRM_BYTES;
	if (len > 0) {
		wchar_t *w = xf[0];
		int b = 0;
		unsigned char buf[XFRM_BYTES];
		unsigned char *bp;
		while (len > 1) {
			if (!b) {
				if (!*w)
					break;
				xfrm(bp = buf, *w++);
				b = XFRM_BYTES;
			}
			*dest++ = *(char *)bp++;
			b--;
			len--;
		}
		if ((w = xf[1]) != NULL) {
			xfrm(bp = buf, 0);
			b = XFRM_BYTES;
			while (len > 1) {
				if (!b)
					break;
				*dest++ = *(char *)bp++;
				b--;
				len--;
			}
			b = 0;
			while (len > 1) {
				if (!b) {
					if (!*w)
						break;
					xfrm(bp = buf, *w++);
					b = XFRM_BYTES;
				}
				*dest++ = *(char *)bp++;
				b--;
				len--;
			}
		}
		*dest = 0;
 	}
	sverrno = errno;
	free(wcs);
	free(xf[0]);
	free(xf[1]);
	errno = sverrno;

	return slen;
}

size_t
strxfrm(char * __restrict dest, const char * __restrict src, size_t len)
{
	return strxfrm_l(dest, src, len, __current_locale());
}
