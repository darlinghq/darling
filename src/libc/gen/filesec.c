/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
#include <stdlib.h>
#include <fcntl.h>
#include <strings.h>
#include <errno.h>
#include <uuid/uuid.h>

/*
 * Versions of copy_int/copy_ext that retain native endianity.
 */
extern ssize_t	acl_copy_ext_native(void *buf_p, acl_t acl, ssize_t size);
extern acl_t	acl_copy_int_native(const void *buf_p);

struct _filesec {
	int	fs_valid;
#define FS_VALID_UID		(1<<0)
#define FS_VALID_GID		(1<<1)
#define FS_VALID_UUID		(1<<2)
#define FS_VALID_GRPUUID	(1<<3)
#define	FS_VALID_MODE		(1<<4)
#define FS_VALID_ACL		(1<<5)
	uid_t	fs_uid;
	gid_t	fs_gid;
	uuid_t	fs_uuid;
	uuid_t	fs_grpuuid;
	mode_t	fs_mode;
	size_t	fs_aclsize;
	void	*fs_aclbuf;
};

#define FS_ISVALID(_fsec, _bit)	((_fsec)->fs_valid & (_bit))
#define FS_CLEAR(_fsec, _bit)	((_fsec)->fs_valid &= ~(_bit))
#define FS_SET(_fsec, _bit)	((_fsec)->fs_valid |= (_bit))

static void	filesec_discard_aclbuf(struct _filesec *fsec);

filesec_t
filesec_init(void)
{
	struct _filesec	*fsp;

	fsp = malloc(sizeof(*fsp));
	if (fsp != NULL)
		fsp->fs_valid = 0;
	return(fsp);
}

filesec_t
filesec_dup(filesec_t fsec)
{
	struct _filesec *fsp;

	fsp = malloc(sizeof(*fsp));
	if (fsp != NULL) {
		*fsp = *fsec;
		if (FS_ISVALID(fsec, FS_VALID_ACL)) {
			if (fsec->fs_aclbuf != _FILESEC_REMOVE_ACL) {
				fsp->fs_aclbuf = malloc(fsec->fs_aclsize);
				if (fsp->fs_aclbuf == NULL) {
					free(fsp);
					return(NULL);
				}
				bcopy(fsec->fs_aclbuf, fsp->fs_aclbuf, fsec->fs_aclsize);
			}
		}
	}
	return(fsp);
}

void
filesec_free(filesec_t fsec)
{
	filesec_discard_aclbuf(fsec);
	free(fsec);
}

int
filesec_get_property(filesec_t fsec, filesec_property_t property, void *propptr)
{
	int		error;

	error = 0;
	switch(property) {
	case FILESEC_OWNER:
		if (!FS_ISVALID(fsec, FS_VALID_UID)) {
			error = ENOENT;
			break;
		}
		if (propptr != NULL)
			*(uid_t *)propptr = fsec->fs_uid;
		break;
	case FILESEC_GROUP:
		if (!FS_ISVALID(fsec, FS_VALID_GID)) {
			error = ENOENT;
			break;
		}
		if (propptr != NULL)
			*(gid_t *)propptr = fsec->fs_gid;
		break;
	case FILESEC_UUID:
		if (!FS_ISVALID(fsec, FS_VALID_UUID)) {
			error = ENOENT;
			break;
		}
		if (propptr != NULL)
			uuid_copy(*(uuid_t *)propptr, fsec->fs_uuid);
		break;
	case FILESEC_GRPUUID:
		if (!FS_ISVALID(fsec, FS_VALID_GRPUUID)) {
			error = ENOENT;
			break;
		}
		if (propptr != NULL)
			uuid_copy(*(uuid_t *)propptr, fsec->fs_grpuuid);
		break;
	case FILESEC_MODE:
		if (!FS_ISVALID(fsec, FS_VALID_MODE)) {
			error = ENOENT;
			break;
		}
		if (propptr != NULL)
			*(mode_t *)propptr = fsec->fs_mode;
		break;
	case FILESEC_ACL:
		if (!FS_ISVALID(fsec, FS_VALID_ACL)) {
			error = ENOENT;
			break;
		}
		if (propptr != NULL) {
			if (fsec->fs_aclbuf == _FILESEC_REMOVE_ACL) {
				*(acl_t *)propptr = _FILESEC_REMOVE_ACL;
			} else {
				*(acl_t *)propptr = acl_copy_int_native(fsec->fs_aclbuf);
				if (*(acl_t *)propptr == NULL)
					error = errno;
			}
		}
		break;
	case FILESEC_ACL_RAW:
		if (!FS_ISVALID(fsec, FS_VALID_ACL)) {
			error = ENOENT;
			break;
		}
		if (propptr != NULL)
			*(void **)propptr = fsec->fs_aclbuf;
		break;
	case FILESEC_ACL_ALLOCSIZE:
		if (!FS_ISVALID(fsec, FS_VALID_ACL)) {
			error = ENOENT;
			break;
		}
		if (propptr != NULL)
			*(size_t *)propptr = fsec->fs_aclsize;
		break;
	default:
		error = EINVAL;
		break;
	}
	if (error) {
		errno = error;
		return(-1);
	}
	return(0);
}

