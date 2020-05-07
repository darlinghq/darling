/*
 * Copyright (c) 2016 Apple Computer, Inc. All rights reserved.
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

#ifndef KPERF_KDEBUG_TRIGGER_H
#define KPERF_KDEBUG_TRIGGER_H

#include <mach/boolean.h>
#include <stdint.h>

#define KPERF_KDEBUG_DEBUGIDS_MAX (32)

struct kperf_kdebug_filter;

#define KPERF_KDEBUG_FILTER_SIZE(N_DEBUGIDS) ((2 * sizeof(uint64_t)) + ((N_DEBUGIDS) * sizeof(uint32_t)))
/* UNSAFE */
#define KPERF_KDEBUG_N_DEBUGIDS(FILTER_SIZE) \
	(((FILTER_SIZE) <= (2 * sizeof(uint64_t))) ? 0 : \
	  (((FILTER_SIZE) - (2 * sizeof(uint64_t))) / sizeof(uint32_t)))

int kperf_kdebug_init(void);
void kperf_kdebug_reset(void);

boolean_t kperf_kdebug_should_trigger(uint32_t debugid);

int kperf_kdebug_set_action(int action_id);
int kperf_kdebug_get_action(void);

int kperf_kdebug_set_n_debugids(uint32_t n_debugids_in);
int kperf_kdebug_set_filter(user_addr_t user_filter, uint32_t user_size);
uint32_t kperf_kdebug_get_filter(struct kperf_kdebug_filter **filter);

#endif /* !defined(KPERF_KDEBUG_TRIGGER_H) */
