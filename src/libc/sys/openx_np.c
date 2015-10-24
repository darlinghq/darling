/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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
#include <sys/types.h>
#include <sys/acl.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

enum {OPENX, MKFIFOX, MKDIRX};

extern int __open_extended(const char *, int, uid_t, gid_t, int, struct kauth_filesec *);
extern int __mkfifo_extended(const char *, uid_t, gid_t, int, struct kauth_filesec *);
extern int __mkdir_extended(const char *, uid_t, gid_t, int, struct kauth_filesec *);

static int
_mkfilex_np(int opcode, const char *path, int flags, filesec_t fsec)
{
	uid_t owner = KAUTH_UID_NONE;
	gid_t group = KAUTH_GID_NONE;
	mode_t mode = 0;
	size_t size = 0;
	int fsacl_used = 0;
	struct kauth_filesec *fsacl = NULL;
	struct kauth_filesec static_filesec;

	/* handle extended security data */
	if (fsec != NULL) {
		/* fetch basic parameters */
		if ((filesec_get_property(fsec, FILESEC_OWNER, &owner) != 0) && (errno != ENOENT))
			return(-1);
		if ((filesec_get_property(fsec, FILESEC_GROUP, &group) != 0) && (errno != ENOENT))
			return(-1);
		if ((filesec_get_property(fsec, FILESEC_MODE, &mode) != 0) && (errno != ENOENT))
			return(-1);

		/* try to fetch the ACL */
		if (((filesec_get_property(fsec, FILESEC_ACL_RAW, &fsacl) != 0) ||
			(filesec_get_property(fsec, FILESEC_ACL_ALLOCSIZE, &size) != 0)) &&
		    (errno != ENOENT))
			return(-1);

		/* only valid for chmod */
		if (fsacl == _FILESEC_REMOVE_ACL) {
			errno = EINVAL;
			return(-1);
		}

		/* no ACL, use local filesec */
		if (fsacl == NULL) {
			bzero(&static_filesec, sizeof(static_filesec));
			fsacl = &static_filesec;
			fsacl->fsec_magic = KAUTH_FILESEC_MAGIC;
			fsacl->fsec_entrycount = KAUTH_FILESEC_NOACL;
		} else {
			fsacl_used = 1;
		}

		/* grab the owner and group UUID if present */
		if (filesec_get_property(fsec, FILESEC_UUID, &fsacl->fsec_owner) != 0) {
			if (errno != ENOENT)
				return(-1);
			bzero(&fsacl->fsec_owner, sizeof(fsacl->fsec_owner));
		} else {
			fsacl_used = 1;
		}
		if (filesec_get_property(fsec, FILESEC_GRPUUID, &fsacl->fsec_group) != 0) {
			if (errno != ENOENT)
				return(-1);
			bzero(&fsacl->fsec_group, sizeof(fsacl->fsec_group));
		} else {
			fsacl_used = 1;
		}

		/* after all this, if we didn't find anything that needs it, don't pass it in */
		if (!fsacl_used)
			fsacl = NULL;
	}

	switch (opcode) {
	case OPENX:
		return(__open_extended(path, flags, owner, group, mode, fsacl));
	case MKFIFOX:
		return(__mkfifo_extended(path, owner, group, mode, fsacl));
	case MKDIRX:
		return(__mkdir_extended(path, owner, group, mode, fsacl));
	}
	/* should never get here */
	errno = EINVAL;
	return(-1);
}

int
openx_np(const char *path, int flags, filesec_t fsec)
{
	/* optimise for the simple case */
	if (!(flags & O_CREAT) || (fsec == NULL))
		return(open(path, flags));
	return(_mkfilex_np(OPENX, path, flags, fsec));
}

int
mkfifox_np(const char *path, filesec_t fsec)
{
	return(_mkfilex_np(MKFIFOX, path, 0, fsec));
}

int
mkdirx_np(const char *path, filesec_t fsec)
{
	return(_mkfilex_np(MKDIRX, path, 0, fsec));
}
