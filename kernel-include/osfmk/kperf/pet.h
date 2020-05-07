/*
 * Copyright (c) 2011 Apple Computer, Inc. All rights reserved.
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

#include <kern/thread.h>

#define KPERF_PET_DEFAULT_IDLE_RATE (15)

extern boolean_t kperf_lightweight_pet_active;
extern uint32_t kperf_pet_gen;

/* prepare PET to be able to fire action with given ID, or disable PET */
void kperf_pet_config(unsigned int action_id);

/* fire off a PET sample, both before and after on-core samples */
void kperf_pet_fire_before(void);
void kperf_pet_fire_after(void);

/* notify PET of new threads switching on */
void kperf_pet_on_cpu(thread_t thread, thread_continue_t continuation,
    uintptr_t *starting_frame);

/* get/set rate at which idle threads are sampled by PET */
int kperf_get_pet_idle_rate(void);
int kperf_set_pet_idle_rate(int val);

/* get/set whether lightweight PET is enabled */
int kperf_get_lightweight_pet(void);
int kperf_set_lightweight_pet(int val);

void kperf_lightweight_pet_active_update(void);
