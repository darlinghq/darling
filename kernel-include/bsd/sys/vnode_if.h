/*
 * Copyright (c) 2000-2016 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved
 * Copyright (c) 1992, 1993, 1994, 1995
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
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS AND
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
 */
/*
 * NOTICE: This file was modified by SPARTA, Inc. in 2005 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */

/*
 * Warning: This file is generated automatically.
 * (Modifications made here may easily be lost!)
 *
 * Created by the script:
 *	@(#)vnode_if.sh	8.7 (Berkeley) 5/11/95
 */


#ifndef _SYS_VNODE_IF_H_
#define _SYS_VNODE_IF_H_

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>
#include <sys/kernel_types.h>
#include <sys/buf.h>
#ifdef BSD_KERNEL_PRIVATE
#include <sys/vm.h>
#endif
#include <mach/memory_object_types.h>


#ifdef KERNEL

extern struct vnodeop_desc vnop_default_desc;
extern struct vnodeop_desc vnop_lookup_desc;
#ifdef KERNEL_PRIVATE
extern struct vnodeop_desc vnop_compound_open_desc;
extern struct vnodeop_desc vnop_compound_remove_desc;
extern struct vnodeop_desc vnop_compound_rename_desc;
extern struct vnodeop_desc vnop_compound_mkdir_desc;
extern struct vnodeop_desc vnop_compound_rmdir_desc;
#endif /* KERNEL_PRIVATE */
extern struct vnodeop_desc vnop_create_desc;
extern struct vnodeop_desc vnop_whiteout_desc; // obsolete
extern struct vnodeop_desc vnop_mknod_desc;
extern struct vnodeop_desc vnop_open_desc;
extern struct vnodeop_desc vnop_close_desc;
extern struct vnodeop_desc vnop_access_desc;
extern struct vnodeop_desc vnop_getattr_desc;
extern struct vnodeop_desc vnop_setattr_desc;
extern struct vnodeop_desc vnop_read_desc;
extern struct vnodeop_desc vnop_write_desc;
extern struct vnodeop_desc vnop_ioctl_desc;
extern struct vnodeop_desc vnop_select_desc;
extern struct vnodeop_desc vnop_exchange_desc;
extern struct vnodeop_desc vnop_revoke_desc;
extern struct vnodeop_desc vnop_mmap_check_desc;
extern struct vnodeop_desc vnop_mmap_desc;
extern struct vnodeop_desc vnop_mnomap_desc;
extern struct vnodeop_desc vnop_fsync_desc;
extern struct vnodeop_desc vnop_remove_desc;
extern struct vnodeop_desc vnop_link_desc;
extern struct vnodeop_desc vnop_rename_desc;
extern struct vnodeop_desc vnop_renamex_desc;
extern struct vnodeop_desc vnop_mkdir_desc;
extern struct vnodeop_desc vnop_rmdir_desc;
extern struct vnodeop_desc vnop_symlink_desc;
extern struct vnodeop_desc vnop_readdir_desc;
extern struct vnodeop_desc vnop_readdirattr_desc;
extern struct vnodeop_desc vnop_getattrlistbulk_desc;
extern struct vnodeop_desc vnop_readlink_desc;
extern struct vnodeop_desc vnop_inactive_desc;
extern struct vnodeop_desc vnop_reclaim_desc;
extern struct vnodeop_desc vnop_print_desc;
extern struct vnodeop_desc vnop_pathconf_desc;
extern struct vnodeop_desc vnop_advlock_desc;
extern struct vnodeop_desc vnop_truncate_desc;
extern struct vnodeop_desc vnop_allocate_desc;
extern struct vnodeop_desc vnop_pagein_desc;
extern struct vnodeop_desc vnop_pageout_desc;
extern struct vnodeop_desc vnop_searchfs_desc;
extern struct vnodeop_desc vnop_copyfile_desc;
extern struct vnodeop_desc vnop_clonefile_desc;
extern struct vnodeop_desc vnop_blktooff_desc;
extern struct vnodeop_desc vnop_offtoblk_desc;
extern struct vnodeop_desc vnop_blockmap_desc;
extern struct vnodeop_desc vnop_strategy_desc;
extern struct vnodeop_desc vnop_bwrite_desc;

#ifdef __APPLE_API_UNSTABLE

#if NAMEDSTREAMS
extern struct vnodeop_desc vnop_getnamedstream_desc;
extern struct vnodeop_desc vnop_makenamedstream_desc;
extern struct vnodeop_desc vnop_removenamedstream_desc;
#endif

#endif

#ifdef KERNEL_PRIVATE
/*
 * This pair of functions register and unregister callout with
 * buffer_cache_gc() code path. This callout enables underlying
 * fs to kick off any memory reclamation that would be otherwise
 * satisfied by buffer_cache_gc(). callout() will be called in the
 * vm_pageout code path, so precautions should be taken to not
 * allocate memory or take any locks which might have memory
 * allocation behind them. callout() can be called with first parameter
 * set to false, in which case memory reclamation should be
 * limited in scope. In case of the first parameter set to true, fs
 * MUST free some memory if possible. Second parameter to the
 * register function will be passed as a second parameter to the
 * callout() as is.
 * fs_buffer_cache_gc_unregister() second parameter will be used
 * to distinguish between same callout() and this parameter should
 * match the one passed during registration. It will unregister all
 * instances of the matching callout() and argument from the callout
 * list.
 */


extern int fs_buffer_cache_gc_register(void (* callout)(int, void *), void *);
extern int fs_buffer_cache_gc_unregister(void (* callout)(int, void *), void *);
#endif

__BEGIN_DECLS

