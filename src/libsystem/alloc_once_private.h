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

#ifndef __OS_ALLOC__
#define __OS_ALLOC__

#ifndef __OS_ALLOC_INDIRECT__
#define __OS_ALLOC_INDIRECT__
#endif // __OS_ALLOC_INDIRECT__

#include <os/alloc_once_impl.h>

/* Keys for use with os_once_alloc. */

/* Keys for Libsystem. */
#define OS_ALLOC_ONCE_KEY_LIBSYSTEM_NOTIFY	0
#define OS_ALLOC_ONCE_KEY_LIBXPC	1
#define OS_ALLOC_ONCE_KEY_LIBSYSTEM_C	2
#define OS_ALLOC_ONCE_KEY_LIBSYSTEM_INFO	3
#define OS_ALLOC_ONCE_KEY_LIBSYSTEM_NETWORK	4
#define OS_ALLOC_ONCE_KEY_LIBCACHE	5
#define OS_ALLOC_ONCE_KEY_LIBCOMMONCRYPTO	6
#define OS_ALLOC_ONCE_KEY_LIBDISPATCH	7
#define OS_ALLOC_ONCE_KEY_LIBDYLD	8
#define OS_ALLOC_ONCE_KEY_LIBKEYMGR	9
#define OS_ALLOC_ONCE_KEY_LIBLAUNCH	10
#define OS_ALLOC_ONCE_KEY_LIBMACHO	11
#define OS_ALLOC_ONCE_KEY_OS_TRACE	12
#define OS_ALLOC_ONCE_KEY_LIBSYSTEM_BLOCKS	13
#define OS_ALLOC_ONCE_KEY_LIBSYSTEM_MALLOC	14
#define OS_ALLOC_ONCE_KEY_LIBSYSTEM_PLATFORM	15
#define OS_ALLOC_ONCE_KEY_LIBSYSTEM_PTHREAD	16
#define OS_ALLOC_ONCE_KEY_LIBSYSTEM_STATS	17
#define OS_ALLOC_ONCE_KEY_LIBSECINIT	18
#define OS_ALLOC_ONCE_KEY_LIBSYSTEM_CORESERVICES 19

/* Keys OS_ALLOC_ONCE_KEY_MAX - 10 upwards are reserved for the system. */
#define OS_ALLOC_ONCE_KEY_RESERVED_0	(OS_ALLOC_ONCE_KEY_MAX - 10)
#define OS_ALLOC_ONCE_KEY_RESERVED_1	(OS_ALLOC_ONCE_KEY_MAX - 9)
#define OS_ALLOC_ONCE_KEY_RESERVED_2	(OS_ALLOC_ONCE_KEY_MAX - 8)
#define OS_ALLOC_ONCE_KEY_RESERVED_3	(OS_ALLOC_ONCE_KEY_MAX - 7)
#define OS_ALLOC_ONCE_KEY_RESERVED_4	(OS_ALLOC_ONCE_KEY_MAX - 6)
#define OS_ALLOC_ONCE_KEY_RESERVED_5	(OS_ALLOC_ONCE_KEY_MAX - 5)
#define OS_ALLOC_ONCE_KEY_RESERVED_6	(OS_ALLOC_ONCE_KEY_MAX - 4)
#define OS_ALLOC_ONCE_KEY_RESERVED_7	(OS_ALLOC_ONCE_KEY_MAX - 3)
#define OS_ALLOC_ONCE_KEY_RESERVED_8	(OS_ALLOC_ONCE_KEY_MAX - 2)
#define OS_ALLOC_ONCE_KEY_RESERVED_9	(OS_ALLOC_ONCE_KEY_MAX - 1)

/* OS_ALLOC_ONCE_KEY_MAX cannot be used. */

#endif // __OS_ALLOC__
