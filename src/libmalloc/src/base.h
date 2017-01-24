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

#ifndef __BASE_H
#define __BASE_H

#ifndef __has_extension
#define __has_extension(x) 0
#endif

#if __has_extension(c_static_assert)
#define MALLOC_STATIC_ASSERT(x, y) _Static_assert((x), y)
#else
#define MALLOC_STATIC_ASSERT(x, y)
#endif

#define MALLOC_ASSERT(e) ({ \
	if (__builtin_expect(!(e), 0)) { \
		__asm__ __volatile__ (""); \
		__builtin_trap(); \
	} \
})

#define MALLOC_FATAL_ERROR(cause, message) ({ \
		_os_set_crash_log_cause_and_message((cause), "FATAL ERROR - " message); \
		__asm__ __volatile__ (""); \
		__builtin_trap(); \
})

#define MALLOC_PRINTF_FATAL_ERROR(cause, message) ({ \
		malloc_printf("*** FATAL ERROR - " message ".\n"); \
		MALLOC_FATAL_ERROR((cause), message); \
})

#if defined(__i386__) || defined(__x86_64__) || defined(__arm__) || defined(__arm64__)
#   define __APPLE_API_PRIVATE
#   include <machine/cpu_capabilities.h>
#   define _COMM_PAGE_VERSION_REQD 9
#   undef __APPLE_API_PRIVATE
#else
#   include <sys/sysctl.h>
#endif

#if defined(__i386__) || defined(__x86_64__)
// <rdar://problem/23495834> nano vs. magazine have different definitions
// for this cache-line size.
#   define MALLOC_CACHE_LINE 128
#   define MALLOC_NANO_CACHE_LINE 64
#elif defined(__arm__) || defined(__arm64__)
#   define MALLOC_CACHE_LINE 64
#   define MALLOC_NANO_CACHE_LINE 64
#else
#   define MALLOC_CACHE_LINE 32
#   define MALLOC_NANO_CACHE_LINE 32
#endif

#define MALLOC_CACHE_ALIGN __attribute__ ((aligned (MALLOC_CACHE_LINE) ))
#define MALLOC_NANO_CACHE_ALIGN __attribute__ ((aligned (MALLOC_NANO_CACHE_LINE) ))
#define MALLOC_EXPORT extern __attribute__((visibility("default")))
#define MALLOC_NOEXPORT __attribute__((visibility("hidden")))
#define MALLOC_NOINLINE __attribute__((noinline))
#define MALLOC_INLINE __inline__
#define MALLOC_ALWAYS_INLINE __attribute__((always_inline))
#define MALLOC_PACKED __attribute__((packed))
#define MALLOC_USED __attribute__((used))
#define MALLOC_UNUSED __attribute__((unused))
#define CHECK_MAGAZINE_PTR_LOCKED(szone, mag_ptr, fun) {}

#define SCRIBBLE_BYTE 0xaa /* allocated scribble */
#define SCRABBLE_BYTE 0x55 /* free()'d scribble */
#define SCRUBBLE_BYTE 0xdd /* madvise(..., MADV_FREE) scriblle */

#define NDEBUG 1
#define trunc_page_quanta(x) trunc_page((x))
#define round_page_quanta(x) round_page((x))
#define vm_page_quanta_size (vm_page_size)
#define vm_page_quanta_shift (vm_page_shift)

// add a guard page before and after each VM region to help debug
#define MALLOC_ADD_GUARD_PAGES (1 << 0)
// do not protect prelude page
#define MALLOC_DONT_PROTECT_PRELUDE (1 << 1)
// do not protect postlude page
#define MALLOC_DONT_PROTECT_POSTLUDE (1 << 2)
// write 0x55 onto free blocks
#define MALLOC_DO_SCRIBBLE (1 << 3)
// call abort() on any malloc error, such as double free or out of memory.
#define MALLOC_ABORT_ON_ERROR (1 << 4)
// allocate objects such that they may be used with VM purgability APIs
#define MALLOC_PURGEABLE (1 << 5)
// call abort() on malloc errors, but not on out of memory.
#define MALLOC_ABORT_ON_CORRUPTION (1 << 6)

/*
 * msize - a type to refer to the number of quanta of a tiny or small
 * allocation.  A tiny block with an msize of 3 would be 3 << SHIFT_TINY_QUANTUM
 * bytes in size.
 */
typedef unsigned short msize_t;

typedef unsigned int grain_t; // N.B. wide enough to index all free slots
typedef struct large_entry_s large_entry_t;
typedef struct szone_s szone_t;
typedef struct magazine_s magazine_t;
typedef int mag_index_t;
typedef void *region_t;

#endif // __BASE_H
