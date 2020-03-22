/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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

#ifdef __need_mcontext_t
#ifndef __need_struct_mcontext
#define __need_struct_mcontext
#endif /* __need_struct_mcontext */
#endif /* __need_mcontext_t */

#ifdef __need_mcontext64_t
#ifndef __need_struct_mcontext64
#define __need_struct_mcontext64
#endif /* __need_struct_mcontext64 */
#endif /* __need_mcontext64_t */

#if defined(__need_struct_mcontext) || defined(__need_struct_mcontext64)
#include <mach/ppc/_structs.h>
#endif /* __need_struct_mcontext || __need_struct_mcontext64 */

#ifdef __need_struct_mcontext
#undef __need_struct_mcontext
#ifndef _STRUCT_MCONTEXT
#if __DARWIN_UNIX03
#define	_STRUCT_MCONTEXT	struct __darwin_mcontext
_STRUCT_MCONTEXT
{
	_STRUCT_PPC_EXCEPTION_STATE	__es;
	_STRUCT_PPC_THREAD_STATE	__ss;
	_STRUCT_PPC_FLOAT_STATE		__fs;
	_STRUCT_PPC_VECTOR_STATE	__vs;
};
#else /* !__DARWIN_UNIX03 */
#define	_STRUCT_MCONTEXT	struct mcontext
_STRUCT_MCONTEXT
{
	_STRUCT_PPC_EXCEPTION_STATE	es;
	_STRUCT_PPC_THREAD_STATE	ss;
	_STRUCT_PPC_FLOAT_STATE		fs;
	_STRUCT_PPC_VECTOR_STATE	vs;
};
#endif /* __DARWIN_UNIX03 */
#endif /* _STRUCT_MCONTEXT */
#endif /* __need_struct_mcontext */

#ifdef __need_struct_mcontext64
#undef __need_struct_mcontext64
#ifndef _STRUCT_MCONTEXT64
#if __DARWIN_UNIX03
#define _STRUCT_MCONTEXT64	struct __darwin_mcontext64
_STRUCT_MCONTEXT64
{
	_STRUCT_PPC_EXCEPTION_STATE64	__es;
	_STRUCT_PPC_THREAD_STATE64	__ss;
	_STRUCT_PPC_FLOAT_STATE		__fs;
	_STRUCT_PPC_VECTOR_STATE	__vs;
};
#else /* !__DARWIN_UNIX03 */
#define _STRUCT_MCONTEXT64	struct mcontext64
_STRUCT_MCONTEXT64
{
	_STRUCT_PPC_EXCEPTION_STATE64	es;
	_STRUCT_PPC_THREAD_STATE64	ss;
	_STRUCT_PPC_FLOAT_STATE		fs;
	_STRUCT_PPC_VECTOR_STATE	vs;
};
#endif /* __DARWIN_UNIX03 */
#endif /* _STRUCT_MCONTEXT64 */
#endif /* __need_struct_mcontext64 */

#ifdef __need_mcontext_t
#undef __need_mcontext_t
#ifndef _MCONTEXT_T
#define _MCONTEXT_T
typedef _STRUCT_MCONTEXT	*mcontext_t;
#endif /* _MCONTEXT_T */
#endif /* __need_mcontext_t */

#ifdef __need_mcontext64_t
#undef __need_mcontext64_t
#ifndef _MCONTEXT64_T
#define _MCONTEXT64_T
typedef _STRUCT_MCONTEXT64	*mcontext64_t;
#endif /* _MCONTEXT64_T */
#endif /* __need_mcontext64_t */

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
#ifndef PPC_MCONTEXT_SIZE
#define PPC_MCONTEXT_SIZE	(PPC_THREAD_STATE_COUNT + PPC_FLOAT_STATE_COUNT + PPC_EXCEPTION_STATE_COUNT + PPC_VECTOR_STATE_COUNT) * sizeof(int)
#endif /* PPC_MCONTEXT_SIZE */
#ifndef PPC_MCONTEXT64_SIZE
#define PPC_MCONTEXT64_SIZE	(PPC_THREAD_STATE64_COUNT + PPC_FLOAT_STATE_COUNT + PPC_EXCEPTION_STATE_COUNT + PPC_VECTOR_STATE_COUNT) * sizeof(int)
#endif /* PPC_MCONTEXT64_SIZE */
#endif /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */

