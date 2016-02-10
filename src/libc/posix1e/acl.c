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

#include <sys/appleapiopts.h>
#include <sys/types.h>
#include <sys/acl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "aclvar.h"

acl_t
acl_dup(acl_t acl)
{
	struct _acl	*ap;

	if (!_ACL_VALID_ACL(acl)) {
		errno = EINVAL;
		return(NULL);
	}
	
	if ((ap = malloc(sizeof(*ap))) != NULL)
		bcopy(acl, ap, sizeof(*ap));
	return(ap);
}

int
acl_free(void *obj)
{
	/*
	 * Without tracking the addresses of text buffers and qualifiers,
	 * we can't validate the obj argument here at all.
	 */
	if(obj != _FILESEC_REMOVE_ACL)
		free(obj);
	return(0);
}

acl_t
acl_init(int count)
{
	struct _acl	*ap;

	/* validate count */
	if (count < 0) {
		errno = EINVAL;
		return(NULL);
	}
	if (count > ACL_MAX_ENTRIES) {
		errno = ENOMEM;
		return(NULL);
	}

	if ((ap = malloc(sizeof (*ap))) != NULL) {
		bzero(ap, sizeof(*ap));
		ap->a_magic = _ACL_ACL_MAGIC;
		ap->a_last_get = -1;
	}
	return(ap);
}

int
acl_valid(acl_t acl)
{
	_ACL_VALIDATE_ACL(acl);

	/* XXX */
	return(0);
}

int
acl_valid_fd_np(int fd, acl_type_t type, acl_t acl)
{
	errno = ENOTSUP;	/* XXX */
	return(-1);
}

int
acl_valid_file_np(const char *path, acl_type_t type, acl_t acl)
{
	errno = ENOTSUP;	/* XXX */
	return(-1);
}

int
acl_valid_link(const char *path, acl_type_t type, acl_t acl)
{
	errno = ENOTSUP;	/* XXX */
	return(-1);
}

/*
 * Not applicable; not supportedl
 */
int
acl_calc_mask(__unused acl_t *acl_p)
{
	errno = ENOTSUP;
	return(-1);
}
