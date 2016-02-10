/*
 * Copyright (c) 2004-2006 Apple Computer, Inc. All rights reserved.
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

#include <sys/cdefs.h>
#include <sys/_types.h>

#ifdef __need_ucontext_t
#ifndef __need_struct_ucontext
#define __need_struct_ucontext
#endif /* __need_struct_ucontext */
#endif /* __need_ucontext_t */

#ifdef __need_ucontext64_t
#ifndef __need_struct_ucontext64
#define __need_struct_ucontext64
#endif /* __need_struct_ucontext64 */
#endif /* __need_ucontext64_t */

#ifdef __need_struct_ucontext
#ifndef __need_struct_mcontext
#define __need_struct_mcontext
#endif /* __need_struct_mcontext */
#endif /* __need_struct_ucontext */

#ifdef __need_struct_ucontext64
#ifndef __need_struct_mcontext64
#define __need_struct_mcontext64
#endif /* __need_struct_mcontext64 */
#endif /* __need_struct_ucontext64 */

#if defined(__need_struct_mcontext) || defined(__need_struct_mcontext64)
#include <machine/_structs.h>
#endif /* __need_struct_mcontext || __need_struct_mcontext64 */

#if defined(__need_stack_t) || defined(__need_struct_ucontext) || defined(__need_struct_ucontext64)
#ifndef __need_struct_sigaltstack
#define __need_struct_sigaltstack
#endif /* __need_struct_sigaltstack */
#endif /* __need_stack_t || __need_struct_ucontext || __need_struct_ucontext64 */

#ifdef __need_struct_sigaltstack
#undef __need_struct_sigaltstack
/* Structure used in sigaltstack call. */
#ifndef _STRUCT_SIGALTSTACK
#if __DARWIN_UNIX03
#define _STRUCT_SIGALTSTACK	struct __darwin_sigaltstack
#else /* !__DARWIN_UNIX03 */
#define _STRUCT_SIGALTSTACK	struct sigaltstack
#endif /* __DARWIN_UNIX03 */
_STRUCT_SIGALTSTACK
{
	void		*ss_sp;		/* signal stack base */
	__darwin_size_t ss_size;	/* signal stack length */
	int		ss_flags;	/* SA_DISABLE and/or SA_ONSTACK */
};
#endif /* _STRUCT_SIGALTSTACK */
#endif /* __need_struct_sigaltstack */

#ifdef __need_struct_timespec
#undef __need_struct_timespec
#ifndef _STRUCT_TIMESPEC
#define _STRUCT_TIMESPEC	struct timespec
_STRUCT_TIMESPEC
{
	__darwin_time_t	tv_sec;
	long		tv_nsec;
};
#endif /* _STRUCT_TIMESPEC */
#endif /* __need_struct_timespec */

#ifdef __need_struct_timeval
#undef __need_struct_timeval
#ifndef _STRUCT_TIMEVAL
#define _STRUCT_TIMEVAL		struct timeval
_STRUCT_TIMEVAL
{
	__darwin_time_t		tv_sec;		/* seconds */
	__darwin_suseconds_t	tv_usec;	/* and microseconds */
};
#endif /* _STRUCT_TIMEVAL */
#endif /* __need_struct_timeval */

#ifdef __need_struct_timeval32
#undef __need_struct_timeval32
#ifndef _STRUCT_TIMEVAL32
#define _STRUCT_TIMEVAL32	struct timeval32
_STRUCT_TIMEVAL32
{
	__int32_t		tv_sec;		/* seconds */
	__int32_t		tv_usec;	/* and microseconds */
};
#endif /* _STRUCT_TIMEVAL32 */
#endif /* __need_struct_timeval32 */

#ifdef __need_struct_ucontext
#undef __need_struct_ucontext
#ifndef _STRUCT_UCONTEXT
#if __DARWIN_UNIX03
#define _STRUCT_UCONTEXT	struct __darwin_ucontext
#else /* !__DARWIN_UNIX03 */
#define _STRUCT_UCONTEXT	struct ucontext
#endif /* __DARWIN_UNIX03 */
_STRUCT_UCONTEXT
{
	int			uc_onstack;
	__darwin_sigset_t	uc_sigmask;	/* signal mask used by this context */
	_STRUCT_SIGALTSTACK 	uc_stack;	/* stack used by this context */
	_STRUCT_UCONTEXT	*uc_link;	/* pointer to resuming context */
	__darwin_size_t		uc_mcsize;	/* size of the machine context passed in */
	_STRUCT_MCONTEXT	*uc_mcontext;	/* pointer to machine specific context */
#ifdef _XOPEN_SOURCE
	_STRUCT_MCONTEXT	__mcontext_data;
#endif /* _XOPEN_SOURCE */
};
#endif /* _STRUCT_UCONTEXT */
#endif /* __need_struct_ucontext */

