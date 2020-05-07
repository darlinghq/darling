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
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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
 *	@(#)fifo.h	8.3 (Berkeley) 8/10/94
 */
#ifndef __FIFOFS_FOFO_H__
#define __FIFOFS_FOFO_H__

__BEGIN_DECLS

#ifdef BSD_KERNEL_PRIVATE


/*
 * This structure is associated with the FIFO vnode and stores
 * the state associated with the FIFO.
 */
struct fifoinfo {
	unsigned int    fi_flags;
	struct socket   *fi_readsock;
	struct socket   *fi_writesock;
	long        fi_readers;
	long        fi_writers;
	unsigned int    fi_count;
};

#define FIFO_INCREATE   1
#define FIFO_CREATEWAIT 2
#define FIFO_CREATED    4

int     fifo_close_internal(vnode_t, int, vfs_context_t, int);
int fifo_freespace(struct vnode *vp, long *count);
int fifo_charcount(struct vnode *vp, int *count);

#endif /* BSD_KERNEL_PRIVATE */
#ifdef KERNEL
/*
 * Prototypes for fifo operations on vnodes.
 */
int     fifo_ebadf(void *);

#define fifo_create (int (*) (struct  vnop_create_args *))err_create
#define fifo_mknod (int (*) (struct  vnop_mknod_args *))err_mknod
#define fifo_access (int (*) (struct  vnop_access_args *))fifo_ebadf
#define fifo_getattr (int (*) (struct  vnop_getattr_args *))fifo_ebadf
#define fifo_setattr (int (*) (struct  vnop_setattr_args *))fifo_ebadf
#define fifo_revoke nop_revoke
#define fifo_mmap (int (*) (struct  vnop_mmap_args *))err_mmap
#define fifo_fsync (int (*) (struct  vnop_fsync_args *))nullop
#define fifo_remove (int (*) (struct  vnop_remove_args *))err_remove
#define fifo_link (int (*) (struct  vnop_link_args *))err_link
#define fifo_rename (int (*) (struct  vnop_rename_args *))err_rename
#define fifo_mkdir (int (*) (struct  vnop_mkdir_args *))err_mkdir
#define fifo_rmdir (int (*) (struct  vnop_rmdir_args *))err_rmdir
#define fifo_symlink (int (*) (struct  vnop_symlink_args *))err_symlink
#define fifo_readdir (int (*) (struct  vnop_readdir_args *))err_readdir
#define fifo_readlink (int (*) (struct  vnop_readlink_args *))err_readlink
#define fifo_reclaim (int (*) (struct  vnop_reclaim_args *))nullop
#define fifo_strategy (int (*) (struct  vnop_strategy_args *))err_strategy
#define fifo_valloc (int (*) (struct  vnop_valloc_args *))err_valloc
#define fifo_vfree (int (*) (struct  vnop_vfree_args *))err_vfree
#define fifo_bwrite (int (*) (struct  vnop_bwrite_args *))nullop
#define fifo_blktooff (int (*) (struct vnop_blktooff_args *))err_blktooff

int     fifo_lookup(struct vnop_lookup_args *);
int     fifo_open(struct vnop_open_args *);
int     fifo_close(struct vnop_close_args *);
int     fifo_read(struct vnop_read_args *);
int     fifo_write(struct vnop_write_args *);
int     fifo_ioctl(struct vnop_ioctl_args *);
int     fifo_select(struct vnop_select_args *);
int     fifo_inactive(struct  vnop_inactive_args *);
int     fifo_pathconf(struct vnop_pathconf_args *);
int     fifo_advlock(struct vnop_advlock_args *);

#endif /* KERNEL */

__END_DECLS

#endif /* __FIFOFS_FOFO_H__ */
