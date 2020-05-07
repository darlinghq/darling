/*
 * Copyright (c) 2000-2002 Apple Computer, Inc. All rights reserved.
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
/*
 * Copyright 1997,1998 Julian Elischer.  All rights reserved.
 * julian@freebsd.org
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * devfsdefs.h
 */
/*
 * NOTICE: This file was modified by McAfee Research in 2004 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */

/*
 * HISTORY
 *  8-April-1999 Dieter Siegmund (dieter@apple.com)
 *	Ported to from FreeBSD 3.1
 *	Removed unnecessary/unused defines
 *	Renamed structures/elements to clarify usage in code.
 */

#ifndef __DEVFS_DEVFSDEFS_H__
#define __DEVFS_DEVFSDEFS_H__

#include  <sys/appleapiopts.h>

__BEGIN_DECLS
#ifdef __APPLE_API_PRIVATE
#define DEVMAXNAMESIZE  32              /* XXX */
#define DEVMAXPATHSIZE  128             /* XXX */

typedef enum {
	DEV_DIR,
	DEV_BDEV,
	DEV_CDEV,
	DEV_SLNK,
#if FDESC
	DEV_DEVFD
#endif /* FDESC */
} devfstype_t;

extern int(**devfs_vnodeop_p)(void *);  /* our own vector array for dirs */
extern int(**devfs_spec_vnodeop_p)(void *);  /* our own vector array for devs */
extern const struct vfsops devfs_vfsops;

typedef struct devnode          devnode_t;
typedef struct devdirent        devdirent_t;
typedef union devnode_type      devnode_type_t;

struct devfs_stats {
	int                 nodes;
	int                 entries;
	int                 mounts;
	int                 stringspace;
};

union devnode_type {
	dev_t               dev;
	struct {
		devdirent_t *   dirlist;
		devdirent_t * * dirlast;
		devnode_t *     parent;
		devdirent_t *   myname; /* my entry in .. */
		int             entrycount;
	}Dir;
	struct {
		char *          name;/* must be allocated separately */
		int             namelen;
	}Slnk;
};

#define DEV_MAX_VNODE_RETRY  8          /* Max number of retries when we try to
	                                 *  get a vnode for the devnode */
struct devnode {
	devfstype_t         dn_type;
	/*
	 * Number of vnodes that point to this devnode.  Note, we do not
	 * add another reference for a lookup which finds an existing
	 * vnode; a reference is added when a vnode is created and removed
	 * when a vnode is reclaimed.  A devnode will not be freed while
	 * there are outstanding references.  A refcount can be added to
	 * prevent the free of a devnode in situations where there is not
	 * guaranteed to be a vnode holding a ref, but it is important to
	 * make sure that a deferred delete eventually happens if it is
	 * blocked behind that reference.
	 */
	os_ref_atomic_t     dn_refcount;
	u_short             dn_mode;
	uid_t               dn_uid;
	gid_t               dn_gid;
	struct timespec     dn_atime;/* time of last access */
	struct timespec     dn_mtime;/* time of last modification */
	struct timespec     dn_ctime;/* time file changed */
	int(***dn_ops)(void *);/* yuk... pointer to pointer(s) to funcs */
	int                 dn_links;/* how many file links does this node have? */
	struct devfsmount * dn_dvm; /* the mount structure for this 'plane' */
	struct vnode *      dn_vn;/* address of last vnode that represented us */
	int                 dn_len;/* of any associated info (e.g. dir data) */
	devdirent_t *       dn_linklist;/* circular list of hardlinks to this node */
	devnode_t *         dn_nextsibling;/* the list of equivalent nodes */
	devnode_t * *       dn_prevsiblingp;/* backpointer for the above */
	devnode_type_t      dn_typeinfo;
	int                 dn_change;
	int                 dn_update;
	int                 dn_access;
	int                 dn_lflags;
	ino_t               dn_ino;
	int                 (*dn_clone)(dev_t dev, int action);/* get minor # */
	struct label *      dn_label;   /* security label */
};

#define DN_DELETE               0x02
#define DN_CREATE               0x04
#define DN_CREATEWAIT           0x08


struct devdirent {
	/*-----------------------directory entry fields-------------*/
	char                de_name[DEVMAXNAMESIZE];
	devnode_t *         de_dnp;     /* the "inode" (devnode) pointer */
	devnode_t *         de_parent;  /* backpointer to the directory itself */
	devdirent_t *       de_next;    /* next object in this directory */
	devdirent_t *       *de_prevp;  /* previous pointer in directory linked list */
	devdirent_t *       de_nextlink;/* next hardlink to this node */
	devdirent_t *       *de_prevlinkp;/* previous hardlink pointer for this node */
};

