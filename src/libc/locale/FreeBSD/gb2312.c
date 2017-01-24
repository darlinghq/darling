/*-
 * Copyright (c) 2004 Tim J. Robbins. All rights reserved.
 * Copyright (c) 2003 David Xu <davidxu@freebsd.org>
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

#include <sys/param.h>
__FBSDID("$FreeBSD: src/lib/libc/locale/gb2312.c,v 1.10 2007/10/13 16:28:21 ache Exp $");

#include "xlocale_private.h"

#include <errno.h>
#include <runetype.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "mblocal.h"

#define GB2312_MB_CUR_MAX	2

static size_t	_GB2312_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict, locale_t);
static int	_GB2312_mbsinit(const mbstate_t *, locale_t);
static size_t	_GB2312_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict, locale_t);
typedef struct {
	int	count;
	u_char	bytes[2];
} _GB2312State;

int
_GB2312_init(struct __xlocale_st_runelocale *xrl)
{

	xrl->__mbrtowc = _GB2312_mbrtowc;
	xrl->__wcrtomb = _GB2312_wcrtomb;
	xrl->__mbsinit = _GB2312_mbsinit;
	xrl->__mb_cur_max = GB2312_MB_CUR_MAX;
	xrl->__mb_sb_limit = 128;
	return (0);
}

static int
_GB2312_mbsinit(const mbstate_t *ps, locale_t loc __unused)
{

	return (ps == NULL || ((const _GB2312State *)ps)->count == 0);
}

static __inline int
_GB2312_check(const char *str, size_t n)
{
	const u_char *s = (const u_char *)str;

	if (n == 0)
		/* Incomplete multibyte sequence */
		return (-2);
	if (s[0] >= 0xa1 && s[0] <= 0xfe) {
		if (n < 2)
			/* Incomplete multibyte sequence */
			return (-2);
		if (s[1] < 0xa1 || s[1] > 0xfe)
			/* Invalid multibyte sequence */
			return (-1);
		return (2);
	} else if (s[0] & 0x80) {
		/* Invalid multibyte sequence */
		return (-1);
	} 
	return (1);
}

static size_t
_GB2312_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps, locale_t loc __unused)
{
	_GB2312State *gs;
	wchar_t wc;
	int i, len, ocount;
	size_t ncopy;

	gs = (_GB2312State *)ps;

	if (gs->count < 0 || gs->count > sizeof(gs->bytes)) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL) {
		s = "";
		n = 1;
		pwc = NULL;
	}

	ncopy = MIN(MIN(n, GB2312_MB_CUR_MAX), sizeof(gs->bytes) - gs->count);
	strncpy(gs->bytes + gs->count, s, ncopy);
	ocount = gs->count;
	gs->count += ncopy;
	s = (char *)gs->bytes;
	n = gs->count;

	if ((len = _GB2312_check(s, n)) < 0)
		return ((size_t)len);
	wc = 0;
	i = len;
	while (i-- > 0)
		wc = (wc << 8) | (unsigned char)*s++;
	if (pwc != NULL)
		*pwc = wc;
	gs->count = 0;
	return (wc == L'\0' ? 0 : len - ocount);
}

static size_t
_GB2312_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps, locale_t loc __unused)
{
	_GB2312State *gs;

	gs = (_GB2312State *)ps;

	if (gs->count != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);
	if (wc & 0x8000) {
		*s++ = (wc >> 8) & 0xff;
		*s = wc & 0xff;
		return (2);
	}
	*s = wc & 0xff;
	return (1);
}
