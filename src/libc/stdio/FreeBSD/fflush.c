/*-
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
static char sccsid[] = "@(#)fflush.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdio/fflush.c,v 1.14 2007/01/09 00:28:06 imp Exp $");

#include "namespace.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "local.h"

static int	sflush_locked(FILE *);

/*
 * Flush a single file, or (if fp is NULL) all files.
 * MT-safe version
 */
int
fflush(FILE *fp)
{
	int retval = 0;

	if (fp == NULL) {
		return (_fwalk(sflush_locked));
	}

	FLOCKFILE(fp);
	retval = __sflush(fp);
	FUNLOCKFILE(fp);
	return (retval);
}

/*
 * Flush a single file, or (if fp is NULL) all files.
 * Non-MT-safe version
 */
int
__fflush(FILE *fp)
{
	int retval;

	if (fp == NULL)
		return (_fwalk(sflush_locked));
	if ((fp->_flags & (__SWR | __SRW)) == 0) {
		retval = 0;
	} else
		retval = __sflush(fp);
	return (retval);
}

int
__sflush(FILE *fp)
{
	unsigned char *p;
	int n, t;

	t = fp->_flags;

	if ((p = fp->_bf._base) == NULL)
		return (0);

	/*
	 * SUSv3 requires that fflush() on a seekable input stream updates the file
	 * position indicator with the underlying seek function.  Use a dumb fseek
	 * for this (don't attempt to preserve the buffers).
	 */
	if ((t & __SRD) != 0) {
		if (fp->_seek == NULL) {
			/*
			 * No way to seek this file -- just return "success."
			 */
			return (0);
		}

		n = fp->_r;

		if (n > 0) {
			/*
			 * See _fseeko's dumb path.
			 */
			if (_sseek(fp, (fpos_t)-n, SEEK_CUR) == -1) {
				if (errno == ESPIPE) {
					/*
					 * Ignore ESPIPE errors, since there's no way to put the bytes
					 * back into the pipe.
					 */
					return (0);
				}
				return (EOF);
			}

			if (HASUB(fp)) {
				FREEUB(fp);
			}
			fp->_p = fp->_bf._base;
			fp->_r = 0;
			fp->_flags &= ~__SEOF;
			memset(&fp->_mbstate, 0, sizeof(mbstate_t));
		}
		return (0);
	}

	if ((t & __SWR) != 0) {
		n = fp->_p - p;		/* write this much */

		/*
		 * Set these immediately to avoid problems with longjmp and to allow
		 * exchange buffering (via setvbuf) in user write function.
		 */
		fp->_p = p;
		fp->_w = t & (__SLBF|__SNBF) ? 0 : fp->_bf._size;

		for (; n > 0; n -= t, p += t) {
			t = _swrite(fp, (char *)p, n);
			if (t <= 0) {
				/* 5340694: reset _p and _w on EAGAIN */
				if (t < 0 && errno == EAGAIN) {
					if (p > fp->_p) {
						/* some was written */
						memmove(fp->_p, p, n);
						fp->_p += n;
						if (!(fp->_flags & (__SLBF|__SNBF)))
							fp->_w -= n;
					}
				}
				fp->_flags |= __SERR;
				return (EOF);
			}
		}
	}
	return (0);
}

static int
sflush_locked(FILE *fp)
{
	int	ret;

	FLOCKFILE(fp);
	ret = __sflush(fp);
	FUNLOCKFILE(fp);
	return (ret);
}
