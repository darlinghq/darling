/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
#include <sys/types.h>
#include <machine/cpu_capabilities.h>
#include <kern/remote_time.h>
#include <mach/mach_time.h>
#include "strings.h"
#include <TargetConditionals.h>

#define BT_RESET_SENTINEL_TS  (~3ULL) /* from machine/machine_remote_time.h */

extern uint64_t __mach_bridge_remote_time(uint64_t local_time);

#if TARGET_OS_BRIDGE && defined(__arm64__)
static uint64_t
absolutetime_to_nanoseconds(uint64_t abs_time)
{
	mach_timebase_info_data_t info;
	mach_timebase_info(&info);
	uint64_t time_in_ns = (uint64_t)(((double)info.numer / (double)info.denom) * abs_time);

	return time_in_ns;
}

uint64_t
mach_bridge_remote_time(__unused uint64_t local_time)
{
	uint64_t remote_time = 0;
	uint64_t local_time_ns = 0;
	uint64_t now = 0;
	struct bt_params params = {};

	volatile struct bt_params *commpage_bt_params_p = (struct bt_params *)_COMM_PAGE_REMOTETIME_PARAMS;
	volatile uint64_t *base_local_ts_p = &commpage_bt_params_p->base_local_ts;
	volatile uint64_t *base_remote_ts_p = &commpage_bt_params_p->base_remote_ts;
	volatile double *rate_p = &commpage_bt_params_p->rate;

	do {
		params.base_local_ts = *base_local_ts_p;
		if (*base_local_ts_p == BT_RESET_SENTINEL_TS) {
			return 0;
		}
		/*
		 * This call contains an instruction barrier that ensures the second read of
		 * base_local_ts is not speculated above the first read of base_local_ts.
		 */
		now = mach_absolute_time();
		params.base_remote_ts = *base_remote_ts_p;
		params.rate = *rate_p;
		/*
		 * This barrier prevents the second read of base_local_ts from being reordered
		 * w.r.t the reads of other values in bt_params.
		 */
		__asm__ volatile ("dmb ishld" ::: "memory");
	} while (params.base_local_ts && (params.base_local_ts != commpage_bt_params_p->base_local_ts));

	if (!local_time) {
		local_time = now;
	}
	local_time_ns = absolutetime_to_nanoseconds(local_time);
	if (local_time_ns < params.base_local_ts) {
		remote_time = __mach_bridge_remote_time(local_time);
	} else {
		remote_time = mach_bridge_compute_timestamp(local_time_ns, &params);
	}
	return remote_time;
}
#endif /* TARGET_OS_BRIDGE && defined(__arm64__) */
