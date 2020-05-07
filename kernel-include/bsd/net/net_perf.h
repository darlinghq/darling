/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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
#ifndef _NET_NET_PERF_H_
#define _NET_NET_PERF_H_

#include <stdint.h>

#ifdef KERNEL_PRIVATE
#include <sys/time.h>
#include <mach/boolean.h>
#endif /* KERNEL_PRIVATE */

/* five histogram bins are separated by four dividing "bars" */
#define NET_PERF_BARS 4

typedef struct net_perf {
	uint64_t np_total_pkts; /* total packets input or output during measurement */
	uint64_t np_total_usecs;        /* microseconds elapsed during measurement */
	uint64_t np_hist1;              /* histogram bin 1 */
	uint64_t np_hist2;              /* histogram bin 2 */
	uint64_t np_hist3;              /* histogram bin 3 */
	uint64_t np_hist4;              /* histogram bin 4 */
	uint64_t np_hist5;              /* histogram bin 5 */
	uint8_t np_hist_bars[NET_PERF_BARS];
} net_perf_t;

#ifdef KERNEL_PRIVATE
void net_perf_initialize(net_perf_t *npp, uint64_t bins);
void net_perf_start_time(net_perf_t *npp, struct timeval *tv);
void net_perf_measure_time(net_perf_t *npp, struct timeval *start, uint64_t num_pkts);
void net_perf_histogram(net_perf_t *npp, uint64_t num_pkts);
boolean_t net_perf_validate_bins(uint64_t bins);

#endif /* KERNEL_PRIVATE */

#endif /* _NET_NET_PERF_H_ */
