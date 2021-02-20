/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
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
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)makebuf.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "un-namespace.h"

#include "libc_private.h"
#include "local.h"
#include <xlocale/_stdio.h>
#include <xlocale/_stdlib.h>
#include <os/once_private.h>

#ifdef FEATURE_SMALL_STDIOBUF
# define MAXBUFSIZE	(1 << 12)
#else
# define MAXBUFSIZE	(1 << 16)
#endif

#define TTYBUFSIZE	4096
#define MAXEVPSIZE 16

static char __fallback_evp[MAXEVPSIZE];
static char __stdout_evp[MAXEVPSIZE];

static void
__loadevp(const char* key, char destination[MAXEVPSIZE])
{
	char* evp = getenv(key);
	if (evp != NULL) {
		strlcpy(destination, evp, MAXEVPSIZE);
	} else {
		destination[0] = '\0';
	}
}

static void
__evpinit(void* __unused unused)
{
	__loadevp("STDBUF", __fallback_evp);
	__loadevp("STDBUF1", __stdout_evp);
}

static char*
__getevp(int fd)
{
	static os_once_t predicate;
	os_once(&predicate, NULL, __evpinit);

	if (fd == STDOUT_FILENO && __stdout_evp[0] != '\0') {
		return __stdout_evp;
	} else if (__fallback_evp[0] != '\0') {
		return __fallback_evp;
	} else {
		return NULL;
	}

}

/*
 * Internal routine to determine environment override buffering for a file.
 *
 * Sections of the below taken from NetBSD's version of this file under the same license.
 */
static int
__senvbuf(FILE *fp, size_t *bufsize, int *couldbetty)
{
	char* evp;
	char* end;
	int flags;
	long size;

	flags = 0; // Default to fully buffered

	if ((evp = __getevp(fp->_file)) == NULL) {
		return flags;
	}

	// Look at the first character only to determine buffering mode
	switch (*evp) {
		case 'u':
		case 'U':
			flags |= __SNBF;
			break;
		case 'l':
		case 'L':
			flags |= __SLBF;
			break;
		case 'f':
		case 'F':
			// Default flags is fully buffered
			break;
		default:
			// Unexpected buffering mode, use default fully buffered
			return flags;
	}
	// User specified envrionment defaults have higher priority than tty defaults
	*couldbetty = 0;

	// Advance the envrionment variable pointer, so we can attempt to parse the number
	evp++;
	if (!isdigit(*evp)) {
		return flags; // No number found, this protects us from negative size values
	}

	size = strtol_l(evp, &end, 10, LC_C_LOCALE);
	if (*end != '\0') {
		return flags;
	}

	if (size <= 0) {
		return __SNBF; // Override with unbuffered if the buffer size is 0
	}

	// We had a non zero buffer, cap it and return the flags;
	*bufsize = size > MAXBUFSIZE ? MAXBUFSIZE : size;
	return flags;
}

/*
 * Allocate a file buffer, or switch to unbuffered I/O.
 * Per the ANSI C standard, ALL tty devices default to line buffered.
 *
 * As a side effect, we set __SOPT or __SNPT (en/dis-able fseek
 * optimisation) right after the _fstat() that finds the buffer size.
 */
void
__smakebuf(FILE *fp)
{
	void *p;
	int flags;
	size_t size;
	int couldbetty;

	if (fp->_flags & __SNBF) {
		fp->_bf._base = fp->_p = fp->_nbuf;
		fp->_bf._size = 1;
		return;
	}
	flags = __swhatbuf(fp, &size, &couldbetty);
	if (fp->_file >= 0) {
		flags |= __senvbuf(fp, &size, &couldbetty);

		if (flags & __SNBF) {
			fp->_flags |= __SNBF;
			fp->_bf._base = fp->_p = fp->_nbuf;
			fp->_bf._size = 1;
			return;
		}
	}

	if (couldbetty && isatty(fp->_file)) {
		flags |= __SLBF;
		/* st_blksize for ttys is 128K, so make it more reasonable */
		if (size > TTYBUFSIZE)
			fp->_blksize = size = TTYBUFSIZE;
	}
	if ((p = malloc(size)) == NULL) {
		fp->_flags |= __SNBF;
		fp->_bf._base = fp->_p = fp->_nbuf;
		fp->_bf._size = 1;
		return;
	}
	__cleanup = _cleanup;
	flags |= __SMBF;
	fp->_bf._base = fp->_p = p;
	fp->_bf._size = size;
	fp->_flags |= flags;
}

/*
 * Internal routine to determine `proper' buffering for a file.
 */
int
__swhatbuf(FILE *fp, size_t *bufsize, int *couldbetty)
{
	struct stat st;

	if (fp->_file < 0 || _fstat(fp->_file, &st) < 0) {
		*couldbetty = 0;
		*bufsize = BUFSIZ;
		return (__SNPT);
	}

	/* could be a tty iff it is a character device */
	*couldbetty = (st.st_mode & S_IFMT) == S_IFCHR;
	if (st.st_blksize <= 0) {
		*bufsize = BUFSIZ;
		return (__SNPT);
	}

	/*
	 * Optimise fseek() only if it is a regular file.  (The test for
	 * __sseek is mainly paranoia.)  It is safe to set _blksize
	 * unconditionally; it will only be used if __SOPT is also set.
	 */
	fp->_blksize = *bufsize = st.st_blksize > MAXBUFSIZE ? MAXBUFSIZE : st.st_blksize;
	return ((st.st_mode & S_IFMT) == S_IFREG && fp->_seek == __sseek ?
	    __SOPT : __SNPT);
}
