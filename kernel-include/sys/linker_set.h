/*
 * Copyright (c) 2006-2008 Apple Inc. All rights reserved.
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
 *
 *
 * Copyright (c) 1999 John D. Polstra
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef _SYS_LINKER_SET_H_
#define _SYS_LINKER_SET_H_

#include <sys/appleapiopts.h>
#if !defined(KERNEL) || defined(__APPLE_API_PRIVATE)

/*
 * The following macros are used to declare global sets of objects, which
 * are collected by the linker into a `linker set' as defined below.
 * For Mach-O, this is done by constructing a separate segment inside the
 * __DATA section for each set.  The contents of this segment are an array
 * of pointers to the objects in the set.
 * 
 * Note that due to limitations of the Mach-O format, there cannot
 * be more than 255 sections in a segment, so linker set usage should be
 * conserved.  Set names may not exceed 16 characters.
 */

#ifdef KERNEL
# include <mach-o/loader.h>
# include <libkern/kernel_mach_header.h>
#else
# include <mach-o/ldsyms.h>
# include <mach-o/getsect.h>
# include <mach-o/loader.h>
#endif


/*
 * Private macros, not to be used outside this header file.
 *
 * The objective of this macro stack is to produce the following output,
 * given SET and SYM as arguments:
 *
 *  void const * __set_SET_sym_SYM __attribute__((section("__DATA,SET"))) = & SYM
 */
#ifdef __LS_VA_STRINGIFY__
#  undef __LS_VA_STRINGIFY__
#endif
#ifdef __LS_VA_STRCONCAT__
#  undef __LS_VA_STRCONCAT__
#endif
#define __LS_VA_STRINGIFY(_x...)	#_x
#define __LS_VA_STRCONCAT(_x,_y)	__LS_VA_STRINGIFY(_x,_y)
#define __LINKER_MAKE_SET(_set, _sym)					\
	/*__unused*/ /*static*/ void const * /*const*/ __set_##_set##_sym_##_sym		\
	__attribute__ ((section(__LS_VA_STRCONCAT(__DATA,_set)))) = (void *)&_sym
/* the line above is very fragile - if your compiler breaks linker sets,
   just play around with "static", "const" etc. :-) */

/*
 * Public macros.
 */
#define LINKER_SET_ENTRY(_set, _sym)	__LINKER_MAKE_SET(_set, _sym)

/*
 * FreeBSD compatibility.
 */
#ifdef __APPLE_API_OBSOLETE
# define TEXT_SET(_set, _sym)	__LINKER_MAKE_SET(_set, _sym)
# define DATA_SET(_set, _sym)	__LINKER_MAKE_SET(_set, _sym)
# define BSS_SET(_set, _sym)	__LINKER_MAKE_SET(_set, _sym)
# define ABS_SET(_set, _sym)	__LINKER_MAKE_SET(_set, _sym)
# define SET_ENTRY(_set, _sym)	__LINKER_MAKE_SET(_set, _sym)
#endif /* __APPLE_API_OBSOLETE */

/*
 * Extended linker set API.
 *
 * Since linker sets are per-object-file, and we may have multiple
 * object files, we need to be able to specify which object's set
 * to scan.
 *
 * The set itself is a contiguous array of pointers to the objects
 * within the set.
 */

/*
 * Public interface.
 *
 * void **LINKER_SET_OBJECT_BEGIN(_object, _set)
 *	Preferred interface to linker_set_object_begin(), takes set name unquoted.
 * void **LINKER_SET_OBJECT_LIMIT(_object, _set)
 *	Preferred interface to linker_set_object_begin(), takes set name unquoted.
 * LINKER_SET_OBJECT_FOREACH(_object, (set_member_type **)_pvar, _cast, _set)
 *	Iterates over the members of _set within _object.  Since the set contains
 *	pointers to its elements, for a set of elements of type etyp, _pvar must
 *	be (etyp **).
 * set_member_type **LINKER_SET_OBJECT_ITEM(_object, _set, _i)
 *	Returns a pointer to the _i'th element of _set within _object.
 *
 * void **LINKER_SET_BEGIN(_set)
 * void **LINKER_SET_LIMINT(_set)
 * LINKER_SET_FOREACH((set_member_type **)_pvar, _cast, _set)
 * set_member_type **LINKER_SET_ITEM(_set, _i)
 *	These versions implicitly reference the kernel/application object.
 * 
 * Example of _cast: For the _pvar "struct sysctl_oid **oidpp", _cast would be
 * 	 	     "struct sysctl_oid **"
 *
 */

#define LINKER_SET_OBJECT_BEGIN(_object, _set)	__linker_set_object_begin(_object, _set)
#define LINKER_SET_OBJECT_LIMIT(_object, _set)	__linker_set_object_limit(_object, _set)

#define LINKER_SET_OBJECT_FOREACH(_object, _pvar, _cast, _set)		\
	for (_pvar = (_cast) LINKER_SET_OBJECT_BEGIN(_object, _set);	\
     	     _pvar < (_cast) LINKER_SET_OBJECT_LIMIT(_object, _set);	\
	     _pvar++)

#define LINKER_SET_OBJECT_ITEM(_object, _set, _i)			\
	((LINKER_SET_OBJECT_BEGIN(_object, _set))[_i])

#define LINKER_SET_BEGIN(_set)						\
		LINKER_SET_OBJECT_BEGIN((kernel_mach_header_t *)&_mh_execute_header, _set)
#define LINKER_SET_LIMIT(_set)						\
		LINKER_SET_OBJECT_LIMIT((kernel_mach_header_t *)&_mh_execute_header, _set)
#define LINKER_SET_FOREACH(_pvar, _cast, _set)					\
	LINKER_SET_OBJECT_FOREACH((kernel_mach_header_t *)&_mh_execute_header, _pvar, _cast, _set)
#define LINKER_SET_ITEM(_set, _i)					\
      	LINKER_SET_OBJECT_ITEM((kernel_mach_header_t *)&_mh_execute_header, _set, _i)

/*
 * Implementation.
 *
 * void **__linker_set_object_begin(_header, _set)
 *	Returns a pointer to the first pointer in the linker set.
 * void **__linker_set_object_limi(_header, _set)
 *	Returns an upper bound to the linker set (base + size).
 */

static __inline void **
__linker_set_object_begin(kernel_mach_header_t *_header, const char *_set)
     __attribute__((__const__));
static __inline void **
__linker_set_object_begin(kernel_mach_header_t *_header, const char *_set)
{
	void *_set_begin;
	unsigned long _size;

	_set_begin = getsectdatafromheader(_header, "__DATA", _set, &_size);
	return( (void **) _set_begin );
}

static __inline void **
__linker_set_object_limit(kernel_mach_header_t *_header, const char *_set)
     __attribute__((__const__));
static __inline void **
__linker_set_object_limit(kernel_mach_header_t *_header, const char *_set)
{
	void *_set_begin;
	unsigned long _size;

	_set_begin = getsectdatafromheader(_header, "__DATA", _set, &_size);
	
	return ((void **) ((uintptr_t) _set_begin + _size));
}

#endif /* !KERNEL || __APPLE_API_PRIVATE */

#endif /* _SYS_LINKER_SET_H_ */


