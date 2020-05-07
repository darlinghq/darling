/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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

#ifndef MOCKFS_H
#define MOCKFS_H

#if MOCKFS

#include <kern/locks.h>
#include <miscfs/mockfs/mockfs_fsnode.h>
#include <sys/kernel_types.h>

/*
 * mockfs is effectively a "fake" filesystem; the primary motivation for it being that we may have cases
 *   where our userspace needs are extremely simple/consistent and can provided by a single binary.  mockfs
 *   uses an in-memory tree to define the structure for an extremely simple filesystem, which makes the
 *   assumption that our root device is in fact a mach-o file, and provides a minimal filesystem to support
 *   this:  the root directory, a mountpoint for devfs (given that very basic userspace code may assume the
 *   existance of /dev/), and an executable representing the root device.
 *
 * The functionality supported by mockfs is minimal: it is read-only, and does not support user initiated IO,
 *   but it supports lookup (so it should be possible for the user to access /dev/).
 *
 * mockfs is primarily targeted towards memory-backed devices, and will (when possible) attempt to inform the
 *   VM that we are using a memory-backed device, so that we can eschew IO to the backing device completely,
 *   and avoid having an extra copy of the data in the UBC (as well as the overhead associated with creating
 *   that copy).
 *
 * For the moment, mockfs is not marked in vfs_conf.c as being threadsafe.
 */

extern lck_attr_t     * mockfs_mtx_attr;
extern lck_grp_attr_t * mockfs_grp_attr;
extern lck_grp_t      * mockfs_mtx_grp;

struct mockfs_mount {
	lck_mtx_t       mockfs_mnt_mtx;         /* Mount-wide (and tree-wide) mutex */
	mockfs_fsnode_t mockfs_root;            /* Root of the node tree */
	boolean_t       mockfs_memory_backed;   /* Does the backing store reside in memory */
	boolean_t       mockfs_physical_memory; /* (valid if memory backed) */
	uint32_t        mockfs_memdev_base;     /* Base page of the backing store (valid if memory backed) */
	uint64_t        mockfs_memdev_size;     /* Size of the backing store (valid if memory backed) */
};

typedef struct mockfs_mount * mockfs_mount_t;

#endif /* MOCKFS */

#endif /* MOCKFS_H */
