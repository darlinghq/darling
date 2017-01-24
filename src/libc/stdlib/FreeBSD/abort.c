/*
 * Copyright (c) 1985, 1993
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
static char sccsid[] = "@(#)abort.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/stdlib/abort.c,v 1.11 2007/01/09 00:28:09 imp Exp $");

#include "namespace.h"
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>
#include <pthread_workqueue.h>
#include "un-namespace.h"

#include "libc_private.h"

#include <CrashReporterClient.h>
#include "_simple.h"

extern void (*__cleanup)();
extern void __abort(void) __dead2;

#define TIMEOUT	10000	/* 10 milliseconds */

void
abort()
{
	struct sigaction act;

	if (!CRGetCrashLogMessage())
		CRSetCrashLogMessage("abort() called");

	/*
	 * POSIX requires we flush stdio buffers on abort.
	 * XXX ISO C requires that abort() be async-signal-safe.
	 */
	if (__cleanup)
		(*__cleanup)();

	sigfillset(&act.sa_mask);
	/*
	 * Don't block SIGABRT to give any handler a chance; we ignore
	 * any errors -- ISO C doesn't allow abort to return anyway.
	 */
	sigdelset(&act.sa_mask, SIGABRT);

	/* <rdar://problem/7397932> abort() should call pthread_kill to deliver a signal to the aborting thread 
	 * This helps gdb focus on the thread calling abort()
	 */

	/* Block all signals on all other threads */
	sigset_t fullmask;
	sigfillset(&fullmask);
	(void)_sigprocmask(SIG_SETMASK, &fullmask, NULL);

	/* <rdar://problem/8400096> Set the workqueue killable */
	__pthread_workqueue_setkill(1);

	(void)pthread_sigmask(SIG_SETMASK, &act.sa_mask, NULL);
	(void)pthread_kill(pthread_self(), SIGABRT);

	usleep(TIMEOUT); /* give time for signal to happen */

	/*
	 * If SIGABRT was ignored, or caught and the handler returns, do
	 * it again, only harder.
	 */
	 __abort();
}

__private_extern__ void
__abort()
{
	struct sigaction act;

	if (!CRGetCrashLogMessage())
		CRSetCrashLogMessage("__abort() called");
	act.sa_handler = SIG_DFL;
	act.sa_flags = 0;
	sigfillset(&act.sa_mask);
	(void)_sigaction(SIGABRT, &act, NULL);
	sigdelset(&act.sa_mask, SIGABRT);

	/* <rdar://problem/7397932> abort() should call pthread_kill to deliver a signal to the aborting thread 
	 * This helps gdb focus on the thread calling abort()
	 */

	/* Block all signals on all other threads */
	sigset_t fullmask;
	sigfillset(&fullmask);
	(void)_sigprocmask(SIG_SETMASK, &fullmask, NULL);

	/* <rdar://problem/8400096> Set the workqueue killable */
	__pthread_workqueue_setkill(1);

	(void)pthread_sigmask(SIG_SETMASK, &act.sa_mask, NULL);
	(void)pthread_kill(pthread_self(), SIGABRT);

	usleep(TIMEOUT); /* give time for signal to happen */

	/* If for some reason SIGABRT was not delivered, we exit using __builtin_trap
	 * which generates an illegal instruction on i386: <rdar://problem/8400958>
	 * and SIGTRAP on arm.
	 */
	sigfillset(&act.sa_mask);
	sigdelset(&act.sa_mask, SIGILL);
	sigdelset(&act.sa_mask, SIGTRAP);
	(void)_sigprocmask(SIG_SETMASK, &act.sa_mask, NULL);
	__builtin_trap();
}

void
abort_report_np(const char *fmt, ...)
{
	_SIMPLE_STRING s;
	va_list ap;

	if ((s = _simple_salloc()) != NULL) {
		va_start(ap, fmt);
		_simple_vsprintf(s, fmt, ap);
		va_end(ap);
		CRSetCrashLogMessage(_simple_string(s));
	} else
		CRSetCrashLogMessage(fmt); /* the format string is better than nothing */
	abort();
}
