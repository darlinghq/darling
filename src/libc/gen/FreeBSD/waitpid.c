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
static char sccsid[] = "@(#)waitpid.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/waitpid.c,v 1.7 2007/01/09 00:27:56 imp Exp $");

#include "namespace.h"
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include "un-namespace.h"

#if __DARWIN_UNIX03
#include <errno.h>
#endif /* __DARWIN_UNIX03 */
#ifdef VARIANT_CANCELABLE
int __wait4(pid_t, int *, int , struct rusage *);
#else /* !VARIANT_CANCELABLE */
int __wait4_nocancel(pid_t, int *, int , struct rusage *);
#endif /* VARIANT_CANCELABLE */

pid_t
__waitpid(pid_t pid, int *istat, int options)
{
#if __DARWIN_UNIX03
	/* POSIX: Validate waitpid() options before calling wait4() */
	if ((options & (WCONTINUED | WNOHANG | WUNTRACED)) != options) {
		errno = EINVAL;
		return ((pid_t)-1);
	}
#endif	/* __DARWIN_UNIX03 */

#ifdef VARIANT_CANCELABLE
	return (__wait4(pid, istat, options, (struct rusage *)0));
#else /* !VARIANT_CANCELABLE */
	return (__wait4_nocancel(pid, istat, options, (struct rusage *)0));
#endif /* VARIANT_CANCELABLE */
}

__weak_reference(__waitpid, waitpid);
__weak_reference(__waitpid, _waitpid);
