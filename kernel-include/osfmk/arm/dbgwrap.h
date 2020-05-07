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

#pragma once

#include <mach/thread_status.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

#if defined(__arm64__)
typedef arm_thread_state64_t __attribute__((aligned(16))) dbgwrap_thread_state_t;
#else
typedef arm_thread_state32_t dbgwrap_thread_state_t;
#endif

typedef enum {
	DBGWRAP_ERR_SELF_HALT = -6,
	DBGWRAP_ERR_UNSUPPORTED = -5,
	DBGWRAP_ERR_INPROGRESS = -4,
	DBGWRAP_ERR_INSTR_ERROR = -3,
	DBGWRAP_ERR_INSTR_TIMEOUT = -2,
	DBGWRAP_ERR_HALT_TIMEOUT = -1,
	DBGWRAP_SUCCESS = 0,
	DBGWRAP_WARN_ALREADY_HALTED,
	DBGWRAP_WARN_CPU_OFFLINE
} dbgwrap_status_t;

static inline const char*
ml_dbgwrap_strerror(dbgwrap_status_t status)
{
	switch (status) {
	case DBGWRAP_ERR_SELF_HALT:             return "CPU attempted to halt itself";
	case DBGWRAP_ERR_UNSUPPORTED:           return "halt not supported for this configuration";
	case DBGWRAP_ERR_INPROGRESS:            return "halt in progress on another CPU";
	case DBGWRAP_ERR_INSTR_ERROR:           return "instruction-stuffing failure";
	case DBGWRAP_ERR_INSTR_TIMEOUT:         return "instruction-stuffing timeout";
	case DBGWRAP_ERR_HALT_TIMEOUT:          return "halt ack timeout, CPU likely wedged";
	case DBGWRAP_SUCCESS:                   return "halt succeeded";
	case DBGWRAP_WARN_ALREADY_HALTED:       return "CPU already halted";
	case DBGWRAP_WARN_CPU_OFFLINE:          return "CPU offline";
	default:                                return "unrecognized status";
	}
}

boolean_t ml_dbgwrap_cpu_is_halted(int cpu_index);

dbgwrap_status_t ml_dbgwrap_wait_cpu_halted(int cpu_index, uint64_t timeout_ns);

dbgwrap_status_t ml_dbgwrap_halt_cpu(int cpu_index, uint64_t timeout_ns);

dbgwrap_status_t ml_dbgwrap_halt_cpu_with_state(int cpu_index, uint64_t timeout_ns, dbgwrap_thread_state_t *state);

__END_DECLS
