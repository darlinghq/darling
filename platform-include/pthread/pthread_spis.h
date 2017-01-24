/*
 * Copyright (c) 2000-2012 Apple Inc. All rights reserved.
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
 * Copyright 1996 1995 by Open Software Foundation, Inc. 1997 1996 1995 1994 1993 1992 1991
 *              All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and
 * that both the copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT,
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
/*
 * MkLinux
 */

/*
 * Extension SPIs; installed to /usr/include.
 */

#ifndef _PTHREAD_SPIS_H
#define _PTHREAD_SPIS_H


#include <pthread/pthread.h>

#if __has_feature(assume_nonnull)
_Pragma("clang assume_nonnull begin")
#endif
__BEGIN_DECLS

#if (!defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE)) || defined(_DARWIN_C_SOURCE)
/* firstfit */
#define PTHREAD_FIRSTFIT_MUTEX_INITIALIZER {_PTHREAD_FIRSTFIT_MUTEX_SIG_init, {0}}
/*
 * Mutex attributes
 */
#define _PTHREAD_MUTEX_POLICY_NONE		0
#define _PTHREAD_MUTEX_POLICY_FAIRSHARE		1
#define _PTHREAD_MUTEX_POLICY_FIRSTFIT		2

/* sets the mutex policy attributes */
__OSX_AVAILABLE_STARTING(__MAC_10_7, __IPHONE_5_0)
int pthread_mutexattr_setpolicy_np(pthread_mutexattr_t *, int );

#endif /* (!_POSIX_C_SOURCE && !_XOPEN_SOURCE) || _DARWIN_C_SOURCE */

__OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_NA)
void _pthread_mutex_enable_legacy_mode(void);

/*
 * A version of pthread_create that is safely callable from an injected mach thread.
 *
 * The _create introspection hook will not fire for threads created from this function.
 *
 * It is not safe to call this function concurrently.
 */
__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0)
__TVOS_AVAILABLE(10.0) __WATCHOS_AVAILABLE(3.0)
#if !_PTHREAD_SWIFT_IMPORTER_NULLABILITY_COMPAT
int pthread_create_from_mach_thread(
		pthread_t _Nullable * _Nonnull __restrict,
		const pthread_attr_t * _Nullable __restrict,
		void * _Nullable (* _Nonnull)(void * _Nullable),
		void * _Nullable __restrict);
#else
int pthread_create_from_mach_thread(pthread_t * __restrict,
		const pthread_attr_t * _Nullable __restrict,
		void *(* _Nonnull)(void *), void * _Nullable __restrict);
#endif


__END_DECLS
#if __has_feature(assume_nonnull)
_Pragma("clang assume_nonnull end")
#endif

#endif /* _PTHREAD_SPIS_H */
