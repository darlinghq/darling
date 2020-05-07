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
		return -1;
	}

	return getattrlistbulk(dirfd, alist, attrbuf, bufsize,
	           FSOPT_LIST_SNAPSHOT);
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

int
fs_snapshot_root(int dirfd, const char *name, uint32_t flags)
{
	return __fs_snapshot(SNAPSHOT_OP_ROOT, dirfd, name, NULL, NULL, flags);
}

int
fs_snapshot_mount(int dirfd, const char *dir, const char *snapshot,
    uint32_t flags)
{
	return __fs_snapshot(SNAPSHOT_OP_MOUNT, dirfd, snapshot, dir,
	           NULL, flags);
}
