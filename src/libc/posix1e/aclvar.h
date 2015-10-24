/*
 * Copyright (c) 2004, 2008, 2010 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _ACLVAR_H
#define _ACLVAR_H

#include <sys/kauth.h>

/*
 * Internal access control list entry representation.
 */
struct _acl_entry {
	u_int32_t	ae_magic;
#define _ACL_ENTRY_MAGIC	0xac1ac101
	u_int32_t	ae_tag;
	guid_t		ae_applicable;
	u_int32_t	ae_flags;
	u_int32_t	ae_perms;
};

/*
 * Internal representation of an ACL.
 * XXX static allocation is wasteful. 
 */
struct _acl {
	u_int32_t	a_magic;
#define _ACL_ACL_MAGIC		0xac1ac102
	unsigned	a_entries;
	int		a_last_get;
	u_int32_t	a_flags;
	struct _acl_entry a_ace[ACL_MAX_ENTRIES];
};

/*
 * ACL/entry flags.
 */
struct _acl_flagset {
	u_int32_t	af_flags;
};

/*
 * ACL entry permissions.
 */
struct _acl_permset {
	u_int32_t	ap_perms;
};

/*
 * Argument validation.
 */
/*
 * Because of the use of special values for structure pointer (like
 * _FILESEC_REMOVE_ACL), dereferences causes crashes.  Rather than try to
 * enumerate all such special values, we will assume there are a small
 * number of these values, centered about zero, so we can just check the
 * values are in this range.  We have to do the check for both ACLs and
 * ACEs, because the API uses the same routines on ACLs and ACEs.
 */

#define _ACL_SPECIAL_RANGE	16

#define _ACL_VALID_ENTRY(_e)	((((intptr_t)(_e)) > _ACL_SPECIAL_RANGE || ((intptr_t)(_e)) < -(_ACL_SPECIAL_RANGE)) && (_e)->ae_magic == _ACL_ENTRY_MAGIC)

#define _ACL_VALID_ACL(_a)	((((intptr_t)(_a)) > _ACL_SPECIAL_RANGE || ((intptr_t)(_a)) < -(_ACL_SPECIAL_RANGE)) && (_a)->a_magic == _ACL_ACL_MAGIC)

#define _ACL_ENTRY_CONTAINED(_a, _e) \
	((_e) >= &(_a)->a_ace[0]) && ((_e) < &(_a)->a_ace[ACL_MAX_ENTRIES])

#define _ACL_VALID_FLAG(_f) (((_f) & _ACL_FLAGS_MASK) == (_f))

#define _ACL_VALID_ENTRY_FLAG(_f) (((_f) & _ACL_ENTRY_FLAGS_MASK) == (_f))

#define _ACL_PERMS_MASK	(ACL_READ_DATA |					\
			ACL_LIST_DIRECTORY |				\
			ACL_WRITE_DATA |				\
			ACL_ADD_FILE |					\
			ACL_EXECUTE |					\
			ACL_SEARCH |					\
			ACL_DELETE |					\
			ACL_APPEND_DATA |				\
			ACL_ADD_SUBDIRECTORY |				\
			ACL_DELETE_CHILD |				\
			ACL_READ_ATTRIBUTES |				\
			ACL_WRITE_ATTRIBUTES |				\
			ACL_READ_EXTATTRIBUTES |			\
			ACL_WRITE_EXTATTRIBUTES |			\
			ACL_READ_SECURITY |				\
			ACL_WRITE_SECURITY |				\
			ACL_CHANGE_OWNER)

#define _ACL_VALID_PERM(_f)	(((_f) & ~_ACL_PERMS_MASK) == 0)

#define _ACL_VALIDATE_ACL(_a)						\
do {									\
	if (!_ACL_VALID_ACL((_a))) {					\
		errno = EINVAL;						\
		return(-1);						\
	}								\
} while (0)

#define _ACL_VALIDATE_ENTRY(_e)						\
do {									\
	if (!_ACL_VALID_ENTRY((_e))) {					\
		errno = EINVAL;						\
		return(-1);						\
	}								\
} while (0)

#define _ACL_VALIDATE_ENTRY_CONTAINED(_a, _e)				\
do {									\
	if (!_ACL_ENTRY_CONTAINED((_a), (_e))) {			\
		errno = EINVAL;						\
		return(-1);						\
	}								\
} while (0)
	
#define _ACL_VALIDATE_FLAG(_f)						\
do {									\
	if (!_ACL_VALID_FLAG((_f))) {					\
		errno = EINVAL;						\
		return(-1);						\
	}								\
} while (0)

#define _ACL_VALIDATE_ENTRY_FLAG(_f)					\
do {									\
	if (!_ACL_VALID_ENTRY_FLAG((_f))) {				\
		errno = EINVAL;						\
		return(-1);						\
	}								\
} while (0)

#define _ACL_VALIDATE_PERM(_f)						\
do {									\
	if (!_ACL_VALID_PERM((_f))) {					\
		errno = EINVAL;						\
		return(-1);						\
	}								\
} while (0)

#endif /* _ACLVAR_H */
