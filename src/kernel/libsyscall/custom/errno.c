/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 * 
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#include <TargetConditionals.h>
#include <stddef.h>
#include <stdint.h>
#include "tsd.h"

/*
 * cerror takes the return value of the syscall, being non-zero, and
 * stores it in errno. It needs to return -1 to indicate failure but
 * 64-bit platforms need to ensure that possible 128-bit wide return
 * values are also properly set.
 */
#ifdef __LP64__
typedef unsigned __int128 cerror_return_t;
#else
typedef uint64_t cerror_return_t;
#endif

extern void _pthread_exit_if_canceled(int error);

#undef errno
int errno;

int *
__error(void)
{
	void *ptr = _os_tsd_get_direct(__TSD_ERRNO);
	if (ptr != NULL) {
		return (int*)ptr;
	}
	return &errno;
}

__attribute__((noinline))
cerror_return_t
cerror_nocancel(int err)
{
	errno = err;
	int *tsderrno = (int*)_os_tsd_get_direct(__TSD_ERRNO);
	if (tsderrno) {
		*tsderrno = err;
	}
	return -1;
}

__attribute__((noinline))
cerror_return_t
cerror(int err)
{
	_pthread_exit_if_canceled(err);
	return cerror_nocancel(err);
}