/*
 * For now, just duplicate the 32-bit context as the generic one.
 */
#ifdef __need_struct_sigcontext
#undef __need_struct_sigcontext
#ifndef _STRUCT_SIGCONTEXT
#if __DARWIN_UNIX03		/* signal.h needs struct sigcontext visible */
#define _STRUCT_SIGCONTEXT	struct __darwin_sigcontext
_STRUCT_SIGCONTEXT
{
    int		__sc_onstack;	/* sigstack state to restore */
    int		__sc_mask;	/* signal mask to restore */
    int		__sc_ir;	/* pc */
    int		__sc_psw;	/* processor status word */
    int		__sc_sp;	/* stack pointer if sc_regs == NULL */
    void	*__sc_regs;	/* (kernel private) saved state */
};
#else /* !__DARWIN_UNIX03 */
#define _STRUCT_SIGCONTEXT	struct sigcontext
_STRUCT_SIGCONTEXT
{
    int		sc_onstack;	/* sigstack state to restore */
    int		sc_mask;	/* signal mask to restore */
    int		sc_ir;		/* pc */
    int		sc_psw;		/* processor status word */
    int		sc_sp;		/* stack pointer if sc_regs == NULL */
    void	*sc_regs;	/* (kernel private) saved state */
};
#endif /* __DARWIN_UNIX03 */
#endif /* _STRUCT_SIGCONTEXT */
#endif /* __need_struct_sigcontext */

/*
 * Information pushed on stack when a signal is delivered.
 * This is used by the kernel to restore state following
 * execution of the signal handler.  It is also made available
 * to the handler to allow it to properly restore state if
 * a non-standard exit is performed.
 */
#ifdef __need_struct_sigcontext32
#undef __need_struct_sigcontext32
#ifndef _STRUCT_SIGCONTEXT32
#if __DARWIN_UNIX03
#define _STRUCT_SIGCONTEXT32	struct __darwin_sigcontext32
_STRUCT_SIGCONTEXT32
{
    int		__sc_onstack;	/* sigstack state to restore */
    int		__sc_mask;	/* signal mask to restore */
    int		__sc_ir;	/* pc */
    int		__sc_psw;	/* processor status word */
    int		__sc_sp;	/* stack pointer if sc_regs == NULL */
    void	*__sc_regs;	/* (kernel private) saved state */
};
#else /* !__DARWIN_UNIX03 */
#define _STRUCT_SIGCONTEXT32	struct sigcontext32
_STRUCT_SIGCONTEXT32
{
    int		sc_onstack;	/* sigstack state to restore */
    int		sc_mask;	/* signal mask to restore */
    int		sc_ir;		/* pc */
    int		sc_psw;		/* processor status word */
    int		sc_sp;		/* stack pointer if sc_regs == NULL */
    void	*sc_regs;	/* (kernel private) saved state */
};
#endif /* __DARWIN_UNIX03 */
#endif /* _STRUCT_SIGCONTEXT32 */
#endif /* __need_struct_sigcontext32 */

#ifdef __need_struct_sigcontext64
#undef __need_struct_sigcontext64
#ifndef _STRUCT_SIGCONTEXT64
#if __DARWIN_UNIX03
#define _STRUCT_SIGCONTEXT64	struct __darwin_sigcontext64
_STRUCT_SIGCONTEXT64
{
    int		__sc_onstack;	/* sigstack state to restore */
    int		__sc_mask;	/* signal mask to restore */
    long long	__sc_ir;	/* pc */
    long long	__sc_psw;	/* processor status word */
    long long	__sc_sp;	/* stack pointer if sc_regs == NULL */
    void	*__sc_regs;	/* (kernel private) saved state */
};
#else /* !__DARWIN_UNIX03 */
#define _STRUCT_SIGCONTEXT64	struct sigcontext64
_STRUCT_SIGCONTEXT64
{
    int		sc_onstack;	/* sigstack state to restore */
    int		sc_mask;	/* signal mask to restore */
    long long	sc_ir;		/* pc */
    long long	sc_psw;		/* processor status word */
    long long	sc_sp;		/* stack pointer if sc_regs == NULL */
    void	*sc_regs;	/* (kernel private) saved state */
};
#endif /* __DARWIN_UNIX03 */
#endif /* _STRUCT_SIGCONTEXT64 */
#endif /* __need_struct_sigcontext64 */
