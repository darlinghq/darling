/*
 * Copyright (c) 2005, 2007 Apple Inc. All rights reserved.
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

#if defined(__LP64__) && (defined(VARIANT_CANCELABLE) || defined(VARIANT_PRE1050))
#undef __DARWIN_NON_CANCELABLE
#define __DARWIN_NON_CANCELABLE 0
#endif /* __LP64__ && (VARIANT_CANCELABLE || VARIANT_PRE1050) */

#include <sys/select.h>
#include "_errno.h"

#if defined(VARIANT_CANCELABLE) || defined(VARIANT_PRE1050)
extern int __select(int, fd_set * __restrict, fd_set * __restrict,
	fd_set * __restrict, struct timeval * __restrict);
#else /* !VARIANT_CANCELABLE && !VARIANT_PRE1050 */
int __select_nocancel(int, fd_set * __restrict, fd_set * __restrict,
	fd_set * __restrict, struct timeval * __restrict);
#endif /* VARIANT_CANCELABLE || VARIANT_PRE1050 */

/*
 * select stub, return error if nfds > FD_SETSIZE
 * add pthread cancelability
 * mandated for conformance.
 *
 * This is only for (non DARWINEXTSN) UNIX03 (both cancelable and
 * non-cancelable) and for legacy
 */
int
select(int nfds, fd_set * __restrict readfds, fd_set * __restrict writefds,
	fd_set * __restrict exceptfds, struct timeval * __restrict
#if defined(VARIANT_LEGACY) || defined(VARIANT_PRE1050)
	intimeout
#else /* !VARIANT_LEGACY && !VARIANT_PRE1050 */
	timeout
#endif /* VARIANT_LEGACY || VARIANT_PRE1050 */
	)
{

#if defined(VARIANT_LEGACY) || defined(VARIANT_PRE1050)
	struct timeval tb, *timeout;

	/*
	 * Legacy select behavior is minimum 10 msec when tv_usec is non-zero
	 */
	if (intimeout && intimeout->tv_sec == 0 && intimeout->tv_usec > 0 && intimeout->tv_usec < 10000) {
		tb.tv_sec = 0;
		tb.tv_usec = 10000;
		timeout = &tb;
	} else
		timeout = intimeout;
#else /* !VARIANT_LEGACY && !VARIANT_PRE1050 */
	if (nfds > FD_SETSIZE) {
		errno = EINVAL;
		return -1;
	}
#endif /* VARIANT_LEGACY || VARIANT_PRE1050 */
#if defined(VARIANT_CANCELABLE) || defined(VARIANT_PRE1050)
	return __select(nfds, readfds, writefds, exceptfds, timeout);
#else /* !VARIANT_CANCELABLE && !VARIANT_PRE1050 */
	return __select_nocancel(nfds, readfds, writefds, exceptfds, timeout);
#endif /* VARIANT_CANCELABLE || VARIANT_PRE1050 */
}
