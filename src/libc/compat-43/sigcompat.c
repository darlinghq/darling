/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
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

#if __DARWIN_UNIX03
#ifdef VARIANT_CANCELABLE
#include <pthread.h>
#endif /* VARIANT_CANCELABLE */
extern int __unix_conforming;
#endif /* __DARWIN_UNIX03 */

#include <sys/param.h>
#include <signal.h>
#include <errno.h>

#ifndef BUILDING_VARIANT
#if defined(__DYNAMIC__)
extern int _sigaction_nobind (int sig, const struct sigaction *nsv, struct sigaction *osv);
extern int _sigvec_nobind (int, struct sigvec *, struct sigvec *);
#endif

static int
sigvec__(int signo,
	 struct sigvec *sv,
	 struct sigvec *osv,
	 int bind)
{
	int ret;

	if (sv)
		sv->sv_flags ^= SV_INTERRUPT;	/* !SA_INTERRUPT */
#if defined(__DYNAMIC__)
        if (bind) {
#endif
            ret = sigaction(signo, (struct sigaction *)sv, (struct sigaction *)osv);
#if defined(__DYNAMIC__)
	} else {
            ret = _sigaction_nobind(signo, (struct sigaction *)sv, (struct sigaction *)osv);
        }
#endif
        if (ret == 0 && osv)
		osv->sv_flags ^= SV_INTERRUPT;	/* !SA_INTERRUPT */
	return (ret);
}

int
sigvec(int signo,
       struct sigvec *sv,
       struct sigvec *osv)
{
    return sigvec__(signo, sv, osv, 1);
}

#if defined(__DYNAMIC__)
int
_sigvec_nobind(int signo,
	       struct sigvec *sv,
	       struct sigvec *osv)
{
    return sigvec__(signo, sv, osv, 0);
}
#endif

int
sigsetmask(int mask)
{
	int omask, n;

	n = sigprocmask(SIG_SETMASK, (sigset_t *) &mask, (sigset_t *) &omask);
	if (n)
		return (n);
	return (omask);
}

int
sigblock(int mask)
{
	int omask, n;

	n = sigprocmask(SIG_BLOCK, (sigset_t *) &mask, (sigset_t *) &omask);
	if (n)
		return (n);
	return (omask);
}
#endif /* !BUILDING_VARIANT */

#if __DARWIN_UNIX03
int
sigpause(int sig)
{
	sigset_t mask;

	if (__unix_conforming == 0)
		__unix_conforming = 1;
#ifdef VARIANT_CANCELABLE
	pthread_testcancel();
#endif /* VARIANT_CANCELABLE */

	if ((sig <= 0) || (sig >= NSIG)) {
		errno = EINVAL;
		return(-1);
	}
	if (sigprocmask(SIG_BLOCK, (sigset_t *) 0, (sigset_t *) &mask) < 0) {
		return(-1);
	}
	sigdelset(&mask, sig);
	return (sigsuspend(&mask));
}
#else
int
sigpause(int mask)
{
	return (sigsuspend((sigset_t *)&mask));
}
#endif /* __DARWIN_UNIX03 */

#ifndef BUILDING_VARIANT
int
sighold(int sig)
{
	sigset_t mask;

	if ((sig <= 0) || (sig >= NSIG)) {
		errno = EINVAL;
		return(-1);
	}
	sigemptyset(&mask);
	sigaddset(&mask, sig);
	return(sigprocmask(SIG_BLOCK, &mask,(sigset_t *)0));
}

int
sigrelse(int sig)
{
	sigset_t mask;

	if ((sig <= 0) || (sig >= NSIG)) {
		errno = EINVAL;
		return(-1);
	}
	sigemptyset(&mask);
	sigaddset(&mask, sig);
	return(sigprocmask(SIG_UNBLOCK, &mask,(sigset_t *)0));
}


int
sigignore(int sig)
{
	return (signal(sig, SIG_IGN) == SIG_ERR ? -1 : 0);
}

void (*sigset(int sig, void (*disp)(int)))(int) {
	sigset_t omask;
	int blocked;
	struct sigaction oact;

	if ((sig <= 0) || (sig >= NSIG)) {
		errno = EINVAL;
		return (SIG_ERR);
	}
	if (-1 == sigprocmask(0, NULL, &omask))
		return (SIG_ERR);
	blocked = sigismember(&omask, sig);
	if (disp == SIG_HOLD) {
		if (blocked)
			return (SIG_HOLD);
		if ((-1 == sigaction(sig, NULL, &oact)) ||
		    (-1 == sighold(sig)))
			return (SIG_ERR);
		return (sig_t)oact.sa_handler;
	} else {
		if (blocked) {
			if (-1 == sigrelse(sig))
				return (SIG_ERR);
		}
		sig_t rv = signal(sig, disp);
		if (rv != SIG_ERR)
			return blocked ? SIG_HOLD : rv;
		else
			return (rv);
	}
}
#endif /* !BUILDING_VARIANT */
