/*
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
static char sccsid[] = "@(#)getmntinfo.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/getmntinfo.c,v 1.5 2007/01/09 00:27:54 imp Exp $");

#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#include <stdlib.h>

/*
 * Return information about mounted filesystems.
 */
int
getmntinfo64(mntbufp, flags)
	struct statfs64 **mntbufp;
	int flags;
{
	static struct statfs64 *mntbuf;
	static int mntsize;
	static long bufsize;

	if (mntsize <= 0 && (mntsize = getfsstat64(0, 0, MNT_NOWAIT)) < 0)
		return (0);
	if (bufsize > 0 && (mntsize = getfsstat64(mntbuf, bufsize, flags)) < 0)
		return (0);
	while (bufsize <= mntsize * sizeof(struct statfs64)) {
		if (mntbuf)
			free(mntbuf);
		bufsize = (mntsize + 1) * sizeof(struct statfs64);
		if ((mntbuf = (struct statfs64 *)malloc(bufsize)) == 0)
			return (0);
		if ((mntsize = getfsstat64(mntbuf, bufsize, flags)) < 0)
			return (0);
	}
	*mntbufp = mntbuf;
	return (mntsize);
}