int
filesec_set_property(filesec_t fsec, filesec_property_t property, const void *propptr)
{
	void	*aclbuf;
	ssize_t	aclsize, copysize;
	acl_t	acl;
	int	error;

	error = 0;
	switch(property) {
	case FILESEC_OWNER:
		if (propptr == _FILESEC_UNSET_PROPERTY) {
			FS_CLEAR(fsec, FS_VALID_UID);
		} else {
			fsec->fs_uid = *(uid_t *)propptr;
			FS_SET(fsec, FS_VALID_UID);
		}
		break;
	case FILESEC_GROUP:
		if (propptr == _FILESEC_UNSET_PROPERTY) {
			FS_CLEAR(fsec, FS_VALID_GID);
		} else {
			fsec->fs_gid = *(gid_t *)propptr;
			FS_SET(fsec, FS_VALID_GID);
		}
		break;
	case FILESEC_UUID:
		if (propptr == _FILESEC_UNSET_PROPERTY) {
			FS_CLEAR(fsec, FS_VALID_UUID);
		} else {
			uuid_copy(fsec->fs_uuid, *(uuid_t *)propptr);
			FS_SET(fsec, FS_VALID_UUID);
		}
		break;
	case FILESEC_GRPUUID:
		if (propptr == _FILESEC_UNSET_PROPERTY) {
			FS_CLEAR(fsec, FS_VALID_GRPUUID);
		} else {
			uuid_copy(fsec->fs_grpuuid, *(uuid_t *)propptr);
			FS_SET(fsec, FS_VALID_GRPUUID);
		}
		break;
	case FILESEC_MODE:
		if (propptr == _FILESEC_UNSET_PROPERTY) {
			FS_CLEAR(fsec, FS_VALID_MODE);
		} else {
			fsec->fs_mode = *(mode_t *)propptr;
			FS_SET(fsec, FS_VALID_MODE);
		}
		break;
	case FILESEC_ACL:
		if (propptr == _FILESEC_UNSET_PROPERTY) {
			filesec_discard_aclbuf(fsec);
			FS_CLEAR(fsec, FS_VALID_ACL);
		} else if (propptr == _FILESEC_REMOVE_ACL) {
			filesec_discard_aclbuf(fsec);
			fsec->fs_aclbuf = _FILESEC_REMOVE_ACL;
			fsec->fs_aclsize = 0;
			FS_SET(fsec, FS_VALID_ACL);
		} else {
			acl = *(acl_t *)propptr;
			aclsize = acl_size(acl);
			if (aclsize < 0) {
				error = errno;
				break;
			}
			if ((aclbuf = malloc(aclsize)) == NULL) {
				error = errno;
				break;
			}
			copysize = acl_copy_ext_native(aclbuf, acl, aclsize);
			if (copysize < 0) {
				free(aclbuf);
				error = EINVAL;
				break;
			}
			filesec_discard_aclbuf(fsec);
			fsec->fs_aclbuf = aclbuf;
			fsec->fs_aclsize = aclsize;
			FS_SET(fsec, FS_VALID_ACL);
		}
		break;
	case FILESEC_ACL_RAW:
		filesec_discard_aclbuf(fsec);
		if ((propptr != NULL) && (*(void **)propptr != NULL)) {
			fsec->fs_aclbuf = *(void **)propptr;
			FS_SET(fsec, FS_VALID_ACL);
		}
		break;
	case FILESEC_ACL_ALLOCSIZE:
		if (propptr != NULL) {
			fsec->fs_aclsize = *(size_t *)propptr;
		} else {
			error = EINVAL;
		}
		break;
	default:
		error = EINVAL;
		break;
	}
	if (error != 0) {
		errno = error;
		return (-1);
	}
	return (0);
}

int
filesec_unset_property(filesec_t fsec, filesec_property_t property)
{
	return filesec_set_property(fsec, property, _FILESEC_UNSET_PROPERTY);
}

int
filesec_query_property(filesec_t fsec, filesec_property_t property, int *validptr)
{
	int	valid, error;

	error = 0;
	switch(property) {
	case FILESEC_OWNER:
		valid = FS_ISVALID(fsec, FS_VALID_UID);
		break;
	case FILESEC_GROUP:
		valid = FS_ISVALID(fsec, FS_VALID_GID);
		break;
	case FILESEC_UUID:
		valid = FS_ISVALID(fsec, FS_VALID_UUID);
		break;
	case FILESEC_GRPUUID:
		valid = FS_ISVALID(fsec, FS_VALID_GRPUUID);
		break;
	case FILESEC_MODE:
		valid = FS_ISVALID(fsec, FS_VALID_MODE);
		break;
	case FILESEC_ACL:
	case FILESEC_ACL_RAW:
	case FILESEC_ACL_ALLOCSIZE:
		valid = FS_ISVALID(fsec, FS_VALID_ACL);
		break;
	default:
		error = EINVAL;
		break;
	}

	if (error != 0) {
		errno = error;
		return(-1);
	}
	*validptr = valid;
	return(0);
}

static void
filesec_discard_aclbuf(struct _filesec *fsec)
{
	if (FS_ISVALID(fsec, FS_VALID_ACL)) {
		if (fsec->fs_aclbuf != _FILESEC_REMOVE_ACL)
			free(fsec->fs_aclbuf);
		fsec->fs_aclbuf = NULL;
		fsec->fs_aclsize = 0;
		FS_CLEAR(fsec, FS_VALID_ACL);
	}
}
