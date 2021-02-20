/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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
 
#ifndef __NANO_ZONE_COMMON_H
#define __NANO_ZONE_COMMON_H

#define NANO_MAX_SIZE			256 /* Buckets sized {16, 32, 48, ..., 256} */
#define SHIFT_NANO_QUANTUM		4
#define NANO_REGIME_QUANTA_SIZE	(1 << SHIFT_NANO_QUANTUM)	// 16
#define NANO_QUANTA_MASK		(NANO_REGIME_QUANTA_SIZE - 1)
#define NANO_SIZE_CLASSES		(NANO_MAX_SIZE/NANO_REGIME_QUANTA_SIZE)

#if TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR

// Nanozone follows the shared region.
#define SHIFT_NANO_SIGNATURE	29
#define NANOZONE_SIGNATURE_BITS	35
#define NANOZONE_BASE_REGION_ADDRESS (SHARED_REGION_BASE + SHARED_REGION_SIZE)
#define NANOZONE_SIGNATURE (NANOZONE_BASE_REGION_ADDRESS >> SHIFT_NANO_SIGNATURE)

#else // TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR

#define SHIFT_NANO_SIGNATURE	44
#define NANOZONE_SIGNATURE_BITS	20
#define NANOZONE_SIGNATURE		0x6ULL
#define NANOZONE_BASE_REGION_ADDRESS (NANOZONE_SIGNATURE << SHIFT_NANO_SIGNATURE)

#endif // TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR

static MALLOC_INLINE size_t
_nano_common_good_size(size_t size)
{
	return (size <= NANO_REGIME_QUANTA_SIZE) ? NANO_REGIME_QUANTA_SIZE
		: (((size + NANO_REGIME_QUANTA_SIZE - 1) >> SHIFT_NANO_QUANTUM) << SHIFT_NANO_QUANTUM);
}

#endif // __NANO_ZONE_COMMON_H
