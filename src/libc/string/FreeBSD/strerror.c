/*-
 * Copyright (c) 1988, 1993
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
static char sccsid[] = "@(#)strerror.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/string/strerror.c,v 1.16 2007/01/09 00:28:12 imp Exp $");

#if defined(NLS)
#include <nl_types.h>
#endif

#include <limits.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define	UPREFIX		"Unknown error"

/*
 * Define a buffer size big enough to describe a 64-bit signed integer
 * converted to ASCII decimal (19 bytes), with an optional leading sign
 * (1 byte); finally, we get the prefix, delimiter (": ") and a trailing
 * NUL from UPREFIX.
 */
#define	EBUFSIZE	(20 + 2 + sizeof(UPREFIX))

#ifndef BUILDING_VARIANT
/*
 * Doing this by hand instead of linking with stdio(3) avoids bloat for
 * statically linked binaries.
 */
__private_extern__ void
__errstr(int num, char *uprefix, char *buf, size_t len)
{
	char *t;
	unsigned int uerr;
	char tmp[EBUFSIZE];

	t = tmp + sizeof(tmp);
	*--t = '\0';
	uerr = (num >= 0) ? num : -num;
	do {
		*--t = "0123456789"[uerr % 10];
	} while (uerr /= 10);
	if (num < 0)
		*--t = '-';
	*--t = ' ';
	*--t = ':';
	strlcpy(buf, uprefix, len);
	strlcat(buf, t, len);
}

int
strerror_r(int errnum, char *strerrbuf, size_t buflen)
{
	int retval = 0;
#if defined(NLS)
	int saved_errno = errno;
	nl_catd catd;
	catd = catopen("libc", NL_CAT_LOCALE);
#endif

	if (errnum < 0 || errnum >= sys_nerr) {
		__errstr(errnum,
#if defined(NLS)
			catgets(catd, 1, 0xffff, UPREFIX),
#else
			UPREFIX,
#endif
			strerrbuf, buflen);
		retval = EINVAL;
	} else {
		if (strlcpy(strerrbuf,
#if defined(NLS)
			catgets(catd, 1, errnum, sys_errlist[errnum]),
#else
			sys_errlist[errnum],
#endif
			buflen) >= buflen)
		retval = ERANGE;
	}

#if defined(NLS)
	catclose(catd);
	errno = saved_errno;
#endif

	return (retval);
}

__private_extern__ char *__strerror_ebuf = NULL;
#else /* BUILDING_VARIANT */
__private_extern__ void __errstr(int, char *, size_t);

extern char *__strerror_ebuf;
#endif /* !BUILDING_VARIANT */

char *
strerror(int num)
{
	// Dynamically allocate a big buffer to receive the text then shrink it
	// down to the actual size needed.
	size_t ebufsiz = NL_TEXTMAX;

	if (__strerror_ebuf == NULL) {
		__strerror_ebuf = calloc(1, ebufsiz);
		if (__strerror_ebuf == NULL) {
			return NULL;
		}
	}
	
	if (strerror_r(num, __strerror_ebuf, ebufsiz) != 0) {
#if !__DARWIN_UNIX03
		errno = EINVAL;
#endif
	}
	return __strerror_ebuf;
}
