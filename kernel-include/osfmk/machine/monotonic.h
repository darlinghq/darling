/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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
#ifndef MACHINE_MONOTONIC_H
#define MACHINE_MONOTONIC_H

#if defined(__x86_64__)
#include <x86_64/monotonic.h>
#elif defined(__arm64__)
#include <arm64/monotonic.h>
#elif defined(__arm__)
#include <arm/monotonic.h>
#else
#error unsupported architecture
#endif

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

extern bool mt_core_supported;

struct mt_cpu {
	uint64_t mtc_snaps[MT_CORE_NFIXED];
	uint64_t mtc_counts[MT_CORE_NFIXED];
	uint64_t mtc_counts_last[MT_CORE_NFIXED];
	uint64_t mtc_npmis;
	/*
	 * Whether this CPU should be using PMCs.
	 */
	bool mtc_active;
};

struct mt_thread {
	_Atomic uint64_t mth_gen;
	uint64_t mth_counts[MT_CORE_NFIXED];
};

struct mt_task {
	uint64_t mtk_counts[MT_CORE_NFIXED];
};

struct mt_cpu *mt_cur_cpu(void);

uint64_t mt_count_pmis(void);
void mt_mtc_update_fixed_counts(struct mt_cpu *mtc, uint64_t *counts,
    uint64_t *counts_since);
uint64_t mt_mtc_update_count(struct mt_cpu *mtc, unsigned int ctr);
uint64_t mt_core_snap(unsigned int ctr);
void mt_core_set_snap(unsigned int ctr, uint64_t snap);
void mt_mtc_set_snap(struct mt_cpu *mtc, unsigned int ctr, uint64_t snap);

typedef void (*mt_pmi_fn)(bool user_mode, void *ctx);
extern bool mt_microstackshots;
extern unsigned int mt_microstackshot_ctr;
extern mt_pmi_fn mt_microstackshot_pmi_handler;
extern void *mt_microstackshot_ctx;
extern uint64_t mt_core_reset_values[MT_CORE_NFIXED];
int mt_microstackshot_start_arch(uint64_t period);

#endif /* !defined(MACHINE_MONOTONIC_H) */
