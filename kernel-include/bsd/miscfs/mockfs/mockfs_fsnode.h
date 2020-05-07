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

#ifndef MOCKFS_FSNODE_H
#define MOCKFS_FSNODE_H

#if MOCKFS

#include <sys/kernel_types.h>

/*
 * Types for the filesystem nodes; for the moment, these will effectively serve as unique
 * identifiers. This can be generalized later, but at least for the moment, the read-only
 * nature of the filesystem and the terse semantics (you have VDIR and VREG, and VREG
 * always represents the entire backing device) makes this sufficient for now.
 *
 * TODO: Should this include MOCKFS_SBIN?  Right now we tell lookup that when looking in
 *   MOCKFS_ROOT, "sbin" resolves back onto MOCKFS_ROOT; this is a handy hack for aliasing,
 *   but may not mesh well with VFS.
 */
enum mockfs_fsnode_type {
	MOCKFS_ROOT,
	MOCKFS_DEV,
	MOCKFS_FILE
};

/*
 * For the moment, pretend everything is a directory with support for two entries; the
 *   executable binary is a one-to-one mapping with the backing devnode, so this may
 *   actually be all we're interested in.
 *
 * Stash the filesize in here too (this is easier then looking at the devnode for every
 *   VREG access).
 */
struct mockfs_fsnode {
	uint64_t               size;    /* Bytes of data; 0 unless type is MOCKFS_FILE */
	uint8_t                type;    /* Serves as a unique identifier for now */
	mount_t                mnt;     /* The mount that this node belongs to */
	vnode_t                vp;      /* vnode for this node (if one exists) */
	struct mockfs_fsnode * parent;  /* Parent of this node (NULL for root) */
	                                /* TODO: Replace child_a/child_b with something more flexible */
	struct mockfs_fsnode * child_a; /* TEMPORARY */
	struct mockfs_fsnode * child_b; /* TEMPORARY */
};

typedef struct mockfs_fsnode * mockfs_fsnode_t;

/*
 * See mockfs_fsnode.c for function details.
 */
int mockfs_fsnode_create(mount_t mp, uint8_t type, mockfs_fsnode_t * fsnpp);
int mockfs_fsnode_destroy(mockfs_fsnode_t fsnp);
int mockfs_fsnode_adopt(mockfs_fsnode_t parent, mockfs_fsnode_t child);
int mockfs_fsnode_orphan(mockfs_fsnode_t fsnp);
int mockfs_fsnode_child_by_type(mockfs_fsnode_t parent, uint8_t type, mockfs_fsnode_t * child);
int mockfs_fsnode_vnode(mockfs_fsnode_t fsnp, vnode_t * vpp);
int mockfs_fsnode_drop_vnode(mockfs_fsnode_t fsnp);

#endif /* MOCKFS */

#endif /* MOCKFS_FSNODE_H */
