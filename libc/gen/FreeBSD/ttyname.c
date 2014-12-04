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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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
static char sccsid[] = "@(#)ttyname.c	8.2 (Berkeley) 1/27/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/ttyname.c,v 1.16 2004/01/06 18:26:14 nectar Exp $");

#include "namespace.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <paths.h>
#include <pthread.h>
#include <errno.h>
#include "un-namespace.h"

#include "libc_private.h"

#ifndef BUILDING_VARIANT
static pthread_once_t ttyname_buf_control = PTHREAD_ONCE_INIT;
static char *buf = NULL;
static char *ttyname_threaded(int fd);
static char *ttyname_unthreaded(int fd);

static pthread_mutex_t	ttyname_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_key_t	ttyname_key;
static int		ttyname_init = 0;

char *
ttyname(int fd)
{
	char           *ret;

	if (__isthreaded == 0)
		ret = ttyname_unthreaded(fd);
	else
		ret = ttyname_threaded(fd);
	return (ret);
}
#endif /* !BUILDING_VARIANT */

#if __DARWIN_UNIX03
int
#else /* !__DARWIN_UNIX03 */
char *
#endif /* __DARWIN_UNIX03 */
ttyname_r(int fd, char *thrbuf, size_t len)
{
	struct stat	sb;

#if __DARWIN_UNIX03
	if (_fstat(fd, &sb) < 0)
		return (EBADF);
	/* Must be a terminal. */
	if (!isatty(fd))
		return (ENOTTY);
	/* Must be a character device. */
	if (!S_ISCHR(sb.st_mode))
		return (ENOTTY);
	/* Must have enough room */
	if (len <= sizeof(_PATH_DEV))
		return (ERANGE);
#else /* !__DARWIN_UNIX03 */
	/* Must be a terminal. */
	if (!isatty(fd))
		return (NULL);
	/* Must be a character device. */
	if (_fstat(fd, &sb))
		return (NULL);
	if (!S_ISCHR(sb.st_mode)) {
		errno = ENOTTY;
		return (NULL);
	}
	/* Must have enough room */
	if (len <= sizeof(_PATH_DEV)) {
		errno = ERANGE;
		return (NULL);
	}
#endif /* __DARWIN_UNIX03 */

	strlcpy(thrbuf, _PATH_DEV, len);
	if (devname_r(sb.st_rdev, S_IFCHR,
	    thrbuf + strlen(thrbuf), len - strlen(thrbuf)) == NULL)
#if __DARWIN_UNIX03
		return (ERANGE);
	return (0);
#else /* !__DARWIN_UNIX03 */
	{
		errno = ERANGE;
		return (NULL);
	}
	return (thrbuf);
#endif /* __DARWIN_UNIX03 */
}

#ifndef BUILDING_VARIANT
static char *
ttyname_threaded(int fd)
{
	char	*buf;

	if (ttyname_init == 0) {
		_pthread_mutex_lock(&ttyname_lock);
		if (ttyname_init == 0) {
			/* __PTK_LIBC_TTYNAME_KEY */
			ttyname_key = __LIBC_PTHREAD_KEY_TTYNAME;
			if (pthread_key_init_np(ttyname_key, free)) {
				int save = errno;
				_pthread_mutex_unlock(&ttyname_lock);
				errno = save;
				return (NULL);
			}
			ttyname_init = 1;
		}
		_pthread_mutex_unlock(&ttyname_lock);
	}

	/* Must have thread specific data field to put data */
	if ((buf = _pthread_getspecific(ttyname_key)) == NULL) {
		if ((buf = malloc(sizeof(_PATH_DEV) + MAXNAMLEN)) != NULL) {
			if (_pthread_setspecific(ttyname_key, buf) != 0) {
				int save = errno;
				free(buf);
				errno = save;
				return (NULL);
			}
		} else {
			return (NULL);
		}
	}
#if __DARWIN_UNIX03
	return (ttyname_r(fd, buf, sizeof(_PATH_DEV) + MAXNAMLEN) == 0 ? buf : NULL);
#else /* !__DARWIN_UNIX03 */
	return (ttyname_r(fd, buf, sizeof(_PATH_DEV) + MAXNAMLEN));
#endif /* __DARWIN_UNIX03 */
}

static void
ttyname_buf_allocate(void)
{
	buf = malloc(sizeof(_PATH_DEV) + MAXNAMLEN);
}

static char *
ttyname_unthreaded(int fd)
{
	struct stat	sb;
	struct termios	ttyb;

	/* Must be a terminal. */
	if (tcgetattr(fd, &ttyb) < 0)
		return (NULL);
	/* Must be a character device. */
	if (_fstat(fd, &sb))
		return (NULL);
	if (!S_ISCHR(sb.st_mode)) {
		errno = ENOTTY;
		return (NULL);
	}

	if (pthread_once(&ttyname_buf_control, ttyname_buf_allocate)
		|| !buf) {
		errno = ENOMEM;
		return (NULL);
	}

	strlcpy(buf, _PATH_DEV, sizeof(_PATH_DEV) + MAXNAMLEN);
	if (devname_r(sb.st_rdev, S_IFCHR,
		buf + strlen(buf), sizeof(_PATH_DEV) + MAXNAMLEN - strlen(buf)) == NULL) {
		errno = ERANGE;
		return (NULL);
	}
	return (buf);
}
#endif /* !BUILDING_VARIANT */
