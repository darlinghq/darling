/*
 * Copyright 2000 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that both the above copyright notice and this
 * permission notice appear in all copies, that both the above
 * copyright notice and this permission notice appear in all
 * supporting documentation, and that the name of M.I.T. not be used
 * in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  M.I.T. makes
 * no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * THIS SOFTWARE IS PROVIDED BY M.I.T. ``AS IS''.  M.I.T. DISCLAIMS
 * ALL EXPRESS OR IMPLIED WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT
 * SHALL M.I.T. BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if defined(VARIANT_CANCELABLE) || defined(VARIANT_PRE1050)
#undef __DARWIN_NON_CANCELABLE
#endif /* VARIANT_CANCELABLE */

#ifdef VARIANT_DARWINEXTSN
#define _DARWIN_UNLIMITED_SELECT
#endif /* VARIANT_DARWINEXTSN */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/pselect.c,v 1.6 2002/10/12 16:13:37 mike Exp $");

#include "namespace.h"
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#include <errno.h>
#include <signal.h>
#include "un-namespace.h"

__weak_reference(__pselect, pselect);

/*
 * Emulate the POSIX 1003.1g-2000 `pselect' interface.  This is the
 * same as the traditional BSD `select' function, except that it uses
 * a timespec rather than a timeval, doesn't modify the timeout argument,
 * and allows the user to specify a signal mask to apply during the select.
 */
int
__pselect(int count, fd_set * __restrict rfds, fd_set * __restrict wfds, 
	fd_set * __restrict efds, const struct timespec * __restrict timo, 
	const sigset_t * __restrict mask)
{
	sigset_t omask;
	struct timeval tvtimo, *tvp;
	int rv, sverrno;

	if (timo) {
		TIMESPEC_TO_TIMEVAL(&tvtimo, timo);
		tvp = &tvtimo;
	} else
		tvp = 0;

	if (mask != 0) {
		rv = _sigprocmask(SIG_SETMASK, mask, &omask);
		if (rv != 0)
			return rv;
	}

	rv = _select(count, rfds, wfds, efds, tvp);
	if (mask != 0) {
		sverrno = errno;
		_sigprocmask(SIG_SETMASK, &omask, (sigset_t *)0);
		errno = sverrno;
	}

	return rv;
}
