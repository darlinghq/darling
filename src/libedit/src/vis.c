/*	$NetBSD: vis.c,v 1.44 2011/03/12 19:52:48 christos Exp $	*/

/*-
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
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

/*-
 * Copyright (c) 1999, 2005 The NetBSD Foundation, Inc.
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
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if defined(LIBC_SCCS) && !defined(lint)
__RCSID("$NetBSD: vis.c,v 1.44 2011/03/12 19:52:48 christos Exp $");
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>

#include <assert.h>
#include <vis.h>
#include <errno.h>
#include <stdlib.h>

#ifdef __weak_alias
__weak_alias(strvisx,_strvisx)
#endif

#if !HAVE_VIS || !HAVE_SVIS
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static char *do_svis(char *, size_t *, int, int, int, const char *);

#undef BELL
#define BELL '\a'

#define isoctal(c)	(((u_char)(c)) >= '0' && ((u_char)(c)) <= '7')
#define iswhite(c)	(c == ' ' || c == '\t' || c == '\n')
#define issafe(c)	(c == '\b' || c == BELL || c == '\r')
#define xtoa(c)		"0123456789abcdef"[c]
#define XTOA(c)		"0123456789ABCDEF"[c]

#define MAXEXTRAS	5

#define MAKEEXTRALIST(flag, extra, orig_str)				      \
do {									      \
	const char *orig = orig_str;					      \
	const char *o = orig;						      \
	char *e;							      \
	while (*o++)							      \
		continue;						      \
	extra = malloc((size_t)((o - orig) + MAXEXTRAS));		      \
	if (!extra) break;						      \
	for (o = orig, e = extra; (*e++ = *o++) != '\0';)		      \
		continue;						      \
	e--;								      \
	if (flag & VIS_SP) *e++ = ' ';					      \
	if (flag & VIS_TAB) *e++ = '\t';				      \
	if (flag & VIS_NL) *e++ = '\n';					      \
	if ((flag & VIS_NOSLASH) == 0) *e++ = '\\';			      \
	*e = '\0';							      \
} while (/*CONSTCOND*/0)

/*
 * This is do_hvis, for HTTP style (RFC 1808)
 */
static char *
do_hvis(char *dst, size_t *dlen, int c, int flag, int nextc, const char *extra)
{

	if ((isascii(c) && isalnum(c))
	    /* safe */
	    || c == '$' || c == '-' || c == '_' || c == '.' || c == '+'
	    /* extra */
	    || c == '!' || c == '*' || c == '\'' || c == '(' || c == ')'
	    || c == ',') {
		dst = do_svis(dst, dlen, c, flag, nextc, extra);
	} else {
		if (dlen) {
			if (*dlen < 3)
				return NULL;
			*dlen -= 3;
		}
		*dst++ = '%';
		*dst++ = xtoa(((unsigned int)c >> 4) & 0xf);
		*dst++ = xtoa((unsigned int)c & 0xf);
	}

	return dst;
}

/*
 * This is do_mvis, for Quoted-Printable MIME (RFC 2045)
 * NB: No handling of long lines or CRLF.
 */
static char *
do_mvis(char *dst, size_t *dlen, int c, int flag, int nextc, const char *extra)
{
	if ((c != '\n') &&
	    /* Space at the end of the line */
	    ((isspace(c) && (nextc == '\r' || nextc == '\n')) ||
	    /* Out of range */
	    (!isspace(c) && (c < 33 || (c > 60 && c < 62) || c > 126)) ||
	    /* Specific char to be escaped */ 
	    strchr("#$@[\\]^`{|}~", c) != NULL)) {
		if (dlen) {
			if (*dlen < 3)
				return NULL;
			*dlen -= 3;
		}
		*dst++ = '=';
		*dst++ = XTOA(((unsigned int)c >> 4) & 0xf);
		*dst++ = XTOA((unsigned int)c & 0xf);
	} else {
		dst = do_svis(dst, dlen, c, flag, nextc, extra);
	}
	return dst;
}

/*
 * This is do_vis, the central code of vis.
 * dst:	      Pointer to the destination buffer
 * c:	      Character to encode
 * flag:      Flag word
 * nextc:     The character following 'c'
 * extra:     Pointer to the list of extra characters to be
 *	      backslash-protected.
 */
