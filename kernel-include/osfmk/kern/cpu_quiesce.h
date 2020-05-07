/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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

#ifndef _KERN_CPU_QUIESCE_H_
#define _KERN_CPU_QUIESCE_H_

#ifdef XNU_KERNEL_PRIVATE

#include <sys/cdefs.h>

/* State field kept on each CPU to track participation */
typedef enum {
	/* Processor that never participated */
	CPU_QUIESCE_COUNTER_NONE = 0,
	/* Processor ready to participate, waiting for return to user */
	CPU_QUIESCE_COUNTER_PENDING_JOIN = 1,
	/* Processor currently participating in counter */
	CPU_QUIESCE_COUNTER_JOINED = 2,
	/* Processor currently not participating in counter */
	CPU_QUIESCE_COUNTER_LEFT = 3,
} cpu_quiescent_state_t;

#if CONFIG_QUIESCE_COUNTER

extern void cpu_quiescent_counter_join(uint64_t ctime);
extern void cpu_quiescent_counter_leave(uint64_t ctime);
extern void cpu_quiescent_counter_checkin(uint64_t ctime);
extern void cpu_quiescent_counter_ast(void);
extern void cpu_quiescent_counter_init(void);

/* use of these is guarded by the config */
extern void cpu_quiescent_counter_set_min_interval_us(uint32_t new_value);
extern uint32_t cpu_quiescent_counter_get_min_interval_us(void);

#else /* CONFIG_QUIESCE_COUNTER */

/* stub routines for platforms without the counter */

static inline void
cpu_quiescent_counter_join(__unused uint64_t ctime)
{
}
static inline void
cpu_quiescent_counter_leave(__unused uint64_t ctime)
{
}
static inline void
cpu_quiescent_counter_checkin(__unused uint64_t ctime)
{
}
static inline void
cpu_quiescent_counter_ast(void)
{
}
static inline void
cpu_quiescent_counter_init(void)
{
}

#endif /* CONFIG_QUIESCE_COUNTER */

#if MACH_ASSERT && CONFIG_QUIESCE_COUNTER
extern void cpu_quiescent_counter_assert_ast(void);
#else
static inline void
cpu_quiescent_counter_assert_ast(void)
{
}
#endif

#endif /* XNU_KERNEL_PRIVATE */

#endif /* _KERN_CPU_QUIESCE_H_ */
