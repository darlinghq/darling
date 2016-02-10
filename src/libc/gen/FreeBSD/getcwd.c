/*
 * Copyright (c) 1989, 1991, 1993, 1995
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
static char sccsid[] = "@(#)getcwd.c	8.5 (Berkeley) 2/7/95";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/getcwd.c,v 1.29 2007/01/09 00:27:53 imp Exp $");

#include "namespace.h"
#include <sys/param.h>
#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "un-namespace.h"

#define	ISDOT(dp) \
	(dp->d_name[0] == '.' && (dp->d_name[1] == '\0' || \
	    (dp->d_name[1] == '.' && dp->d_name[2] == '\0')))

/*
 * If __getcwd() ever becomes a syscall, we can remove this workaround.
 * The problem here is that fcntl() assumes a buffer of size MAXPATHLEN,
 * if size is less than MAXPATHLEN, we need to use a temporary buffer
 * and see if it fits.  We also have to assume that open() or fcntl()
 * don't fail with errno=ERANGE.
 */
static inline int
__getcwd(char *buf, size_t size)
{
	int fd, err, save;
	struct stat dot, pt;
	char *b;

	if ((fd = open(".", O_RDONLY)) < 0)
		return -1;
	if (fstat(fd, &dot) < 0) {
		save = errno;
		close(fd);
		errno = save;
		return -1;
	}
	/* check that the device and inode are non-zero, otherwise punt */
	if (dot.st_dev == 0 || dot.st_ino == 0) {
		close(fd);
		errno = EINVAL;
		return -1;
	}
	if (size < MAXPATHLEN) {
		/* the hard case; allocate a buffer of size MAXPATHLEN to use */
		b = (char *)alloca(MAXPATHLEN);
		if (b == NULL) {
			close(fd);
			errno = ENOMEM; /* make sure it isn't ERANGE */
			return -1;
		}
	} else
		b = buf;

	err = fcntl(fd, F_GETPATH, b);
	if (err) {
		save = errno;
		close(fd);
		errno = save;
		return err;
	}
	close(fd);
	/*
	 * now double-check that the path returned by fcntl() has the same
	 * device and inode number as '.'.
	 */
	if (stat(b, &pt) < 0)
		return -1;
	/*
	 * Since dot.st_dev and dot.st_ino are non-zero, we don't need to
	 * separately test for pt.st_dev and pt.st_ino being non-zero, because
	 * they have to match
	 */
	if (dot.st_dev != pt.st_dev || dot.st_ino != pt.st_ino) {
		errno = EINVAL;
		return -1;
	}
	/*
	 * For the case where we allocated a buffer, check that it can fit
	 * in the real buffer, and copy it over.
	 */
	if (size < MAXPATHLEN) {
		if (strlen(b) >= size) {
			errno = ERANGE;
			return -1;
		}
		strcpy(buf, b);
	}
	return 0;
}

