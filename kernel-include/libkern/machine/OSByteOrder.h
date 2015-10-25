/*
 * Copyright (c) 2000-2005 Apple Computer, Inc. All rights reserved.
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

#ifndef _OS_OSBYTEORDERMACHINE_H
#define _OS_OSBYTEORDERMACHINE_H

#include <stdint.h>

#if !defined(OS_INLINE)
# if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#        define OS_INLINE static inline
# elif defined(__MWERKS__) || defined(__cplusplus)
#        define OS_INLINE static inline
# else
#        define OS_INLINE static __inline__
# endif
#endif

/* Generic byte swapping functions. */

OS_INLINE
uint16_t
_OSSwapInt16(
    uint16_t                      data
)
{
    return OSSwapConstInt16(data);
}

OS_INLINE
uint32_t
_OSSwapInt32(
    uint32_t                      data
)
{
    return OSSwapConstInt32(data);
}

OS_INLINE
uint64_t
_OSSwapInt64(
    uint64_t                        data
)
{
    return OSSwapConstInt64(data);
}

/* Functions for byte reversed loads. */

OS_INLINE
uint16_t
OSReadSwapInt16(
    const volatile void               * base,
    uintptr_t                     byteOffset
)
{
    uint16_t data = *(volatile uint16_t *)((uintptr_t)base + byteOffset);
    return _OSSwapInt16(data);
}

OS_INLINE
uint32_t
OSReadSwapInt32(
    const volatile void               * base,
    uintptr_t                     byteOffset
)
{
    uint32_t data = *(volatile uint32_t *)((uintptr_t)base + byteOffset);
    return _OSSwapInt32(data);
}

OS_INLINE
uint64_t
OSReadSwapInt64(
    const volatile void               * base,
    uintptr_t                     byteOffset
)
{
    uint64_t data = *(volatile uint64_t *)((uintptr_t)base + byteOffset);
    return _OSSwapInt64(data);
}

/* Functions for byte reversed stores. */

OS_INLINE
void
OSWriteSwapInt16(
    volatile void               * base,
    uintptr_t                     byteOffset,
    uint16_t                      data
)
{
    *(volatile uint16_t *)((uintptr_t)base + byteOffset) = _OSSwapInt16(data);
}

OS_INLINE
void
OSWriteSwapInt32(
    volatile void               * base,
    uintptr_t                     byteOffset,
    uint32_t                      data
)
{
    *(volatile uint32_t *)((uintptr_t)base + byteOffset) = _OSSwapInt32(data);
}

OS_INLINE
void
OSWriteSwapInt64(
    volatile void               * base,
    uintptr_t                     byteOffset,
    uint64_t                      data
)
{
    *(volatile uint64_t *)((uintptr_t)base + byteOffset) = _OSSwapInt64(data);
}

#endif /* ! _OS_OSBYTEORDERMACHINE_H */
