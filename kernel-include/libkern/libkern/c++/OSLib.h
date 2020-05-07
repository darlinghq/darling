/*
 * Copyright (c) 1999-2006 Apple Computer, Inc. All rights reserved.
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
#ifndef _OS_OSLIB_H
#define _OS_OSLIB_H

#include <libkern/OSBase.h>

#ifdef KERNEL
#define MACH_ASSERT 1
#endif

__BEGIN_DECLS

#include <stdarg.h>
#include <sys/systm.h>

#include <kern/assert.h>
#ifdef KERNEL_PRIVATE
#include <kern/kalloc.h>
#endif

__END_DECLS


#if XNU_KERNEL_PRIVATE
#include <libkern/OSAtomic.h>

#define kalloc_container(size)  \
	({ kalloc_tag_bt(size, VM_KERN_MEMORY_LIBKERN); })

#define kallocp_container(size) \
	({ kallocp_tag_bt(size, VM_KERN_MEMORY_LIBKERN); })

#if OSALLOCDEBUG
extern "C" int debug_container_malloc_size;
extern "C" int debug_ivars_size;
#if IOTRACKING
#define OSCONTAINER_ACCUMSIZE(s) do { OSAddAtomic((SInt32)(s), &debug_container_malloc_size); trackingAccumSize(s); } while(0)
#else
#define OSCONTAINER_ACCUMSIZE(s) do { OSAddAtomic((SInt32)(s), &debug_container_malloc_size); } while(0)
#endif
#define OSMETA_ACCUMSIZE(s)      do { OSAddAtomic((SInt32)(s), &debug_container_malloc_size); } while(0)
#define OSIVAR_ACCUMSIZE(s)      do { OSAddAtomic((SInt32)(s), &debug_ivars_size);            } while(0)

#else /* OSALLOCDEBUG */

#define OSCONTAINER_ACCUMSIZE(s)
#define OSMETA_ACCUMSIZE(s)
#define OSIVAR_ACCUMSIZE(s)

#endif  /* !OSALLOCDEBUG */
#endif  /* XNU_KERNEL_PRIVATE */

#ifndef NULL
#if defined (__cplusplus)
#if __cplusplus >= 201103L
#define NULL nullptr
#else
#define NULL 0
#endif
#else
#define NULL ((void *)0)
#endif
#endif

#endif /* _OS_OSLIB_H  */
