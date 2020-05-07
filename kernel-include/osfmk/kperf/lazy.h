/*
 * Copyright (c) 2018 Apple Computer, Inc. All rights reserved.
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

#ifndef KPERF_LAZY_H
#define KPERF_LAZY_H

#include <stdbool.h>
#include <stdint.h>
#include <kern/thread.h>

extern unsigned int kperf_lazy_wait_action;
extern unsigned int kperf_lazy_cpu_action;

void kperf_lazy_reset(void);
void kperf_lazy_off_cpu(thread_t thread);
void kperf_lazy_make_runnable(thread_t thread, bool in_interrupt);
void kperf_lazy_wait_sample(thread_t thread,
    thread_continue_t continuation, uintptr_t *starting_fp);
void kperf_lazy_cpu_sample(thread_t thread, unsigned int flags, bool interrupt);

/* accessors for configuration */
int kperf_lazy_get_wait_action(void);
int kperf_lazy_get_cpu_action(void);
int kperf_lazy_set_wait_action(int action_id);
int kperf_lazy_set_cpu_action(int action_id);
uint64_t kperf_lazy_get_wait_time_threshold(void);
uint64_t kperf_lazy_get_cpu_time_threshold(void);
int kperf_lazy_set_wait_time_threshold(uint64_t threshold);
int kperf_lazy_set_cpu_time_threshold(uint64_t threshold);

#endif /* !defined(KPERF_LAZY_H) */
