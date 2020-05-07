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

#ifndef __KPERF_KPC_H__
#define __KPERF_KPC_H__

#if KPC
#include <kern/kpc.h> /* KPC_MAX_COUNTERS */
#endif

void kperf_kpc_thread_ast(thread_t thread);

/* KPC sample data */
struct kpcdata {
	int      curcpu;
	uint32_t running;
	uint32_t counterc;
	uint64_t counterv[KPC_MAX_COUNTERS + 1];
	uint32_t configc;
	uint64_t configv[KPC_MAX_COUNTERS];
};

void kperf_kpc_thread_sample(struct kpcdata *, int);
void kperf_kpc_cpu_sample(struct kpcdata *, int);
void kperf_kpc_thread_log(const struct kpcdata *);
void kperf_kpc_cpu_log(const struct kpcdata *);
void kperf_kpc_config_log(const struct kpcdata *);

#endif /* __KPERF_KPC_H__ */
