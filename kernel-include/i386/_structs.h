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

#include <sys/appleapiopts.h>

#ifdef __need_mcontext_t
#ifndef __need_struct_mcontext
#define __need_struct_mcontext
#endif /* __need_struct_mcontext */
#endif /* __need_mcontext_t */

#if defined(__need_struct_mcontext)
#include <mach/i386/_structs.h>
#endif /* __need_struct_mcontext */


#ifdef __need_struct_mcontext
#undef __need_struct_mcontext

#ifndef _STRUCT_MCONTEXT32
#if __DARWIN_UNIX03
#define	_STRUCT_MCONTEXT32	struct __darwin_mcontext32
_STRUCT_MCONTEXT32
{
	_STRUCT_X86_EXCEPTION_STATE32	__es;
	_STRUCT_X86_THREAD_STATE32	__ss;
	_STRUCT_X86_FLOAT_STATE32	__fs;
};

#define	_STRUCT_MCONTEXT_AVX32	struct __darwin_mcontext_avx32
_STRUCT_MCONTEXT_AVX32
{
	_STRUCT_X86_EXCEPTION_STATE32	__es;
	_STRUCT_X86_THREAD_STATE32	__ss;
	_STRUCT_X86_AVX_STATE32		__fs;
};

#else /* !__DARWIN_UNIX03 */
#define	_STRUCT_MCONTEXT32	struct mcontext32
_STRUCT_MCONTEXT32
{
	_STRUCT_X86_EXCEPTION_STATE32	es;
	_STRUCT_X86_THREAD_STATE32	ss;
	_STRUCT_X86_FLOAT_STATE32	fs;
};

#define	_STRUCT_MCONTEXT_AVX32	struct mcontext_avx32
_STRUCT_MCONTEXT_AVX32
{
	_STRUCT_X86_EXCEPTION_STATE32	es;
	_STRUCT_X86_THREAD_STATE32	ss;
	_STRUCT_X86_AVX_STATE32		fs;
};

#endif /* __DARWIN_UNIX03 */
#endif /* _STRUCT_MCONTEXT32 */

#ifndef _STRUCT_MCONTEXT64
#if __DARWIN_UNIX03
#define	_STRUCT_MCONTEXT64	struct __darwin_mcontext64
_STRUCT_MCONTEXT64
{
	_STRUCT_X86_EXCEPTION_STATE64	__es;
	_STRUCT_X86_THREAD_STATE64	__ss;
	_STRUCT_X86_FLOAT_STATE64	__fs;
};

#define	_STRUCT_MCONTEXT_AVX64	struct __darwin_mcontext_avx64
_STRUCT_MCONTEXT_AVX64
{
	_STRUCT_X86_EXCEPTION_STATE64	__es;
	_STRUCT_X86_THREAD_STATE64	__ss;
	_STRUCT_X86_AVX_STATE64		__fs;
};

#else /* !__DARWIN_UNIX03 */
#define	_STRUCT_MCONTEXT64	struct mcontext64
_STRUCT_MCONTEXT64
{
	_STRUCT_X86_EXCEPTION_STATE64	es;
	_STRUCT_X86_THREAD_STATE64	ss;
	_STRUCT_X86_FLOAT_STATE64	fs;
};

#define	_STRUCT_MCONTEXT_AVX64	struct mcontext_avx64
_STRUCT_MCONTEXT_AVX64
{
	_STRUCT_X86_EXCEPTION_STATE64	es;
	_STRUCT_X86_THREAD_STATE64	ss;
	_STRUCT_X86_AVX_STATE64		fs;
};

#endif /* __DARWIN_UNIX03 */
#endif /* _STRUCT_MCONTEXT64 */
#endif /* __need_struct_mcontext */

#ifdef __need_mcontext_t
#undef __need_mcontext_t
#ifndef _MCONTEXT_T
#define _MCONTEXT_T
#if defined(__LP64__)
typedef _STRUCT_MCONTEXT64	*mcontext_t;
#define _STRUCT_MCONTEXT _STRUCT_MCONTEXT64
#else
typedef _STRUCT_MCONTEXT32	*mcontext_t;
#define _STRUCT_MCONTEXT	_STRUCT_MCONTEXT32
#endif
#endif /* _MCONTEXT_T */
#endif /* __need_mcontext_t */

#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
#ifndef I386_MCONTEXT_SIZE
#define I386_MCONTEXT_SIZE	sizeof(struct mcontext)	
#endif /* I386_MCONTEXT_SIZE */
#endif /* (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) */

