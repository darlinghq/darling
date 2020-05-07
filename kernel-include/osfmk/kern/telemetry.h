/*
 * Copyright (c) 2012-2013 Apple Inc. All rights reserved.
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

#ifndef _KERNEL_TELEMETRY_H_
#define _KERNEL_TELEMETRY_H_

#include <stdint.h>
#include <sys/cdefs.h>
#include <mach/mach_types.h>
#include <kern/thread.h>

__BEGIN_DECLS

#define TELEMETRY_CMD_TIMER_EVENT 1
#define TELEMETRY_CMD_VOUCHER_NAME 2
#define TELEMETRY_CMD_VOUCHER_STAIN TELEMETRY_CMD_VOUCHER_NAME

enum telemetry_pmi {
	TELEMETRY_PMI_NONE,
	TELEMETRY_PMI_INSTRS,
	TELEMETRY_PMI_CYCLES,
};
#define TELEMETRY_CMD_PMI_SETUP 3

#if XNU_KERNEL_PRIVATE

extern volatile boolean_t telemetry_needs_record;

extern void telemetry_init(void);

extern void compute_telemetry(void *);

extern void telemetry_ast(thread_t thread, uint32_t reasons);

extern int telemetry_gather(user_addr_t buffer, uint32_t *length, boolean_t mark);

extern void telemetry_mark_curthread(boolean_t interrupted_userspace,
    boolean_t pmi);

extern void telemetry_task_ctl(task_t task, uint32_t reason, int enable_disable);
extern void telemetry_task_ctl_locked(task_t task, uint32_t reason, int enable_disable);
extern void telemetry_global_ctl(int enable_disable);

extern int telemetry_timer_event(uint64_t deadline, uint64_t interval, uint64_t leeway);
extern int telemetry_pmi_setup(enum telemetry_pmi pmi_type, uint64_t interval);

extern void bootprofile_init(void);
extern void bootprofile_wake_from_sleep(void);
extern void bootprofile_get(void **buffer, uint32_t *length);
extern int bootprofile_gather(user_addr_t buffer, uint32_t *length);

#endif /* XNU_KERNEL_PRIVATE */

__END_DECLS

#endif /* _KERNEL_TELEMETRY_H_ */
