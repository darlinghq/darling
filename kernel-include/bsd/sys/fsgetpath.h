/*
 * Copyright (c) 2008-2017 Apple Inc. All rights reserved.
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

#ifndef _FSGETPATH_H_
#define _FSGETPATH_H_

#ifndef KERNEL

#include <machine/_types.h>
#include <sys/_types/_ssize_t.h>
#include <sys/_types/_size_t.h>
#include <sys/_types/_fsid_t.h>
#include <_types/_uint64_t.h>
#include <Availability.h>

#ifdef PRIVATE
/*
 * These are only included for compatibility with previous header
 */
#include <sys/types.h>
#include <sys/mount.h>
#ifdef __APPLE_API_PRIVATE
#include <sys/attr.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif  /* __APPLE_API_PRIVATE */
#endif /* PRIVATE */

__BEGIN_DECLS

/*
 * Obtain the full pathname of a file system object by id.
 */
ssize_t fsgetpath(char *, size_t, fsid_t *, uint64_t) __OSX_AVAILABLE(10.13) __IOS_AVAILABLE(11.0) __TVOS_AVAILABLE(11.0) __WATCHOS_AVAILABLE(4.0);

#ifdef PRIVATE
#include <sys/_types/_fsobj_id_t.h>

#ifndef FSOPT_NOFIRMLINKPATH     /* also in attr.h */
#define FSOPT_NOFIRMLINKPATH     0x00000080
#endif

#ifndef FSOPT_ISREALFSID     /* also in attr.h */
#ifdef  FSOPT_RETURN_REALDEV
#define FSOPT_ISREALFSID         FSOPT_RETURN_REALDEV
#else
#define FSOPT_ISREALFSID         0x00000200
#endif
#endif /* FSOPT_ISREALFSID */

#ifdef __APPLE_API_PRIVATE


/*
 * openbyid_np: open a file given a file system id and a file system object id
 *
 * fsid :	value corresponding to getattlist ATTR_CMN_FSID attribute, or
 *			value of stat's st.st_dev ; set fsid = {st.st_dev, 0}
 *
 * objid: value (link id/node id) corresponding to getattlist ATTR_CMN_OBJID
 *		  attribute , or
 *		  value of stat's st.st_ino (node id); set objid =  st.st_ino
 *
 * For hfs the value of getattlist ATTR_CMN_FSID is a link id which uniquely identifies a
 * parent in the case of hard linked files; this allows unique path access validation.
 * Not all file systems support getattrlist ATTR_CMN_OBJID (link id).
 * A node id does not uniquely identify a parent in the case of hard linked files and may
 * resolve to a path for which access validation can fail.
 */
int openbyid_np(fsid_t* fsid, fsobj_id_t* objid, int flags);

ssize_t fsgetpath_ext(char *, size_t, fsid_t *, uint64_t, uint32_t) __OSX_AVAILABLE(10.15) __IOS_AVAILABLE(13.0) __TVOS_AVAILABLE(13.0) __WATCHOS_AVAILABLE(6.0);

#endif /* __APPLE_API_PRIVATE */
#endif /* PRIVATE */

__END_DECLS

#endif /* KERNEL */

#endif /* !_FSGETPATH_H_ */