struct vnop_lookup_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_dvp;
	vnode_t *a_vpp;
	struct componentname *a_cnp;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_LOOKUP
 *  @abstract Call down to a filesystem to look for a directory entry by name.
 *  @discussion VNOP_LOOKUP is the key pathway through which VFS asks a filesystem to find a file.  The vnode
 *  should be returned with an iocount to be dropped by the caller.  A VNOP_LOOKUP() calldown can come without
 *  a preceding VNOP_OPEN().
 *  @param dvp Directory in which to look up file.
 *  @param vpp Destination for found vnode.
 *  @param cnp Structure describing filename to find, reason for lookup, and various other data.
 *  @param ctx Context against which to authenticate lookup request.
 *  @return 0 for success or a filesystem-specific error.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_LOOKUP(vnode_t, vnode_t *, struct componentname *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_create_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_dvp;
	vnode_t *a_vpp;
	struct componentname *a_cnp;
	struct vnode_attr *a_vap;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_CREATE
 *  @abstract Call down to a filesystem to create a regular file (VREG).
 *  @discussion If file creation succeeds, "vpp" should be returned with an iocount to be dropped by the caller.
 *  A VNOP_CREATE() calldown can come without a preceding VNOP_OPEN().
 *  @param dvp Directory in which to create file.
 *  @param vpp Destination for vnode for newly created file.
 *  @param cnp Description of filename to create.
 *  @param vap File creation properties, as seen in vnode_getattr().  Manipulated with VATTR_ISACTIVE, VATTR_RETURN,
 *  VATTR_SET_SUPPORTED, and so forth.
 *  @param ctx Context against which to authenticate file creation.
 *  @return 0 for success or a filesystem-specific error.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_CREATE(vnode_t, vnode_t *, struct componentname *, struct vnode_attr *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_whiteout_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_dvp;
	struct componentname *a_cnp;
	int a_flags;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_WHITEOUT
 *  @abstract Obsolete - no longer supported.
 *  @discussion Whiteouts are used to support the union filesystem, whereby one filesystem is mounted "transparently"
 *  on top of another.  A whiteout in the upper layer of a union mount is a "deletion" of a file in the lower layer;
 *  lookups will catch the whiteout and fail, setting ISWHITEOUT in the componentname structure, even if an underlying
 *  file of the same name exists.  The whiteout vnop is used for creation, deletion, and checking whether a directory
 *  supports whiteouts (see flags).
 *  also support the LOOKUP flag, which is used to test whether a directory supports whiteouts.
 *  @param dvp Directory in which to create.
 *  @param cnp Name information for whiteout.
 *  @param flags CREATE: create a whiteout. LOOKUP: check whether a directory supports whiteouts, DELETE: remove a whiteout.
 *  @param ctx Context against which to authenticate whiteout creation.
 *  @return 0 for success or a filesystem-specific error.  Returning 0 for LOOKUP indicates that a directory does support whiteouts.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_WHITEOUT(vnode_t, struct componentname *, int, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_mknod_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_dvp;
	vnode_t *a_vpp;
	struct componentname *a_cnp;
	struct vnode_attr *a_vap;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_MKNOD
 *  @abstract Call down to a filesystem to create a special file.
 *  @discussion The mknod vnop is used to create character and block device files, named pipe (FIFO) files, and named sockets.
 *  The newly created file should be returned with an iocount which will be dropped by the caller.  A VNOP_MKNOD() call
 *  can come down without a preceding VNOP_OPEN().
 *  @param dvp Directory in which to create the special file.
 *  @param vpp Destination for newly created vnode.
 *  @param cnp Name information for new file.
 *  @param vap Attributes for new file, including type.
 *  @param ctx Context against which to authenticate node creation.
 *  @return 0 for success or a filesystem-specific error.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_MKNOD(vnode_t, vnode_t *, struct componentname *, struct vnode_attr *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_open_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	int a_mode;
	vfs_context_t a_context;
};

#ifdef KERNEL_PRIVATE
struct vnop_compound_open_args {
	struct vnodeop_desc *a_desc;

	vnode_t a_dvp;                          /* Directory in which to open/create */
	vnode_t *a_vpp;                         /* Resulting vnode */
	int a_fmode;                            /* Open mode */
	struct componentname *a_cnp;            /* Path to look up */
	struct vnode_attr *a_vap;               /* Attributes with which to create, if appropriate */
	uint32_t a_flags;                       /* VNOP-control flags */
	uint32_t *a_status;                     /* Information about results */

	vfs_context_t a_context;                        /* Authorization context */

	int (*a_open_create_authorizer)(        /* Authorizer for create case */
		vnode_t dvp,                            /* Directory in which to create */
		struct componentname *cnp,              /* As passed to VNOP */
		struct vnode_attr *vap,                 /* As passed to VNOP */
		vfs_context_t ctx,                      /* Context */
		void *reserved);                        /* Who knows */

	int (*a_open_existing_authorizer)(      /* Authorizer for preexisting case */
		vnode_t vp,                             /* vp to open */
		struct componentname *cnp,              /* Lookup state */
		int fmode,                              /* As passed to VNOP */
		vfs_context_t ctx,                      /* Context */
		void *reserved);                        /* Who knows */

	void *a_reserved;
};

/* Results */
#define COMPOUND_OPEN_STATUS_DID_CREATE 0x00000001
#endif /* KERNEL_PRIVATE */

/*!
 *  @function VNOP_OPEN
 *  @abstract Call down to a filesystem to open a file.
 *  @discussion The open vnop gives a filesystem a chance to initialize a file for
 *  operations like reading, writing, and ioctls.  VFS promises to send down exactly one VNOP_CLOSE()
 *  for each VNOP_OPEN().
 *  @param vp File to open.
 *  @param mode FREAD and/or FWRITE.
 *  @param ctx Context against which to authenticate open.
 *  @return 0 for success or a filesystem-specific error.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_OPEN(vnode_t, int, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

#ifdef BSD_KERNEL_PRIVATE
struct nameidata;
extern int VNOP_COMPOUND_OPEN(vnode_t dvp, vnode_t *vpp, struct nameidata *ndp, int32_t flags, int32_t fmode, uint32_t *status, struct vnode_attr *vap, vfs_context_t ctx);
#endif

struct vnop_close_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	int a_fflag;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_CLOSE
 *  @abstract Call down to a filesystem to close a file.
 *  @discussion The close vnop gives a filesystem a chance to release state set up
 *  by a VNOP_OPEN(). VFS promises to send down exactly one VNOP_CLOSE() for each VNOP_OPEN().
 *  @param vp File to close.
 *  @param fflag FREAD and/or FWRITE; in the case of a file opened with open(2), fflag corresponds
 *  to how the file was opened.
 *  @param ctx Context against which to authenticate close.
 *  @return 0 for success or a filesystem-specific error.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_CLOSE(vnode_t, int, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_access_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	int a_action;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_ACCESS
 *  @abstract Call down to a filesystem to see if a kauth-style operation is permitted.
 *  @discussion VNOP_ACCESS is currently only called on filesystems which mark themselves
 *  as doing their authentication remotely (vfs_setauthopaque(), vfs_authopaque()).  A VNOP_ACCESS()
 *  calldown may come without any preceding VNOP_OPEN().
 *  @param vp File to authorize action for.
 *  @param action kauth-style action to be checked for permissions, e.g. KAUTH_VNODE_DELETE.
 *  @param ctx Context against which to authenticate action.
 *  @return 0 for success or a filesystem-specific error.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_ACCESS(vnode_t, int, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_getattr_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	struct vnode_attr *a_vap;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_GETATTR
 *  @abstract Call down to a filesystem to get vnode attributes.
 *  @discussion Supported attributes ("Yes, I am returning this information") are set with VATTR_SET_SUPPORTED.
 *  Which attributes have been requested is checked with VATTR_IS_ACTIVE.  Attributes
 *  are returned with VATTR_RETURN.  It is through VNOP_GETATTR that routines like stat() get their information.
 *  A VNOP_GETATTR() calldown may come without any preceding VNOP_OPEN().
 *  @param vp The vnode whose attributes to get.
 *  @param vap Container for which attributes are requested, which attributes are supported by the filesystem, and attribute values.
 *  @param ctx Context against which to authenticate request for attributes.
 *  @return 0 for success or a filesystem-specific error. VNOP_GETATTR() can return success even if not
 *  all requested attributes were returned; returning an error-value should indicate that something went wrong, rather than that
 *  some attribute is not supported.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_GETATTR(vnode_t, struct vnode_attr *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_setattr_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	struct vnode_attr *a_vap;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_SETATTR
 *  @abstract Call down to a filesystem to set vnode attributes.
 *  @discussion Supported attributes ("Yes, I am setting this attribute.") are set with VATTR_SET_SUPPORTED.
 *  Requested attributes are checked with VATTR_IS_ACTIVE.  Attribute values are accessed directly through
 *  structure fields.  VNOP_SETATTR() is the core of the KPI function vnode_setattr(), which is used by chmod(),
 *  chown(), truncate(), and many others.  A VNOP_SETATTR() call may come without any preceding VNOP_OPEN().
 *  @param vp The vnode whose attributes to set.
 *  @param vap Container for which attributes are to be set and their desired values, as well as for the filesystem to
 *  return information about which attributes were successfully set.
 *  @param ctx Context against which to authenticate request for attribute change.
 *  @return 0 for success or a filesystem-specific error.  VNOP_SETATTR() can return success even if not
 *  all requested attributes were set; returning an error-value should indicate that something went wrong, rather than that
 *  some attribute is not supported.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_SETATTR(vnode_t, struct vnode_attr *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_read_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	struct uio *a_uio;
	int a_ioflag;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_READ
 *  @abstract Call down to a filesystem to read file data.
 *  @discussion VNOP_READ() is where the hard work of of the read() system call happens.  The filesystem may use
 *  the buffer cache, the cluster layer, or an alternative method to get its data; uio routines will be used to see that data
 *  is copied to the correct virtual address in the correct address space and will update its uio argument
 *  to indicate how much data has been moved.
 *  @param vp The vnode to read from.
 *  @param uio Description of request, including file offset, amount of data requested, destination address for data,
 *  and whether that destination is in kernel or user space.
 *  @param ctx Context against which to authenticate read request.
 *  @return 0 for success or a filesystem-specific error.  VNOP_READ() can return success even if less data was
 *  read than originally requested; returning an error value should indicate that something actually went wrong.
 */
extern errno_t VNOP_READ(vnode_t vp, struct uio *uio, int, vfs_context_t ctx);

struct vnop_write_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	struct uio *a_uio;
	int a_ioflag;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_WRITE
 *  @abstract Call down to the filesystem to write file data.
 *  @discussion VNOP_WRITE() is to write() as VNOP_READ() is to read().  The filesystem may use
 *  the buffer cache, the cluster layer, or an alternative method to write its data; uio routines will be used to see that data
 *  is copied to the correct virtual address in the correct address space and will update its uio argument
 *  to indicate how much data has been moved.
 *  @param vp The vnode to write to.
 *  @param uio Description of request, including file offset, amount of data to write, source address for data,
 *  and whether that destination is in kernel or user space.
 *  @param ctx Context against which to authenticate write request.
 *  @return 0 for success or a filesystem-specific error.  VNOP_WRITE() can return success even if less data was
 *  written than originally requested; returning an error value should indicate that something actually went wrong.
 */
extern errno_t VNOP_WRITE(vnode_t vp, struct uio *uio, int ioflag, vfs_context_t ctx);

struct vnop_ioctl_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	u_long a_command;
	caddr_t a_data;
	int a_fflag;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_IOCTL
 *  @abstract Call down to a filesystem or device driver to execute various control operations on or request data about a file.
 *  @discussion Ioctl controls are typically associated with devices, but they can in fact be passed
 *  down for any file; they are used to implement any of a wide range of controls and information requests.
 *  fcntl() calls VNOP_IOCTL for several commands, and will attempt a VNOP_IOCTL if it is passed an unknown command,
 *  though no copyin or copyout of  arguments can occur in this case--the "arg" must be an integer value.
 *  Filesystems can define their own fcntls using this mechanism.  How ioctl commands are structured
 *  is slightly complicated; see the manual page for ioctl(2).
 *  @param vp The vnode to execute the command on.
 *  @param command Identifier for action to take.
 *  @param data Pointer to data; this can be an integer constant (of 32 bits only) or an address to be read from or written to,
 *  depending on "command."  If it is an address, it is valid and resides in the kernel; callers of VNOP_IOCTL() are
 *  responsible for copying to and from userland.
 *  @param ctx Context against which to authenticate ioctl request.
 *  @return 0 for success or a filesystem-specific error.
 */
extern errno_t VNOP_IOCTL(vnode_t vp, u_long command, caddr_t data, int fflag, vfs_context_t ctx);

struct vnop_select_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	int a_which;
	int a_fflags;
	void *a_wql;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_SELECT
 *  @abstract Call down to a filesystem or device to check if a file is ready for I/O and request later notification if it is not currently ready.
 *  @discussion In general, regular are always "ready for I/O" and their select vnops simply return "1."
 *  Devices, though, may or may not be read; they keep track of who is selecting on them and send notifications
 *  when they become ready.  xnu provides structures and routines for tracking threads waiting for I/O and waking up
 *  those threads: see selrecord(), selthreadclear(), seltrue(), selwait(), selwakeup(), and the selinfo structure (sys/select.h).
 *  @param vp The vnode to check for I/O readiness.
 *  @param which What kind of I/O is desired: FREAD, FWRITE.
 *  @param fflags Flags from fileglob as seen in fcntl.h, e.g. O_NONBLOCK, O_APPEND.
 *  @param wql Opaque object to pass to selrecord().
 *  @param ctx Context to authenticate for select request.
 *  @return Nonzero indicates that a file is ready for I/O.  0 indicates that the file is not ready for I/O;
 *  there is no way to return an error.  0 should be returned if the device (or file) is not ready for I/O
 *  and the driver (or filesystem) is going to track the request and provide subsequent wakeups.
 *  the device (or filesystem) will provide a wakeup.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_SELECT(vnode_t, int, int, void *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_exchange_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_fvp;
	vnode_t a_tvp;
	int a_options;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_EXCHANGE
 *  @abstract Call down to a filesystem to atomically exchange the data of two files.
 *  @discussion VNOP_EXCHANGE() is currently only called by the exchangedata() system call.  It will only
 *  be applied to files on the same volume.
 *  @param fvp First vnode.
 *  @param tvp Second vnode.
 *  @param options Unused.
 *  @param ctx Context to authenticate for exchangedata request.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_EXCHANGE(vnode_t, vnode_t, int, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_revoke_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	int a_flags;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_REVOKE
 *  @abstract Call down to a filesystem to invalidate all open file descriptors for a vnode.
 *  @discussion This function is typically called as part of a TTY revoke, but can also be
 *  used on regular files.  Most filesystems simply use nop_revoke(), which calls vn_revoke(),
 *  as their revoke vnop implementation.
 *  @param vp The vnode to revoke.
 *  @param flags Unused.
 *  @param ctx Context to authenticate for revoke request.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_REVOKE(vnode_t, int, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_mmap_check_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	int a_flags;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_MMAP_CHECK
 *  @abstract Check with a filesystem if a file can be mmap-ed.
 *  @discussion VNOP_MMAP_CHECK is used to check with the file system if a
 *  file can be mmap-ed. It will be called before any call to VNOP_MMAP().
 *  @param vp The vnode being mmapped.
 *  @param flags Memory protection: PROT_READ, PROT_WRITE, PROT_EXEC.
 *  @param ctx Context to authenticate for mmap request.
 *  @return 0 for success; EPERM if the operation is not permitted; other
 *  errors (except ENOTSUP) may be returned at the discretion of the file
 *  system.  ENOTSUP will never be returned by VNOP_MMAP_CHECK().
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_MMAP_CHECK(vnode_t, int, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */


struct vnop_mmap_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	int a_fflags;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_MMAP
 *  @abstract Notify a filesystem that a file is being mmap-ed.
 *  @discussion VNOP_MMAP is an advisory calldown to say that the system is mmap-ing a file.
 *  @param vp The vnode being mmapped.
 *  @param flags Memory protection: PROT_READ, PROT_WRITE, PROT_EXEC.
 *  @param ctx Context to authenticate for mmap request.
 *  @return 0 for success; all errors except EPERM are ignored.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_MMAP(vnode_t, int, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_mnomap_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_MNOMAP
 *  @abstract Inform a filesystem that a file is no longer mapped.
 *  @discussion In general, no action is required of a filesystem for VNOP_MNOMAP.
 *  @param vp The vnode which is no longer mapped.
 *  @param ctx Context to authenticate for mnomap request.
 *  @return Return value is ignored.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_MNOMAP(vnode_t, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_fsync_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	int a_waitfor;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_FSYNC
 *  @abstract Call down to a filesystem to synchronize a file with on-disk state.
 *  @discussion VNOP_FSYNC is called whenever we need to make sure that a file's data has been
 *  pushed to backing store, for example when recycling; it is also the heart of the fsync() system call.
 *  @param vp The vnode whose data to flush to backing store.
 *  @param ctx Context to authenticate for fsync request.
 *  @return 0 for success, else an error code.
 */
extern errno_t VNOP_FSYNC(vnode_t vp, int waitfor, vfs_context_t ctx);

struct vnop_remove_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_dvp;
	vnode_t a_vp;
	struct componentname *a_cnp;
	int a_flags;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_REMOVE
 *  @abstract Call down to a filesystem to delete a file.
 *  @discussion VNOP_REMOVE is called to remove a file from a filesystem's namespace, for example by unlink().
 *  It can operate on regular files, named pipes, special files, and in some cases on directories.
 *  @param dvp Directory in which to delete a file.
 *  @param vp The file to delete.
 *  @param cnp Filename information.
 *  @param ctx Context to authenticate for fsync request.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_REMOVE(vnode_t, vnode_t, struct componentname *, int, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

#ifdef KERNEL_PRIVATE
struct vnop_compound_remove_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_dvp;                          /* Directory in which to lookup and remove */
	vnode_t *a_vpp;                         /* File to remove; may or may not point to NULL pointer */
	struct componentname *a_cnp;            /* Name of file to remove */
	struct vnode_attr *a_vap;               /* Destination for file attributes on successful delete */
	uint32_t a_flags;                       /* Control flags (unused) */
	vfs_context_t a_context;                /* Authorization context */
	int (*a_remove_authorizer)(             /* Authorizer callback */
		vnode_t dvp,                            /* Directory in which to delete */
		vnode_t vp,                             /* File to delete */
		struct componentname *cnp,              /* As passed to VNOP */
		vfs_context_t ctx,                      /* As passed to VNOP */
		void *reserved);                        /* Always NULL */
	void *a_reserved;                       /* Unused */
};
#endif /* KERNEL_PRIVATE */

#ifdef BSD_KERNEL_PRIVATE
extern errno_t VNOP_COMPOUND_REMOVE(vnode_t, vnode_t*, struct nameidata *, int32_t flags, struct vnode_attr *vap, vfs_context_t);
#endif
struct vnop_link_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	vnode_t a_tdvp;
	struct componentname *a_cnp;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_LINK
 *  @abstract Call down to a filesystem to create a hardlink to a file.
 *  @discussion See "man 2 link".
 *  @param vp File to link to.
 *  @param dvp Directory in which to create the link.
 *  @param cnp Filename information for new link.
 *  @param ctx Context to authenticate for link request.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_LINK(vnode_t, vnode_t, struct componentname *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_rename_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_fdvp;
	vnode_t a_fvp;
	struct componentname *a_fcnp;
	vnode_t a_tdvp;
	vnode_t a_tvp;
	struct componentname *a_tcnp;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_RENAME
 *  @abstract Call down to a filesystem to rename a file.
 *  @discussion VNOP_RENAME() will only be called with a source and target on the same volume.
 *  @param fdvp Directory in which source file resides.
 *  @param fvp File being renamed.
 *  @param fcnp Name information for source file.
 *  @param tdvp Directory file is being moved to.
 *  @param tvp Existing file with same name as target, should one exist.
 *  @param tcnp Name information for target path.
 *  @param ctx Context to authenticate for rename request.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_RENAME(vnode_t, vnode_t, struct componentname *, vnode_t, vnode_t, struct componentname *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

typedef unsigned int vfs_rename_flags_t;

// Must match sys/stdio.h
enum {
	VFS_RENAME_SECLUDE              = 0x00000001,
	VFS_RENAME_SWAP                 = 0x00000002,
	VFS_RENAME_EXCL                 = 0x00000004,

	/*
	 * VFS_RENAME_DATALESS is kernel-only and is intentionally
	 * not included in VFS_RENAME_FLAGS_MASK.
	 */
	VFS_RENAME_DATALESS             = 0x00000008,

	VFS_RENAME_FLAGS_MASK   = (VFS_RENAME_SECLUDE | VFS_RENAME_SWAP
	    | VFS_RENAME_EXCL),
};

struct vnop_renamex_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_fdvp;
	vnode_t a_fvp;
	struct componentname *a_fcnp;
	vnode_t a_tdvp;
	vnode_t a_tvp;
	struct componentname *a_tcnp;
	struct vnode_attr *a_vap;               // Reserved for future use
	vfs_rename_flags_t a_flags;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_RENAMEX
 *  @abstract Call down to a filesystem to rename a file.
 *  @discussion VNOP_RENAMEX() will only be called with a source and target on the same volume.
 *  @param fdvp Directory in which source file resides.
 *  @param fvp File being renamed.
 *  @param fcnp Name information for source file.
 *  @param tdvp Directory file is being moved to.
 *  @param tvp Existing file with same name as target, should one exist.
 *  @param tcnp Name information for target path.
 *  @param flags Control certain rename semantics.
 *  @param ctx Context to authenticate for rename request.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_RENAMEX(vnode_t, vnode_t, struct componentname *, vnode_t, vnode_t, struct componentname *, vfs_rename_flags_t, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

#ifdef KERNEL_PRIVATE
struct vnop_compound_rename_args {
	struct vnodeop_desc *a_desc;

	vnode_t a_fdvp;                 /* Directory from which to rename */
	vnode_t *a_fvpp;                /* Vnode to rename (can point to a NULL pointer) */
	struct componentname *a_fcnp;   /* Source name */
	struct vnode_attr *a_fvap;

	vnode_t a_tdvp;                 /* Directory to which to rename */
	vnode_t *a_tvpp;                /* Vnode to rename over (can point to a NULL pointer) */
	struct componentname *a_tcnp;   /* Destination name */
	struct vnode_attr *a_tvap;

	uint32_t a_flags;               /* Control flags: currently unused */
	vfs_context_t a_context;        /* Authorization context */
	int (*a_rename_authorizer)(                     /* Authorization callback */
		vnode_t fdvp,                           /* As passed to VNOP */
		vnode_t fvp,                            /* Vnode to rename */
		struct componentname *fcnp,             /* As passed to VNOP */
		vnode_t tdvp,                           /* As passed to VNOP */
		vnode_t tvp,                            /* Vnode to rename over (can be NULL) */
		struct componentname *tcnp,             /* As passed to VNOP */
		vfs_context_t ctx,                      /* As passed to VNOP */
		void *reserved);                        /* Always NULL */
	void *a_reserved;               /* Currently unused */
};
#endif /* KERNEL_PRIVATE */

#ifdef XNU_KERNEL_PRIVATE
errno_t
VNOP_COMPOUND_RENAME(
	struct vnode *fdvp, struct vnode **fvpp, struct componentname *fcnp, struct vnode_attr *fvap,
	struct vnode *tdvp, struct vnode **tvpp, struct componentname *tcnp, struct vnode_attr *tvap,
	uint32_t flags, vfs_context_t ctx);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_mkdir_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_dvp;
	vnode_t *a_vpp;
	struct componentname *a_cnp;
	struct vnode_attr *a_vap;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_MKDIR
 *  @abstract Call down to a filesystem to create a directory.
 *  @discussion The newly created directory should be returned with an iocount which will be dropped by the caller.
 *  @param dvp Directory in which to create new directory.
 *  @param vpp Destination for pointer to new directory's vnode.
 *  @param cnp Name information for new directory.
 *  @param vap Attributes for new directory.
 *  @param ctx Context to authenticate for mkdir request.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_MKDIR(vnode_t, vnode_t *, struct componentname *, struct vnode_attr *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */


#ifdef KERNEL_PRIVATE
struct vnop_compound_mkdir_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_dvp;                  /* Directory in which to create */
	vnode_t *a_vpp;                 /* Destination for found or created vnode */
	struct componentname *a_cnp;    /* Name of directory to create */
	struct vnode_attr *a_vap;       /* Creation attributes */
	uint32_t a_flags;               /* Control flags (unused) */
	vfs_context_t a_context;        /* Authorization context */
#if 0
	int (*a_mkdir_authorizer)(vnode_t dvp, struct componentname *cnp, struct vnode_attr *vap, vfs_context_t ctx, void *reserved);
#endif /* 0 */
	void *a_reserved;               /* Unused */
};
#endif /* KERNEL_PRIVATE */

#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_COMPOUND_MKDIR(vnode_t, vnode_t *, struct nameidata *, struct vnode_attr *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_rmdir_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_dvp;
	vnode_t a_vp;
	struct componentname *a_cnp;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_RMDIR
 *  @abstract Call down to a filesystem to delete a directory.
 *  @param dvp Parent of directory to be removed.
 *  @param vp Directory to remove.
 *  @param cnp Name information for directory to be deleted.
 *  @param ctx Context to authenticate for rmdir request.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_RMDIR(vnode_t, vnode_t, struct componentname *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

#ifdef KERNEL_PRIVATE
struct vnop_compound_rmdir_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_dvp;                          /* Directory in which to look up and delete */
	vnode_t *a_vpp;                         /* Destination for found vnode */
	struct componentname *a_cnp;            /* Name to delete */
	struct vnode_attr *a_vap;               /* Location in which to store attributes if delete succeeds (can be NULL) */
	uint32_t a_flags;                       /* Control flags (currently unused) */
	vfs_context_t a_context;                /* Context for authorization */
	int (*a_rmdir_authorizer)(              /* Authorization callback */
		vnode_t dvp,                            /* As passed to VNOP */
		vnode_t vp,                             /* Directory to delete */
		struct componentname *cnp,              /* As passed to VNOP */
		vfs_context_t ctx,                      /* As passed to VNOP */
		void *reserved);                        /* Always NULL */
	void *a_reserved;                       /* Unused */
};
#endif /* KERNEL_PRIVATE */

#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_COMPOUND_RMDIR(vnode_t, vnode_t*, struct nameidata *, struct vnode_attr *vap, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */


struct vnop_symlink_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_dvp;
	vnode_t *a_vpp;
	struct componentname *a_cnp;
	struct vnode_attr *a_vap;
	char *a_target;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_SYMLINK
 *  @abstract Call down to a filesystem to create a symbolic link.
 *  @param If VNOP_SYMLINK() is successful, the new file should be returned with an iocount which will
 *  be dropped by the caller.  VFS does not ensure that the target path will have a length shorter
 *  than the max symlink length for the filesystem.
 *  @param dvp Parent directory for new symlink file.
 *  @param vpp
 *  @param cnp Name information for new symlink.
 *  @param vap Attributes for symlink.
 *  @param target Path for symlink to store; for "ln -s /var/vardir linktovardir", "target" would be "/var/vardir"
 *  @param ctx Context to authenticate for symlink request.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_SYMLINK(vnode_t, vnode_t *, struct componentname *, struct vnode_attr *, char *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

/*
 *
 *  When VNOP_READDIR is called from the NFS Server, the nfs_data
 *  argument is non-NULL.
 *
 *  The value of nfs_eofflag should be set to TRUE if the end of
 *  the directory was reached while reading.
 *
 *  The directory seek offset (cookies) are returned to the NFS client and
 *  may be used later to restart a directory read part way through
 *  the directory. There is one cookie returned for each directory
 *  entry returned and its size is determince from nfs_sizeofcookie.
 *  The value of the cookie should be the logical offset within the
 *  directory where the on-disc version of the appropriate directory
 *  entry starts. Memory for the cookies is allocated from M_TEMP
 *  and it is freed by the caller of VNOP_READDIR.
 *
 */

struct vnop_readdir_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	struct uio *a_uio;
	int a_flags;
	int *a_eofflag;
	int *a_numdirent;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_READDIR
 *  @abstract Call down to a filesystem to enumerate directory entries.
 *  @discussion VNOP_READDIR() packs a buffer with "struct dirent" directory entry representations as described
 *  by the "getdirentries" manual page.
 *  @param vp Directory to enumerate.
 *  @param uio Destination information for resulting direntries.
 *  @param flags VNODE_READDIR_EXTENDED, VNODE_READDIR_REQSEEKOFF, VNODE_READDIR_SEEKOFF32: Apple-internal flags.
 *  @param eofflag Should be set to 1 if the end of the directory has been reached.
 *  @param numdirent Should be set to number of entries written into buffer.
 *  @param ctx Context to authenticate for readdir request.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_READDIR(vnode_t, struct uio *, int, int *, int *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_readdirattr_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	struct attrlist *a_alist;
	struct uio *a_uio;
	uint32_t a_maxcount;
	uint32_t a_options;
	uint32_t *a_newstate;
	int *a_eofflag;
	uint32_t *a_actualcount;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_READDIRATTR
 *  @abstract Call down to get file attributes for many files in a directory at once.
 *  @discussion VNOP_READDIRATTR() packs a buffer  with file attributes, as if the results of many "getattrlist" calls.
 *  @param vp Directory in which to enumerate entries' attributes.
 *  @param alist Which attributes are wanted for each directory entry.
 *  @param uio Destination information for resulting attributes.
 *  @param maxcount Maximum count of files to get attributes for.
 *  @param options FSOPT_NOFOLLOW: do not follow symbolic links. FSOPT_NOINMEMUPDATE: do not use data which have been
 *  updated since an inode was loaded into memory.
 *  @param newstate The "newstate" should be set to a value which changes if the contents of a directory change
 *  through an addition or deletion but stays the same otherwise.
 *  @param eofflag Should be set to 1 if the end of the directory has been reached.
 *  @param actualcount Should be set to number of files whose attributes were  written into buffer.
 *  @param ctx Context to authenticate for readdirattr request.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_READDIRATTR(vnode_t, struct attrlist *, struct uio *, uint32_t, uint32_t, uint32_t *, int *, uint32_t *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_getattrlistbulk_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	struct attrlist *a_alist;
	struct vnode_attr *a_vap;
	struct uio *a_uio;
	void *a_private;
	uint64_t a_options;
	int32_t *a_eofflag;
	int32_t *a_actualcount;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_GETATTRLISTBULK
 *  @abstract Call down to get file attributes for many files in a directory at once.
 *  @discussion VNOP_GETATTRLISTBULK() packs a buffer  with file attributes, as if the results of many "getattrlist" calls.
 *  @param vp Directory in which to enumerate entries' attributes.
 *  @param alist Which attributes are wanted for each directory entry.
 *  @param uio Destination information for resulting attributes.
 *  @param vap initialised vnode_attr structure pointer. This structure also has memory allocated (MAXPATHLEN bytes) and assigned to the va_name field for filesystems to use.
 *  @param private reserved for future use.
 *  @param options
 *  @param eofflag Should be set to 1 if the end of the directory has been reached.
 *  @param actualcount Should be set to number of files whose attributes were  written into buffer.
 *  @param ctx Context to authenticate for getattrlistbulk request.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_GETATTRLISTBULK(vnode_t, struct attrlist *, struct vnode_attr *, uio_t, void *, uint64_t, int32_t *, int32_t *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_readlink_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	struct uio *a_uio;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_READLINK
 *  @abstract Call down to a filesystem to get the pathname represented by a symbolic link.
 *  @discussion VNOP_READLINK() gets the path stored in a symbolic link; it is called by namei() and the readlink() system call.
 *  @param vp Symbolic link to read from.
 *  @param uio Destination information for link path.
 *  @param ctx Context to authenticate for readlink request.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_READLINK(vnode_t, struct uio *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_inactive_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_INACTIVE
 *  @abstract Notify a filesystem that the last usecount (persistent reference) on a vnode has been dropped.
 *  @discussion VNOP_INACTVE() gives a filesystem a chance to aggressively release resources assocated with a vnode, perhaps
 *  even to call vnode_recycle(), but no action is prescribed; it is acceptable for VNOP_INACTIVE to be a no-op and
 *  to defer all reclamation until VNOP_RECLAIM().
 *  VNOP_INACTVE() will not be called on a vnode if no persistent reference is ever taken; an
 *  important example is a stat(), which takes an iocount, reads its data, and drops that iocount.
 *  @param vp The vnode which is now inactive.
 *  @param ctx Context to authenticate for inactive message.
 *  @return 0 for success, else an error code, but return value is currently ignored.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_INACTIVE(vnode_t, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_reclaim_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_RECLAIM
 *  @abstract Release filesystem-internal resources for a vnode.
 *  @discussion VNOP_RECLAIM() is called as part of the process of recycling a vnode.  During
 *  a reclaim routine, a filesystem should remove a vnode from its hash and deallocate any resources
 *  allocated to that vnode.  VFS guarantees that when VNOP_RECLAIM() is called, there are no more
 *  iocount references on a vnode (though there may still be usecount references--these are invalidated
 *  by the reclaim) and that no more will be granted.  This means in practice that there will be no
 *  filesystem calls on the vnode being reclaimed until the reclaim has finished and the vnode has
 *  been reused.
 *  @param vp The vnode to reclaim.
 *  @param ctx Context to authenticate for reclaim.
 *  @return 0 for success, or an error code.  A nonzero return value results in a panic.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_RECLAIM(vnode_t, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_pathconf_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	int a_name;
	int32_t *a_retval;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_PATHCONF
 *  @abstract Query a filesystem for path properties.
 *  @param vp The vnode whose filesystem to query.
 *  @param name Which property to request: see unistd.h.  For example: _PC_CASE_SENSITIVE (is
 *  a filesystem case-sensitive?).  Only one property can be requested at a time.
 *  @param retval Destination for value of property.
 *  @param ctx Context to authenticate for pathconf request.
 *  @return 0 for success, or an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_PATHCONF(vnode_t, int, int32_t *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_advlock_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	caddr_t a_id;
	int a_op;
	struct flock *a_fl;
	int a_flags;
	vfs_context_t a_context;
	struct timespec *a_timeout;
};

/*!
 *  @function VNOP_ADVLOCK
 *  @abstract Aquire or release and advisory lock on a vnode.
 *  @discussion Advisory locking is somewhat complicated.  VNOP_ADVLOCK is overloaded for
 *  both flock() and POSIX advisory locking usage, though not all filesystems support both (or any).  VFS
 *  provides an advisory locking mechanism for filesystems which can take advantage of it; vfs_setlocklocal()
 *  marks a filesystem as using VFS advisory locking support.
 *  @param vp The vnode to lock or unlock.
 *  @param id Identifier for lock holder: ignored by most filesystems.
 *  @param op Which locking operation: F_SETLK: set locking information about a region.
 *  F_GETLK: get locking information about the specified region.  F_UNLCK: Unlock a region.
 *  @param fl Description of file region to lock. l_whence is as with "lseek."
 *  Includes a type: F_RDLCK (shared lock), F_UNLCK (unlock) , and F_WRLCK (exclusive lock).
 *  @param flags F_FLOCK: use flock() semantics. F_POSIX: use POSIX semantics.  F_WAIT: sleep if necessary.
 *  F_PROV: Non-coelesced provisional lock (unused in xnu).
 *  @param ctx Context to authenticate for advisory locking request.
 *  @param timeout Timespec for timeout in case of F_SETLKWTIMEOUT.
 *  @return 0 for success, or an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_ADVLOCK(vnode_t, caddr_t, int, struct flock *, int, vfs_context_t, struct timespec *);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_allocate_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	off_t a_length;
	u_int32_t a_flags;
	off_t *a_bytesallocated;
	off_t a_offset;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_ALLOCATE
 *  @abstract Pre-allocate space for a file.
 *  @discussion VNOP_ALLOCATE() changes the amount of backing store set aside to
 *  a file.  It can be used to either shrink or grow a file.  If the file shrinks,
 *  its ubc size will be modified accordingly, but if it grows, then the ubc size is unchanged;
 *  space is set aside without being actively used by the file.  VNOP_ALLOCATE() is currently only
 *  called as part of the F_PREALLOCATE fcntl.
 *  @param vp The vnode for which to preallocate space.
 *  @param length Desired preallocated file length.
 *  @param flags
 *  PREALLOCATE:     preallocate allocation blocks.
 *  ALLOCATECONTIG:  allocate contigious space.
 *  ALLOCATEALL:     allocate all requested space or no space at all.
 *  FREEREMAINDER:   deallocate allocated but unfilled blocks.
 *  ALLOCATEFROMPEOF: allocate from the physical eof.
 *  ALLOCATEFROMVOL:  allocate from the volume offset.
 *  @param bytesallocated  Additional bytes set aside for file. Set to 0 if none are allocated
 *  OR if the file is contracted.
 *  @param offset Hint for where to find free blocks.
 *  @param ctx Context to authenticate for allocation request.
 *  @return 0 for success, or an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_ALLOCATE(vnode_t, off_t, u_int32_t, off_t *, off_t, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_pagein_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	upl_t a_pl;
	upl_offset_t a_pl_offset;
	off_t a_f_offset;
	size_t a_size;
	int a_flags;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_PAGEIN
 *  @abstract Pull file data into memory.
 *  @discussion VNOP_PAGEIN() is called by when a process faults on data mapped from a file or
 *  when madvise() demands pre-fetching.  It is conceptually somewhat similar to VNOP_READ().  Filesystems
 *  are typically expected to call cluster_pagein() to handle the labor of mapping and committing the UPL.
 *  @param vp The vnode for which to page in data.
 *  @param pl UPL describing pages needing to be paged in.
 *  @param pl_offset Offset in UPL at which to start placing data.
 *  @param f_offset Offset in file of data needing to be paged in.
 *  @param size Amount of data to page in (in bytes).
 *  @param flags UPL-style flags: UPL_IOSYNC, UPL_NOCOMMIT, UPL_NORDAHEAD, UPL_VNODE_PAGER, UPL_MSYNC.
 *  Filesystems should generally leave it to the cluster layer to handle these flags. See the
 *  memory_object_types.h header in the kernel framework if interested.
 *  @param ctx Context to authenticate for pagein request.
 *  @return 0 for success, or an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_PAGEIN(vnode_t, upl_t, upl_offset_t, off_t, size_t, int, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_pageout_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	upl_t a_pl;
	upl_offset_t a_pl_offset;
	off_t a_f_offset;
	size_t a_size;
	int a_flags;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_PAGEOUT
 *  @abstract Write data from a mapped file back to disk.
 *  @discussion VNOP_PAGEOUT() is called when data from a mapped file needs to be flushed to disk, either
 *  because of an msync() call or due to memory pressure.  Filesystems are for the most part expected to
 *  just call cluster_pageout().   However, if they opt into the VFC_VFSVNOP_PAGEOUTV2 flag, then
 *  they will be responsible for creating their own UPLs.
 *  @param vp The vnode for which to page out data.
 *  @param pl UPL describing pages needed to be paged out.  If UPL is NULL, then it means the filesystem
 *  has opted into VFC_VFSVNOP_PAGEOUTV2 semantics, which means that it will create and operate on its own UPLs
 *  as opposed to relying on the one passed down into the filesystem.  This means that the filesystem must be
 *  responsible for N cluster_pageout calls for N dirty ranges in the UPL.
 *  @param pl_offset Offset in UPL from which to start paging out data.  Under the new VFC_VFSVNOP_PAGEOUTV2
 *  semantics, this is the offset in the range specified that must be paged out if the associated page is dirty.
 *  @param f_offset Offset in file of data needing to be paged out.    Under the new VFC_VFSVNOP_PAGEOUTV2
 *  semantics, this represents the offset in the file where we should start looking for dirty pages.
 *  @param size Amount of data to page out (in bytes).   Under VFC_VFSVNOP_PAGEOUTV2, this represents
 *  the size of the range to be considered.  The fileystem is free to extend or shrink the specified range
 *  to better fit its blocking model as long as the page at 'pl_offset' is included.
 *  @param flags UPL-style flags: UPL_IOSYNC, UPL_NOCOMMIT, UPL_NORDAHEAD, UPL_VNODE_PAGER, UPL_MSYNC.
 *  Filesystems should generally leave it to the cluster layer to handle these flags. See the
 *  memory_object_types.h header in the kernel framework if interested.
 *  @param ctx Context to authenticate for pageout request.
 *  @return 0 for success, or an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_PAGEOUT(vnode_t, upl_t, upl_offset_t, off_t, size_t, int, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_searchfs_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	void *a_searchparams1;
	void *a_searchparams2;
	struct attrlist *a_searchattrs;
	uint32_t a_maxmatches;
	struct timeval *a_timelimit;
	struct attrlist *a_returnattrs;
	uint32_t *a_nummatches;
	uint32_t a_scriptcode;
	uint32_t a_options;
	struct uio *a_uio;
	struct searchstate *a_searchstate;
	vfs_context_t a_context;
};

/*
 *  @function VNOP_SEARCHFS
 *  @abstract Search a filesystem quickly for files or directories that match the passed-in search criteria.
 *  @discussion VNOP_SEARCHFS is a getattrlist-based system call which is implemented almost entirely inside
 *  supported filesystems.  Callers provide a set of criteria to match against, and the filesystem is responsible
 *  for finding all files or directories that match the criteria.  Once these files or directories are found,
 *  the user-requested attributes of these files is provided as output.  The set of searchable attributes is a
 *  subset of the getattrlist  attributes.  For example, ATTR_CMN_UUID is not a valid searchable attribute as of
 *  10.6.  A common usage scenario could be to request all files whose mod dates is greater than time X, less than
 *  time Y, and provide the inode ID and filename of the matching objects as output.
 *  @param vp The vnode representing the mountpoint of the filesystem to be searched.
 *  @param a_searchparams1 If one-argument search criteria is requested, the search criteria would go here. However,
 *  some search criteria, like ATTR_CMN_MODTIME, can be bounded.  The user could request files modified between time X
 *  and time Y.  In this case, the lower bound goes in a_searchparams1.
 *  @param a_searchparams2 If two-argument search criteria is requested, the upper bound goes in here.
 *  @param a_searchattrs Contains the getattrlist-style attribute bits which are requested by the current search.
 *  @param a_maxmatches The maximum number of matches to return in a single system call.
 *  @param a_timelimit The suggested maximum amount of time we can spend in the kernel to service this system call.
 *  Filesystems should use this as a guide only, and set their own internal maximum time to avoid denial of service.
 *  @param a_returnattrs The getattrlist-style attributes to return for items in the filesystem that match the search
 *  criteria above.
 *  @param a_scriptcode Currently ignored.
 *  @param a_uio The uio in which to write out the search matches.
 *  @param a_searchstate Sometimes searches cannot be completed in a single system call.  In this case, we provide
 *  an identifier back to the user which indicates where to resume a previously-started search.  This is an opaque structure
 *  used by the filesystem to identify where to resume said search.
 *  @param a_context The context in which to perform the filesystem search.
 *  @return 0 on success, EAGAIN for searches which could not be completed in 1 call, and other ERRNOS as needed.
 */

#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_SEARCHFS(vnode_t, void *, void *, struct attrlist *, uint32_t, struct timeval *, struct attrlist *, uint32_t *, uint32_t, uint32_t, struct uio *, struct searchstate *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_copyfile_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_fvp;
	vnode_t a_tdvp;
	vnode_t a_tvp;
	struct componentname *a_tcnp;
	int a_mode;
	int a_flags;
	vfs_context_t a_context;
};

#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_COPYFILE(vnode_t, vnode_t, vnode_t, struct componentname *, int, int, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

typedef enum dir_clone_authorizer_op {
	OP_AUTHORIZE = 0,           /* request authorization of action */
	OP_VATTR_SETUP = 1,         /* query for attributes that are required for OP_AUTHORIZE */
	OP_VATTR_CLEANUP = 2        /* request to cleanup any state or free any memory allocated in OP_AUTHORIZE */
} dir_clone_authorizer_op_t;

struct vnop_clonefile_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_fvp;
	vnode_t a_dvp;
	vnode_t *a_vpp;
	struct componentname *a_cnp;
	struct vnode_attr *a_vap;
	uint32_t a_flags;
	vfs_context_t a_context;
	int (*a_dir_clone_authorizer)(  /* Authorization callback */
		struct vnode_attr *vap,         /* attribute to be authorized */
		kauth_action_t action,         /* action for which attribute is to be authorized */
		struct vnode_attr *dvap,         /* target directory attributes */
		vnode_t sdvp,         /* source directory vnode pointer (optional) */
		mount_t mp,         /* mount point of filesystem */
		dir_clone_authorizer_op_t vattr_op,         /* specific operation requested : setup, authorization or cleanup  */
		uint32_t flags,         /* needs to have the value passed to a_flags */
		vfs_context_t ctx,                      /* As passed to VNOP */
		void *reserved);                        /* Always NULL */
	void *a_reserved;               /* Currently unused */
};

/*!
 *  @function VNOP_CLONEFILE
 *  @abstract Call down to a filesystem to clone a filesystem object (regular file, directory or symbolic link.)
 *  @discussion If file creation succeeds, "vpp" should be returned with an iocount to be dropped by the caller.
 *  @param dvp Directory in which to clone object.
 *  @param vpp Destination for vnode for newly cloned object.
 *  @param cnp Description of name of object to clone.
 *  @param vap File creation properties, as seen in vnode_getattr().  Manipulated with VATTR_ISACTIVE, VATTR_RETURN,
 *  VATTR_SET_SUPPORTED, and so forth. All attributes not set here should either be copied
 *  from the source object
 *  or set to values which are used for creating new filesystem objects
 *  @param ctx Context against which to authenticate file creation.
 *  @return 0 for success or a filesystem-specific error.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_CLONEFILE(vnode_t, vnode_t, vnode_t *, struct componentname *, struct vnode_attr *, uint32_t, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_getxattr_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	const char * a_name;
	uio_t a_uio;
	size_t *a_size;
	int a_options;
	vfs_context_t a_context;
};
extern struct vnodeop_desc vnop_getxattr_desc;

/*!
 *  @function VNOP_GETXATTR
 *  @abstract Get extended file attributes.
 *  @param vp The vnode to get extended attributes for.
 *  @param name Which property to extract.
 *  @param uio Destination information for attribute value.
 *  @param size Should be set to the amount of data written.
 *  @param options XATTR_NOSECURITY: bypass security-checking.
 *  @param ctx Context to authenticate for getxattr request.
 *  @return 0 for success, or an error code.
 */
extern errno_t VNOP_GETXATTR(vnode_t vp, const char *name, uio_t uio, size_t *size, int options, vfs_context_t ctx);

struct vnop_setxattr_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	const char * a_name;
	uio_t a_uio;
	int a_options;
	vfs_context_t a_context;
};
extern struct vnodeop_desc vnop_setxattr_desc;

/*!
 *  @function VNOP_SETXATTR
 *  @abstract Set extended file attributes.
 *  @param vp The vnode to set extended attributes for.
 *  @param name Which property to extract.
 *  @param uio Source information for attribute value.
 *  @param options XATTR_NOSECURITY: bypass security-checking. XATTR_CREATE: set value, fail if exists.
 *  XATTR_REPLACE: set value, fail if does not exist.
 *  @param ctx Context to authenticate for setxattr request.
 *  @return 0 for success, or an error code.
 */
extern errno_t VNOP_SETXATTR(vnode_t vp, const char *name, uio_t uio, int options, vfs_context_t ctx);

struct vnop_removexattr_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	const char * a_name;
	int a_options;
	vfs_context_t a_context;
};
extern struct vnodeop_desc vnop_removexattr_desc;

/*!
 *  @function VNOP_REMOVEXATTR
 *  @abstract Remove extended file attributes.
 *  @param vp The vnode from which to remove extended attributes.
 *  @param name Which attribute to delete.
 *  @param options XATTR_NOSECURITY: bypass security-checking.
 *  @param ctx Context to authenticate for attribute delete request.
 *  @return 0 for success, or an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_REMOVEXATTR(vnode_t, const char *, int, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_listxattr_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	uio_t a_uio;
	size_t *a_size;
	int a_options;
	vfs_context_t a_context;
};
extern struct vnodeop_desc vnop_listxattr_desc;

/*!
 *  @function VNOP_LISTXATTR
 *  @abstract List extended attribute keys.
 *  @discussion Should write a sequence of unseparated, null-terminated extended-attribute
 *  names into the space described by the provided uio.  These keys can then be passed to
 *  getxattr() (and VNOP_GETXATTR()).
 *  @param vp The vnode for which to get extended attribute keys.
 *  @param uio Description of target memory for attribute keys.
 *  @param size Should be set to amount of data written to buffer.
 *  @param options XATTR_NOSECURITY: bypass security checking.
 *  @param ctx Context to authenticate for attribute name request.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_LISTXATTR(vnode_t, uio_t, size_t *, int, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_blktooff_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	daddr64_t a_lblkno;
	off_t *a_offset;
};

/*!
 *  @function VNOP_BLKTOOFF
 *  @abstract Call down to a filesystem to convert a logical block number to a file offset.
 *  @discussion VNOP_BLKTOOFF() converts a logical block to a file offset in bytes.  That offset
 *  can be passed to VNOP_BLOCKMAP(), then, to get a physical block number--buf_strategy() does this.
 *  @param vp The vnode for which to convert a logical block to an offset.
 *  @param lblkno Logical block number to turn into offset.
 *  @param offset Destination for file offset.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_BLKTOOFF(vnode_t, daddr64_t, off_t *);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_offtoblk_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	off_t a_offset;
	daddr64_t *a_lblkno;
};

/*!
 *  @function VNOP_OFFTOBLK
 *  @abstract Call down to a filesystem to convert a file offset to a logical block number.
 *  @param vp The vnode for which to convert an offset to a logical block number.
 *  @param offset File offset to convert.
 *  @param lblkno Destination for corresponding logical block number.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_OFFTOBLK(vnode_t, off_t, daddr64_t *);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_blockmap_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	off_t a_foffset;
	size_t a_size;
	daddr64_t *a_bpn;
	size_t *a_run;
	void *a_poff;
	int a_flags;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_BLOCKMAP
 *  @abstract Call down to a filesystem to get information about the on-disk layout of a file region.
 *  @discussion VNOP_BLOCKMAP() returns the information required to pass a request for a contiguous region
 *  down to a device's strategy routine.
 *  @param vp The vnode for which to get on-disk information.
 *  @param foffset Offset (in bytes) at which region starts.
 *  @param size Size of region.
 *  @param bpn Destination for physical block number at which region begins on disk.
 *  @param run Destination for number of bytes which can be found contiguously on-disk before
 *  first discontinuity.
 *  @param poff Currently unused.
 *  @param flags VNODE_READ: request is for a read. VNODE_WRITE: request is for a write.
 *  @param ctx Context to authenticate for blockmap request; currently often set to NULL.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_BLOCKMAP(vnode_t, off_t, size_t, daddr64_t *, size_t *, void *,
    int, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_strategy_args {
	struct vnodeop_desc *a_desc;
	struct buf *a_bp;
};

/*!
 *  @function VNOP_STRATEGY
 *  @abstract Initiate I/O on a file (both read and write).
 *  @discussion A filesystem strategy routine takes a buffer, performs whatever manipulations are necessary for passing
 *  the I/O request down to the device layer, and calls the appropriate device's strategy routine.  Most filesystems should
 *  just call buf_strategy() with "bp" as the argument.
 *  @param bp Complete specificiation of requested I/O: region of data involved, whether request is for read or write, and so on.
 *  @return 0 for success, else an error code.
 */
extern errno_t VNOP_STRATEGY(struct buf *bp);

struct vnop_bwrite_args {
	struct vnodeop_desc *a_desc;
	buf_t a_bp;
};

/*!
 *  @function VNOP_BWRITE
 *  @abstract Write a buffer to backing store.
 *  @discussion VNOP_BWRITE() is called by buf_bawrite() (asynchronous write) and potentially by buf_bdwrite() (delayed write)
 *  but not by buf_bwrite().  A filesystem may choose to perform some kind of manipulation of the buffer in this routine; it
 *  generally will end up calling VFS's default implementation, vn_bwrite() (which calls buf_bwrite() without further ado).
 *  @param bp The buffer to write.
 *  @return 0 for success, else an error code.
 */
extern errno_t VNOP_BWRITE(buf_t bp);

struct vnop_kqfilt_add_args {
	struct vnodeop_desc *a_desc;
	struct vnode *a_vp;
	struct knote *a_kn;
	vfs_context_t a_context;
};
extern struct vnodeop_desc vnop_kqfilt_add_desc;

#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_KQFILT_ADD(vnode_t, struct knote *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_kqfilt_remove_args {
	struct vnodeop_desc *a_desc;
	struct vnode *a_vp;
	uintptr_t a_ident;
	vfs_context_t a_context;
};
extern struct vnodeop_desc vnop_kqfilt_remove_desc;

#ifdef XNU_KERNEL_PRIVATE
errno_t VNOP_KQFILT_REMOVE(vnode_t, uintptr_t, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */


#ifdef KERNEL_PRIVATE
#define VNODE_MONITOR_BEGIN     0x01
#define VNODE_MONITOR_END       0x02
#define VNODE_MONITOR_UPDATE    0x04
struct vnop_monitor_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	uint32_t a_events;
	uint32_t a_flags;
	void *a_handle;
	vfs_context_t a_context;
};
extern struct vnodeop_desc vnop_monitor_desc;
#endif /* KERNEL_PRIVATE */

#ifdef XNU_KERNEL_PRIVATE
/*!
 *  @function VNOP_MONITOR
 *  @abstract Indicate to a filesystem that the number of watchers of a file has changed.
 *  @param vp The vnode whose watch state has changed.
 *  @param events Unused.  Filesystems can ignore this parameter.
 *  @param flags Type of change to the watch state.  VNODE_MONITOR_BEGIN is passed when the kernel
 *  begins tracking a new watcher of a file.  VNODE_MONITOR_END is passed when a watcher stops watching a file.
 *  VNODE_MONITOR_UPDATE is currently unused.  A filesystem is guaranteed that each VNODE_MONITOR_BEGIN
 *  will be matched by a VNODE_MONITOR_END with the same "handle" argument.
 *  @param handle Unique identifier for a given watcher. A VNODE_MONITOR_BEGIN for a given handle will be matched with a
 *  VNODE_MONITOR_END for the same handle; a filesystem need not consider this parameter unless
 *  it for some reason wants be able to match specific VNOP_MONITOR calls rather than just keeping
 *  a count.
 *  @param ctx The context which is starting to monitor a file or ending a watch on a file.  A matching
 *  pair of VNODE_MONITOR_BEGIN and VNODE_MONITOR_END need not have the same context.
 *  @discussion VNOP_MONITOR() is intended to let networked filesystems know when they should bother
 *  listening for changes to files which occur remotely, so that they can post notifications using
 *  vnode_notify().  Local filesystems should not implement a monitor vnop.
 *  It is called when there is a new watcher for a file or when a watcher for a file goes away.
 *  Each BEGIN will be matched with an END with the same handle.  Note that vnode_ismonitored() can
 *  be used to see if there are currently watchers for a file.
 */
errno_t VNOP_MONITOR(vnode_t vp, uint32_t events, uint32_t flags, void *handle, vfs_context_t ctx);
#endif /* XNU_KERNEL_PRIVATE */

struct label;
struct vnop_setlabel_args {
	struct vnodeop_desc *a_desc;
	struct vnode *a_vp;
	struct label *a_vl;
	vfs_context_t a_context;
};
extern struct vnodeop_desc vnop_setlabel_desc;

/*!
 *  @function VNOP_SETLABEL
 *  @abstract Associate a MACF label with a file.
 *  @param vp The vnode to label.
 *  @param label The desired label.
 *  @param ctx Context to authenticate for label change.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
errno_t VNOP_SETLABEL(vnode_t, struct label *, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

#ifdef __APPLE_API_UNSTABLE

#if NAMEDSTREAMS

enum nsoperation        { NS_OPEN, NS_CREATE, NS_DELETE };

/* a_flags for vnop_getnamedstream_args: */
#define NS_GETRAWENCRYPTED 0x00000001

struct vnop_getnamedstream_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	vnode_t *a_svpp;
	const char *a_name;
	enum nsoperation a_operation;
	int a_flags;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_GETNAMEDSTREAM
 *  @abstract Get a named stream associated with a file.
 *  @discussion If this call sucecss, svpp should be returned with an iocount which the caller
 *  will drop.  VFS provides a facility for simulating named streams when interacting with filesystems
 *  which do not support them.
 *  @param vp The vnode for which to get a named stream.
 *  @param svpp Destination for pointer to named stream's vnode.
 *  @param name The name of the named stream, e.g. "com.apple.ResourceFork".
 *  @param operation Operation to perform.  In HFS and AFP, this parameter is only considered as follows:
 *  if the resource fork has not been opened and the operation is not NS_OPEN, fail with ENOATTR.  Currently
 *  only passed as NS_OPEN by VFS.
 *  @param flags Flags used to control getnamedstream behavior. Currently only used for raw-encrypted-requests.
 *  @param ctx Context to authenticate for getting named stream.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_GETNAMEDSTREAM(vnode_t, vnode_t *, const char *, enum nsoperation, int flags, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_makenamedstream_args {
	struct vnodeop_desc *a_desc;
	vnode_t *a_svpp;
	vnode_t a_vp;
	const char *a_name;
	int a_flags;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_MAKENAMEDSTREAM
 *  @abstract Create a named stream associated with a file.
 *  @discussion If this call succeeds, svpp should be returned with an iocount which the caller will drop.
 *  VFS provides a facility for simulating named streams when interacting with filesystems
 *  which do not support them.
 *  @param vp The vnode for which to get a named stream.
 *  @param svpp Destination for pointer to named stream's vnode.
 *  @param name The name of the named stream, e.g. "com.apple.ResourceFork".
 *  @param flags Currently unused.
 *  @param ctx Context to authenticate creating named stream.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_MAKENAMEDSTREAM(vnode_t, vnode_t *, const char *, int flags, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

struct vnop_removenamedstream_args {
	struct vnodeop_desc *a_desc;
	vnode_t a_vp;
	vnode_t a_svp;
	const char *a_name;
	int a_flags;
	vfs_context_t a_context;
};

/*!
 *  @function VNOP_REMOVENAMEDSTREAM
 *  @abstract Delete a named stream associated with a file.
 *  @discussion  VFS provides a facility for simulating named streams when interacting with filesystems
 *  which do not support them.
 *  @param vp The vnode to which the named stream belongs.
 *  @param svp The named stream's vnode.
 *  @param name The name of the named stream, e.g. "com.apple.ResourceFork".
 *  @param flags Currently unused.
 *  @param ctx Context to authenticate deleting named stream.
 *  @return 0 for success, else an error code.
 */
#ifdef XNU_KERNEL_PRIVATE
extern errno_t VNOP_REMOVENAMEDSTREAM(vnode_t, vnode_t, const char *, int flags, vfs_context_t);
#endif /* XNU_KERNEL_PRIVATE */

#endif // NAMEDSTREAMS

#endif // defined(__APPLE_API_UNSTABLE)

__END_DECLS

#endif /* KERNEL */

#endif /* !_SYS_VNODE_IF_H_ */
