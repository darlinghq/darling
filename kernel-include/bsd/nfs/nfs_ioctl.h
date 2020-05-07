/*
 * Copyright (c) 2012,2015 Apple Inc. All rights reserved.
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

/*
 * Header file to export nfs defined ioctls for nfs_vnop_ioctls
 */

#ifndef _NFS_NFS_IOCTL_H_
#define _NFS_NFS_IOCTL_H_
#include <sys/ioccom.h>

/*
 * fsctl (vnop_ioctl) to detroy the callers credentials associated with the vnode's mount
 */
#define NFS_IOC_DESTROY_CRED            _IO('n', 1)

/*
 * fsctl (vnop_ioctl) to set the callers credentials associated with the vnode's mount
 */
struct nfs_gss_principal {
	uint32_t        princlen;       /* length of data */
	uint32_t        nametype;       /* nametype of data */
#ifdef KERNEL
	user32_addr_t   principal;      /* principal data in userspace */
#else
	uint8_t         *principal;
#endif
	uint32_t        flags;          /* Return flags */
};

#ifdef KERNEL
/* LP64 version of nfs_gss_principal */
struct user_nfs_gss_principal {
	uint32_t        princlen;       /* length of data */
	uint32_t        nametype;       /* nametype of data */
	user64_addr_t   principal;      /* principal data in userspace */
	uint32_t        flags;          /* Returned flags */
};
#endif

/* If no credential was found returned NFS_IOC_NO_CRED_FLAG in the flags field. */
#define NFS_IOC_NO_CRED_FLAG            1       /* No credential was found */
#define NFS_IOC_INVALID_CRED_FLAG       2       /* Found a credential, but its not valid */

#define NFS_IOC_SET_CRED                _IOW('n', 2, struct nfs_gss_principal)

#define NFS_IOC_GET_CRED                _IOWR('n', 3, struct nfs_gss_principal)

#define NFS_IOC_DISARM_TRIGGER          _IO('n', 4)

#ifdef KERNEL

#define NFS_IOC_SET_CRED64              _IOW('n', 2, struct user_nfs_gss_principal)

#define NFS_IOC_GET_CRED64              _IOWR('n', 3, struct user_nfs_gss_principal)
#endif

#endif
