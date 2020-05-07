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

#ifndef KPERF_TASK_SAMPLERS_H
#define KPERF_TASK_SAMPLERS_H

#include <kperf/context.h>
#include <kern/task.h>

struct kperf_task_snapshot {
	uint64_t kptksn_flags;
	uint64_t kptksn_user_time_in_terminated_threads;
	uint64_t kptksn_system_time_in_terminated_threads;
	int kptksn_suspend_count;
	int kptksn_pageins;
};

#define KPERF_TASK_FLAG_DARWIN_BG               (1U << 0)
#define KPERF_TASK_FLAG_FOREGROUND              (1U << 1)
#define KPERF_TASK_FLAG_BOOSTED                 (1U << 2)
#define KPERF_TASK_FLAG_DIRTY                   (1U << 3)
#define KPERF_TASK_FLAG_WQ_FLAGS_VALID          (1U << 4)
#define KPERF_TASK_FLAG_WQ_EXCEEDED_TOTAL       (1U << 5)
#define KPERF_TASK_FLAG_WQ_EXCEEDED_CONSTRAINED (1U << 6)
#define KPERF_TASK_FLAG_DIRTY_TRACKED           (1U << 7)
#define KPERF_TASK_ALLOW_IDLE_EXIT              (1U << 8)

void kperf_task_snapshot_sample(task_t task, struct kperf_task_snapshot *tksn);
void kperf_task_snapshot_log(struct kperf_task_snapshot *tksn);
void kperf_task_info_log(struct kperf_context *ctx);

#endif /* !defined(KPERF_TASK_SAMPLERS_H) */
