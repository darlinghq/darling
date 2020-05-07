/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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


#ifndef _LOCKD_MACH_TYPES_H_
#define _LOCKD_MACH_TYPES_H_

/*
 * XXX NFSV3_MAX_FH_SIZE is defined in sys/mount.h, but we can't include
 * that here. Osfmk includes libsa/types.h which causes massive conflicts
 * with sys/types.h that get indirectly included with sys/mount.h. In user
 * land below will work on a build that does not yet have the new macro
 * definition.
 */

#ifndef NFSV3_MAX_FH_SIZE
#define NFSV3_MAX_FH_SIZE 64
#endif

typedef uint32_t sock_storage[32];
typedef uint32_t xcred[19];
typedef uint8_t nfs_handle[NFSV3_MAX_FH_SIZE];

#endif
