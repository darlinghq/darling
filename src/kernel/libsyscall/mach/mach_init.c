/*
 * Copyright (c) 1999-2012 Apple Inc. All rights reserved.
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

#include <mach/mach.h>
#include <mach/boolean.h>
#include <mach/machine/ndr_def.h>
#include <mach/mach_traps.h>
#include <mach/mach_host.h>
#include <mach/mach_init.h>
#include <mach/vm_param.h>
#include <machine/cpu_capabilities.h>
#include <stdbool.h>
#include "externs.h"

mach_port_t bootstrap_port = MACH_PORT_NULL;
mach_port_t mach_task_self_ = MACH_PORT_NULL;
#ifdef __i386__
mach_port_t mach_host_self_ = MACH_PORT_NULL;
#endif
extern mach_port_t _task_reply_port;

vm_size_t vm_kernel_page_size = 0;
vm_size_t vm_kernel_page_mask = 0;
int vm_kernel_page_shift = 0;

vm_size_t vm_page_size = 0;
vm_size_t vm_page_mask = 0;
int vm_page_shift = 0;

int mach_init(void);
int _mach_fork_child(void);

static void mach_init_doit(void);

extern void _pthread_set_self(void *);
extern void _init_cpu_capabilities(void);

kern_return_t
host_page_size(__unused host_t host, vm_size_t *out_page_size)
{
	*out_page_size = vm_kernel_page_size;
	return KERN_SUCCESS;
}

/*
 * mach_init() must be called explicitly in static executables (including dyld).
 * called by libSystem_initializer() in dynamic executables
 */
int
mach_init(void)
{
	static bool mach_init_inited = false;
	if (!mach_init_inited) {
		mach_init_doit();
		mach_init_inited = true;
	}
	return 0;
}

// called by libSystem_atfork_child()
int
_mach_fork_child(void)
{
	mach_init_doit();
	return 0;
}

#if defined(__arm__) || defined(__arm64__)
#if !defined(_COMM_PAGE_USER_PAGE_SHIFT_64) && defined(_COMM_PAGE_UNUSED0)
#define _COMM_PAGE_USER_PAGE_SHIFT_32 (_COMM_PAGE_UNUSED0)
#define _COMM_PAGE_USER_PAGE_SHIFT_64 (_COMM_PAGE_UNUSED0+1)
#endif
#endif

void
mach_init_doit(void)
{
	// Initialize cached mach ports defined in mach_init.h
	mach_task_self_ = task_self_trap();
	_task_reply_port = mach_reply_port();

	if (vm_kernel_page_shift == 0) {
#ifdef  _COMM_PAGE_KERNEL_PAGE_SHIFT
		vm_kernel_page_shift = *(uint8_t*) _COMM_PAGE_KERNEL_PAGE_SHIFT;
		vm_kernel_page_size = 1 << vm_kernel_page_shift;
		vm_kernel_page_mask = vm_kernel_page_size - 1;
#else
		vm_kernel_page_size = PAGE_SIZE;
		vm_kernel_page_mask = PAGE_MASK;
		vm_kernel_page_shift = PAGE_SHIFT;
#endif /* _COMM_PAGE_KERNEL_PAGE_SHIFT */
	}

	if (vm_page_shift == 0) {
#if defined(__arm64__)
		vm_page_shift = *(uint8_t*) _COMM_PAGE_USER_PAGE_SHIFT_64;
#elif defined(__arm__)
		vm_page_shift = *(uint8_t*) _COMM_PAGE_USER_PAGE_SHIFT_32;
#else
		vm_page_shift = vm_kernel_page_shift;
#endif
		vm_page_size = 1 << vm_page_shift;
		vm_page_mask = vm_page_size - 1;
	}

	_init_cpu_capabilities();
	_pthread_set_self(0);
}