extern devdirent_t *            dev_root;
extern struct devfs_stats       devfs_stats;
extern lck_mtx_t                devfs_mutex;
extern lck_mtx_t                devfs_attr_mutex;

/*
 * Rules for front nodes:
 * Dirs hava a strict 1:1 relationship with their OWN devnode
 * Symlinks similarly
 * Device Nodes ALWAYS point to the devnode that is linked
 * to the Backing node. (with a ref count)
 */

/*
 * DEVFS specific per/mount information, used to link a monted fs to a
 * particular 'plane' of front nodes.
 */
struct devfsmount {
	struct mount *      mount;/* vfs mount struct for this fs	*/
	devdirent_t *       plane_root;/* the root of this 'plane'	*/
};

/*
 * Prototypes for DEVFS virtual filesystem operations
 */
#include <sys/lock.h>
#include <miscfs/devfs/devfs_proto.h>
#include <libkern/OSAtomic.h>           /* required for OSAddAtomic() */

//#define HIDDEN_MOUNTPOINT	1

/* misc */
#define M_DEVFSNAME     M_DEVFS
#define M_DEVFSNODE     M_DEVFS
#define M_DEVFSMNT      M_DEVFS

#define VTODN(vp)       ((devnode_t *)(vp)->v_data)

#define DEVFS_LOCK()    lck_mtx_lock(&devfs_mutex)
#define DEVFS_UNLOCK()  lck_mtx_unlock(&devfs_mutex)

#define DEVFS_ATTR_LOCK_SPIN()  lck_mtx_lock_spin(&devfs_attr_mutex);
#define DEVFS_ATTR_UNLOCK()     lck_mtx_unlock(&devfs_attr_mutex);

/*
 * XXX all the (SInt32 *) casts below assume sizeof(int) == sizeof(long)
 */
static __inline__ void
DEVFS_INCR_ENTRIES(void)
{
	OSAddAtomic(1, &devfs_stats.entries);
}

static __inline__ void
DEVFS_DECR_ENTRIES(void)
{
	OSAddAtomic(-1, &devfs_stats.entries);
}

static __inline__ void
DEVFS_INCR_NODES(void)
{
	OSAddAtomic(1, &devfs_stats.nodes);
}

static __inline__ void
DEVFS_DECR_NODES(void)
{
	OSAddAtomic(-1, &devfs_stats.nodes);
}

static __inline__ void
DEVFS_INCR_MOUNTS(void)
{
	OSAddAtomic(1, &devfs_stats.mounts);
}

static __inline__ void
DEVFS_DECR_MOUNTS(void)
{
	OSAddAtomic(-1, &devfs_stats.mounts);
}

static __inline__ void
DEVFS_INCR_STRINGSPACE(int space)
{
	OSAddAtomic(space, &devfs_stats.stringspace);
}

static __inline__ void
DEVFS_DECR_STRINGSPACE(int space)
{
	OSAddAtomic(-space, &devfs_stats.stringspace);
}

/*
 * Access, change, and modify times are protected by a separate lock,
 * which allows tty times to be updated (no more than once per second)
 * in the I/O path without too much fear of contention.
 *
 * For getattr, update times to current time if the last update was recent;
 * preserve  legacy behavior that frequent stats can yield sub-second resolutions.
 * If the last time is old, however, we know that the event that triggered
 * the need for an update was no more than 1s after the last update.  In that case,
 * use (last update + 1s) as the time, avoiding the illusion that last update happened
 * much later than it really did.
 */
#define DEVFS_LAZY_UPDATE_SECONDS       1

#define DEVFS_UPDATE_CHANGE             0x1
#define DEVFS_UPDATE_MOD                0x2
#define DEVFS_UPDATE_ACCESS             0x4

static __inline__ void
dn_copy_times(devnode_t * target, devnode_t * source)
{
	DEVFS_ATTR_LOCK_SPIN();
	target->dn_atime = source->dn_atime;
	target->dn_mtime = source->dn_mtime;
	target->dn_ctime = source->dn_ctime;
	DEVFS_ATTR_UNLOCK();
	return;
}

#ifdef BSD_KERNEL_PRIVATE
int     devfs_make_symlink(devnode_t *dir_p, char *name, int mode, char *target, devdirent_t **newent);
#endif /* BSD_KERNEL_PRIVATE */

#endif /* __APPLE_API_PRIVATE */

__END_DECLS

#endif /* __DEVFS_DEVFSDEFS_H__ */
