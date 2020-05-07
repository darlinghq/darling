/*
 * Copyright (c) 2000-2016 Apple Inc. All rights reserved.
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
 * @OSF_COPYRIGHT@
 */
/*
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 */

#ifndef _KERN_ASSERT_H_
#define _KERN_ASSERT_H_

/*	assert.h	4.2	85/01/21	*/

#include <kern/macro_help.h>
#include <sys/cdefs.h>

#ifdef  MACH_KERNEL_PRIVATE
#include <mach_assert.h>
#endif

__BEGIN_DECLS
/* Assert error */
#if !CONFIG_NONFATAL_ASSERTS
__abortlike
#endif
extern void     Assert(
	const char      *file,
	int             line,
	const char      *expression) __attribute__((noinline));

extern int kext_assertions_enable;

#define __Panic(fmt, args...) panic(fmt, ##args)

__END_DECLS

#ifndef APPLE_KEXT_ASSERTIONS
#define APPLE_KEXT_ASSERTIONS   0
#endif

#if     MACH_ASSERT

#define assert(ex)  \
	(__builtin_expect(!!((ex)), 1L) ? (void)0 : Assert(__FILE__, __LINE__, # ex))
#define assertf(ex, fmt, args...) \
	(__builtin_expect(!!((ex)), 1L) ? (void)0 : __Panic("%s:%d Assertion failed: %s : " fmt, __FILE__, __LINE__, # ex, ##args))
#define __assert_only

#elif APPLE_KEXT_ASSERTIONS && !XNU_KERNEL_PRIVATE      /* MACH_ASSERT */

#define assert(ex)  \
	(__builtin_expect(!!(((!kext_assertions_enable) || (ex))), 1L) ? (void)0 : Assert(__FILE__, __LINE__, # ex))
#define assertf(ex, fmt, args...) \
	(__builtin_expect(!!(((!kext_assertions_enable) || (ex))), 1L) ? (void)0 : __Panic("%s:%d Assertion failed: %s : " fmt, __FILE__, __LINE__, # ex, ##args))
#define __assert_only

#else                           /* APPLE_KEXT_ASSERTIONS && !XNU_KERNEL_PRIVATE */

#define assert(ex) ((void)0)
#define assertf(ex, fmt, args...) ((void)0)
#define __assert_only __unused

#endif  /* MACH_ASSERT */

/*
 * static_assert is a C11 / C++0x / C++1z feature.
 *
 * Beginning with C++0x, it is a keyword and should not be #defined
 *
 * static_assert is not disabled by MACH_ASSERT or NDEBUG
 */

#ifndef __cplusplus
	#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
		#define _STATIC_ASSERT_OVERLOADED_MACRO(_1, _2, NAME, ...) NAME
		#define static_assert(...) _STATIC_ASSERT_OVERLOADED_MACRO(__VA_ARGS__, _static_assert_2_args, _static_assert_1_arg)(__VA_ARGS__)

		#define _static_assert_2_args(ex, str) _Static_assert((ex), str)
		#define _static_assert_1_arg(ex) _Static_assert((ex), #ex)
	#endif
#else
	#if !defined(__cpp_static_assert)
/* pre C++11 support */
		#define _STATIC_ASSERT_OVERLOADED_MACRO(_1, _2, NAME, ...) NAME
		#define static_assert(...) _STATIC_ASSERT_OVERLOADED_MACRO(__VA_ARGS__, _static_assert_2_args, _static_assert_1_arg)(__VA_ARGS__)

		#define _static_assert_2_args(ex, str) _Static_assert((ex), str)
		#define _static_assert_1_arg(ex) _Static_assert((ex), #ex)
	#else
/*
 * C++11 only supports the 2 argument version of static_assert.
 * C++1z has added support for the 1 argument version.
 */
		#define _static_assert_1_arg(ex) static_assert((ex), #ex)
	#endif
#endif

#endif  /* _KERN_ASSERT_H_ */
