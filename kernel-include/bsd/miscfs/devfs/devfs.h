/*
 * Copyright (c) 2000-2002 Apple Computer, Inc. All rights reserved.
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
 * Copyright 1997,1998 Julian Elischer.  All rights reserved.
 * julian@freebsd.org
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * miscfs/devfs/devfs.h
 */

#ifndef _MISCFS_DEVFS_DEVFS_H_
#define _MISCFS_DEVFS_DEVFS_H_

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>

#define DEVFS_CHAR      0
#define DEVFS_BLOCK     1

/*
 * Argument to clone callback after dev
 */
#define DEVFS_CLONE_ALLOC       1       /* Allocate minor number slot */
#define DEVFS_CLONE_FREE        0       /* Free minor number slot */

__BEGIN_DECLS

/*
 * Function: devfs_make_node_clone
 *
 * Purpose
 *   Create a device node with the given pathname in the devfs namespace;
 *   before returning a dev_t value for an open instance, the dev_t has
 *   it's minor number updated by calling the supplied clone function on
 *   the supplied dev..
 *
 * Parameters:
 *   dev        - the dev_t value to associate
 *   chrblk	- block or character device (DEVFS_CHAR or DEVFS_BLOCK)
 *   uid, gid	- ownership
 *   perms	- permissions
 *   clone	- minor number cloning function
 *   fmt, ...	- print format string and args to format the path name
 * Returns:
 *   A handle to a device node if successful, NULL otherwise.
 */
void *  devfs_make_node_clone(dev_t dev, int chrblk, uid_t uid, gid_t gid,
    int perms, int (*clone)(dev_t dev, int action),
    const char *fmt, ...);

/*
 * Function: devfs_make_node
 *
 * Purpose
 *   Create a device node with the given pathname in the devfs namespace.
 *
 * Parameters:
 *   dev        - the dev_t value to associate
 *   chrblk	- block or character device (DEVFS_CHAR or DEVFS_BLOCK)
 *   uid, gid	- ownership
 *   perms	- permissions
 *   fmt, ...	- print format string and args to format the path name
 * Returns:
 *   A handle to a device node if successful, NULL otherwise.
 */
void *  devfs_make_node(dev_t dev, int chrblk, uid_t uid, gid_t gid,
    int perms, const char *fmt, ...);

#ifdef BSD_KERNEL_PRIVATE
/*
 * Function: devfs_make_link
 *
 * Purpose:
 *   Create a link to a previously created device node.
 *
 * Returns:
 *   0 if successful, -1 if failed
 */
int     devfs_make_link(void * handle, char *fmt, ...);
#endif /* BSD_KERNEL_PRIVATE */

/*
 * Function: devfs_remove
 *
 * Purpose:
 *   Remove the device node returned by devfs_make_node() along with
 *   any links created with devfs_make_link().
 */
void    devfs_remove(void * handle);

__END_DECLS

#ifdef __APPLE_API_PRIVATE
/* XXX */
#define UID_ROOT        0
#define UID_BIN         3
#define UID_UUCP        66

/* XXX */
#define GID_WHEEL       0
#define GID_KMEM        2
#define GID_TTY         4
#define GID_OPERATOR    5
#define GID_BIN         7
#define GID_GAMES       13
#define GID_DIALER      68
#define GID_WINDOWSERVER 88
#endif /* __APPLE_API_PRIVATE */

#endif /* !_MISCFS_DEVFS_DEVFS_H_ */