__private_extern__ char *
__private_getcwd(pt, size, usegetpath)
	char *pt;
	size_t size;
	int usegetpath;
{
	struct dirent *dp;
	DIR *dir = NULL;
	dev_t dev;
	ino_t ino;
	int first;
	char *bpt, *bup;
	struct stat s;
	dev_t root_dev;
	ino_t root_ino;
	size_t ptsize, upsize;
	int save_errno;
	char *ept, *eup, *up, c;

	/*
	 * If no buffer specified by the user, allocate one as necessary.
	 * If a buffer is specified, the size has to be non-zero.  The path
	 * is built from the end of the buffer backwards.
	 */
	if (pt) {
		ptsize = 0;
		if (!size) {
			errno = EINVAL;
			return (NULL);
		}
		if (size == 1) {
			errno = ERANGE;
			return (NULL);
		}
		ept = pt + size;
	} else {
		if ((pt = malloc(ptsize = MAXPATHLEN)) == NULL)
			return (NULL);
		ept = pt + ptsize;
	}
	if (usegetpath) {
		if (__getcwd(pt, ept - pt) == 0) {
			return (pt);
		} else if (errno == ERANGE) /* failed because buffer too small */
			return NULL;
	}
	bpt = ept - 1;
	*bpt = '\0';

	/*
	 * Allocate MAXPATHLEN bytes for the string of "../"'s.
	 * Should always be enough.  If it's not, allocate
	 * as necessary.  Special case the first stat, it's ".", not "..".
	 */
	if ((up = malloc(upsize = MAXPATHLEN)) == NULL)
		goto err;
	eup = up + MAXPATHLEN;
	bup = up;
	up[0] = '.';
	up[1] = '\0';

	/* Save root values, so know when to stop. */
	if (stat("/", &s))
		goto err;
	root_dev = s.st_dev;
	root_ino = s.st_ino;

	errno = 0;			/* XXX readdir has no error return. */

	for (first = 1;; first = 0) {
		/* Stat the current level. */
		if (lstat(up, &s))
			goto err;

		/* Save current node values. */
		ino = s.st_ino;
		dev = s.st_dev;

		/* Check for reaching root. */
		if (root_dev == dev && root_ino == ino) {
			*--bpt = '/';
			/*
			 * It's unclear that it's a requirement to copy the
			 * path to the beginning of the buffer, but it's always
			 * been that way and stuff would probably break.
			 */
			bcopy(bpt, pt, ept - bpt);
			free(up);
			return (pt);
		}

		/*
		 * Build pointer to the parent directory, allocating memory
		 * as necessary.  Max length is 3 for "../", the largest
		 * possible component name, plus a trailing NUL.
		 */
		while (bup + 3  + MAXNAMLEN + 1 >= eup) {
			if ((up = reallocf(up, upsize *= 2)) == NULL)
				goto err;
			bup = up;
			eup = up + upsize;
		}
		*bup++ = '.';
		*bup++ = '.';
		*bup = '\0';

		/* Open and stat parent directory. */
		if (!(dir = opendir(up)) || _fstat(dirfd(dir), &s))
			goto err;

		/* Add trailing slash for next directory. */
		*bup++ = '/';
		*bup = '\0';

		/*
		 * If it's a mount point, have to stat each element because
		 * the inode number in the directory is for the entry in the
		 * parent directory, not the inode number of the mounted file.
		 */
		save_errno = 0;
		if (s.st_dev == dev) {
			for (;;) {
				if (!(dp = readdir(dir)))
					goto notfound;
				if (dp->d_fileno == ino)
					break;
			}
		} else
			for (;;) {
				if (!(dp = readdir(dir)))
					goto notfound;
				if (ISDOT(dp))
					continue;
				bcopy(dp->d_name, bup, dp->d_namlen + 1);

				/* Save the first error for later. */
				if (lstat(up, &s)) {
					if (!save_errno)
						save_errno = errno;
					errno = 0;
					continue;
				}
				if (s.st_dev == dev && s.st_ino == ino)
					break;
			}

		/*
		 * Check for length of the current name, preceding slash,
		 * leading slash.
		 */
		while (bpt - pt < dp->d_namlen + (first ? 1 : 2)) {
			size_t len, off;

			if (!ptsize) {
				errno = ERANGE;
				goto err;
			}
			off = bpt - pt;
			len = ept - bpt;
			if ((pt = reallocf(pt, ptsize *= 2)) == NULL)
				goto err;
			bpt = pt + off;
			ept = pt + ptsize;
			bcopy(bpt, ept - len, len);
			bpt = ept - len;
		}
		if (!first)
			*--bpt = '/';
		bpt -= dp->d_namlen;
		bcopy(dp->d_name, bpt, dp->d_namlen);
		(void) closedir(dir);
		dir = NULL;

		/* Truncate any file name. */
		*bup = '\0';
	}

notfound:
	/*
	 * If readdir set errno, use it, not any saved error; otherwise,
	 * didn't find the current directory in its parent directory, set
	 * errno to ENOENT.
	 */
	if (!errno)
		errno = save_errno ? save_errno : ENOENT;
	/* FALLTHROUGH */
err:
	save_errno = errno;

	if (ptsize)
		free(pt);
	if (dir)
		(void) closedir(dir);
	free(up);

	errno = save_errno;
	return (NULL);
}

char *
getcwd(pt, size)
	char *pt;
	size_t size;
{
	return __private_getcwd(pt, size, 1);
}
