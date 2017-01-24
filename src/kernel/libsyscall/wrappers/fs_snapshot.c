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

#include <sys/snapshot.h>
#include <sys/attr.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

extern int __fs_snapshot(uint32_t, int, const char *, const char *, void *,
    uint32_t);

int
fs_snapshot_create(int dirfd, const char *name, uint32_t flags)
{
	return __fs_snapshot(SNAPSHOT_OP_CREATE, dirfd, name, NULL, NULL, flags);
}

int
fs_snapshot_list(int dirfd, struct attrlist *alist, void *attrbuf, size_t bufsize,
    uint32_t flags)
{
	if (flags != 0) {
		errno = EINVAL;
		return (-1);
	}

	return (getattrlistbulk(dirfd, alist, attrbuf, bufsize,
	    FSOPT_LIST_SNAPSHOT));
}

int
fs_snapshot_delete(int dirfd, const char *name, uint32_t flags)
{
	return __fs_snapshot(SNAPSHOT_OP_DELETE, dirfd, name, NULL, NULL, flags);
}

int
fs_snapshot_rename(int dirfd, const char *old, const char *new, uint32_t flags)
{
	return __fs_snapshot(SNAPSHOT_OP_RENAME, dirfd, old, new, NULL, flags);
}

int
fs_snapshot_revert(int dirfd, const char *name, uint32_t flags)
{
    return __fs_snapshot(SNAPSHOT_OP_REVERT, dirfd, name, NULL, NULL, flags);
}

/*
 * XXX Temporary hack to do what mount_apfs(8) does. This will be removed and
 * replaced with a VFS_IOCTL based implementation in the kernel.
 */
#include <sys/stat.h>
#include "strings.h"

#ifndef SNAPSHOT_OP_MOUNT
#define SNAPSHOT_OP_MOUNT 0x4
#endif

#define	FS_MOUNT_SNAPSHOT 2
#define MAX_SNAPSHOT_NAMELEN	256

struct fs_mount_options {
	uint32_t fs_flags;
	uint8_t	_padding_[2];
};

struct fs_mount_args {
	char *specdev;
	struct fs_mount_options options;
	uint16_t mode;
	uint16_t _padding_[3];
	union {
		struct {			// FS_MOUNT_SNAPSHOT
			dev_t snap_fsys;
			char snap_name[MAX_SNAPSHOT_NAMELEN];
		};
		struct {			// APFS_MOUNT_FOR_CONVERSION
		};
	};
};

int
fs_snapshot_mount(int dirfd, const char *dir, const char *snapshot,
    uint32_t flags)
{
	struct stat st;
	struct fs_mount_args mnt_args;

	mnt_args.specdev = NULL;
	mnt_args.mode = FS_MOUNT_SNAPSHOT;
	if (fstat(dirfd, &st) == -1)
		return (-1);

	mnt_args.snap_fsys = st.st_dev;
	strlcpy(mnt_args.snap_name, snapshot, sizeof(mnt_args.snap_name));
	return (__fs_snapshot(SNAPSHOT_OP_MOUNT, dirfd, snapshot, dir,
	    (void *)&mnt_args, flags));
}
