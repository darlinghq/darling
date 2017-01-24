/*-
 * Copyright (c) 2005 Poul-Henning Kamp
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
 *
 * $FreeBSD: src/lib/libc/stdio/xprintf_str.c,v 1.1 2005/12/16 18:56:38 phk Exp $
 */

#include <namespace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <assert.h>
#include <wchar.h>
#include "printf.h"
#include "xprintf_private.h"

/*
 * Convert a wide character string argument for the %ls format to a multibyte
 * string representation. If not -1, prec specifies the maximum number of
 * bytes to output, and also means that we can't assume that the wide char.
 * string ends is null-terminated.
 */
static char *
__wcsconv(wchar_t *wcsarg, int prec, locale_t loc)
{
	static const mbstate_t initial;
	mbstate_t mbs;
	char buf[MB_LEN_MAX];
	wchar_t *p;
	char *convbuf;
	size_t clen, nbytes;

	/* Allocate space for the maximum number of bytes we could output. */
	if (prec < 0) {
		p = wcsarg;
		mbs = initial;
		nbytes = wcsrtombs_l(NULL, (const wchar_t **)&p, 0, &mbs, loc);
		if (nbytes == (size_t)-1)
			return (NULL);
	} else {
		/*
		 * Optimisation: if the output precision is small enough,
		 * just allocate enough memory for the maximum instead of
		 * scanning the string.
		 */
		if (prec < 128)
			nbytes = prec;
		else {
			nbytes = 0;
			p = wcsarg;
			mbs = initial;
			for (;;) {
				clen = wcrtomb_l(buf, *p++, &mbs, loc);
				if (clen == 0 || clen == (size_t)-1 ||
				    (int)(nbytes + clen) > prec)
					break;
				nbytes += clen;
			}
		}
	}
	if ((convbuf = MALLOC(nbytes + 1)) == NULL)
		return (NULL);

	/* Fill the output buffer. */
	p = wcsarg;
	mbs = initial;
	if ((nbytes = wcsrtombs_l(convbuf, (const wchar_t **)&p,
	    nbytes, &mbs, loc)) == (size_t)-1) {
		free(convbuf);
		return (NULL);
	}
	convbuf[nbytes] = '\0';
	return (convbuf);
}


/* 's' ---------------------------------------------------------------*/

__private_extern__ int
__printf_arginfo_str(const struct printf_info *pi, size_t n, int *argt)
{

	assert (n > 0);
	if (pi->is_long || pi->spec == 'C')
		argt[0] = PA_WSTRING;
	else
		argt[0] = PA_STRING;
	return (1);
}

__private_extern__ int
__printf_render_str(struct __printf_io *io, const struct printf_info *pi, const void *const *arg)
{
	const char *p;
	wchar_t *wcp;
	char *convbuf;
	int l;

	if (pi->is_long || pi->spec == 'S') {
		wcp = *((wint_t **)arg[0]);
		if (wcp == NULL)
			return (__printf_out(io, pi, "(null)", 6));
		convbuf = __wcsconv(wcp, pi->prec, pi->loc);
		if (convbuf == NULL) 
			return (-1);
		l = __printf_out(io, pi, convbuf, strlen(convbuf));
		__printf_flush(io);
		free(convbuf);
		return (l);
	} 
	p = *((char **)arg[0]);
	if (p == NULL)
		return (__printf_out(io, pi, "(null)", 6));
	l = strlen(p);
	if (pi->prec >= 0 && pi->prec < l)
		l = pi->prec;
	return (__printf_out(io, pi, p, l));
}

/* 'c' ---------------------------------------------------------------*/

__private_extern__ int
__printf_arginfo_chr(const struct printf_info *pi, size_t n, int *argt)
{

	assert (n > 0);
#ifdef VECTORS
	if (pi->is_vec)
		argt[0] = PA_VECTOR;
	else
#endif /* VECTORS */
	if (pi->is_long || pi->spec == 'C')
		argt[0] = PA_WCHAR;
	else
		argt[0] = PA_INT;
	return (1);
}

__private_extern__ int
__printf_render_chr(struct __printf_io *io, const struct printf_info *pi, const void *const *arg)
{
	int i;
	wint_t ii;
	unsigned char c;
	static const mbstate_t initial;		/* XXX: this is bogus! */
	mbstate_t mbs;
	size_t mbseqlen;
	char buf[MB_CUR_MAX_L(pi->loc)];

#ifdef VECTORS
	if (pi->is_vec) return __xprintf_vector(io, pi, arg);
#endif /* VECTORS */

	if (pi->is_long || pi->spec == 'C') {
		int ret;
		ii = *((wint_t *)arg[0]);

		mbs = initial;
		mbseqlen = wcrtomb_l(buf, (wchar_t)ii, &mbs, pi->loc);
		if (mbseqlen == (size_t) -1)
			return (-1);
		ret = __printf_out(io, pi, buf, mbseqlen);
		__printf_flush(io);
		return ret;
	}
	i = *((int *)arg[0]);
	c = i;
	i = __printf_out(io, pi, &c, 1);
	__printf_flush(io);
	return (i);
}