#ifdef __need_struct_ucontext64
#undef __need_struct_ucontext64
#ifndef _STRUCT_UCONTEXT64
#if __DARWIN_UNIX03
#define _STRUCT_UCONTEXT64	struct __darwin_ucontext64
#else /* !__DARWIN_UNIX03 */
#define _STRUCT_UCONTEXT64	struct ucontext64
#endif /* __DARWIN_UNIX03 */
_STRUCT_UCONTEXT64
{
	int			uc_onstack;
	__darwin_sigset_t	uc_sigmask;	/* signal mask used by this context */
	_STRUCT_SIGALTSTACK 	uc_stack;	/* stack used by this context */
	_STRUCT_UCONTEXT64	*uc_link;	/* pointer to resuming context */
	__darwin_size_t		uc_mcsize;	/* size of the machine context passed in */
	_STRUCT_MCONTEXT64	*uc_mcontext64;	/* pointer to machine specific context */
};
#endif /* _STRUCT_UCONTEXT64 */
#endif /* __need_struct_ucontext64 */

#ifdef KERNEL
/* LP64 version of struct timespec.  time_t is a long and must grow when 
 * we're dealing with a 64-bit process.
 * WARNING - keep in sync with struct timespec
 */
#ifdef __need_struct_user_timespec
#undef __need_struct_user_timespec
#ifndef _STRUCT_USER_TIMESPEC
#define _STRUCT_USER_TIMESPEC	struct user_timespec
_STRUCT_USER_TIMESPEC
{
	user_time_t	tv_sec;		/* seconds */
	user_long_t	tv_nsec;	/* and nanoseconds */
};
#endif /* _STRUCT_USER_TIMESPEC */
#endif /* __need_struct_user_timespec */

#ifdef __need_struct_user64_timespec
#undef __need_struct_user64_timespec
#ifndef _STRUCT_USER64_TIMESPEC
#define _STRUCT_USER64_TIMESPEC	struct user64_timespec
_STRUCT_USER64_TIMESPEC
{
	user64_time_t	tv_sec;		/* seconds */
	user64_long_t	tv_nsec;	/* and nanoseconds */
};
#endif /* _STRUCT_USER64_TIMESPEC */
#endif /* __need_struct_user64_timespec */

#ifdef __need_struct_user32_timespec
#undef __need_struct_user32_timespec
#ifndef _STRUCT_USER32_TIMESPEC
#define _STRUCT_USER32_TIMESPEC	struct user32_timespec
_STRUCT_USER32_TIMESPEC
{
	user32_time_t	tv_sec;		/* seconds */
	user32_long_t	tv_nsec;	/* and nanoseconds */
};
#endif /* _STRUCT_USER32_TIMESPEC */
#endif /* __need_struct_user32_timespec */

#ifdef __need_struct_user_timeval
#undef __need_struct_user_timeval
#ifndef _STRUCT_USER_TIMEVAL
#define _STRUCT_USER_TIMEVAL		struct user_timeval
_STRUCT_USER_TIMEVAL
{
	user_time_t		tv_sec;		/* seconds */
	__int32_t		tv_usec;	/* and microseconds */
};
#endif /* _STRUCT_USER_TIMEVAL */
#endif /* __need_struct_user_timeval */

#ifdef __need_struct_user64_timeval
#undef __need_struct_user64_timeval
#ifndef _STRUCT_USER64_TIMEVAL
#define _STRUCT_USER64_TIMEVAL		struct user64_timeval
_STRUCT_USER64_TIMEVAL
{
	user64_time_t		tv_sec;		/* seconds */
	__int32_t			tv_usec;	/* and microseconds */
};
#endif /* _STRUCT_USER64_TIMEVAL */
#endif /* __need_struct_user64_timeval */

#ifdef __need_struct_user32_timeval
#undef __need_struct_user32_timeval
#ifndef _STRUCT_USER32_TIMEVAL
#define _STRUCT_USER32_TIMEVAL		struct user32_timeval
_STRUCT_USER32_TIMEVAL
{
	user32_time_t		tv_sec;		/* seconds */
	__int32_t		tv_usec;	/* and microseconds */
};
#endif /* _STRUCT_USER32_TIMEVAL */
#endif /* __need_struct_user32_timeval */

