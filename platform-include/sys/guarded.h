/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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

#ifndef _SYS_GUARDED_H_
#define _SYS_GUARDED_H_

#include <sys/types.h>
#include <sys/cdefs.h>

#include <sys/_types/_iovec_t.h>
#ifdef PRIVATE

__BEGIN_DECLS

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)

#ifndef _GUARDID_T
#define _GUARDID_T
typedef __uint64_t guardid_t;
#endif /* _GUARDID_T */

#if !defined(KERNEL)
extern int guarded_open_np(const char *path, 
	const guardid_t *guard, u_int guardflags, int flags, ...);
extern int guarded_open_dprotected_np(const char *path, 
	const guardid_t *guard, u_int guardflags, int flags, 
	int dpclass, int dpflags, ...);
extern int guarded_kqueue_np(const guardid_t *guard, u_int guardflags);
extern int guarded_close_np(int fd, const guardid_t *guard);
extern int change_fdguard_np(int fd, const guardid_t *guard, u_int guardflags,
	const guardid_t *nguard, u_int nguardflags, int *fdflagsp);
extern ssize_t guarded_write_np(int fd, const guardid_t *guard, const void *buf, size_t nbyte);
extern ssize_t guarded_pwrite_np(int fd, const guardid_t *guard, const void *buf, size_t nbyte, off_t offset);
extern ssize_t guarded_writev_np(int fd, const guardid_t *guard, const struct iovec *iovp, int iovcnt);
#endif /* KERNEL */

/*
 * Guard types.
 *
 * GUARD_TYPE_FD: Guarded file descriptor.
 */
#define	GUARD_TYPE_FD		0x2

/*
 * File descriptor guard flavors.
 */

/*
 * Forbid close(2), and the implicit close() that a dup2(2) may do.
 * Forces close-on-fork to be set immutably too.
 */
#define GUARD_CLOSE		(1u << 0)

/*
 * Forbid dup(2), dup2(2), and fcntl(2) subcodes F_DUPFD, F_DUPFD_CLOEXEC
 * on a guarded fd. Also forbids open's of a guarded fd via /dev/fd/
 * (an implicit dup.)
 */
#define GUARD_DUP		(1u << 1)

/*
 * Forbid sending a guarded fd via a socket
 */
#define GUARD_SOCKET_IPC	(1u << 2)

/*
 * Forbid creating a fileport from a guarded fd
 */
#define GUARD_FILEPORT		(1u << 3)

/*
 * Forbid writes on a guarded fd
 */
#define GUARD_WRITE		(1u << 4)

/*
 * Violating a guard results in an error (EPERM), and potentially
 * an exception with one or more of the following bits set.
 */
enum guard_exception_codes {
	kGUARD_EXC_CLOSE	= 1u << 0,	/* close of a guarded fd */
	kGUARD_EXC_DUP	   	= 1u << 1,	/* dup of a guarded fd */
	kGUARD_EXC_NOCLOEXEC	= 1u << 2,	/* clear close-on-exec */
	kGUARD_EXC_SOCKET_IPC	= 1u << 3,	/* sendmsg of a guarded fd */
	kGUARD_EXC_FILEPORT   	= 1u << 4,	/* fileport_makeport .. */
	kGUARD_EXC_MISMATCH	= 1u << 5,	/* wrong guard for guarded fd */
	kGUARD_EXC_WRITE   	= 1u << 6	/* write on a guarded fd */
};

#endif /* (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */

__END_DECLS

#endif /* PRIVATE */

#endif /* !_SYS_GUARDED_H_ */
