/*
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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

/*-
 * Portions Copyright (c) 1992, 1993
 *  The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software donated to Berkeley by
 * Jan-Simon Pendry.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *  @(#)null.h  8.3 (Berkeley) 8/20/94
 *
 * $FreeBSD$
 */

#ifndef FS_NULL_H
#define FS_NULL_H

#include <sys/appleapiopts.h>
#include <libkern/libkern.h>
#include <sys/vnode.h>
#include <sys/vnode_if.h>
#include <sys/ubc.h>
#include <vfs/vfs_support.h>
#include <sys/lock.h>

#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/syslimits.h>

#if KERNEL
#include <libkern/tree.h>
#else
#include <System/libkern/tree.h>
#endif

//#define NULLFS_DEBUG 0

#define NULLM_CACHE 0x0001
#define NULLM_CASEINSENSITIVE 0x0000000000000002

typedef int (*vop_t)(void *);

struct null_mount {
	struct vnode * nullm_rootvp;       /* Reference to root null_node (inode 1) */
	struct vnode * nullm_secondvp;     /* Reference to virtual directory vnode to wrap app
	                                    *  bundles (inode 2) */
	struct vnode * nullm_thirdcovervp; /* Reference to vnode that covers
	                                    *  lowerrootvp  (inode 3) */
	struct vnode * nullm_lowerrootvp;  /* reference to the root of the tree we are
	                                   *  relocating (in the other file system) */
	uint32_t nullm_lowerrootvid;       /* store the lower root vid so we can check
	                                    *  before we build the shadow vnode lazily*/
	lck_mtx_t nullm_lock;              /* lock to protect vps above */
	uint64_t nullm_flags;
};

#ifdef KERNEL

#define NULL_FLAG_HASHED 0x000000001

/*
 * A cache of vnode references
 */
struct null_node {
	LIST_ENTRY(null_node) null_hash; /* Hash list */
	struct vnode * null_lowervp;     /* VREFed once */
	struct vnode * null_vnode;       /* Back pointer */
	uint32_t null_lowervid;          /* vid for lowervp to detect lowervp getting recycled out
	                                  *  from under us */
	uint32_t null_myvid;
	uint32_t null_flags;
};

struct vnodeop_desc_fake {
	int vdesc_offset;
	const char * vdesc_name;
	/* other stuff */
};

#define NULLV_NOUNLOCK 0x0001
#define NULLV_DROP 0x0002

#define MOUNTTONULLMOUNT(mp) ((struct null_mount *)(vfs_fsprivate(mp)))
#define VTONULL(vp) ((struct null_node *)vnode_fsnode(vp))
#define NULLTOV(xp) ((xp)->null_vnode)

__BEGIN_DECLS

int nullfs_init(struct vfsconf * vfsp);
int nullfs_init_lck(lck_mtx_t * lck);
int nullfs_destroy_lck(lck_mtx_t * lck);
int nullfs_uninit(void);
int null_nodeget(
	struct mount * mp, struct vnode * lowervp, struct vnode * dvp, struct vnode ** vpp, struct componentname * cnp, int root);
int null_hashget(struct mount * mp, struct vnode * lowervp, struct vnode ** vpp);
int null_getnewvnode(
	struct mount * mp, struct vnode * lowervp, struct vnode * dvp, struct vnode ** vpp, struct componentname * cnp, int root);
void null_hashrem(struct null_node * xp);

int nullfs_getbackingvnode(vnode_t in_vp, vnode_t* out_vpp);

#define NULLVPTOLOWERVP(vp) (VTONULL(vp)->null_lowervp)
#define NULLVPTOLOWERVID(vp) (VTONULL(vp)->null_lowervid)
#define NULLVPTOMYVID(vp) (VTONULL(vp)->null_myvid)

extern const struct vnodeopv_desc nullfs_vnodeop_opv_desc;

extern vop_t * nullfs_vnodeop_p;

__END_DECLS

#ifdef NULLFS_DEBUG
#define NULLFSDEBUG(format, args...) printf(format, ##args)
#else
#define NULLFSDEBUG(format, args...)
#endif /* NULLFS_DEBUG */

#endif /* KERNEL */

#endif
