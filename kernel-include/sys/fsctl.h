/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
/*-
 * Copyright (c) 1982, 1986, 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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
 *	@(#)fsctl.h	8.6 (Berkeley) 3/28/94
 */

#ifndef	_SYS_FSCTL_H_
#define	_SYS_FSCTL_H_

#include <sys/ioccom.h>
#include <sys/mount.h>

#ifdef XNU_KERNEL_PRIVATE

typedef struct user64_namespace_handler_info {
	user64_addr_t  token;
	user64_addr_t  flags;
	user64_addr_t  fdptr;
} user64_namespace_handler_info;

typedef struct user32_namespace_handler_info {
	user32_addr_t  token;
	user32_addr_t  flags;
	user32_addr_t  fdptr;
} user32_namespace_handler_info;

typedef struct namespace_handler_info {
	user_addr_t  token;
	user_addr_t  flags;
	user_addr_t  fdptr;
} namespace_handler_info;

typedef struct user64_namespace_handler_info_ext {
	user64_addr_t  token;
	user64_addr_t  flags;
	user64_addr_t  fdptr;
	user64_addr_t  infoptr;
} user64_namespace_handler_info_ext;

typedef struct user32_namespace_handler_info_ext {
	user32_addr_t  token;
	user32_addr_t  flags;
	user32_addr_t  fdptr;
	user32_addr_t  infoptr;
} user32_namespace_handler_info_ext;

typedef struct namespace_handler_info_ext {
	user_addr_t  token;
	user_addr_t  flags;
	user_addr_t  fdptr;
	user_addr_t  infoptr;
} namespace_handler_info_ext;

extern int resolve_nspace_item(struct vnode *vp, uint64_t op);
extern int resolve_nspace_item_ext(struct vnode *vp, uint64_t op, void *arg);
extern int get_nspace_item_status(struct vnode *vp, int32_t *status);

#else

typedef struct namespace_handler_info {
	int32_t    *token;
	int64_t    *flags;
	int32_t    *fdptr;
} namespace_handler_info;

typedef struct namespace_handler_info_ext {
	int32_t    *token;
	int64_t    *flags;
	int32_t    *fdptr;
	int64_t    *infoptr;     // for snapshot write events, the kernel puts an offset/length pair here
} namespace_handler_info_ext;


#endif /* XNU_KERNEL_PRIVATE */

#define NAMESPACE_HANDLER_READ_OP             0x0001
#define NAMESPACE_HANDLER_WRITE_OP            0x0002
#define NAMESPACE_HANDLER_DELETE_OP           0x0004
#define NAMESPACE_HANDLER_TRUNCATE_OP         0x0008
#define NAMESPACE_HANDLER_RENAME_OP           0x0010
#define NAMESPACE_HANDLER_METADATA_WRITE_OP   0x0020
#define NAMESPACE_HANDLER_METADATA_DELETE_OP  0x0040
#define NAMESPACE_HANDLER_METADATA_MOD        0x0080
#define NAMESPACE_HANDLER_LINK_CREATE         0x0200

#define NAMESPACE_HANDLER_NSPACE_EVENT        0x1000
#define NAMESPACE_HANDLER_SNAPSHOT_EVENT      0x0100
#define NAMESPACE_HANDLER_TRACK_EVENT         0x2000

#define NAMESPACE_HANDLER_EVENT_TYPE_MASK (NAMESPACE_HANDLER_NSPACE_EVENT | NAMESPACE_HANDLER_SNAPSHOT_EVENT | NAMESPACE_HANDLER_TRACK_EVENT)

#define DATALESS_CMPFS_TYPE     0x80000001


typedef int32_t nspace_handler_info[2];
typedef char fstypename_t[MFSTYPENAMELEN];

#ifdef KERNEL

typedef struct user64_package_ext_info {
    user64_addr_t strings;
    uint32_t      num_entries;
    uint32_t      max_width;
} user64_package_ext_info;

typedef struct user32_package_ext_info {
    user32_addr_t strings;
    uint32_t      num_entries;
    uint32_t      max_width;
} user32_package_ext_info;

#endif  // KERNEL

typedef struct package_ext_info {
    const char *strings;
    uint32_t    num_entries;
    uint32_t    max_width;
} package_ext_info;

#define	FSCTL_SYNC_FULLSYNC	(1<<0)	/* Flush the data fully to disk, if supported by the filesystem */
#define	FSCTL_SYNC_WAIT		(1<<1)	/* Wait for the sync to complete */


#define FSIOC_SYNC_VOLUME			  _IOW('A', 1, uint32_t)
#define	FSCTL_SYNC_VOLUME			  IOCBASECMD(FSIOC_SYNC_VOLUME)

#define FSIOC_SET_PACKAGE_EXTS			  _IOW('A', 2, struct package_ext_info)
#define	FSCTL_SET_PACKAGE_EXTS			  IOCBASECMD(FSIOC_SET_PACKAGE_EXTS)

