/*
 * Copyright (c) 2004-2012 Apple Inc. All rights reserved.
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

#ifndef _SYS_XATTR_H_
#define _SYS_XATTR_H_

#include <sys/types.h>

/* Options for pathname based xattr calls */
#define XATTR_NOFOLLOW   0x0001     /* Don't follow symbolic links */

/* Options for setxattr calls */
#define XATTR_CREATE     0x0002     /* set the value, fail if attr already exists */
#define XATTR_REPLACE    0x0004     /* set the value, fail if attr does not exist */

/* Set this to bypass authorization checking (eg. if doing auth-related work) */
#define XATTR_NOSECURITY 0x0008

/* Set this to bypass the default extended attribute file (dot-underscore file) */
#define XATTR_NODEFAULT  0x0010

/* option for f/getxattr() and f/listxattr() to expose the HFS Compression extended attributes */
#define XATTR_SHOWCOMPRESSION 0x0020

#define	XATTR_MAXNAMELEN   127

/* See the ATTR_CMN_FNDRINFO section of getattrlist(2) for details on FinderInfo */
#define	XATTR_FINDERINFO_NAME	  "com.apple.FinderInfo"

#define	XATTR_RESOURCEFORK_NAME	  "com.apple.ResourceFork"



__BEGIN_DECLS

ssize_t getxattr(const char *path, const char *name, void *value, size_t size, u_int32_t position, int options);

ssize_t fgetxattr(int fd, const char *name, void *value, size_t size, u_int32_t position, int options);

int setxattr(const char *path, const char *name, const void *value, size_t size, u_int32_t position, int options);

int fsetxattr(int fd, const char *name, const void *value, size_t size, u_int32_t position, int options);

int removexattr(const char *path, const char *name, int options);

int fremovexattr(int fd, const char *name, int options);

ssize_t listxattr(const char *path, char *namebuff, size_t size, int options);
 
ssize_t flistxattr(int fd, char *namebuff, size_t size, int options);

__END_DECLS

#endif /* _SYS_XATTR_H_ */
