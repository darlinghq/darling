/*
 * Copyright (c) 2004-2014 Apple Inc. All rights reserved.
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

#ifndef _SYS_SYSENT_H_
#define _SYS_SYSENT_H_

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>

#ifdef KERNEL_PRIVATE
#ifdef __APPLE_API_PRIVATE

typedef int32_t sy_call_t(struct proc *, void *, int *);
#if CONFIG_REQUIRES_U32_MUNGING
typedef void    sy_munge_t(void *);
#elif __arm__ && (__BIGGEST_ALIGNMENT__ > 4)
typedef int     sy_munge_t(const void *, void *);
#endif

struct sysent {         /* system call table */
	sy_call_t       *sy_call;       /* implementing function */
#if CONFIG_REQUIRES_U32_MUNGING || (__arm__ && (__BIGGEST_ALIGNMENT__ > 4))
	sy_munge_t      *sy_arg_munge32; /* system call arguments munger for 32-bit process */
#endif
	int32_t         sy_return_type; /* system call return types */
	int16_t         sy_narg;        /* number of args */
	uint16_t        sy_arg_bytes;   /* Total size of arguments in bytes for
	                                 * 32-bit system calls
	                                 */
};

#ifndef __INIT_SYSENT_C__
extern struct sysent sysent[];
#endif  /* __INIT_SYSENT_C__ */

extern const unsigned int nsysent;

/*
 * Valid values for sy_cancel
 */
#define _SYSCALL_CANCEL_NONE    0               /* Not a cancellation point */
#define _SYSCALL_CANCEL_PRE             1               /* Canbe cancelled on entry itself */
#define _SYSCALL_CANCEL_POST    2               /* Can only be cancelled after syscall is run */

/*
 * Valid values for sy_return_type
 */
#define _SYSCALL_RET_NONE               0
#define _SYSCALL_RET_INT_T              1
#define _SYSCALL_RET_UINT_T             2
#define _SYSCALL_RET_OFF_T              3
#define _SYSCALL_RET_ADDR_T             4
#define _SYSCALL_RET_SIZE_T             5
#define _SYSCALL_RET_SSIZE_T    6
#define _SYSCALL_RET_UINT64_T   7

#endif /* __APPLE_API_PRIVATE */
#endif /* KERNEL_PRIVATE */

#endif /* !_SYS_SYSENT_H_ */
