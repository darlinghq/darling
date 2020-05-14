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
#include <errno.h>
#include <stdlib.h>

struct getmntinfo_vars {
	struct statfs *mntbuf;
	int mntsize;
	long bufsize;
};

/*
 * Return information about mounted filesystems.
 */
static int
getmntinfo_internal(struct getmntinfo_vars *vars, int flags)
{

	if (vars->mntsize <= 0 &&
	    (vars->mntsize = getfsstat(0, 0, MNT_NOWAIT)) < 0) {
		return (0);
	}
	if (vars->bufsize > 0 &&
	    (vars->mntsize =
	     getfsstat(vars->mntbuf, vars->bufsize, flags)) < 0) {
		return (0);
	}
	while (vars->bufsize <= vars->mntsize * sizeof(struct statfs)) {
		if (vars->mntbuf) {
			free(vars->mntbuf);
		}
		vars->bufsize = (vars->mntsize + 1) * sizeof(struct statfs);
		if ((vars->mntbuf =
		     (struct statfs *)malloc(vars->bufsize)) == 0) {
			return (0);
		}
		if ((vars->mntsize =
		     getfsstat(vars->mntbuf, vars->bufsize, flags)) < 0) {
			return (0);
		}
	}
	return (vars->mntsize);
}

/* Legacy version that keeps the buffer around. */
int
getmntinfo(struct statfs **mntbufp, int flags)
{
	static struct getmntinfo_vars vars;
	int rv;

	rv = getmntinfo_internal(&vars, flags);
	/* Unconditional assignment matches legacy behavior. */
	*mntbufp = vars.mntbuf;
	return (rv);
}

/* Thread-safe version where the caller owns the newly-allocated buffer. */
int
getmntinfo_r_np(struct statfs **mntbufp, int flags)
{
	struct getmntinfo_vars vars = { 0 };
	int rv, save_errno;

	if ((rv = getmntinfo_internal(&vars, flags)) != 0) {
		*mntbufp = vars.mntbuf;
	} else {
		save_errno = errno;
		free(vars.mntbuf);
		errno = save_errno;
	}
	return (rv);
}