#ifdef __need_struct_user64_itimerval
#undef __need_struct_user64_itimerval
#ifndef _STRUCT_USER64_ITIMERVAL
#define _STRUCT_USER64_ITIMERVAL		struct user64_itimerval
_STRUCT_USER64_ITIMERVAL
{
	_STRUCT_USER64_TIMEVAL	it_interval;	/* timer interval */
	_STRUCT_USER64_TIMEVAL	it_value;		/* current value */
};
#endif /* _STRUCT_USER64_TIMEVAL */
#endif /* __need_struct_user64_itimerval */

#ifdef __need_struct_user32_itimerval
#undef __need_struct_user32_itimerval
#ifndef _STRUCT_USER32_ITIMERVAL
#define _STRUCT_USER32_ITIMERVAL		struct user32_itimerval
_STRUCT_USER32_ITIMERVAL
{
	_STRUCT_USER32_TIMEVAL	it_interval;	/* timer interval */
	_STRUCT_USER32_TIMEVAL	it_value;		/* current value */
};
#endif /* _STRUCT_USER32_TIMEVAL */
#endif /* __need_struct_user32_itimerval */

#endif	/* KERNEL */

#ifdef __need_fd_set
#undef __need_fd_set
#ifndef _FD_SET
#define _FD_SET
/*
 * Select uses bit masks of file descriptors in longs.  These macros
 * manipulate such bit fields (the filesystem macros use chars).  The
 * extra protection here is to permit application redefinition above
 * the default size.
 */
#ifdef FD_SETSIZE
#define	__DARWIN_FD_SETSIZE	FD_SETSIZE
#else /* !FD_SETSIZE */
#define	__DARWIN_FD_SETSIZE	1024
#endif /* FD_SETSIZE */
#define	__DARWIN_NBBY		8				/* bits in a byte */
#define __DARWIN_NFDBITS	(sizeof(__int32_t) * __DARWIN_NBBY) /* bits per mask */
#define	__DARWIN_howmany(x, y)	((((x) % (y)) == 0) ? ((x) / (y)) : (((x) / (y)) + 1)) /* # y's == x bits? */

__BEGIN_DECLS
typedef	struct fd_set {
	__int32_t	fds_bits[__DARWIN_howmany(__DARWIN_FD_SETSIZE, __DARWIN_NFDBITS)];
} fd_set;
__END_DECLS

/* This inline avoids argument side-effect issues with FD_ISSET() */
static __inline int
__darwin_fd_isset(int _n, const struct fd_set *_p)
{
	return (_p->fds_bits[_n/__DARWIN_NFDBITS] & (1<<(_n % __DARWIN_NFDBITS)));
}

#define	__DARWIN_FD_SET(n, p)	do { int __fd = (n); ((p)->fds_bits[__fd/__DARWIN_NFDBITS] |= (1<<(__fd % __DARWIN_NFDBITS))); } while(0)
#define	__DARWIN_FD_CLR(n, p)	do { int __fd = (n); ((p)->fds_bits[__fd/__DARWIN_NFDBITS] &= ~(1<<(__fd % __DARWIN_NFDBITS))); } while(0)
#define	__DARWIN_FD_ISSET(n, p)	__darwin_fd_isset((n), (p))

#if __GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ >= 3
/*
 * Use the built-in bzero function instead of the library version so that
 * we do not pollute the namespace or introduce prototype warnings.
 */
#define	__DARWIN_FD_ZERO(p)	__builtin_bzero(p, sizeof(*(p)))
#else
#define	__DARWIN_FD_ZERO(p)	bzero(p, sizeof(*(p)))
#endif

#define	__DARWIN_FD_COPY(f, t)	bcopy(f, t, sizeof(*(f)))
#endif /* _FD_SET */
#endif /* __need_fd_set */

#ifdef __need_stack_t
#undef __need_stack_t
#ifndef _STACK_T
#define _STACK_T
typedef _STRUCT_SIGALTSTACK	stack_t; /* [???] signal stack */
#endif /* _STACK_T */
#endif /* __need_stack_t */

#ifdef __need_ucontext_t
#undef __need_ucontext_t
/* user context */
#ifndef _UCONTEXT_T
#define _UCONTEXT_T
typedef _STRUCT_UCONTEXT	ucontext_t;	/* [???] user context */
#endif /* _UCONTEXT_T */
#endif /* __need_ucontext_t */

#ifdef __need_ucontext64_t
#undef __need_ucontext64_t
#ifndef _UCONTEXT64_T
#define _UCONTEXT64_T
typedef _STRUCT_UCONTEXT64	ucontext64_t;	/* [???] user context */
#endif /* _UCONTEXT64_T */
#endif /* __need_ucontext64_t */