#define FSIOC_WAIT_FOR_SYNC			  _IOR('A', 3, int32_t)
#define	FSCTL_WAIT_FOR_SYNC			  IOCBASECMD(FSIOC_WAIT_FOR_SYNC)

#define FSIOC_NAMESPACE_HANDLER_GET		  _IOW('A', 4, struct namespace_handler_info)
#define	FSCTL_NAMESPACE_HANDLER_GET		  IOCBASECMD(FSIOC_NAMESPACE_HANDLER_GET)

#define FSIOC_NAMESPACE_HANDLER_UPDATE		  _IOW('A', 5, nspace_handler_info)
#define	FSCTL_NAMESPACE_HANDLER_UPDATE		  IOCBASECMD(FSIOC_NAMESPACE_HANDLER_UPDATE)

#define FSIOC_NAMESPACE_HANDLER_UNBLOCK		  _IOW('A', 6, nspace_handler_info)
#define	FSCTL_NAMESPACE_HANDLER_UNBLOCK		  IOCBASECMD(FSIOC_NAMESPACE_HANDLER_UNBLOCK)

#define FSIOC_NAMESPACE_HANDLER_CANCEL		  _IOW('A', 7, nspace_handler_info)
#define	FSCTL_NAMESPACE_HANDLER_CANCEL		  IOCBASECMD(FSIOC_NAMESPACE_HANDLER_CANCEL)

#define FSIOC_NAMESPACE_HANDLER_SET_SNAPSHOT_TIME _IOW('A', 8, int32_t)
#define	FSCTL_NAMESPACE_HANDLER_SET_SNAPSHOT_TIME IOCBASECMD(FSIOC_NAMESPACE_HANDLER_SET_SNAPSHOT_TIME)

#define FSIOC_OLD_SNAPSHOT_HANDLER_GET		  _IOW('A', 9, struct namespace_handler_info)
#define FSCTL_OLD_SNAPSHOT_HANDLER_GET		  IOCBASECMD(FSIOC_OLD_SNAPSHOT_HANDLER_GET)

#define FSIOC_SET_FSTYPENAME_OVERRIDE		  _IOW('A', 10, fstypename_t)
#define	FSCTL_SET_FSTYPENAME_OVERRIDE	          IOCBASECMD(FSIOC_SET_FSTYPENAME_OVERRIDE)

#define FSIOC_NAMESPACE_ALLOW_DMG_SNAPSHOT_EVENTS _IOW('A', 11, int32_t)
#define	FSCTL_NAMESPACE_ALLOW_DMG_SNAPSHOT_EVENTS IOCBASECMD(FSIOC_NAMESPACE_ALLOW_DMG_SNAPSHOT_EVENTS)

#define FSIOC_TRACKED_HANDLER_GET		  _IOW('A', 12, struct namespace_handler_info)
#define FSCTL_TRACKED_HANDLER_GET		  IOCBASECMD(FSIOC_TRACKED_HANDLER_GET)

#define FSIOC_SNAPSHOT_HANDLER_GET_EXT		  _IOW('A', 13, struct namespace_handler_info_ext)
#define FSCTL_SNAPSHOT_HANDLER_GET_EXT		  IOCBASECMD(FSIOC_SNAPSHOT_HANDLER_GET_EXT)

//
// IO commands 14, 15, 16, and 17 are currently unused
//

//
// Spotlight and fseventsd use these fsctl()'s to find out 
// the mount time of a volume and the last time it was 
// unmounted.  Both HFS and ZFS support these calls.
//
// User space code should pass the "_IOC_" macros while the 
// kernel should test for the "_FSCTL_" variant of the macro 
// in its vnop_ioctl function.
//
// NOTE: the values for these defines should _not_ be changed
//       or else it will break binary compatibility with mds
//       and fseventsd.
//
#define SPOTLIGHT_IOC_GET_MOUNT_TIME		  _IOR('h', 18, u_int32_t)
#define SPOTLIGHT_FSCTL_GET_MOUNT_TIME		  IOCBASECMD(SPOTLIGHT_IOC_GET_MOUNT_TIME)
#define SPOTLIGHT_IOC_GET_LAST_MTIME		  _IOR('h', 19, u_int32_t)
#define SPOTLIGHT_FSCTL_GET_LAST_MTIME		  IOCBASECMD(SPOTLIGHT_IOC_GET_LAST_MTIME)


#ifndef KERNEL

#include <sys/cdefs.h>

__BEGIN_DECLS

int	fsctl(const char *,unsigned long,void*,unsigned int);
int	ffsctl(int,unsigned long,void*,unsigned int);

__END_DECLS

#endif /* !KERNEL */
#endif /* !_SYS_FSCTL_H_ */
