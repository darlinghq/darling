/*
 * Copyright (c) 2011-2019 Apple Computer, Inc. All rights reserved.
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

#ifndef KPERF_CALLSTACK_H
#define KPERF_CALLSTACK_H

#define MAX_KCALLSTACK_FRAMES (128)
#define MAX_UCALLSTACK_FRAMES (256)

/* the callstack contains valid data */
#define CALLSTACK_VALID        (1U << 0)
/* the callstack has been deferred */
#define CALLSTACK_DEFERRED     (1U << 1)
/* the callstack is 64-bit */
#define CALLSTACK_64BIT        (1U << 2)
/* the callstack is from the kernel */
#define CALLSTACK_KERNEL       (1U << 3)
/* the callstack was cut off */
#define CALLSTACK_TRUNCATED    (1U << 4)
/* the callstack is only holding a continuation "frame" */
#define CALLSTACK_CONTINUATION (1U << 5)
/* the frames field is filled with uintptr_t, not uint64_t */
#define CALLSTACK_KERNEL_WORDS (1U << 6)

struct kp_ucallstack {
	uint32_t kpuc_flags;
	uint32_t kpuc_nframes;
	uintptr_t kpuc_frames[MAX_UCALLSTACK_FRAMES];
};

struct kp_kcallstack {
	uint32_t kpkc_flags;
	uint32_t kpkc_nframes;
	union {
		uintptr_t kpkc_word_frames[MAX_KCALLSTACK_FRAMES];
		uint64_t kpkc_frames[MAX_KCALLSTACK_FRAMES];
	};
};

struct kperf_context;

void kperf_kcallstack_sample(struct kp_kcallstack *cs, struct kperf_context *);
void kperf_kcallstack_log(struct kp_kcallstack *cs);
void kperf_continuation_sample(struct kp_kcallstack *cs, struct kperf_context *);
void kperf_backtrace_sample(struct kp_kcallstack *cs, struct kperf_context *context);

void kperf_ucallstack_sample(struct kp_ucallstack *cs, struct kperf_context *);
int kperf_ucallstack_pend(struct kperf_context *, uint32_t depth,
    unsigned int actionid);
void kperf_ucallstack_log(struct kp_ucallstack *cs);

#endif /* !defined(KPERF_CALLSTACK_H) */
