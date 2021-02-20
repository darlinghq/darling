/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

#ifndef __PLATFORM_H
#define __PLATFORM_H

#if TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#define MALLOC_TARGET_IOS 1
#else // TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR
#define MALLOC_TARGET_IOS 0
#endif // TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR

#ifdef __LP64__
#define MALLOC_TARGET_64BIT 1
#else // __LP64__
#define MALLOC_TARGET_64BIT 0
#endif

// <rdar://problem/12596555>
#if MALLOC_TARGET_IOS
# define CONFIG_MADVISE_PRESSURE_RELIEF 0
#else // MALLOC_TARGET_IOS
# define CONFIG_MADVISE_PRESSURE_RELIEF 1
#endif // MALLOC_TARGET_IOS

// <rdar://problem/12596555>
#if MALLOC_TARGET_IOS
# define CONFIG_RECIRC_DEPOT 1
# define CONFIG_AGGRESSIVE_MADVISE 1
#else // MALLOC_TARGET_IOS
# define CONFIG_RECIRC_DEPOT 1
# define CONFIG_AGGRESSIVE_MADVISE 0
#endif // MALLOC_TARGET_IOS

// <rdar://problem/10397726>
#define CONFIG_RELAXED_INVARIANT_CHECKS 1

// <rdar://problem/19818071>
#define CONFIG_MADVISE_STYLE MADV_FREE_REUSABLE

#if MALLOC_TARGET_64BIT
#define CONFIG_NANOZONE 1
#define CONFIG_ASLR_INTERNAL 0
#else // MALLOC_TARGET_64BIT
#define CONFIG_NANOZONE 0
#define CONFIG_ASLR_INTERNAL 1
#endif // MALLOC_TARGET_64BIT

// enable nano checking for corrupt free list
#define NANO_FREE_DEQUEUE_DILIGENCE 1

// This governs a last-free cache of 1 that bypasses the free-list for each region size
#define CONFIG_TINY_CACHE 1
#define CONFIG_SMALL_CACHE 1
#define CONFIG_MEDIUM_CACHE 1

// medium allocator enabled or disabled
#if MALLOC_TARGET_64BIT
#if MALLOC_TARGET_IOS
#define CONFIG_MEDIUM_ALLOCATOR 0
#else // MALLOC_TARGET_IOS
#define CONFIG_MEDIUM_ALLOCATOR 1
#endif // MALLOC_TARGET_IOS
#else // MALLOC_TARGET_64BIT
#define CONFIG_MEDIUM_ALLOCATOR 0
#endif // MALLOC_TARGET_64BIT

// The large last-free cache (aka. death row cache)
#if MALLOC_TARGET_IOS
#define CONFIG_LARGE_CACHE 0
#else
#define CONFIG_LARGE_CACHE 1
#endif

#if MALLOC_TARGET_IOS
// The VM system on iOS forces malloc-tagged memory to never be marked as
// copy-on-write, this would include calls we make to vm_copy. Given that the
// kernel would just be doing a memcpy, we force it to happen in userpsace.
#define CONFIG_REALLOC_CAN_USE_VMCOPY 0
#else
#define CONFIG_REALLOC_CAN_USE_VMCOPY 1
#endif

// memory resource exception handling
#if MALLOC_TARGET_IOS || TARGET_OS_SIMULATOR
#define ENABLE_MEMORY_RESOURCE_EXCEPTION_HANDLING 0
#else
#define ENABLE_MEMORY_RESOURCE_EXCEPTION_HANDLING 1
#endif

// presence of commpage memsize
#define CONFIG_HAS_COMMPAGE_MEMSIZE 1

// presence of commpage number of cpu count
#define CONFIG_HAS_COMMPAGE_NCPUS 1

// Use of hyper-shift for magazine selection.
#define CONFIG_NANO_USES_HYPER_SHIFT 0
#define CONFIG_TINY_USES_HYPER_SHIFT 0
#define CONFIG_SMALL_USES_HYPER_SHIFT 0

#endif // __PLATFORM_H
