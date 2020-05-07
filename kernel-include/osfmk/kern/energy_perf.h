/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

/*
 * Interfaces for non-kernel managed devices to inform the kernel of their
 * energy and performance relevant activity and resource utilisation, typically
 * on a per-thread or task basis.
 */

#ifndef _KERN_ENERGY_PERF_H_
#define _KERN_ENERGY_PERF_H_

#include <stdint.h>

#ifdef KERNEL
__BEGIN_DECLS

typedef struct {
	uint32_t gpu_id;
	uint32_t gpu_max_domains;
} gpu_descriptor;

typedef gpu_descriptor *gpu_descriptor_t;
/* The GPU is expected to describe itself with this interface prior to reporting
 * resource usage.
 */
void gpu_describe(gpu_descriptor_t);

#define GPU_SCOPE_CURRENT_THREAD (0x1)
#define GPU_SCOPE_MISC (0x2)

/* GPU utilisation update for the current thread. */
uint64_t gpu_accumulate_time(uint32_t scope, uint32_t gpu_id, uint32_t gpu_domain, uint64_t gpu_accumulated_ns, uint64_t gpu_tstamp_ns);

/* Interfaces for the block storage driver to advise the perf. controller of
 * recent IOs
 */

/* Target medium for this set of IOs. Updates can occur in parallel if
 * multiple devices exist, hence consumers must synchronize internally, ideally
 * in a low-overhead fashion such as per-CPU counters, as this may be invoked
 * within the IO path.
 */

#define IO_MEDIUM_ROTATING (0x0ULL)
#define IO_MEDIUM_SOLID_STATE (0x1ULL)

/* As there are several priority bands whose nature is evolving, we rely on the
 * block storage driver to classify non-performance-critical IOs as "low"
 * priority. Separate updates are expected for low/high priority IOs.
 */

#define IO_PRIORITY_LOW (0x1ULL << 8)

/* Reserved for estimates of bursts of future IOs; could possibly benefit from
 * a time horizon, but it's unclear if it will be specifiable by any layer with
 * reasonable accuracy
 */
#define IO_PRIORITY_PREDICTIVE (0x1ULL << 16)

uint64_t io_rate_update(
	uint64_t io_rate_flags, /* Rotating/NAND, IO priority level */
	uint64_t read_ops_delta,
	uint64_t write_ops_delta,
	uint64_t read_bytes_delta,
	uint64_t write_bytes_delta);

typedef uint64_t (*io_rate_update_callback_t) (uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

void io_rate_update_register(io_rate_update_callback_t);

/* Interfaces for integrated GPUs to supply command submission telemetry.
 */

#define GPU_NCMDS_VALID (0x1)
#define GPU_NOUTSTANDING_VALID (0x2)
#define GPU_BUSY_VALID (0x4)
#define GPU_CYCLE_COUNT_VALID (0x8)
#define GPU_MISC_VALID (0x10)

void gpu_submission_telemetry(
	uint64_t gpu_ncmds_total,
	uint64_t gpu_noutstanding,
	uint64_t gpu_busy_ns_total,
	uint64_t gpu_cycles,
	uint64_t gpu_telemetry_valid_flags,
	uint64_t gpu_telemetry_misc);

typedef uint64_t (*gpu_set_fceiling_t) (uint32_t gpu_fceiling_ratio, uint64_t gpu_fceiling_param);

void gpu_fceiling_cb_register(gpu_set_fceiling_t);

__END_DECLS
#endif /* KERNEL */

#endif /* _KERN_ENERGY_PERF_H_ */
