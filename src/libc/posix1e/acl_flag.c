/*
 * Copyright (c) 2004, 2010 Apple Inc. All rights reserved.
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

#if __DARWIN_ACL_ENTRY_INHERITED != KAUTH_ACE_INHERITED
#  error __DARWIN_ACL_ENTRY_INHERITED != KAUTH_ACE_INHERITED
#endif
#if __DARWIN_ACL_ENTRY_FILE_INHERIT != KAUTH_ACE_FILE_INHERIT
#  error __DARWIN_ACL_ENTRY_FILE_INHERIT != KAUTH_ACE_FILE_INHERIT
#endif
#if __DARWIN_ACL_ENTRY_DIRECTORY_INHERIT != KAUTH_ACE_DIRECTORY_INHERIT
#  error __DARWIN_ACL_ENTRY_DIRECTORY_INHERIT != KAUTH_ACE_DIRECTORY_INHERIT
#endif
#if __DARWIN_ACL_ENTRY_LIMIT_INHERIT != KAUTH_ACE_LIMIT_INHERIT
#  error __DARWIN_ACL_ENTRY_LIMIT_INHERIT != KAUTH_ACE_LIMIT_INHERIT
#endif
#if __DARWIN_ACL_ENTRY_ONLY_INHERIT != KAUTH_ACE_ONLY_INHERIT
#  error __DARWIN_ACL_ENTRY_ONLY_INHERIT != KAUTH_ACE_ONLY_INHERIT
#endif
#if __DARWIN_ACL_FLAG_NO_INHERIT != KAUTH_ACL_NO_INHERIT
#  error __DARWIN_ACL_FLAG_NO_INHERIT != KAUTH_ACL_NO_INHERIT
#endif

int
acl_add_flag_np(acl_flagset_t flags, acl_flag_t flag)
{
	/* XXX validate flags */
	/* XXX validate flag */

	flags->af_flags |= flag;
	return(0);
}

int
acl_clear_flags_np(acl_flagset_t flags)
{
	/* XXX validate flags */

	flags->af_flags = 0;
	return(0);
}

int
acl_delete_flag_np(acl_flagset_t flags, acl_flag_t flag)
{
	/* XXX validate flags */
	/* XXX validate flag */

	flags->af_flags &= ~flag;
	return(0);
}

int
acl_get_flag_np(acl_flagset_t flagset, acl_flag_t flag)
{
	/* XXX validate flags */
	/* XXX validate flag */

	return((flag & flagset->af_flags) ? 1 : 0);
}

int
acl_get_flagset_np(void *obj, acl_flagset_t *flagset_p)
{
	struct _acl		*ap = (struct _acl *)obj;
	struct _acl_entry	*ep = (struct _acl_entry *)obj;
	
	if (_ACL_VALID_ACL(ap)) {
		*flagset_p = (acl_flagset_t)&ap->a_flags;
	} else if (_ACL_VALID_ENTRY(ep)) {
		*flagset_p = (acl_flagset_t)&ep->ae_flags;
	} else {
		errno = EINVAL;
		return(-1);
	}
	return(0);
}

int
acl_set_flagset_np(void *obj, acl_flagset_t flagset)
{
	struct _acl		*ap = (struct _acl *)obj;
	struct _acl_entry	*ep = (struct _acl_entry *)obj;
	
	if (_ACL_VALID_ACL(ap)) {
		ap->a_flags = flagset->af_flags;
	} else if (_ACL_VALID_ENTRY(ep)) {
		ep->ae_flags = flagset->af_flags;
	} else {
		errno = EINVAL;
		return(-1);
	}

	return(0);
}
