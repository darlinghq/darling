/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

#ifndef _KERN_HV_SUPPORT_H_
#define _KERN_HV_SUPPORT_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <mach/kern_return.h>

typedef enum {
	HV_DEBUG_STATE
} hv_volatile_state_t;

typedef enum {
	HV_TASK_TRAP = 0,
	HV_THREAD_TRAP = 1
} hv_trap_type_t;

typedef kern_return_t (*hv_trap_t) (void *target, uint64_t arg);

typedef struct  {
	const hv_trap_t *traps;
	unsigned trap_count;
} hv_trap_table_t;

typedef struct {
	void (*dispatch)(void *vcpu);
	void (*preempt)(void *vcpu);
	void (*suspend)(void);
	void (*thread_destroy)(void *vcpu);
	void (*task_destroy)(void *vm);
	void (*volatile_state)(void *vcpu, int state);
	void (*memory_pressure)(void);
} hv_callbacks_t;

extern hv_callbacks_t hv_callbacks;
extern int hv_support_available;

extern void hv_support_init(void);
extern int hv_get_support(void);
extern void hv_set_task_target(void *target);
extern void hv_set_thread_target(void *target);
extern void *hv_get_task_target(void);
extern void *hv_get_thread_target(void);
extern int hv_get_volatile_state(hv_volatile_state_t state);
extern kern_return_t hv_set_traps(hv_trap_type_t trap_type,
    const hv_trap_t *traps, unsigned trap_count);
extern void hv_release_traps(hv_trap_type_t trap_type);
extern kern_return_t hv_set_callbacks(hv_callbacks_t callbacks);
extern void hv_release_callbacks(void);
extern void hv_suspend(void);
extern kern_return_t hv_task_trap(uint64_t index, uint64_t arg);
extern kern_return_t hv_thread_trap(uint64_t index, uint64_t arg);
extern boolean_t hv_ast_pending(void);

#if defined(__cplusplus)
}
#endif

#endif /* _KERN_HV_SUPPORT_H_ */
