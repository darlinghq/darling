/*
 * Copyright (c) 2000-2008 Apple Inc. All rights reserved.
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
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*
 * Copyright (c) 1982, 1986, 1991, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)types.h	8.4 (Berkeley) 1/21/94
 */

#ifndef _SYS_TYPES_H_
#define	_SYS_TYPES_H_

#include <sys/appleapiopts.h>

#ifndef __ASSEMBLER__
#include <sys/cdefs.h>

/* Machine type dependent parameters. */
#include <machine/types.h>
#include <sys/_types.h>

#include <machine/endian.h>

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
typedef	unsigned char		u_char;
typedef	unsigned short		u_short;
typedef	unsigned int		u_int;
#ifndef _U_LONG
typedef	unsigned long		u_long;
#define _U_LONG
#endif
typedef	unsigned short		ushort;		/* Sys V compatibility */
typedef	unsigned int		uint;		/* Sys V compatibility */
#endif

typedef	u_int64_t		u_quad_t;	/* quads */
typedef	int64_t			quad_t;
typedef	quad_t *		qaddr_t;

typedef	char *			caddr_t;	/* core address */
typedef	int32_t			daddr_t;	/* disk address */

#ifndef _DEV_T
typedef	__darwin_dev_t		dev_t;		/* device number */
#define _DEV_T
#endif

typedef	u_int32_t		fixpt_t;	/* fixed point number */

#ifndef _BLKCNT_T
typedef	__darwin_blkcnt_t	blkcnt_t;
#define	_BLKCNT_T
#endif

#ifndef _BLKSIZE_T
typedef	__darwin_blksize_t	blksize_t;
#define	_BLKSIZE_T
#endif

#ifndef _GID_T
typedef __darwin_gid_t		gid_t;
#define _GID_T
#endif

#ifndef _IN_ADDR_T
#define _IN_ADDR_T
typedef	__uint32_t		in_addr_t;	/* base type for internet address */
#endif

#ifndef _IN_PORT_T
#define _IN_PORT_T
typedef	__uint16_t		in_port_t;
#endif

#ifndef	_INO_T
typedef	__darwin_ino_t		ino_t;		/* inode number */
#define _INO_T
#endif

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
#ifndef	_INO64_T
typedef	__darwin_ino64_t	ino64_t;	/* 64bit inode number */
#define _INO64_T
#endif
#endif /* !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE) */

#ifndef _KEY_T
#define _KEY_T
typedef	__int32_t		key_t;		/* IPC key (for Sys V IPC) */
#endif

#ifndef	_MODE_T
typedef	__darwin_mode_t		mode_t;
#define _MODE_T
#endif

#ifndef _NLINK_T
typedef	__uint16_t		nlink_t;	/* link count */
#define	_NLINK_T
#endif

#ifndef _ID_T
#define _ID_T
typedef __darwin_id_t		id_t;		/* can hold pid_t, gid_t, or uid_t */
#endif

#ifndef _PID_T
typedef __darwin_pid_t		pid_t;
#define _PID_T
#endif

#ifndef _OFF_T
typedef __darwin_off_t		off_t;
#define _OFF_T
#endif

typedef	int32_t			segsz_t;	/* segment size */
typedef	int32_t			swblk_t;	/* swap offset */

#ifndef _UID_T
typedef __darwin_uid_t		uid_t;		/* user id */
#define _UID_T
#endif

#ifndef _ID_T
typedef __darwin_id_t		id_t;
#define _ID_T
#endif

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
/* Major, minor numbers, dev_t's. */
#if defined(__cplusplus)
/*
 * These lowercase macros tend to match member functions in some C++ code,
 * so for C++, we must use inline functions instead.
 */

static inline __int32_t  major(__uint32_t _x)
{
	return (__int32_t)(((__uint32_t)_x >> 24) & 0xff);
}

static inline __int32_t  minor(__uint32_t _x)
{
	return (__int32_t)((_x) & 0xffffff);
}

static inline dev_t  makedev(__uint32_t _major, __uint32_t _minor)
{
	return (dev_t)(((_major) << 24) | (_minor));
}

#else	/* !__cplusplus */

#define	major(x)	((int32_t)(((u_int32_t)(x) >> 24) & 0xff))
#define	minor(x)	((int32_t)((x) & 0xffffff))
#define	makedev(x,y)	((dev_t)(((x) << 24) | (y)))

#endif	/* !__cplusplus */
#endif	/* !_POSIX_C_SOURCE */

#ifndef	_CLOCK_T
#define	_CLOCK_T
typedef	__darwin_clock_t	clock_t;
#endif

#ifndef _SIZE_T
#define _SIZE_T
/* DO NOT REMOVE THIS COMMENT: fixincludes needs to see
 * _GCC_SIZE_T */
typedef __darwin_size_t		size_t;
#endif

