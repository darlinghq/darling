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

extern int __umask_extended(int, acl_t);

int
umaskx_np(filesec_t fsec)
{
	acl_t acl = NULL;
	size_t size = 0;
	mode_t newmask = 0;

	if (fsec)
	{
		if ((filesec_get_property(fsec, FILESEC_MODE, &newmask) != 0) && (errno != ENOENT))
			return(-1);

		if (((filesec_get_property(fsec, FILESEC_ACL_RAW, &acl) != 0) ||
			(filesec_get_property(fsec, FILESEC_ACL_ALLOCSIZE, &size) != 0)) &&
		    (errno != ENOENT))
			return(-1);
		if (size == 0)
			acl = NULL;
	}
	return __umask_extended(newmask, acl);
}
