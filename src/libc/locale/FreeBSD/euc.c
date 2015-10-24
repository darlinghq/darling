/*-
 * Copyright (c) 2002-2004 Tim J. Robbins. All rights reserved.
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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
static char sccsid[] = "@(#)euc.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/param.h>
__FBSDID("$FreeBSD: src/lib/libc/locale/euc.c,v 1.22 2007/10/13 16:28:21 ache Exp $");

#include "xlocale_private.h"

#include <errno.h>
#include <limits.h>
#include <runetype.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "mblocal.h"

static size_t	_EUC_mbrtowc(wchar_t * __restrict, const char * __restrict,
		    size_t, mbstate_t * __restrict, locale_t);
static int	_EUC_mbsinit(const mbstate_t *, locale_t);
static size_t	_EUC_wcrtomb(char * __restrict, wchar_t,
		    mbstate_t * __restrict, locale_t);

typedef struct {
	int	count[4];
	wchar_t	bits[4];
	wchar_t	mask;
} _EucInfo;

typedef struct {
	wchar_t	ch;
	int	set;
	int	want;
} _EucState;

/* This will be called by the XL_RELEASE() macro to free the extra storage */
static void
_EUC_free_extra(struct __xlocale_st_runelocale *xrl)
{
	free(xrl->_CurrentRuneLocale.__variable);
}

__private_extern__ int
_EUC_init(struct __xlocale_st_runelocale *xrl)
{
	_EucInfo *ei;
	int x, new__mb_cur_max;
	char *v, *e;
	_RuneLocale *rl = &xrl->_CurrentRuneLocale;

	if (rl->__variable == NULL)
		return (EFTYPE);

	v = (char *)rl->__variable;

	while (*v == ' ' || *v == '\t')
		++v;

	if ((ei = malloc(sizeof(_EucInfo))) == NULL)
		return (errno == 0 ? ENOMEM : errno);

	new__mb_cur_max = 0;
	for (x = 0; x < 4; ++x) {
		ei->count[x] = (int)strtol(v, &e, 0);
		if (v == e || !(v = e)) {
			free(ei);
			return (EFTYPE);
		}
		if (new__mb_cur_max < ei->count[x])
			new__mb_cur_max = ei->count[x];
		while (*v == ' ' || *v == '\t')
			++v;
		ei->bits[x] = (int)strtol(v, &e, 0);
		if (v == e || !(v = e)) {
			free(ei);
			return (EFTYPE);
		}
		while (*v == ' ' || *v == '\t')
			++v;
	}
	ei->mask = (int)strtol(v, &e, 0);
	if (v == e || !(v = e)) {
		free(ei);
		return (EFTYPE);
	}
	rl->__variable = ei;
	rl->__variable_len = sizeof(_EucInfo);
 	xrl->__mb_cur_max = new__mb_cur_max;
 	xrl->__mbrtowc = _EUC_mbrtowc;
 	xrl->__wcrtomb = _EUC_wcrtomb;
 	xrl->__mbsinit = _EUC_mbsinit;
	xrl->__mb_sb_limit = 256;
 	xrl->__free_extra = (__free_extra_t)_EUC_free_extra;
	return (0);
}

static int
_EUC_mbsinit(const mbstate_t *ps, locale_t loc __unused)
{

	return (ps == NULL || ((const _EucState *)ps)->want == 0);
}

#define	_SS2	0x008e
#define	_SS3	0x008f

#define	GR_BITS	0x80808080 /* XXX: to be fixed */

static __inline int
_euc_set(u_int c)
{

	c &= 0xff;
	return ((c & 0x80) ? c == _SS3 ? 3 : c == _SS2 ? 2 : 1 : 0);
}

static size_t
_EUC_mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
    mbstate_t * __restrict ps, locale_t loc)
{
	_EucState *es;
	int i, set, want;
	wchar_t wc;
	const char *os;
	struct __xlocale_st_runelocale *rl = loc->__lc_ctype;
	_EucInfo *CEI = (_EucInfo *)rl->_CurrentRuneLocale.__variable;

	es = (_EucState *)ps;

	if (es->want < 0 || es->want > rl->__mb_cur_max || es->set < 0 ||
	    es->set > 3) {
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

	os = s;

	if (es->want == 0) {
		want = CEI->count[set = _euc_set(*s)];
		if (set == 2 || set == 3) {
			--want;
			if (--n == 0) {
				/* Incomplete multibyte sequence */
				es->set = set;
				es->want = want;
				es->ch = 0;
				return ((size_t)-2);
			}
			++s;
			if (*s == '\0') {
				errno = EILSEQ;
				return ((size_t)-1);
			}
		}
		wc = (unsigned char)*s++;
	} else {
		set = es->set;
		want = es->want;
		wc = es->ch;
	}
	for (i = (es->want == 0) ? 1 : 0; i < MIN(want, n); i++) {
		if (*s == '\0') {
			errno = EILSEQ;
			return ((size_t)-1);
		}
		wc = (wc << 8) | (unsigned char)*s++;
	}
	if (i < want) {
		/* Incomplete multibyte sequence */
		es->set = set;
		es->want = want - i;
		es->ch = wc;
		return ((size_t)-2);
	}
	wc = (wc & ~CEI->mask) | CEI->bits[set];
	if (pwc != NULL)
		*pwc = wc;
	es->want = 0;
	return (wc == L'\0' ? 0 : s - os);
}

static size_t
_EUC_wcrtomb(char * __restrict s, wchar_t wc, mbstate_t * __restrict ps, locale_t loc)
{
	_EucState *es;
	wchar_t m, nm;
	int i, len;
	_EucInfo *CEI = (_EucInfo *)loc->__lc_ctype->_CurrentRuneLocale.__variable;

	es = (_EucState *)ps;

	if (es->want != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);

	m = wc & CEI->mask;
	nm = wc & ~m;

	if (m == CEI->bits[1]) {
CodeSet1:
		/* Codeset 1: The first byte must have 0x80 in it. */
		i = len = CEI->count[1];
		while (i-- > 0)
			*s++ = (nm >> (i << 3)) | 0x80;
	} else {
		if (m == CEI->bits[0])
			i = len = CEI->count[0];
		else if (m == CEI->bits[2]) {
			i = len = CEI->count[2];
			*s++ = _SS2;
			--i;
			/* SS2 designates G2 into GR */
			nm |= GR_BITS;
		} else if (m == CEI->bits[3]) {
			i = len = CEI->count[3];
			*s++ = _SS3;
			--i;
			/* SS3 designates G3 into GR */
			nm |= GR_BITS;
		} else
			goto CodeSet1;	/* Bletch */
		while (i-- > 0)
			*s++ = (nm >> (i << 3)) & 0xff;
	}
	return (len);
}
