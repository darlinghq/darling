/*
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software written by Ken Arnold and
 * published in UNIX Review, Vol. 6, No. 8.
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

#ifdef VARIANT_DARWINEXTSN
#define _DARWIN_UNLIMITED_STREAMS
#endif /* VARIANT_DARWINEXTSN */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)popen.c	8.3 (Berkeley) 5/3/95";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/popen.c,v 1.21 2009/05/27 19:28:04 ed Exp $");

#include "namespace.h"
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <wchar.h>		/* fwide() */
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <paths.h>
#include <pthread.h>
#include <spawn.h>
#include "un-namespace.h"
#include "libc_private.h"

#include <crt_externs.h>
#define environ (*_NSGetEnviron())

/* Our queue.h doesn't have SLIST_REMOVE_AFTER in it yet
 * <rdar://problem/7431558> API: Add SLIST_REMOVE_AFTER to sys/queue.h (from FreeBSD)
 */
#ifndef SLIST_REMOVE_AFTER
#define SLIST_REMOVE_AFTER(elm, field) do {                             \
        SLIST_NEXT(elm, field) =                                        \
            SLIST_NEXT(SLIST_NEXT(elm, field), field);                  \
} while (0)
#endif

/* 3516149 - store file descriptor and use that to close to prevent blocking */
struct pid {
	SLIST_ENTRY(pid) next;
	FILE *fp;
	int fd;
	pid_t pid;
};
#define pidlist		__popen_pidlist
#define pidlist_mutex	__popen_pidlist_mutex
#ifndef BUILDING_VARIANT
__private_extern__ SLIST_HEAD(, pid) pidlist = SLIST_HEAD_INITIALIZER(pidlist);
__private_extern__ pthread_mutex_t pidlist_mutex = PTHREAD_MUTEX_INITIALIZER;
#else /* BUILDING_VARIANT */
extern SLIST_HEAD(, pid) pidlist;
extern pthread_mutex_t pidlist_mutex;
#endif /* !BUILDING_VARIANT */

#define	THREAD_LOCK()	if (__isthreaded) _pthread_mutex_lock(&pidlist_mutex)
#define	THREAD_UNLOCK()	if (__isthreaded) _pthread_mutex_unlock(&pidlist_mutex)

FILE *
popen(command, type)
	const char *command, *type;
{
	struct pid *cur;
	FILE *iop;
	int pdes[2], pid, twoway, other;
	char *argv[4];
	struct pid *p;
	posix_spawn_file_actions_t file_actions;
	int err;

	if (type == NULL) {
		errno = EINVAL;
		return (NULL);
	}
	if (strcmp(type, "r+") == 0) {
		twoway = 1;
		type = "r+";
		if (socketpair(AF_UNIX, SOCK_STREAM, 0, pdes) < 0)
			return (NULL);
	} else  {
		twoway = 0;
		if ((*type != 'r' && *type != 'w') || type[1]) {
			errno = EINVAL;
			return (NULL);
		}
		if (pipe(pdes) < 0)
			return (NULL);
	}

	/* fdopen can now fail */
	if (*type == 'r') {
		iop = fdopen(pdes[0], type);
		other = pdes[1];
	} else {
		iop = fdopen(pdes[1], type);
		other = pdes[0];
	}
	if (iop == NULL) {
		(void)_close(pdes[0]);
		(void)_close(pdes[1]);
		return (NULL);
	}

	if ((cur = malloc(sizeof(struct pid))) == NULL) {
		(void)fclose(iop);
		(void)_close(other);
		return (NULL);
	}

	if ((err = posix_spawn_file_actions_init(&file_actions)) != 0) {
		(void)fclose(iop);
		(void)_close(other);
		free(cur);
		errno = err;
		return (NULL);
	}
	if (*type == 'r') {
		/*
		 * The dup2() to STDIN_FILENO is repeated to avoid
		 * writing to pdes[1], which might corrupt the
		 * parent's copy.  This isn't good enough in
		 * general, since the _exit() is no return, so
		 * the compiler is free to corrupt all the local
		 * variables.
		 */
		(void)posix_spawn_file_actions_addclose(&file_actions, pdes[0]);
		if (pdes[1] != STDOUT_FILENO) {
			(void)posix_spawn_file_actions_adddup2(&file_actions, pdes[1], STDOUT_FILENO);
			(void)posix_spawn_file_actions_addclose(&file_actions, pdes[1]);
			if (twoway)
				(void)posix_spawn_file_actions_adddup2(&file_actions, STDOUT_FILENO, STDIN_FILENO);
		} else if (twoway && (pdes[1] != STDIN_FILENO))
			(void)posix_spawn_file_actions_adddup2(&file_actions, pdes[1], STDIN_FILENO);
	} else {
		if (pdes[0] != STDIN_FILENO) {
			(void)posix_spawn_file_actions_adddup2(&file_actions, pdes[0], STDIN_FILENO);
			(void)posix_spawn_file_actions_addclose(&file_actions, pdes[0]);
		}
		(void)posix_spawn_file_actions_addclose(&file_actions, pdes[1]);
	}
	SLIST_FOREACH(p, &pidlist, next)
		(void)posix_spawn_file_actions_addclose(&file_actions, p->fd);

	argv[0] = "sh";
	argv[1] = "-c";
	argv[2] = (char *)command;
	argv[3] = NULL;

	err = posix_spawn(&pid, _PATH_BSHELL, &file_actions, NULL, argv, environ);
	posix_spawn_file_actions_destroy(&file_actions);

	if (err == ENOMEM || err == EAGAIN) { /* as if fork failed */
		(void)fclose(iop);
		(void)_close(other);
		free(cur);
		errno = err;
		return (NULL);
	} else if (err != 0) { /* couldn't exec the shell */
		pid = -1;
	}

	if (*type == 'r') {
		cur->fd = pdes[0];
		(void)_close(pdes[1]);
	} else {
		cur->fd = pdes[1];
		(void)_close(pdes[0]);
	}

	/* Link into list of file descriptors. */
	cur->fp = iop;
	cur->pid = pid;
	THREAD_LOCK();
	SLIST_INSERT_HEAD(&pidlist, cur, next);
	THREAD_UNLOCK();
	fwide(iop, -1);		/* byte stream */
	return (iop);
}

#ifndef BUILDING_VARIANT
/*
 * pclose --
 *	Pclose returns -1 if stream is not associated with a `popened' command,
 *	if already `pclosed', or waitpid returns an error.
 */
int
pclose(iop)
	FILE *iop;
{
	struct pid *cur, *last = NULL;
	int pstat;
	pid_t pid;

	/*
	 * Find the appropriate file pointer and remove it from the list.
	 */
	THREAD_LOCK();
	SLIST_FOREACH(cur, &pidlist, next) {
		if (cur->fp == iop)
			break;
		last = cur;
	}
	if (cur == NULL) {
		THREAD_UNLOCK();
		return (-1);
	}
	if (last == NULL)
		SLIST_REMOVE_HEAD(&pidlist, next);
	else
		SLIST_REMOVE_AFTER(last, next);
	THREAD_UNLOCK();

	(void)fclose(iop);

	if (cur->pid < 0) {
		free(cur);
		return W_EXITCODE(127, 0);
	}
	do {
		pid = _wait4(cur->pid, &pstat, 0, (struct rusage *)0);
	} while (pid == -1 && errno == EINTR);

	free(cur);

	return (pid == -1 ? -1 : pstat);
}
#endif /* !BUILDING_VARIANT */
