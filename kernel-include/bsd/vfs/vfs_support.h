/*
 * Copyright (c) 2000-2004 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1998 Apple Computer, Inc.  All rights reserved.
 *
 *  File:  vfs/vfs_support.h
 *
 *	Prototypes for the default vfs routines. A VFS plugin can use these
 *	functions in case it does not want to implement all. These functions
 *	take care of releasing locks and free up memory that they are
 *	supposed to.
 *
 * HISTORY
 *  18-Aug-1998 Umesh Vaishampayan (umeshv@apple.com)
 *      Created.
 */

#ifndef _VFS_VFS_SUPPORT_H_
#define _VFS_VFS_SUPPORT_H_

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/proc.h>
#include <sys/conf.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/vnode_if.h>
#include <sys/malloc.h>
#include <sys/dirent.h>

__BEGIN_DECLS
extern int nop_create(struct vnop_create_args *ap);
extern int err_create(struct vnop_create_args *ap);

extern int nop_whiteout(struct vnop_whiteout_args *ap);
extern int err_whiteout(struct vnop_whiteout_args *ap);

extern int nop_mknod(struct vnop_mknod_args *ap);
extern int err_mknod(struct vnop_mknod_args *ap);

extern int nop_open(struct vnop_open_args *ap);
extern int err_open(struct vnop_open_args *ap);

extern int nop_close(struct vnop_close_args *ap);
extern int err_close(struct vnop_close_args *ap);

extern int nop_access(struct vnop_access_args *ap);
extern int err_access(struct vnop_access_args *ap);

extern int nop_getattr(struct vnop_getattr_args *ap);
extern int err_getattr(struct vnop_getattr_args *ap);

extern int nop_setattr(struct vnop_setattr_args *ap);
extern int err_setattr(struct vnop_setattr_args *ap);

extern int nop_read(struct vnop_read_args *ap);
extern int err_read(struct vnop_read_args *ap);

extern int nop_write(struct vnop_write_args *ap);
extern int err_write(struct vnop_write_args *ap);

extern int nop_ioctl(struct vnop_ioctl_args *ap);
extern int err_ioctl(struct vnop_ioctl_args *ap);

extern int nop_select(struct vnop_select_args *ap);
extern int err_select(struct vnop_select_args *ap);

extern int nop_exchange(struct vnop_exchange_args *ap);
extern int err_exchange(struct vnop_exchange_args *ap);

extern int nop_revoke(struct vnop_revoke_args *ap);
extern int err_revoke(struct vnop_revoke_args *ap);

extern int nop_mmap(struct vnop_mmap_args *ap);
extern int err_mmap(struct vnop_mmap_args *ap);

extern int nop_fsync(struct vnop_fsync_args *ap);
extern int err_fsync(struct vnop_fsync_args *ap);

extern int nop_remove(struct vnop_remove_args *ap);
extern int err_remove(struct vnop_remove_args *ap);

extern int nop_link(struct vnop_link_args *ap);
extern int err_link(struct vnop_link_args *ap);

extern int nop_rename(struct vnop_rename_args *ap);
extern int err_rename(struct vnop_rename_args *ap);

extern int nop_mkdir(struct vnop_mkdir_args *ap);
extern int err_mkdir(struct vnop_mkdir_args *ap);

extern int nop_rmdir(struct vnop_rmdir_args *ap);
extern int err_rmdir(struct vnop_rmdir_args *ap);

extern int nop_symlink(struct vnop_symlink_args *ap);
extern int err_symlink(struct vnop_symlink_args *ap);

extern int nop_readdir(struct vnop_readdir_args *ap);
extern int err_readdir(struct vnop_readdir_args *ap);

extern int nop_readdirattr(struct vnop_readdirattr_args *ap);
extern int err_readdirattr(struct vnop_readdirattr_args *ap);

extern int nop_readlink(struct vnop_readlink_args *ap);
extern int err_readlink(struct vnop_readlink_args *ap);

extern int nop_inactive(struct vnop_inactive_args *ap);
extern int err_inactive(struct vnop_inactive_args *ap);

extern int nop_reclaim(struct vnop_reclaim_args *ap);
extern int err_reclaim(struct vnop_reclaim_args *ap);


extern int nop_strategy(struct vnop_strategy_args *ap);
extern int err_strategy(struct vnop_strategy_args *ap);

extern int nop_pathconf(struct vnop_pathconf_args *ap);
extern int err_pathconf(struct vnop_pathconf_args *ap);

extern int nop_advlock(struct vnop_advlock_args *ap);
extern int err_advlock(struct vnop_advlock_args *ap);


extern int nop_allocate(struct vnop_allocate_args *ap);
extern int err_allocate(struct vnop_allocate_args *ap);

extern int nop_bwrite(struct vnop_bwrite_args *ap);
extern int err_bwrite(struct vnop_bwrite_args *ap);

extern int nop_pagein(struct vnop_pagein_args *ap);
extern int err_pagein(struct vnop_pagein_args *ap);

extern int nop_pageout(struct vnop_pageout_args *ap);
extern int err_pageout(struct vnop_pageout_args *ap);

extern int nop_searchfs(struct vnop_searchfs_args *ap);
extern int err_searchfs(struct vnop_searchfs_args *ap);

extern int nop_copyfile(struct vnop_copyfile_args *ap);
extern int err_copyfile(struct vnop_copyfile_args *ap);

extern int nop_blktooff(struct vnop_blktooff_args *ap);
extern int err_blktooff(struct vnop_blktooff_args *ap);

extern int nop_offtoblk(struct vnop_offtoblk_args *ap);
extern int err_offtoblk(struct vnop_offtoblk_args *ap);

extern int nop_blockmap(struct vnop_blockmap_args *ap);
extern int err_blockmap(struct vnop_blockmap_args *ap);
__END_DECLS

#endif  /* _VFS_VFS_SUPPORT_H_ */
