/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
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

#ifndef __clang__

#include <stdint.h>
#include <libkern/OSAtomic.h>
#include <arm/arch.h>
#include <stdbool.h>

/*
 * Implement gcc atomic "builtins" in terms of OSAtomic routines,
 * since the ARM GCC target does not currently support them
 */

int32_t __sync_fetch_and_add_4 (int32_t *ptr, int32_t value, ...)
{
    return OSAtomicAdd32Barrier(value, ptr) - value;
}

int32_t __sync_fetch_and_sub_4 (int32_t *ptr, int32_t value, ...)
{
    return OSAtomicAdd32Barrier(-value, ptr) + value;
}

uint32_t __sync_fetch_and_or_4(uint32_t *ptr, uint32_t value, ...)
{
    return OSAtomicOr32OrigBarrier(value, ptr);
}

uint32_t __sync_fetch_and_and_4(uint32_t *ptr, uint32_t value, ...)
{
    return OSAtomicAnd32OrigBarrier(value, ptr);
}

uint32_t __sync_fetch_and_xor_4(uint32_t *ptr, uint32_t value, ...)
{
    return OSAtomicXor32OrigBarrier(value, ptr);
}

int32_t __sync_add_and_fetch_4 (int32_t *ptr, int32_t value, ...)
{
    return OSAtomicAdd32Barrier(value, ptr);
}

int32_t __sync_sub_and_fetch_4 (int32_t *ptr, int32_t value, ...)
{
    return OSAtomicAdd32Barrier(-value, ptr);
}

uint32_t __sync_or_and_fetch_4 (uint32_t *ptr, int32_t value, ...)
{
    return OSAtomicOr32Barrier(value, ptr);
}

uint32_t __sync_and_and_fetch_4 (uint32_t *ptr, int32_t value, ...)
{
    return OSAtomicAnd32Barrier(value, ptr);
}

uint32_t __sync_xor_and_fetch_4 (uint32_t *ptr, int32_t value, ...)
{
    return OSAtomicXor32Barrier(value, ptr);
}

bool __sync_bool_compare_and_swap_4(int32_t *ptr, int32_t oldval, int32_t newval, ...)
{
    return OSAtomicCompareAndSwap32Barrier(oldval, newval, ptr);
}

int32_t __sync_val_compare_and_swap_4(int32_t *ptr, int32_t oldval, int32_t newval, ...)
{
    int32_t old = *ptr;
    OSAtomicCompareAndSwap32Barrier(oldval, newval, ptr);
    return old;
}

int32_t __sync_lock_test_and_set_4(int32_t *ptr, int32_t value, ...)
{
    int32_t old;

    do {
		old = *ptr;
	} while (!OSAtomicCompareAndSwap32Barrier(old, value, ptr));

    return old;
}

void __sync_lock_release_4(int32_t *ptr, ...)
{
    *ptr = 0;
}

__private_extern__
void __sync_synchronize(void)
{
	OSMemoryBarrier();
}

#endif
