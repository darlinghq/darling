/*
 * Copyright (c) 2002-2006 Apple Computer, Inc. All rights reserved.
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

#ifndef _SYS_UCONTEXT_H_
#define _SYS_UCONTEXT_H_

#include <sys/cdefs.h>
#include <sys/_types.h>

#include <machine/_mcontext.h>
#include <sys/_types/_sigaltstack.h>
#include <sys/_types/_ucontext.h>

#include <sys/_types/_sigset_t.h>

#ifdef KERNEL
#include <machine/types.h>      /* user_addr_t, user_size_t */

/* kernel representation of struct ucontext64 for 64 bit processes */
typedef struct user_ucontext64 {
	int                             uc_onstack;
	sigset_t                        uc_sigmask;     /* signal mask */
	struct user64_sigaltstack       uc_stack;       /* stack */
	user_addr_t                     uc_link;        /* ucontext pointer */
	user_size_t                     uc_mcsize;      /* mcontext size */
	user_addr_t                     uc_mcontext64;  /* machine context */
} user_ucontext64_t;

typedef struct user_ucontext32 {
	int                             uc_onstack;
	sigset_t                        uc_sigmask;     /* signal mask */
	struct user32_sigaltstack       uc_stack;       /* stack */
	user32_addr_t                   uc_link;        /* ucontext pointer */
	user32_size_t                   uc_mcsize;      /* mcontext size */
	user32_addr_t                   uc_mcontext;    /* machine context */
} user_ucontext32_t;

#endif  /* KERNEL */

#endif /* _SYS_UCONTEXT_H_ */
