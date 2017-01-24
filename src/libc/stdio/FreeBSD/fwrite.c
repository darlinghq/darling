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
static char sccsid[] = "@(#)fwrite.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdio/fwrite.c,v 1.13 2009/07/12 13:09:43 ed Exp $");

#include "namespace.h"
#include <stdio.h>
#include "un-namespace.h"
#include "local.h"
#include "fvwrite.h"
#include "libc_private.h"

/*
 * The maximum amount to write to avoid integer overflow (especially for
 * uio_resid in struct __suio).  INT_MAX is odd, so it make sense to make it
 * even.  We subtract (BUFSIZ - 1) to get a whole number of BUFSIZ chunks.
 */
#define MAXWRITE	(INT_MAX - (BUFSIZ - 1))

/*
 * Write `count' objects (each size `size') from memory to the given file.
 * Return the number of whole objects written.
 */
size_t
fwrite(buf, size, count, fp)
	const void * __restrict buf;
	size_t size, count;
	FILE * __restrict fp;
{
	size_t n, resid;
	struct __suio uio;
	struct __siov iov;
	int s;

	/*
	 * ANSI and SUSv2 require a return value of 0 if size or count are 0.
	 */
	n = count * size;
#if __DARWIN_UNIX03
	if (n == 0)
		return (0);
#endif
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;

	FLOCKFILE(fp);
	ORIENT(fp, -1);

	for (resid = n; resid > 0; buf += s, resid -= s) {
		s = resid > INT_MAX ? MAXWRITE : (int)resid;
		iov.iov_base = (void *)buf;
		uio.uio_resid = iov.iov_len = s;

		/*
		 * The usual case is success (__sfvwrite returns 0);
		 * skip the divide if this happens, since divides are
		 * generally slow and since this occurs whenever size==0.
		 */
		if (__sfvwrite(fp, &uio) != 0) {
			count = (n - resid + s - uio.uio_resid) / size;
			break;
		}
	}
	FUNLOCKFILE(fp);
	return (count);
}
