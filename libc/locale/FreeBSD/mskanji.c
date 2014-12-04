/*
 * Copyright (c) 2002-2004 Tim J. Robbins. All rights reserved.
 *
 *    ja_JP.SJIS locale table for BSD4.4/rune
 *    version 1.0
 *    (C) Sin'ichiro MIYATANI / Phase One, Inc
 *    May 12, 1995
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Phase One, Inc.
 * 4. The name of Phase One, Inc. may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
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
static char sccsid[] = "@(#)mskanji.c	1.0 (Phase One) 5/5/95";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/locale/mskanji.c,v 1.18 2007/10/13 16:28:22 ache Exp $");

#include "xlocale_private.h"

#include <sys/types.h>
#include <errno.h>
#include <runetype.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "mblocal.h"

static size_t	_MSKanji_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict, locale_t);
static int	_MSKanji_mbsinit(const mbstate_t *, locale_t);
static size_t	_MSKanji_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict, locale_t);

typedef struct {
	wchar_t	ch;
} _MSKanjiState;

__private_extern__ int
_MSKanji_init(struct __xlocale_st_runelocale *xrl)
{

	xrl->__mbrtowc = _MSKanji_mbrtowc;
	xrl->__wcrtomb = _MSKanji_wcrtomb;
	xrl->__mbsinit = _MSKanji_mbsinit;
	xrl->__mb_cur_max = 2;
	xrl->__mb_sb_limit = 256;
	return (0);
}

static int
_MSKanji_mbsinit(const mbstate_t *ps, locale_t loc __unused)
{

	return (ps == NULL || ((const _MSKanjiState *)ps)->ch == 0);
}

static size_t
_MSKanji_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps, locale_t loc __unused)
{
	_MSKanjiState *ms;
	wchar_t wc;

	ms = (_MSKanjiState *)ps;

	if ((ms->ch & ~0xFF) != 0) {
		/* Bad conversion state. */
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL) {
		s = "";
		n = 1;
		pwc = NULL;
	}

	if (n == 0)
		/* Incomplete multibyte sequence */
		return ((size_t)-2);

	if (ms->ch != 0) {
		if (*s == '\0') {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wc = (ms->ch << 8) | (*s & 0xFF);
		if (pwc != NULL)
			*pwc = wc;
		ms->ch = 0;
		return (1);
	}
	wc = *s++ & 0xff;
	if ((wc > 0x80 && wc < 0xa0) || (wc >= 0xe0 && wc < 0xfd)) {
		if (n < 2) {
			/* Incomplete multibyte sequence */
			ms->ch = wc;
			return ((size_t)-2);
		}
		if (*s == '\0') {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wc = (wc << 8) | (*s++ & 0xff);
		if (pwc != NULL)
			*pwc = wc;
		return (2);
	} else {
		if (pwc != NULL)
			*pwc = wc;
		return (wc == L'\0' ? 0 : 1);
	}
}

static size_t
_MSKanji_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps, locale_t loc __unused)
{
	_MSKanjiState *ms;
	int len, i;

	ms = (_MSKanjiState *)ps;

	if (ms->ch != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);
	len = (wc > 0x100) ? 2 : 1;
	for (i = len; i-- > 0; )
		*s++ = wc >> (i << 3);
	return (len);
}
