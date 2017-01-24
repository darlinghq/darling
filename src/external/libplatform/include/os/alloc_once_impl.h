/*
 * Copyright (c) 2012-2013 Apple Inc. All rights reserved.
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

#ifndef __OS_ALLOC_ONCE_IMPL__
#define __OS_ALLOC_ONCE_IMPL__

#ifndef __OS_ALLOC_INDIRECT__
#error "Please include <os/alloc_once_private.h> instead of this file directly."
#endif

#include <Availability.h>
#include <sys/types.h>
#include <os/base_private.h>
#include <os/once_private.h>

__BEGIN_DECLS

#define OS_ALLOC_SPI_VERSION 20120430

#define OS_ALLOC_ONCE_KEY_MAX 100

typedef os_once_t os_alloc_token_t;
struct _os_alloc_once_s {
	os_alloc_token_t once;
	void *ptr;
};

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_6_0)
extern struct _os_alloc_once_s _os_alloc_once_table[];

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_6_0)
OS_EXPORT OS_NONNULL1
void*
_os_alloc_once(struct _os_alloc_once_s *slot, size_t sz, os_function_t init);

/* 
 * The region allocated by os_alloc_once is 0-filled when initially
 * returned (or handed off to the initializer).
 */
OS_WARN_RESULT OS_NOTHROW OS_CONST
__header_always_inline void*
os_alloc_once(os_alloc_token_t token, size_t sz, os_function_t init)
{
	struct _os_alloc_once_s *slot = &_os_alloc_once_table[token];
	if (OS_EXPECT(slot->once, ~0l) != ~0l) {
		void *ptr = _os_alloc_once(slot, sz, init);
		OS_COMPILER_CAN_ASSUME(slot->once == ~0l);
		return ptr;
	}
	return slot->ptr;
}

__END_DECLS

#endif // __OS_ALLOC_ONCE_IMPL__
