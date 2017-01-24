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
static char sccsid[] = "@(#)fread.c	8.2 (Berkeley) 12/11/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdio/fread.c,v 1.16 2009/07/12 13:09:43 ed Exp $");

#include "namespace.h"
#include <stdio.h>
#include <string.h>
#include "un-namespace.h"
#include "local.h"
#include "libc_private.h"

/*
 * MT-safe version
 */

size_t
fread(void * __restrict buf, size_t size, size_t count, FILE * __restrict fp)
{
	size_t ret;

	FLOCKFILE(fp);
	ret = __fread(buf, size, count, fp);
	FUNLOCKFILE(fp);
	return (ret);
}

/*
 * The maximum amount to read to avoid integer overflow.  INT_MAX is odd,
 * so it make sense to make it even.  We subtract (BUFSIZ - 1) to get a
 * whole number of BUFSIZ chunks.
 */
#define MAXREAD	(INT_MAX - (BUFSIZ - 1))

/* __fread0: int sized, with size = 1 */
static inline int
__fread0(void * __restrict buf, int count, FILE * __restrict fp)
{
	int resid;
	char *p;
	int r, ret;

	resid = count;
	p = buf;
	/* first deal with anything left in buffer, plus any ungetc buffers */
	while (resid > (r = fp->_r)) {
		(void)memcpy((void *)p, (void *)fp->_p, (size_t)r);
		fp->_p += r;
		/* fp->_r = 0 ... done in __srefill */
		p += r;
		resid -= r;
		if ((ret = __srefill0(fp)) > 0)
			break;
		else if (ret) {
			/* no more input: return partial result */
			return (count - resid);
		}
	}
	/*
	 * 5980080: don't use optimization if __SMBF not set (meaning setvbuf
	 * was called, and the buffer belongs to the user).
	 * 6180417: but for unbuffered (__SMBF is not set), so specifically
	 * test for it.
	 */
	if ((fp->_flags & (__SMBF | __SNBF)) && resid > fp->_bf._size) {
		struct __sbuf save;
		size_t n;

		save = fp->_bf;
		fp->_bf._base = (unsigned char *)p;
		fp->_bf._size = resid;
		while (fp->_bf._size > 0) {
			if ((ret = __srefill1(fp)) != 0) {
				/* no more input: return partial result */
				resid = fp->_bf._size;
				fp->_bf = save;
				fp->_p = fp->_bf._base;
				/* fp->_r = 0;  already set in __srefill1 */
				return (count - resid);
			}
			fp->_bf._base += fp->_r;
			fp->_bf._size -= fp->_r;
		}
		fp->_bf = save;
		n = fp->_bf._size * ((resid - 1) / fp->_bf._size);
		r = resid - n;
		(void)memcpy((void *)fp->_bf._base, (void *)(p + n), (size_t)r);
		fp->_p = fp->_bf._base + r;
		fp->_r = 0;
	} else {
		while (resid > (r = fp->_r)) {
			(void)memcpy((void *)p, (void *)fp->_p, (size_t)r);
			fp->_p += r;
			/* fp->_r = 0 ... done in __srefill */
			p += r;
			resid -= r;
			if (__srefill1(fp)) {
				/* no more input: return partial result */
				return (count - resid);
			}
		}
		(void)memcpy((void *)p, (void *)fp->_p, resid);
		fp->_r -= resid;
		fp->_p += resid;
	}
	return (count);
}

size_t
__fread(void * __restrict buf, size_t size, size_t count, FILE * __restrict fp)
{
	size_t resid;
	int r, ret;
	size_t total;

	/*
	 * ANSI and SUSv2 require a return value of 0 if size or count are 0.
	 */
	if ((resid = count * size) == 0)
		return (0);
	ORIENT(fp, -1);
	if (fp->_r < 0)
		fp->_r = 0;

	for (total = resid; resid > 0; buf += r, resid -= r) {
		r = resid > INT_MAX ? MAXREAD : (int)resid;
		if ((ret = __fread0(buf, r, fp)) != r) {
			count = (total - resid + ret) / size;
			break;
		}
	}
	return (count);
}
