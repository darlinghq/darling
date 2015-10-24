/*
 * Copyright (c) 1983, 1993
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
static char sccsid[] = "@(#)readdir.c	8.3 (Berkeley) 9/29/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/readdir.c,v 1.15 2008/05/05 14:05:23 kib Exp $");

#include "namespace.h"
#include <sys/param.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "un-namespace.h"

#include "libc_private.h"
#include "telldir.h"

/*
 * get next entry in a directory.
 */
struct dirent *
_readdir_unlocked(dirp, skip)
	DIR *dirp;
	int skip;
{
	struct dirent *dp;

	for (;;) {
		if (dirp->dd_loc >= dirp->dd_size) {
			if (dirp->dd_flags & __DTF_READALL)
				return (NULL);
			dirp->dd_loc = 0;
		}
		if (dirp->dd_loc == 0 && !(dirp->dd_flags & __DTF_READALL)) {
#if __DARWIN_64_BIT_INO_T
			dirp->dd_size = __getdirentries64(dirp->dd_fd,
			    dirp->dd_buf, dirp->dd_len, &dirp->dd_td->seekoff);
#else /* !__DARWIN_64_BIT_INO_T */
			dirp->dd_size = _getdirentries(dirp->dd_fd,
			    dirp->dd_buf, dirp->dd_len, &dirp->dd_seek);
#endif /* __DARWIN_64_BIT_INO_T */
			if (dirp->dd_size <= 0)
				return (NULL);
		}
		dp = (struct dirent *)(dirp->dd_buf + dirp->dd_loc);
		if ((long)dp & 03L)	/* bogus pointer check */
			return (NULL);
		if (dp->d_reclen <= 0 ||
		    dp->d_reclen > dirp->dd_len + 1 - dirp->dd_loc)
			return (NULL);
		dirp->dd_loc += dp->d_reclen;
		if (dp->d_ino == 0 && skip)
			continue;
		if (dp->d_type == DT_WHT && (dirp->dd_flags & DTF_HIDEW))
			continue;
		return (dp);
	}
}

struct dirent *
readdir(dirp)
	DIR *dirp;
{
	struct dirent	*dp;

	if (__isthreaded) {
		_pthread_mutex_lock(&dirp->dd_lock);
		dp = _readdir_unlocked(dirp, 1);
		_pthread_mutex_unlock(&dirp->dd_lock);
	}
	else
		dp = _readdir_unlocked(dirp, 1);
	return (dp);
}

int
readdir_r(dirp, entry, result)
	DIR *dirp;
	struct dirent *entry;
	struct dirent **result;
{
	struct dirent *dp;
	int saved_errno;

	saved_errno = errno;
	errno = 0;
	if (__isthreaded) {
		_pthread_mutex_lock(&dirp->dd_lock);
		if ((dp = _readdir_unlocked(dirp, 1)) != NULL)
			memcpy(entry, dp, _GENERIC_DIRSIZ(dp));
		_pthread_mutex_unlock(&dirp->dd_lock);
	}
	else if ((dp = _readdir_unlocked(dirp, 1)) != NULL)
		memcpy(entry, dp, _GENERIC_DIRSIZ(dp));

	if (errno != 0) {
		if (dp == NULL)
			return (errno);
	} else
		errno = saved_errno;

	if (dp != NULL)
		*result = entry;
	else
		*result = NULL;

	return (0);
}
