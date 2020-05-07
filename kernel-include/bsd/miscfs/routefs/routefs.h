/*
 * Copyright (c) 2015 Apple Computer, Inc. All rights reserved.
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
#ifndef _MISCFS_ROUTEFS_DEVFS_H_
#define _MISCFS_ROUTEFS_DEVFS_H_

#include <sys/appleapiopts.h>


__BEGIN_DECLS


#ifdef BSD_KERNEL_PRIVATE

struct routefs_args {
	char    route_path[MAXPATHLEN];/* path name of the target route */
	vnode_t route_rvp; /* vnode of the target of route */
};

struct routefs_mount {
	char    route_path[MAXPATHLEN];/* path name of the target route */
	mount_t route_mount;
	vnode_t route_rvp; /* vnode of the target of route */
	int route_vpvid; /* vnode of the target of route */
};


/*
 * Function: routefs_kernel_mount
 *
 * Purpose:
 *   mount routefs
 *   any links created with devfs_make_link().
 */
int     routefs_kernel_mount(char * routepath);

#endif /* BSD_KERNEL_PRIVATE */

__END_DECLS


#endif /* !_MISCFS_ROUTEFS_DEVFS_H_ */
