/*
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
static char sccsid[] = "@(#)tempnam.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdio/tempnam.c,v 1.11 2007/01/09 00:28:07 imp Exp $");

#include <sys/param.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <paths.h>

__warn_references(tempnam,
    "warning: tempnam() possibly used unsafely; consider using mkstemp()");

extern char *_mktemp(char *);

char *
tempnam(dir, pfx)
	const char *dir, *pfx;
{
	int sverrno;
	char *f, *name;

	if (!(name = malloc(MAXPATHLEN))) {
		return(NULL);
	}

	if (!pfx)
		pfx = "tmp.";

#if !__DARWIN_UNIX03
	if (issetugid() == 0 && (f = getenv("TMPDIR"))) {
		(void)snprintf(name, MAXPATHLEN, "%s%s%sXXXXXX", f,
		    *(f + strlen(f) - 1) == '/'? "": "/", pfx);
		if ((f = _mktemp(name))) {
			return(f);
		}
	}
#endif /* !__DARWIN_UNIX03 */
	if ((f = (char *)dir)) {
#if __DARWIN_UNIX03
	    if (access(dir, W_OK) == 0) {
#endif /* __DARWIN_UNIX03 */
		(void)snprintf(name, MAXPATHLEN, "%s%s%sXXXXXX", f,
		    *(f + strlen(f) - 1) == '/'? "": "/", pfx);
		if ((f = _mktemp(name))) {
			return(f);
		}
#if __DARWIN_UNIX03
	    }
#endif /* __DARWIN_UNIX03 */
	}

	f = P_tmpdir;
#if __DARWIN_UNIX03
	if (access(f, W_OK) == 0) {	/* directory accessible? */
#endif /* __DARWIN_UNIX03 */
	(void)snprintf(name, MAXPATHLEN, "%s%sXXXXXX", f, pfx);
	if ((f = _mktemp(name))) {
		return(f);
	}

#if __DARWIN_UNIX03
	}
	if (issetugid() == 0 && (f = getenv("TMPDIR")) && access(f, W_OK) == 0) {
		(void)snprintf(name, MAXPATHLEN, "%s%s%sXXXXXX", f,
		    *(f + strlen(f) - 1) == '/'? "": "/", pfx);
		if ((f = _mktemp(name))) {
			return(f);
		}
	}
#endif /* __DARWIN_UNIX03 */
	f = _PATH_TMP;
	(void)snprintf(name, MAXPATHLEN, "%s%sXXXXXX", f, pfx);
	if ((f = _mktemp(name))) {
		return(f);
	}

	sverrno = errno;
	free(name);
	errno = sverrno;
	return(NULL);
}
