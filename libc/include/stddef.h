/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/*	$OpenBSD: stddef.h,v 1.2 1997/09/21 10:45:52 niklas Exp $	*/
/*	$NetBSD: stddef.h,v 1.4 1994/10/26 00:56:26 cgd Exp $	*/

/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
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
 *	@(#)stddef.h	5.5 (Berkeley) 4/3/91
 */

#if !defined(__STDDEF_H__)

#if !defined(__need_wchar_t) && !defined(__need_size_t) \
    && !defined(__need_ptrdiff_t) && !defined(__need_NULL) \
    && !defined(__need_wint_t)
#define __STDDEF_H__
#endif /* none of __need_* defined */

#include <_types.h>

#if defined(__STDDEF_H__) || defined(__need_ptrdiff_t)
#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef	__darwin_ptrdiff_t	ptrdiff_t;
#endif /* _PTRDIFF_T */
#endif /* __STDDEF_H__ || __need_ptrdiff_t */

#if defined(__STDDEF_H__) || defined(__need_size_t)
#ifndef	_SIZE_T
#define	_SIZE_T
/* DO NOT REMOVE THIS COMMENT: fixincludes needs to see:
 * _GCC_SIZE_T */
typedef	__darwin_size_t		size_t;
#endif /* _SIZE_T */
#endif /* __STDDEF_H__ || __need_size_t */

#if defined(__STDDEF_H__) || defined(__need_wchar_t)
#ifndef	__cplusplus
#ifndef	_WCHAR_T
#define	_WCHAR_T
typedef	__darwin_wchar_t	wchar_t;
#endif	/* _WCHAR_T */
#endif	/* __cplusplus */
#endif /* __STDDEF_H__ || __need_wchar_t */

#if (defined(__STDDEF_H__) && !defined(_ANSI_SOURCE) && (!defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE))) \
    || defined(__need_wint_t)
#ifndef	_WINT_T
#define	_WINT_T
typedef	__darwin_wint_t		wint_t;
#endif	/* _WINT_T */
#endif /* __STDDEF_H__ && !_ANSI_SOURCE && (!_POSIX_C_SOURCE || _DARWIN_C_SOURCE) || __need_wchar_t */

#if defined(__STDDEF_H__) || defined(__need_NULL)
#ifndef NULL
#define NULL __DARWIN_NULL
#endif /* ! NULL */
#endif /* __STDDEF_H__ || __need_NULL */

#ifdef __STDDEF_H__
#if defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 5 || __GNUC__ > 3)
#ifndef __offsetof	/* Deprecated: for source compatibility only */
#define __offsetof(type, field) __builtin_offsetof(type, field)
#endif
#define offsetof(type, field) __builtin_offsetof(type, field)
#else /* ! (gcc >= 3.5) */
#ifndef __offsetof	/* Deprecated: for source compatibility only */
#define __offsetof(type, field) ((size_t)(&((type *)0)->field))
#endif
#define offsetof(type, field) ((size_t)(&((type *)0)->field))
#endif /* (gcc >= 3.5) */
#endif /* __STDDEF_H__ */

#endif /* __STDDEF_H__ */

#undef __need_ptrdiff_t
#undef __need_size_t
#undef __need_wchar_t
#undef __need_wint_t
#undef __need_NULL
