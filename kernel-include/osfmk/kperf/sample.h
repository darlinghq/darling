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

#ifndef KPERF_SAMPLE_H
#define KPERF_SAMPLE_H

#include <kperf/thread_samplers.h>
#include <kperf/task_samplers.h>
#include "callstack.h"
#include "kperf_kpc.h"
#include "meminfo.h"

/*
 * For data that must be sampled in a fault-able context.
 */
struct kperf_usample {
	struct kperf_thread_dispatch th_dispatch;
	struct kp_ucallstack ucallstack;
	struct kperf_thread_info th_info;
};

struct kperf_sample {
	struct kperf_thread_info       th_info;
	struct kperf_thread_scheduling th_scheduling;
	struct kperf_thread_snapshot   th_snapshot;

	struct kperf_task_snapshot tk_snapshot;

	struct kp_kcallstack kcallstack;
	struct meminfo     meminfo;

	struct kperf_usample usample;

#if KPC
	struct kpcdata    kpcdata;
#endif /* KPC */

#if DEVELOPMENT || DEBUG
	uint64_t sample_time;
#endif /* DEVELOPMENT || DEBUG */
};

/* cache of threads on each CPU during a timer fire */
extern uint64_t *kperf_tid_on_cpus;

#endif /* !defined(KPERF_SAMPLE_H) */
