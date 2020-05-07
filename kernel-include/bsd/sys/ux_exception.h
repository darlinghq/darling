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
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

/*
 *	Codes for Unix software exceptions under EXC_SOFTWARE.
 */

#ifndef _SYS_UX_EXCEPTION_H_
#define _SYS_UX_EXCEPTION_H_

#include <sys/appleapiopts.h>

#ifdef __APPLE_API_UNSTABLE

#define EXC_UNIX_BAD_SYSCALL    0x10000         /* SIGSYS */

#define EXC_UNIX_BAD_PIPE       0x10001         /* SIGPIPE */

#define EXC_UNIX_ABORT          0x10002         /* SIGABRT */

#endif /* __APPLE_API_UNSTABLE */

#ifdef XNU_KERNEL_PRIVATE

/* Kernel functions for Unix exception handler. */

#include <mach/mach_types.h>

extern int
machine_exception(int exception, mach_exception_code_t code,
    mach_exception_subcode_t subcode);

extern kern_return_t
handle_ux_exception(thread_t thread, int exception,
    mach_exception_code_t code,
    mach_exception_subcode_t subcode);

#endif /* XNU_KERNEL_PRIVATE */

#endif  /* _SYS_UX_EXCEPTION_H_ */
