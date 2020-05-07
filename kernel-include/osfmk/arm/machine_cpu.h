/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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
#ifndef _ARM_MACHINE_CPU_H_
#define _ARM_MACHINE_CPU_H_

#include <mach/mach_types.h>
#include <mach/boolean.h>
#include <kern/kern_types.h>
#include <pexpert/pexpert.h>
#include <arm/cpu_data_internal.h>

extern void cpu_machine_init(void);

extern kern_return_t cpu_register(int *slot_nump);

extern void cpu_signal_handler(void);
extern void cpu_signal_handler_internal(boolean_t disable_signal);

extern void cpu_doshutdown(void (*doshutdown)(processor_t), processor_t processor);

extern void cpu_idle(void);
extern void cpu_idle_exit(boolean_t from_reset) __attribute__((noreturn));
extern void cpu_idle_tickle(void);

extern void cpu_machine_idle_init(boolean_t from_boot);

extern void arm_init_cpu(cpu_data_t *args);

extern void arm_init_idle_cpu(cpu_data_t *args);

extern void init_cpu_timebase(boolean_t enable_fiq);

#define cpu_pause() do {} while (0)     /* Not for this architecture */

#endif /* _ARM_MACHINE_CPU_H_ */
