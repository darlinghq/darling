/*
 * Copyright (c) 1985, 1989, 1993
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
static char sccsid[] = "@(#)signal.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/signal.c,v 1.4 2007/01/09 00:27:55 imp Exp $");

/*
 * Almost backwards compatible signal.
 */
#include "namespace.h"
#include <signal.h>
#include "un-namespace.h"
#include "libc_private.h"

sigset_t _sigintr;		/* shared with siginterrupt */

extern int _sigaction_nobind (int sig, const struct sigaction *nsv, struct sigaction *osv);

static sig_t
signal__(s, a, bind)
	int s;
	sig_t a;
	int bind;
{
	struct sigaction sa, osa;

	sa.sa_handler = a;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (!sigismember(&_sigintr, s))
		sa.sa_flags |= SA_RESTART;
#if defined(__DYNAMIC__)
	if (bind) {
#endif /* __DYNAMIC__ */
	if (_sigaction(s, &sa, &osa) < 0)
		return (SIG_ERR);
#if defined(__DYNAMIC__)
	} else {
	    if (_sigaction_nobind(s, &sa, &osa) < 0)
		return (SIG_ERR);
	}
#endif /* __DYNAMIC__ */
	return (osa.sa_handler);
}

sig_t
signal(s, a)
        int s;
        sig_t a;
{
    return signal__(s, a, 1);
}

#if defined(__DYNAMIC__)
sig_t
_signal_nobind(s, a)
        int s;
        sig_t a;
{
    return signal__(s, a, 0);
}
#endif /* __DYNAMIC__ */
