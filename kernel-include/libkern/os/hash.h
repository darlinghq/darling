/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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

#ifndef _OS_HASH_H_
#define _OS_HASH_H_
#if PRIVATE

#include <os/base.h>

__BEGIN_DECLS

/*!
 * @function os_hash_jenkins
 *
 * @brief
 * The original Jenkins "one at a time" hash.
 *
 * @discussion
 * TBD: There may be some value to unrolling here,
 * depending on the architecture.
 *
 * @param data
 * The address of the data to hash.
 *
 * @param length
 * The length of the data to hash
 *
 * @returns
 * The jenkins hash for this data.
 */
static inline uint32_t
os_hash_jenkins(const void *data, size_t length)
{
	const uint8_t *key = (const uint8_t *)data;
	uint32_t hash = 0;

	for (size_t i = 0; i < length; i++) {
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

/*!
 * @function os_hash_kernel_pointer
 *
 * @brief
 * Hashes a pointer from a zone.
 *
 * @discussion
 * This is a really cheap and fast hash that will behave well for pointers
 * allocated by the kernel.
 *
 * This should be not used for untrusted pointer values from userspace,
 * or cases when the pointer is somehow under the control of userspace.
 *
 * This hash function utilizes knowledge about the span of the kernel
 * address space and inherent alignment of zalloc/kalloc.
 *
 * @param pointer
 * The pointer to hash.
 *
 * @returns
 * The hash for this pointer.
 */
static inline uint32_t
os_hash_kernel_pointer(const void *pointer)
{
	uintptr_t key = (uintptr_t)pointer >> 4;
	key *= 0x5052acdb;
	return (uint32_t)key ^ __builtin_bswap32((uint32_t)key);
}

__END_DECLS

#endif // PRIVATE
#endif // _OS_HASH_H_
