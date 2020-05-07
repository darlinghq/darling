/*
 * Copyright (c) 2000-2019 Apple Inc. All rights reserved.
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
 * Copyright (c) 1991,1990,1989,1988 Carnegie Mellon University
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
 *	kern/host.h
 *
 *	Definitions for host data structures.
 *
 */

#ifndef _KERN_HOST_H_
#define _KERN_HOST_H_

#include <mach/mach_types.h>
#include <sys/cdefs.h>

#ifdef  MACH_KERNEL_PRIVATE

#include <kern/locks.h>
#include <kern/exception.h>
#include <mach/exception_types.h>
#include <mach/host_special_ports.h>
#include <kern/kern_types.h>
#include <mach/vm_statistics.h>

struct  host {
	decl_lck_mtx_data(, lock);               /* lock to protect exceptions */
	ipc_port_t special[HOST_MAX_SPECIAL_PORT + 1];
	struct exception_action exc_actions[EXC_TYPES_COUNT];
};

typedef struct host     host_data_t;

extern host_data_t      realhost;

#define host_lock(host)         lck_mtx_lock(&(host)->lock)
#define host_unlock(host)       lck_mtx_unlock(&(host)->lock)

extern vm_extmod_statistics_data_t host_extmod_statistics;

typedef struct {
	uint64_t total_user_time;
	uint64_t total_system_time;
	uint64_t task_interrupt_wakeups;
	uint64_t task_platform_idle_wakeups;
	uint64_t task_timer_wakeups_bin_1;
	uint64_t task_timer_wakeups_bin_2;
	uint64_t total_ptime;
	uint64_t total_pset_switches;
	uint64_t task_gpu_ns;
	uint64_t task_energy;
} expired_task_statistics_t;

extern expired_task_statistics_t dead_task_statistics;

#endif  /* MACH_KERNEL_PRIVATE */

/*
 * Access routines for inside the kernel.
 */

__BEGIN_DECLS

extern host_t                   host_self(void);
extern host_priv_t              host_priv_self(void);
extern host_security_t  host_security_self(void);

__END_DECLS

#endif  /* _KERN_HOST_H_ */
