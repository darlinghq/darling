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

#ifndef __DEBUG_H
#define __DEBUG_H

// set to one to debug malloc itself
#define DEBUG_MALLOC 0
// set to one to debug malloc client
#define DEBUG_CLIENT 0
#define DEBUG_MADVISE 0

#if DEBUG_MALLOC
#   warning DEBUG_MALLOC ENABLED
#   undef MALLOC_INLINE
#   undef MALLOC_UNUSED
#   undef MALLOC_ALWAYS_INLINE
#   undef CHECK_MAGAZINE_PTR_LOCKED

#   define MALLOC_INLINE
#   define MALLOC_UNUSED
#   define MALLOC_ALWAYS_INLINE
#   define CHECK_MAGAZINE_PTR_LOCKED(szone, mag_ptr, fun)				\
	do {										\
	    if (TRY_LOCK(mag_ptr->magazine_lock)) {					\
		malloc_printf("*** magazine_lock was not set %p in %s\n",		\
		mag_ptr->magazine_lock, fun);						\
	    }										\
	} while (0)
#endif // DEBUG_MALLOC

#if DEBUG_MALLOC || DEBUG_CLIENT
#	define CHECK(szone, fun) \
	if ((szone)->debug_flags & CHECK_REGIONS) { \
		szone_check_all(szone, fun) \
	}
#else // DEBUG_MALLOC || DEBUG_CLIENT
#	define CHECK(szone, fun) \
	do {} while (0)
#endif // DEBUG_MALLOC || DEBUG_CLIENT

#endif // __DEBUG_H