static char *
do_svis(char *dst, size_t *dlen, int c, int flag, int nextc, const char *extra)
{
	int isextra;
	size_t odlen = dlen ? *dlen : 0;

	isextra = strchr(extra, c) != NULL;
#define HAVE(x) \
	do { \
		if (dlen) { \
			if (*dlen < (x)) \
				goto out; \
			*dlen -= (x); \
		} \
	} while (/*CONSTCOND*/0)
	if (!isextra && isascii(c) && (isgraph(c) || iswhite(c) ||
	    ((flag & VIS_SAFE) && issafe(c)))) {
		HAVE(1);
		*dst++ = c;
		return dst;
	}
	if (flag & VIS_CSTYLE) {
		HAVE(2);
		switch (c) {
		case '\n':
			*dst++ = '\\'; *dst++ = 'n';
			return dst;
		case '\r':
			*dst++ = '\\'; *dst++ = 'r';
			return dst;
		case '\b':
			*dst++ = '\\'; *dst++ = 'b';
			return dst;
		case BELL:
			*dst++ = '\\'; *dst++ = 'a';
			return dst;
		case '\v':
			*dst++ = '\\'; *dst++ = 'v';
			return dst;
		case '\t':
			*dst++ = '\\'; *dst++ = 't';
			return dst;
		case '\f':
			*dst++ = '\\'; *dst++ = 'f';
			return dst;
		case ' ':
			*dst++ = '\\'; *dst++ = 's';
			return dst;
		case '\0':
			*dst++ = '\\'; *dst++ = '0';
			if (isoctal(nextc)) {
				HAVE(2);
				*dst++ = '0';
				*dst++ = '0';
			}
			return dst;
		default:
			if (isgraph(c)) {
				*dst++ = '\\'; *dst++ = c;
				return dst;
			}
			if (dlen)
				*dlen = odlen;
		}
	}
	if (isextra || ((c & 0177) == ' ') || (flag & VIS_OCTAL)) {
		HAVE(4);
		*dst++ = '\\';
		*dst++ = (u_char)(((u_int32_t)(u_char)c >> 6) & 03) + '0';
		*dst++ = (u_char)(((u_int32_t)(u_char)c >> 3) & 07) + '0';
		*dst++ =			     (c	      & 07) + '0';
	} else {
		if ((flag & VIS_NOSLASH) == 0) {
			HAVE(1);
			*dst++ = '\\';
		}

		if (c & 0200) {
			HAVE(1);
			c &= 0177; *dst++ = 'M';
		}

		if (iscntrl(c)) {
			HAVE(2);
			*dst++ = '^';
			if (c == 0177)
				*dst++ = '?';
			else
				*dst++ = c + '@';
		} else {
			HAVE(2);
			*dst++ = '-'; *dst++ = c;
		}
	}
	return dst;
out:
	*dlen = odlen;
	return NULL;
}

typedef char *(*visfun_t)(char *, size_t *, int, int, int, const char *);

/*
 * Return the appropriate encoding function depending on the flags given.
 */
static visfun_t
getvisfun(int flag)
{
	if (flag & VIS_HTTPSTYLE)
		return do_hvis;
	if (flag & VIS_MIMESTYLE)
		return do_mvis;
	return do_svis;
}

/*
 * isnvis - visually encode characters, also encoding the characters
 *	  pointed to by `extra'
 */
static char *
isnvis(char *dst, size_t *dlen, int c, int flag, int nextc, const char *extra)
{
	char *nextra = NULL;
	visfun_t f;

	_DIAGASSERT(dst != NULL);
	_DIAGASSERT(extra != NULL);
	MAKEEXTRALIST(flag, nextra, extra);
	if (!nextra) {
		if (dlen && *dlen == 0) {
			errno = ENOSPC;
			return NULL;
		}
		*dst = '\0';		/* can't create nextra, return "" */
		return dst;
	}
	f = getvisfun(flag);
	dst = (*f)(dst, dlen, c, flag, nextc, nextra);
	free(nextra);
	if (dst == NULL || (dlen && *dlen == 0)) {
		errno = ENOSPC;
		return NULL;
	}
	*dst = '\0';
	return dst;
}

char *
svis(char *dst, int c, int flag, int nextc, const char *extra)
{
	return isnvis(dst, NULL, c, flag, nextc, extra);
}

char *
snvis(char *dst, size_t dlen, int c, int flag, int nextc, const char *extra)
{
	return isnvis(dst, &dlen, c, flag, nextc, extra);
}


/*
 * strsvis, strsvisx - visually encode characters from src into dst
 *
 *	Extra is a pointer to a \0-terminated list of characters to
 *	be encoded, too. These functions are useful e. g. to
 *	encode strings in such a way so that they are not interpreted
 *	by a shell.
 *
 *	Dst must be 4 times the size of src to account for possible
 *	expansion.  The length of dst, not including the trailing NULL,
 *	is returned.
 *
 *	Strsvisx encodes exactly len bytes from src into dst.
 *	This is useful for encoding a block of data.
 */
static int
istrsnvis(char *dst, size_t *dlen, const char *csrc, int flag, const char *extra)
{
	int c;
	char *start;
	char *nextra = NULL;
	const unsigned char *src = (const unsigned char *)csrc;
	visfun_t f;

	_DIAGASSERT(dst != NULL);
	_DIAGASSERT(src != NULL);
	_DIAGASSERT(extra != NULL);
	MAKEEXTRALIST(flag, nextra, extra);
	if (!nextra) {
		*dst = '\0';		/* can't create nextra, return "" */
		return 0;
	}
	f = getvisfun(flag);
	for (start = dst; (c = *src++) != '\0'; /* empty */) {
		dst = (*f)(dst, dlen, c, flag, *src, nextra);
		if (dst == NULL) {
			errno = ENOSPC;
			return -1;
		}
	}
	free(nextra);
	if (dlen && *dlen == 0) {
		errno = ENOSPC;
		return -1;
	}
	*dst = '\0';
	return (int)(dst - start);
}

