/*
 * Copyright (c) 2004-2010 Apple, Inc. All rights reserved.
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

#include <sys/appleapiopts.h>
#include <sys/types.h>
#include <sys/acl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "aclvar.h"

#if __DARWIN_ACL_READ_DATA != KAUTH_VNODE_READ_DATA
#  error __DARWIN_ACL_READ_DATA != KAUTH_VNODE_READ_DATA
#endif
#if __DARWIN_ACL_LIST_DIRECTORY != KAUTH_VNODE_LIST_DIRECTORY
#  error __DARWIN_ACL_LIST_DIRECTORY != KAUTH_VNODE_LIST_DIRECTORY
#endif
#if __DARWIN_ACL_WRITE_DATA != KAUTH_VNODE_WRITE_DATA
#  error __DARWIN_ACL_WRITE_DATA != KAUTH_VNODE_WRITE_DATA
#endif
#if __DARWIN_ACL_ADD_FILE != KAUTH_VNODE_ADD_FILE
#  error __DARWIN_ACL_ADD_FILE != KAUTH_VNODE_ADD_FILE
#endif
#if __DARWIN_ACL_EXECUTE != KAUTH_VNODE_EXECUTE
#  error __DARWIN_ACL_EXECUTE != KAUTH_VNODE_EXECUTE
#endif
#if __DARWIN_ACL_SEARCH != KAUTH_VNODE_SEARCH
#  error __DARWIN_ACL_SEARCH != KAUTH_VNODE_SEARCH
#endif
#if __DARWIN_ACL_DELETE != KAUTH_VNODE_DELETE
#  error __DARWIN_ACL_DELETE != KAUTH_VNODE_DELETE
#endif
#if __DARWIN_ACL_APPEND_DATA != KAUTH_VNODE_APPEND_DATA
#  error __DARWIN_ACL_APPEND_DATA != KAUTH_VNODE_APPEND_DATA
#endif
#if __DARWIN_ACL_ADD_SUBDIRECTORY != KAUTH_VNODE_ADD_SUBDIRECTORY
#  error __DARWIN_ACL_ADD_SUBDIRECTORY != KAUTH_VNODE_ADD_SUBDIRECTORY
#endif
#if __DARWIN_ACL_DELETE_CHILD != KAUTH_VNODE_DELETE_CHILD
#  error __DARWIN_ACL_DELETE_CHILD != KAUTH_VNODE_DELETE_CHILD
#endif
#if __DARWIN_ACL_READ_ATTRIBUTES != KAUTH_VNODE_READ_ATTRIBUTES
#  error __DARWIN_ACL_READ_ATTRIBUTES != KAUTH_VNODE_READ_ATTRIBUTES
#endif
#if __DARWIN_ACL_WRITE_ATTRIBUTES != KAUTH_VNODE_WRITE_ATTRIBUTES
#  error __DARWIN_ACL_WRITE_ATTRIBUTES != KAUTH_VNODE_WRITE_ATTRIBUTES
#endif
#if __DARWIN_ACL_READ_EXTATTRIBUTES != KAUTH_VNODE_READ_EXTATTRIBUTES
#  error __DARWIN_ACL_READ_EXTATTRIBUTES != KAUTH_VNODE_READ_EXTATTRIBUTES
#endif
#if __DARWIN_ACL_WRITE_EXTATTRIBUTES != KAUTH_VNODE_WRITE_EXTATTRIBUTES
#  error __DARWIN_ACL_WRITE_EXTATTRIBUTES != KAUTH_VNODE_WRITE_EXTATTRIBUTES
#endif
#if __DARWIN_ACL_READ_SECURITY != KAUTH_VNODE_READ_SECURITY
#  error __DARWIN_ACL_READ_SECURITY != KAUTH_VNODE_READ_SECURITY
#endif
#if __DARWIN_ACL_WRITE_SECURITY != KAUTH_VNODE_WRITE_SECURITY
#  error __DARWIN_ACL_WRITE_SECURITY != KAUTH_VNODE_WRITE_SECURITY
#endif
#if __DARWIN_ACL_CHANGE_OWNER != KAUTH_VNODE_CHANGE_OWNER
#  error __DARWIN_ACL_CHANGE_OWNER != KAUTH_VNODE_CHANGE_OWNER
#endif

int
acl_add_perm(acl_permset_t permset, acl_perm_t perm)
{
	/* XXX validate perms */
	_ACL_VALIDATE_PERM(perm);

	permset->ap_perms |= perm;
	return(0);
}

int
acl_clear_perms(acl_permset_t permset)
{
	/* XXX validate perms */

	permset->ap_perms = 0;
	return(0);
}

int
acl_delete_perm(acl_permset_t permset, acl_perm_t perm)
{
	/* XXX validate perms */
	_ACL_VALIDATE_PERM(perm);

	permset->ap_perms &= ~perm;
	return(0);
}

int
acl_get_perm_np(acl_permset_t permset, acl_perm_t perm)
{
	_ACL_VALIDATE_PERM(perm);

	return((perm & permset->ap_perms) ? 1 : 0);
}

int
acl_get_permset(acl_entry_t entry, acl_permset_t *permset_p)
{
	_ACL_VALIDATE_ENTRY(entry);

	*permset_p = (acl_permset_t)&entry->ae_perms;
	return(0);
}

int
acl_set_permset(acl_entry_t entry, acl_permset_t permset)
{
	_ACL_VALIDATE_ENTRY(entry);

	entry->ae_perms = permset->ap_perms;
	return(0);
}

int
acl_maximal_permset_mask_np(acl_permset_mask_t * mask_p)
{
	/* Bitwise or of all possible acl_perm_t values */
	*mask_p = _ACL_PERMS_MASK;
	return (0);
}

int
acl_get_permset_mask_np(acl_entry_t entry, acl_permset_mask_t * mask_p)
{
	_ACL_VALIDATE_ENTRY(entry);

	*mask_p = (acl_permset_mask_t)entry->ae_perms;
	return (0);
}

int
acl_set_permset_mask_np(acl_entry_t entry, acl_permset_mask_t mask)
{
	_ACL_VALIDATE_ENTRY(entry);
	_ACL_VALIDATE_PERM(mask);

	entry->ae_perms = mask;
	return (0);
}
