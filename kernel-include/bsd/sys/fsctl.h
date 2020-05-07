/*
 * Copyright (c) 2000-2014 Apple Computer, Inc. All rights reserved.
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

#ifndef _SYS_FSCTL_H_
#define _SYS_FSCTL_H_

#include <sys/ioccom.h>
#include <sys/mount.h>

#ifdef XNU_KERNEL_PRIVATE

/* Userland structs for namespace handler */
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


/* Kernel-internal of namespace handler info */
typedef struct namespace_handler_info {
	user_addr_t  token;
	user_addr_t  flags;
	user_addr_t  fdptr;
} namespace_handler_info;


/* Userland structs for extended namespace handler */
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


/* Kernel-internal of extended namespace handler */
typedef struct namespace_handler_info_ext {
	user_addr_t  token;
	user_addr_t  flags;
	user_addr_t  fdptr;
	user_addr_t  infoptr;
} namespace_handler_info_ext;

/* Size-Augmented namespace_handler_info */
/* 64 bit userland*/
typedef struct user64_namespace_handler_data {
	user64_addr_t token;
	user64_addr_t flags;
	user64_addr_t fdptr;
	user64_addr_t infoptr;
	user64_addr_t objid;
	user64_addr_t reserved1;
	user64_addr_t reserved2;
	user64_addr_t reserved3;
} user64_namespace_handler_data;

/*32 bit userland*/
typedef struct user32_namespace_handler_data {
	user32_addr_t token;
	user32_addr_t flags;
	user32_addr_t fdptr;
	user32_addr_t infoptr;
	user32_addr_t objid;
	user32_addr_t reserved1;
	user32_addr_t reserved2;
	user32_addr_t reserved3;
} user32_namespace_handler_data;

/* kernel-internal */
typedef struct namespace_handler_data {
	user_addr_t token;
	user_addr_t flags;
	user_addr_t fdptr;
	user_addr_t infoptr;
	user_addr_t objid;
	user_addr_t reserved1;
	user_addr_t reserved2;
	user_addr_t reserved3;
} namespace_handler_data;


extern int resolve_nspace_item_ext(struct vnode *vp, uint64_t op, void *arg);

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

typedef struct namespace_handler_data {
	int32_t *token;
	int64_t *flags;
	int32_t *fdptr;
	int64_t *infoptr;     // for snapshot write events, the kernel puts an offset/length pair here
	int64_t *objid;
	uint32_t *reserved1;
	uint32_t *reserved2;
	uint32_t *reserved3;
} namespace_handler_data;

#endif /* XNU_KERNEL_PRIVATE */

#ifdef KERNEL_PRIVATE

#define NSPACE_REARM_NO_ARG ((void *)1)
int resolve_nspace_item(struct vnode *vp, uint64_t op);
int nspace_snapshot_event(vnode_t vp, time_t ctime, uint64_t op_type, void *arg);

#endif // defined(KERNEL_PRIVATE)

#define NAMESPACE_HANDLER_READ_OP             0x0001
#define NAMESPACE_HANDLER_WRITE_OP            0x0002
#define NAMESPACE_HANDLER_DELETE_OP           0x0004
#define NAMESPACE_HANDLER_TRUNCATE_OP         0x0008
#define NAMESPACE_HANDLER_RENAME_OP           0x0010
#define NAMESPACE_HANDLER_METADATA_WRITE_OP   0x0020
#define NAMESPACE_HANDLER_METADATA_DELETE_OP  0x0040
#define NAMESPACE_HANDLER_METADATA_MOD        0x0080
// #define NAMESPACE_HANDLER_OP_DO_NOT_USE    0x0100   // SNAPSHOT_EVENT uses this value
#define NAMESPACE_HANDLER_LINK_CREATE         0x0200
#define NAMESPACE_HANDLER_RENAME_SUCCESS_OP   0x0400
#define NAMESPACE_HANDLER_RENAME_FAILED_OP    0x0800

#define NAMESPACE_HANDLER_NSPACE_EVENT        0x1000
#define NAMESPACE_HANDLER_SNAPSHOT_EVENT      0x0100
#define NAMESPACE_HANDLER_TRACK_EVENT         0x2000

#define NAMESPACE_HANDLER_EVENT_TYPE_MASK (NAMESPACE_HANDLER_NSPACE_EVENT | NAMESPACE_HANDLER_SNAPSHOT_EVENT | NAMESPACE_HANDLER_TRACK_EVENT)


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

/* Disk conditioner configuration */
typedef struct disk_conditioner_info {
	int enabled;
	uint64_t access_time_usec; // maximum latency until transfer begins
	uint64_t read_throughput_mbps; // maximum throughput for reads
	uint64_t write_throughput_mbps; // maximum throughput for writes
	int is_ssd; // behave like an SSD

	/* revision 2 */
	uint32_t ioqueue_depth;
	uint32_t maxreadcnt;
	uint32_t maxwritecnt;
	uint32_t segreadcnt;
	uint32_t segwritecnt;
} disk_conditioner_info;

/*
 * BSD flags manipulation arguments.
 *
 * This provides a safe way to update the BSD flags field of an inode,
 * which has some user components as well as some system components.
 * What it provides is a compare-and-swap operation, whereby the caller
 * fetches what the expected flags are, computes the new set, and then
 * provides the old along with the new.  If the old that's provided matches
 * what's actually in the inode, the new value is set.  The actual inode
 * value is returned to the caller, and expected == actual is how the
 * caller can determine that the operation succeeded.
 *
 * Some BSD flags (e.g. UF_COMPRESSED) can only be manipulated via this
 * safe mechanism.
 */