int
strsvis(char *dst, const char *csrc, int flag, const char *extra)
{
	return istrsnvis(dst, NULL, csrc, flag, extra);
}

int
strsnvis(char *dst, size_t dlen, const char *csrc, int flag, const char *extra)
{
	return istrsnvis(dst, &dlen, csrc, flag, extra);
}

static int
istrsnvisx(char *dst, size_t *dlen, const char *csrc, size_t len, int flag,
    const char *extra)
{
	unsigned char c;
	char *start;
	char *nextra = NULL;
	const unsigned char *src = (const unsigned char *)csrc;
	visfun_t f;

	_DIAGASSERT(dst != NULL);
	_DIAGASSERT(src != NULL);
	_DIAGASSERT(extra != NULL);
	MAKEEXTRALIST(flag, nextra, extra);
	if (! nextra) {
		if (dlen && *dlen == 0) {
			errno = ENOSPC;
			return -1;
		}
		*dst = '\0';		/* can't create nextra, return "" */
		return 0;
	}

	f = getvisfun(flag);
	for (start = dst; len > 0; len--) {
		c = *src++;
		dst = (*f)(dst, dlen, c, flag, len > 1 ? *src : '\0', nextra);
		if (dst == NULL) {
			errno = ENOSPC;
			return -1;
		}
	}
	free(nextra);
	if (dlen && *dlen == 0) {
		errno = ENOSPC;
		return -1;
	}
	*dst = '\0';
	return (int)(dst - start);
}

int
strsvisx(char *dst, const char *csrc, size_t len, int flag, const char *extra)
{
	return istrsnvisx(dst, NULL, csrc, len, flag, extra);
}

int
strsnvisx(char *dst, size_t dlen, const char *csrc, size_t len, int flag,
    const char *extra)
{
	return istrsnvisx(dst, &dlen, csrc, len, flag, extra);
}
#endif

#if !HAVE_VIS
/*
 * vis - visually encode characters
 */
static char *
invis(char *dst, size_t *dlen, int c, int flag, int nextc)
{
	char *extra = NULL;
	unsigned char uc = (unsigned char)c;
	visfun_t f;

	_DIAGASSERT(dst != NULL);

	MAKEEXTRALIST(flag, extra, "");
	if (! extra) {
		if (dlen && *dlen == 0) {
			errno = ENOSPC;
			return NULL;
		}
		*dst = '\0';		/* can't create extra, return "" */
		return dst;
	}
	f = getvisfun(flag);
	dst = (*f)(dst, dlen, uc, flag, nextc, extra);
	free(extra);
	if (dst == NULL || (dlen && *dlen == 0)) {
		errno = ENOSPC;
		return NULL;
	}
	*dst = '\0';
	return dst;
}

char *
vis(char *dst, int c, int flag, int nextc)
{
	return invis(dst, NULL, c, flag, nextc);
}

char *
nvis(char *dst, size_t dlen, int c, int flag, int nextc)
{
	return invis(dst, &dlen, c, flag, nextc);
}


/*
 * strvis, strvisx - visually encode characters from src into dst
 *
 *	Dst must be 4 times the size of src to account for possible
 *	expansion.  The length of dst, not including the trailing NULL,
 *	is returned.
 *
 *	Strvisx encodes exactly len bytes from src into dst.
 *	This is useful for encoding a block of data.
 */
static int
istrnvis(char *dst, size_t *dlen, const char *src, int flag)
{
	char *extra = NULL;
	int rv;

	MAKEEXTRALIST(flag, extra, "");
	if (!extra) {
		if (dlen && *dlen == 0) {
			errno = ENOSPC;
			return -1;
		}
		*dst = '\0';		/* can't create extra, return "" */
		return 0;
	}
	rv = istrsnvis(dst, dlen, src, flag, extra);
	free(extra);
	return rv;
}

int
strvis(char *dst, const char *src, int flag)
{
	return istrnvis(dst, NULL, src, flag);
}

int
strnvis(char *dst, size_t dlen, const char *src, int flag)
{
	return istrnvis(dst, &dlen, src, flag);
}

static int
istrnvisx(char *dst, size_t *dlen, const char *src, size_t len, int flag)
{
	char *extra = NULL;
	int rv;

	MAKEEXTRALIST(flag, extra, "");
	if (!extra) {
		if (dlen && *dlen == 0) {
			errno = ENOSPC;
			return -1;
		}
		*dst = '\0';		/* can't create extra, return "" */
		return 0;
	}
	rv = istrsnvisx(dst, dlen, src, len, flag, extra);
	free(extra);
	return rv;
}

int
strvisx(char *dst, const char *src, size_t len, int flag)
{
	return istrnvisx(dst, NULL, src, len, flag);
}

int
strnvisx(char *dst, size_t dlen, const char *src, size_t len, int flag)
{
	return istrnvisx(dst, &dlen, src, len, flag);
}

#endif