#ifndef	_SSIZE_T
#define	_SSIZE_T
typedef	__darwin_ssize_t	ssize_t;
#endif

#ifndef	_TIME_T
#define	_TIME_T
typedef	__darwin_time_t		time_t;
#endif

#ifndef _USECONDS_T
#define _USECONDS_T
typedef __darwin_useconds_t	useconds_t;
#endif

#ifndef _SUSECONDS_T
#define _SUSECONDS_T
typedef __darwin_suseconds_t	suseconds_t;
#endif

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
/*
 * This code is present here in order to maintain historical backward
 * compatability, and is intended to be removed at some point in the
 * future; please include <sys/select.h> instead.
 */
#define __need_fd_set
#include <sys/_structs.h>

#define	NBBY		__DARWIN_NBBY		/* bits in a byte */
#define NFDBITS		__DARWIN_NFDBITS	/* bits per mask */
#define	howmany(x, y)	__DARWIN_howmany(x, y)	/* # y's == x bits? */
typedef __int32_t	fd_mask;

/*
 * Select uses bit masks of file descriptors in longs.  These macros
 * manipulate such bit fields (the filesystem macros use chars).  The
 * extra protection here is to permit application redefinition above
 * the default size.
 */
#ifndef	FD_SETSIZE
#define	FD_SETSIZE	__DARWIN_FD_SETSIZE
#endif /* FD_SETSIZE */
#ifndef FD_SET
#define	FD_SET(n, p)	__DARWIN_FD_SET(n, p)
#endif /* FD_SET */
#ifndef FD_CLR
#define	FD_CLR(n, p)	__DARWIN_FD_CLR(n, p)
#endif /* FD_CLR */
#ifndef FD_ISSET
#define	FD_ISSET(n, p)	__DARWIN_FD_ISSET(n, p)
#endif /* FD_ISSET */
#ifndef FD_ZERO
#define	FD_ZERO(p)	__DARWIN_FD_ZERO(p)
#endif /* FD_ZERO */
#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
#ifndef FD_COPY
#define	FD_COPY(f, t)	__DARWIN_FD_COPY(f, t)
#endif /* FD_COPY */
#endif	/* (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */


#if defined(__STDC__) && defined(KERNEL)
/*
 * Forward structure declarations for function prototypes.  We include the
 * common structures that cross subsystem boundaries here; others are mostly
 * used in the same place that the structure is defined.
 */
struct	proc;
struct	pgrp;
struct	ucred;
struct	rusage;
struct	file;
struct	buf;
struct	tty;
struct	uio;
#endif

#endif /* (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */
#endif /* __ASSEMBLER__ */

#ifndef __POSIX_LIB__

#ifndef _PTHREAD_ATTR_T
#define _PTHREAD_ATTR_T
typedef __darwin_pthread_attr_t		pthread_attr_t;
#endif
#ifndef _PTHREAD_COND_T
#define _PTHREAD_COND_T
typedef __darwin_pthread_cond_t		pthread_cond_t;
#endif
#ifndef _PTHREAD_CONDATTR_T
#define _PTHREAD_CONDATTR_T
typedef __darwin_pthread_condattr_t	pthread_condattr_t;
#endif
#ifndef _PTHREAD_MUTEX_T
#define _PTHREAD_MUTEX_T
typedef __darwin_pthread_mutex_t	pthread_mutex_t;
#endif
#ifndef _PTHREAD_MUTEXATTR_T
#define _PTHREAD_MUTEXATTR_T
typedef __darwin_pthread_mutexattr_t	pthread_mutexattr_t;
#endif
#ifndef _PTHREAD_ONCE_T
#define _PTHREAD_ONCE_T
typedef __darwin_pthread_once_t		pthread_once_t;
#endif
#ifndef _PTHREAD_RWLOCK_T
#define _PTHREAD_RWLOCK_T
typedef __darwin_pthread_rwlock_t	pthread_rwlock_t;
#endif
#ifndef _PTHREAD_RWLOCKATTR_T
#define _PTHREAD_RWLOCKATTR_T
typedef __darwin_pthread_rwlockattr_t	pthread_rwlockattr_t;
#endif
#ifndef _PTHREAD_T
#define _PTHREAD_T
typedef __darwin_pthread_t		pthread_t;
#endif

#endif /* __POSIX_LIB__ */

#ifndef _PTHREAD_KEY_T
#define _PTHREAD_KEY_T
typedef __darwin_pthread_key_t		pthread_key_t;
#endif

/* statvfs and fstatvfs */
#ifndef _FSBLKCNT_T
#define _FSBLKCNT_T
typedef __darwin_fsblkcnt_t		fsblkcnt_t;
#endif

#ifndef _FSFILCNT_T
#define _FSFILCNT_T
typedef __darwin_fsfilcnt_t		fsfilcnt_t;
#endif


#endif /* !_SYS_TYPES_H_ */
