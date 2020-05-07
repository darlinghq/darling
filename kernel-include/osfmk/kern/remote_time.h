/*
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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

#ifndef REMOTE_TIME_H
#define REMOTE_TIME_H

#include <sys/cdefs.h>
#include <stdint.h>
#include <os/overflow.h>

__BEGIN_DECLS
/* bt_params is an ABI for tracing tools */
struct bt_params {
	double rate;
	uint64_t base_local_ts;
	uint64_t base_remote_ts;
};

/* local_ts_ns should be in nanoseconds */
static inline uint64_t
mach_bridge_compute_timestamp(uint64_t local_ts_ns, struct bt_params *params)
{
	if (!params || params->rate == 0.0) {
		return 0;
	}
	/*
	 * Formula to compute remote_timestamp
	 * remote_timestamp = (bt_params.rate * (local_ts_ns - bt_params.base_local_ts))
	 *	 +  bt_params.base_remote_ts
	 */
	int64_t remote_ts = 0;
	int64_t rate_prod = 0;
	/* To avoid precision loss due to typecasting from int64_t to double */
	if (params->rate != 1.0) {
		rate_prod = (int64_t)(params->rate * (double)((int64_t)local_ts_ns - (int64_t)params->base_local_ts));
	} else {
		rate_prod = (int64_t)local_ts_ns - (int64_t)params->base_local_ts;
	}
	if (os_add_overflow((int64_t)params->base_remote_ts, rate_prod, &remote_ts)) {
		return 0;
	}

	return (uint64_t)remote_ts;
}

uint64_t mach_bridge_remote_time(uint64_t);
__END_DECLS

#endif /* REMOTE_TIME_H */
