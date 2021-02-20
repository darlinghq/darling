/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

#include <sys/types.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <strings.h>
#include <Availability.h>

#include "gethostuuid_private.h"

extern int __gethostuuid(uuid_t, const struct timespec *);

static volatile int (*_gethostuuid_callback)(uuid_t) = (void *)0;

int
gethostuuid(uuid_t uuid, const struct timespec *timeout)
{
	int result;

	result = __gethostuuid(uuid, timeout);
	if ((result == -1) && (errno == EPERM)) {
		if (_gethostuuid_callback) {
			result = _gethostuuid_callback(uuid);
		} else {
			/* no fallback, return -1/EPERM */
			memset(uuid, 0x00, sizeof(*uuid));
		}
	}

	return result;
}

/* SPI to call gethostuuid syscall directly, without fallback, need an entitlement */
int
_getprivatesystemidentifier(uuid_t uuid, const struct timespec *timeout)
{
	return __gethostuuid(uuid, timeout);
}

int
_register_gethostuuid_callback(int (*new_callback)(uuid_t))
{
	if (__sync_bool_compare_and_swap((void **)&_gethostuuid_callback, (void *)0, (void *)new_callback)) {
		return 0;
	} else {
		return EINVAL;
	}
}
