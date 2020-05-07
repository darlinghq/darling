/*
 * Copyright (c) 2012-2013, 2015 Apple Inc. All rights reserved.
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

#ifndef _TASK_CORPSE_H_
#define _TASK_CORPSE_H_

#include <stdint.h>
#include <mach/mach_types.h>
#include <kern/kern_cdata.h>
#include <kern/kcdata.h>

typedef struct kcdata_item      *task_crashinfo_item_t;

/* Deprecated: use the KCDATA_* macros for all future use */
#define CRASHINFO_ITEM_TYPE(item)                 KCDATA_ITEM_TYPE(item)
#define CRASHINFO_ITEM_SIZE(item)                 KCDATA_ITEM_SIZE(item)
#define CRASHINFO_ITEM_DATA_PTR(item)     KCDATA_ITEM_DATA_PTR(item)

#define CRASHINFO_ITEM_NEXT_HEADER(item)  KCDATA_ITEM_NEXT_HEADER(item)

#define CRASHINFO_ITEM_FOREACH(head)      KCDATA_ITEM_FOREACH(head)


#ifndef KERNEL
#define task_crashinfo_get_data_with_desc kcdata_get_data_with_desc

#endif /* KERNEL */

#ifdef XNU_KERNEL_PRIVATE

#define CORPSEINFO_ALLOCATION_SIZE (1024 * 16)
#define TOTAL_CORPSES_ALLOWED 5
#define TOTAL_USER_FAULTS_ALLOWED 1


extern kern_return_t task_mark_corpse(task_t task);

extern kern_return_t task_deliver_crash_notification(task_t, thread_t, exception_type_t, mach_exception_subcode_t);

/* In the corpseinfo kcd_user_flags */
#define CORPSE_CRASHINFO_HAS_REF    0x1
#define CORPSE_CRASHINFO_USER_FAULT 0x2

extern kcdata_descriptor_t task_get_corpseinfo(task_t task);

extern kcdata_descriptor_t  task_crashinfo_alloc_init(
	mach_vm_address_t crash_data_p,
	unsigned size, uint32_t kc_u_flags, unsigned kc_flags);
extern kern_return_t task_crashinfo_destroy(kcdata_descriptor_t data);

extern void corpses_init(void);

extern unsigned long total_corpses_count(void) __attribute__((pure));
extern boolean_t corpses_enabled(void);

extern kern_return_t task_generate_corpse_internal(
	task_t task,
	task_t *corpse_task,
	thread_t *thread,
	exception_type_t etype,
	mach_exception_data_type_t code,
	mach_exception_data_type_t subcode,
	void *reason);

extern void task_clear_corpse(task_t task);

extern kern_return_t task_duplicate_map_and_threads(
	task_t task,
	void *p,
	task_t new_task,
	thread_t *thread,
	uint64_t **udata_buffer,
	int *size,
	int *num_udata);

extern kern_return_t task_enqueue_exception_with_corpse(
	task_t task,
	exception_type_t etype,
	mach_exception_data_t code,
	mach_msg_type_number_t codeCnt,
	void *reason);

extern void task_add_to_corpse_task_list(task_t corpse_task);
void task_remove_from_corpse_task_list(task_t corpse_task);
void task_purge_all_corpses(void);
extern uint64_t task_corpse_get_crashed_thread_id(task_t corpse_task);

#endif /* XNU_KERNEL_PRIVATE */

#endif /* _TASK_CORPSE_H_ */
