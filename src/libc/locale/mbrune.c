/*-
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
static char sccsid[] = "@(#)mbrune.c	8.1 (Berkeley) 6/27/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/locale/mbrune.c,v 1.3 2002/09/18 06:11:21 tjr Exp $");

#include <limits.h>
#include <rune.h>
#include <stddef.h>
#include <string.h>
#include "runedepreciated.h"

char *
mbrune(string, c)
	const char *string;
	rune_t c;
{
	char const *result;
	rune_t r;
	static int warn_depreciated = 1;

	if (warn_depreciated) {
		warn_depreciated = 0;
		fprintf(stderr, __rune_depreciated_msg, "mbrune");
	}

	while ((r = __sgetrune(string, MB_LEN_MAX, &result))) {
		if (r == c)
			return ((char *)string);
		string = result == string ? string + 1 : result;
	}

	return (c == *string ? (char *)string : NULL);
}

char *
mbrrune(string, c)
	const char *string;
	rune_t c;
{
	const char *last = 0;
	char const *result;
	rune_t  r;
	static int warn_depreciated = 1;

	if (warn_depreciated) {
		warn_depreciated = 0;
		fprintf(stderr, __rune_depreciated_msg, "mbrrune");
	}

	while ((r = __sgetrune(string, MB_LEN_MAX, &result))) {
		if (r == c)
			last = string;
		string = result == string ? string + 1 : result;
	}
	return (c == *string ? (char *)string : (char *)last);
}

char *
mbmb(string, pattern)
	const char *string;
	char *pattern;
{
	rune_t first, r;
	size_t plen, slen;
	char const *result;
	static int warn_depreciated = 1;

	if (warn_depreciated) {
		warn_depreciated = 0;
		fprintf(stderr, __rune_depreciated_msg, "mbmb");
	}

	plen = strlen(pattern);
	slen = strlen(string);
	if (plen > slen)
		return (0);

	first = __sgetrune(pattern, plen, &result);
	if (result == string)
		return (0);

	while (slen >= plen && (r = __sgetrune(string, slen, &result))) {
		if (r == first) {
			if (strncmp(string, pattern, slen) == 0)
				return ((char *) string);
		}
		if (result == string) {
			--slen;
			++string;
		} else {
			slen -= result - string;
			string = result;
		}
	}
	return (0);
}
