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
/* Copyright (c) 1998 Apple Computer, Inc. All Rights Reserved */
/*-
 *	@(#)vstat.h	
 */

#ifndef _SYS_VSTAT_H_
#define	_SYS_VSTAT_H_

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>

#warning obsolete header! delete the include from your sources

#ifdef __APPLE_API_OBSOLETE

#include <sys/time.h>
#include <sys/attr.h>

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)

struct vstat {
	fsid_t			vst_volid;		/* volume identifier */
	fsobj_id_t		vst_nodeid;		/* object's id */
	fsobj_type_t		vst_vnodetype;	/* vnode type (VREG, VDIR, etc.) */
	fsobj_tag_t		vst_vnodetag;	/* vnode tag (HFS, UFS, etc.) */
	mode_t	  		vst_mode;		/* inode protection mode */
	nlink_t	  		vst_nlink;		/* number of hard links */
	uid_t	  		vst_uid;		/* user ID of the file's owner */
	gid_t	  		vst_gid;		/* group ID of the file's group */
	dev_t			vst_dev;		/* inode's device */
	dev_t	  		vst_rdev;		/* device type */
#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
	struct	timespec vst_atimespec;	/* time of last access */
	struct	timespec vst_mtimespec;	/* time of last data modification */
	struct	timespec vst_ctimespec;	/* time of last file status change */
#else
	time_t	  		vst_atime;		/* time of last access */
	long	  		vst_atimensec;	/* nsec of last access */
	time_t	  		vst_mtime;		/* time of last data modification */
	long	  		vst_mtimensec;	/* nsec of last data modification */
	time_t	  		vst_ctime;		/* time of last file status change */
	long	  		vst_ctimensec;	/* nsec of last file status change */
#endif
	off_t	  		vst_filesize;	/* file size, in bytes */
	quad_t	  		vst_blocks;		/* bytes allocated for file */
	u_int32_t 		vst_blksize;	/* optimal blocksize for I/O */
	u_int32_t 		vst_flags;		/* user defined flags for file */
};

#endif /* (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */
#endif /* __APPLE_API_OBSOLETE */

#endif /* !_SYS_VSTAT_H_ */
