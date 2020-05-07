/*
 * Copyright (c) 2006-2010 Apple Inc. All rights reserved.
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

#ifndef NO_SYSCALL_LEGACY

#define _NONSTD_SOURCE
#include <sys/cdefs.h>

/*
 * We need conformance on so that EOPNOTSUPP=102.  But the routine symbol
 * will still be the legacy (undecorated) one.
 */
#undef __DARWIN_UNIX03
#define __DARWIN_UNIX03 1

#include <sys/socket.h>
#include "_errno.h"

extern int __getsockname(int, struct sockaddr * __restrict, socklen_t * __restrict);

/*
 * getsockname stub, legacy version
 */
int
getsockname(int socket, struct sockaddr * __restrict address,
    socklen_t * __restrict address_len)
{
	int ret = __getsockname(socket, address, address_len);

	/* use ENOTSUP for legacy behavior */
	if (ret < 0 && errno == EOPNOTSUPP) {
		errno = ENOTSUP;
	}
	return ret;
}

#endif /* __DARWIN_ONLY_UNIX_CONFORMANCE */
