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
static char sccsid[] = "@(#)getlogin.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/getlogin.c,v 1.11 2009/12/05 19:04:21 ed Exp $");

#include <sys/param.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "namespace.h"
#include <pthread.h>
#include "un-namespace.h"

#include "libc_private.h"

#define	THREAD_LOCK()	if (__isthreaded) _pthread_mutex_lock(&logname_mutex)
#define	THREAD_UNLOCK()	if (__isthreaded) _pthread_mutex_unlock(&logname_mutex)

extern int		__getlogin(char *, int);

int			_logname_valid;		/* known to setlogin() */
static pthread_mutex_t	logname_mutex = PTHREAD_MUTEX_INITIALIZER;

static char *
getlogin_basic(int *status)
{
	static char logname[MAXLOGNAME];

	if (_logname_valid == 0) {
		if (__getlogin(logname, sizeof(logname)) < 0) {
			*status = errno;
			return (NULL);
		}
		_logname_valid = 1;
	}
	*status = 0;
	return (*logname ? logname : NULL);
}

char *
getlogin(void)
{
	char	*result;
	int	status;

	THREAD_LOCK();
	result = getlogin_basic(&status);
	THREAD_UNLOCK();
	return (result);
}

int
getlogin_r(char *logname, size_t namelen)
{
	char	*result;
	int	len;
	int	status;
	
	THREAD_LOCK();
	result = getlogin_basic(&status);
	if (status == 0) {
		if ((len = strlen(result) + 1) > namelen)
			status = ERANGE;
		else
			strncpy(logname, result, len);
	}
	THREAD_UNLOCK();
	return (status);
}
