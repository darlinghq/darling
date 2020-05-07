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

#ifndef SYS_KTRACE_H
#define SYS_KTRACE_H

#include <stdint.h>

#include <kern/locks.h>

/* The states that ktrace can be in. */
enum ktrace_state {
	/* No tool has configured ktrace. */
	KTRACE_STATE_OFF = 0,
	/* A foreground tool has configured ktrace. */
	KTRACE_STATE_FG,
	/* A background tool has configured ktrace. */
	KTRACE_STATE_BG
};

void ktrace_lock(void);
void ktrace_unlock(void);
void ktrace_assert_lock_held(void);
void ktrace_start_single_threaded(void);
void ktrace_end_single_threaded(void);

/*
 * Subsystems that use ktrace to manage ownership.  These values are passed as
 * part of the `*_mask` arguments in `ktrace_configure` and `ktrace_reset`.
 */
#define KTRACE_KDEBUG (1 << 0)
#define KTRACE_KPERF  (1 << 1)

/*
 * Used by subsystems to inform ktrace that a configuration is occurring.
 * Validates whether the current process has privileges to configure
 * ktrace.  Pass the subsystem(s) being configured in config_mask.
 *
 * `ktrace_lock` must be held.
 *
 * Returns 0 if configuration is allowed, EPERM if process is not privileged,
 * and EBUSY if ktrace is owned by another process.
 */
int ktrace_configure(uint32_t config_mask);

/*
 * Tell ktrace to reset a configuration.  Pass the susbsystem(s) that are to
 * be reset in the reset_mask.
 *
 * `ktrace_lock` must be held.
 */
void ktrace_reset(uint32_t reset_mask);

/*
 * Determine if the current process can read the configuration of ktrace.
 * Only the owning process or a root privileged process is allowed.
 *
 * `ktrace_lock` must be held.
 *
 * Returns 0 if allowed, EPERM otherwise.
 */
int ktrace_read_check(void);

/*
 * With certain boot-args, the kernel can start tracing without user space
 * intervention.  With `trace=<n_events>`, the kernel will start tracing at
 * boot.  With `trace_wake=<n_events>`, the kernel will start tracing on the
 * wake path out of hibernation (on Intel only).
 *
 * In these cases, ktrace must be aware of the state changes.  This function
 * should be called whenever the kernel initiates configuring ktrace.
 *
 * `ktrace_lock` must be held.
 */
void ktrace_kernel_configure(uint32_t config_mask);

/*
 * This KPI allows kernel systems to disable ktrace.  ktrace will only be
 * disabled if the state matches the provided state_to_match.
 *
 * This does not reset the configuration of any subsystems -- it just makes
 * them stop logging events or sampling data.
 *
 * `ktrace_lock` must be held.
 */
void ktrace_disable(enum ktrace_state state_to_match);

/*
 * Returns the pid of the process that owns ktrace.  If ktrace is unowned,
 * returns 0.
 *
 * `ktrace_lock` must be held.
 */
int ktrace_get_owning_pid(void);

/*
 * Returns true if background tracing is active, false otherwise.
 *
 * `ktrace_lock` must be held.
 */
bool ktrace_background_active(void);

/*
 * These functions exist for the transition for kperf to allow blessing other
 * processes.  They should not be used by other clients.
 */
extern bool ktrace_keep_ownership_on_reset;
extern int ktrace_root_set_owner_allowed;
int ktrace_set_owning_pid(int pid);

/* Initialize ktrace.  Must only be called by the bootstrap thread. */
void ktrace_init(void);

#endif /* SYS_KTRACE_H */