struct fsioc_cas_bsdflags {
	uint32_t expected_flags;        /* [IN] expected flags */
	uint32_t new_flags;             /* [IN] new value to set */
	uint32_t actual_flags;          /* [OUT] the actual flags in inode */
};

#define FSCTL_SYNC_FULLSYNC     (1<<0)  /* Flush the data fully to disk, if supported by the filesystem */
#define FSCTL_SYNC_WAIT         (1<<1)  /* Wait for the sync to complete */


#define FSIOC_SYNC_VOLUME                         _IOW('A', 1, uint32_t)
#define FSCTL_SYNC_VOLUME                         IOCBASECMD(FSIOC_SYNC_VOLUME)

#define FSIOC_SET_PACKAGE_EXTS                    _IOW('A', 2, struct package_ext_info)
#define FSCTL_SET_PACKAGE_EXTS                    IOCBASECMD(FSIOC_SET_PACKAGE_EXTS)

/* Unsupported - previously FSIOC_WAIT_FOR_SYNC */
#define FSIOC_UNSUPPORTED                         _IOR('A', 3, int32_t)

/* 4 - 9 was used for NAMESPACE handler operation to support dataless file faults
 * no and no longer user */

#define FSIOC_SET_FSTYPENAME_OVERRIDE             _IOW('A', 10, fstypename_t)
#define FSCTL_SET_FSTYPENAME_OVERRIDE             IOCBASECMD(FSIOC_SET_FSTYPENAME_OVERRIDE)

/* 12 was used for TRACKED_HANDLER_GET which has now been removed
 *  as it is no longer used. */

/* 13 was used for FSIOC_SNAPSHOT_HANDLER_GET_EXT and now been removed */

/* 14 was used for NAMESPACE_HANDLER_GETDATA which has now been
 *  removed as it is no longer used. */

#define FSIOC_ROUTEFS_SETROUTEID                          _IO('A', 15)
#define FSCTL_ROUTEFS_SETROUTEID                          IOCBASECMD(FSIOC_ROUTEFS_SETROUTEID)

/* ioctls to support SEEK_HOLE SEEK_DATA */
#define FSIOC_FIOSEEKHOLE                                         _IOWR('A', 16, off_t)
#define FSCTL_FIOSEEKHOLE                                         IOCBASECMD(FSIOC_FIOSEEKHOLE)
#define FSIOC_FIOSEEKDATA                                         _IOWR('A', 17, off_t)
#define FSCTL_FIOSEEKDATA                                         IOCBASECMD(FSIOC_FIOSEEKDATA)

/* Disk conditioner */
#define DISK_CONDITIONER_IOC_GET                  _IOR('A', 18, disk_conditioner_info)
#define DISK_CONDITIONER_FSCTL_GET                IOCBASECMD(DISK_CONDITIONER_IOC_GET)
#define DISK_CONDITIONER_IOC_SET                  _IOW('A', 19, disk_conditioner_info)
#define DISK_CONDITIONER_FSCTL_SET                IOCBASECMD(DISK_CONDITIONER_IOC_SET)

/* Set the value of a file's BSD flags in a safe way. */
#define FSIOC_CAS_BSDFLAGS      _IOWR('A', 20, struct fsioc_cas_bsdflags)

/* Check if a file is only open once (pass zero for the extra arg) */
#define FSIOC_FD_ONLY_OPEN_ONCE _IOWR('A', 21, uint32_t)

//
// Spotlight and fseventsd use these fsctl()'s to find out
// the mount time of a volume and the last time it was
// unmounted.  Both HFS and APFS support these calls.
//
// NOTE: the values for these defines should _not_ be changed
//       or else it will break binary compatibility with mds
//       and fseventsd.
//
#define SPOTLIGHT_IOC_GET_MOUNT_TIME              _IOR('h', 18, u_int32_t)
#define SPOTLIGHT_FSCTL_GET_MOUNT_TIME            IOCBASECMD(SPOTLIGHT_IOC_GET_MOUNT_TIME)
#define SPOTLIGHT_IOC_GET_LAST_MTIME              _IOR('h', 19, u_int32_t)
#define SPOTLIGHT_FSCTL_GET_LAST_MTIME            IOCBASECMD(SPOTLIGHT_IOC_GET_LAST_MTIME)

/* Mark file's extents as "frozen" because someone has references to physical address */
#define FSIOC_FREEZE_EXTENTS                            _IO('h', 20)
#define FSCTL_FREEZE_EXTENTS                            IOCBASECMD(FSIOC_FREEZE_EXTENTS)

/* Clear the "frozen" status of file's extents */
#define FSIOC_THAW_EXTENTS                              _IO('h', 21)
#define FSCTL_THAW_EXTENTS                              IOCBASECMD(FSIOC_THAW_EXTENTS)

/* this FSCTL selector is duplicated in XNU with the intent of making the VFS/generic one the only one eventually */
#define FIRMLINK_STRUCT_LEN 1032
typedef struct generic_firmlink {
	uint8_t array[FIRMLINK_STRUCT_LEN];
} generic_firmlink_t;

#define FSIOC_FIRMLINK_CTL _IOWR ('J', 60, generic_firmlink_t)

#ifndef KERNEL

#include <sys/cdefs.h>

__BEGIN_DECLS

int     fsctl(const char *, unsigned long, void*, unsigned int);
int     ffsctl(int, unsigned long, void*, unsigned int);

__END_DECLS

#endif /* !KERNEL */
#endif /* !_SYS_FSCTL_H_ */
