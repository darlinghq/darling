/*
 * Copyright (c) 2003-2007 Apple Inc. All rights reserved.
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

#ifndef _SYS__TYPES_H_
#define _SYS__TYPES_H_

#include <sys/cdefs.h>
#include <machine/_types.h>

/* pthread opaque structures */
#if defined(__LP64__)
#define __PTHREAD_SIZE__           1168
#define __PTHREAD_ATTR_SIZE__      56
#define __PTHREAD_MUTEXATTR_SIZE__ 8
#define __PTHREAD_MUTEX_SIZE__     56
#define __PTHREAD_CONDATTR_SIZE__  8
#define __PTHREAD_COND_SIZE__      40
#define __PTHREAD_ONCE_SIZE__      8
#define __PTHREAD_RWLOCK_SIZE__      192
#define __PTHREAD_RWLOCKATTR_SIZE__      16
#else /* __LP64__ */
#define __PTHREAD_SIZE__           596 
#define __PTHREAD_ATTR_SIZE__      36
#define __PTHREAD_MUTEXATTR_SIZE__ 8
#define __PTHREAD_MUTEX_SIZE__     40
#define __PTHREAD_CONDATTR_SIZE__  4
#define __PTHREAD_COND_SIZE__      24
#define __PTHREAD_ONCE_SIZE__      4
#define __PTHREAD_RWLOCK_SIZE__    124
#define __PTHREAD_RWLOCKATTR_SIZE__ 12
#endif /* __LP64__ */

struct __darwin_pthread_handler_rec
{
	void           (*__routine)(void *);	/* Routine to call */
	void           *__arg;			/* Argument to pass */
	struct __darwin_pthread_handler_rec *__next;
};
struct _opaque_pthread_attr_t { long __sig; char __opaque[__PTHREAD_ATTR_SIZE__]; };
struct _opaque_pthread_cond_t { long __sig; char __opaque[__PTHREAD_COND_SIZE__]; };
struct _opaque_pthread_condattr_t { long __sig; char __opaque[__PTHREAD_CONDATTR_SIZE__]; };
struct _opaque_pthread_mutex_t { long __sig; char __opaque[__PTHREAD_MUTEX_SIZE__]; };
struct _opaque_pthread_mutexattr_t { long __sig; char __opaque[__PTHREAD_MUTEXATTR_SIZE__]; };
struct _opaque_pthread_once_t { long __sig; char __opaque[__PTHREAD_ONCE_SIZE__]; };
struct _opaque_pthread_rwlock_t { long __sig; char __opaque[__PTHREAD_RWLOCK_SIZE__]; };
struct _opaque_pthread_rwlockattr_t { long __sig; char __opaque[__PTHREAD_RWLOCKATTR_SIZE__]; };
struct _opaque_pthread_t { long __sig; struct __darwin_pthread_handler_rec  *__cleanup_stack; char __opaque[__PTHREAD_SIZE__]; };

/*
 * Type definitions; takes common type definitions that must be used
 * in multiple header files due to [XSI], removes them from the system
 * space, and puts them in the implementation space.
 */

#ifdef __cplusplus
#ifdef __GNUG__
#define __DARWIN_NULL __null
#else /* ! __GNUG__ */
#ifdef __LP64__
#define __DARWIN_NULL (0L)
#else /* !__LP64__ */
#define __DARWIN_NULL 0
#endif /* __LP64__ */
#endif /* __GNUG__ */
#else /* ! __cplusplus */
#define __DARWIN_NULL ((void *)0)
#endif /* __cplusplus */

typedef	__int64_t	__darwin_blkcnt_t;	/* total blocks */
typedef	__int32_t	__darwin_blksize_t;	/* preferred block size */
typedef __int32_t	__darwin_dev_t;		/* dev_t */
typedef unsigned int	__darwin_fsblkcnt_t;	/* Used by statvfs and fstatvfs */
typedef unsigned int	__darwin_fsfilcnt_t;	/* Used by statvfs and fstatvfs */
typedef __uint32_t	__darwin_gid_t;		/* [???] process and group IDs */
typedef __uint32_t	__darwin_id_t;		/* [XSI] pid_t, uid_t, or gid_t*/
typedef __uint64_t	__darwin_ino64_t;	/* [???] Used for 64 bit inodes */
#if __DARWIN_64_BIT_INO_T
typedef __darwin_ino64_t __darwin_ino_t;	/* [???] Used for inodes */
#else /* !__DARWIN_64_BIT_INO_T */
typedef __uint32_t	__darwin_ino_t;		/* [???] Used for inodes */
#endif /* __DARWIN_64_BIT_INO_T */
typedef __darwin_natural_t __darwin_mach_port_name_t; /* Used by mach */
typedef __darwin_mach_port_name_t __darwin_mach_port_t; /* Used by mach */
typedef __uint16_t	__darwin_mode_t;	/* [???] Some file attributes */
typedef __int64_t	__darwin_off_t;		/* [???] Used for file sizes */
typedef __int32_t	__darwin_pid_t;		/* [???] process and group IDs */
typedef struct _opaque_pthread_attr_t
			__darwin_pthread_attr_t; /* [???] Used for pthreads */
typedef struct _opaque_pthread_cond_t
			__darwin_pthread_cond_t; /* [???] Used for pthreads */
typedef struct _opaque_pthread_condattr_t
			__darwin_pthread_condattr_t; /* [???] Used for pthreads */
typedef unsigned long	__darwin_pthread_key_t;	/* [???] Used for pthreads */
typedef struct _opaque_pthread_mutex_t
			__darwin_pthread_mutex_t; /* [???] Used for pthreads */
typedef struct _opaque_pthread_mutexattr_t
			__darwin_pthread_mutexattr_t; /* [???] Used for pthreads */
typedef struct _opaque_pthread_once_t
			__darwin_pthread_once_t; /* [???] Used for pthreads */
typedef struct _opaque_pthread_rwlock_t
			__darwin_pthread_rwlock_t; /* [???] Used for pthreads */
typedef struct _opaque_pthread_rwlockattr_t
			__darwin_pthread_rwlockattr_t; /* [???] Used for pthreads */
typedef struct _opaque_pthread_t
			*__darwin_pthread_t;	/* [???] Used for pthreads */
typedef __uint32_t	__darwin_sigset_t;	/* [???] signal set */
typedef __int32_t	__darwin_suseconds_t;	/* [???] microseconds */
typedef __uint32_t	__darwin_uid_t;		/* [???] user IDs */
typedef __uint32_t	__darwin_useconds_t;	/* [???] microseconds */
typedef	unsigned char	__darwin_uuid_t[16];
typedef	char	__darwin_uuid_string_t[37];

#ifdef KERNEL
#ifndef offsetof
#define offsetof(type, member)	((size_t)(&((type *)0)->member))
#endif /* offsetof */
#endif /* KERNEL */
#endif	/* _SYS__TYPES_H_ */
