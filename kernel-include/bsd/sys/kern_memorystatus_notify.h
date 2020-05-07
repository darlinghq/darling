/*
 * Copyright (c) 2006-2018 Apple Computer, Inc. All rights reserved.
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

#ifndef SYS_MEMORYSTATUS_NOTIFY_H
#define SYS_MEMORYSTATUS_NOTIFY_H

#include <stdint.h>
#include <sys/proc.h>
#include <sys/param.h>

#if VM_PRESSURE_EVENTS

extern vm_pressure_level_t memorystatus_vm_pressure_level;
extern boolean_t memorystatus_hwm_candidates;

boolean_t memorystatus_warn_process(pid_t pid, __unused boolean_t is_active, __unused boolean_t is_fatal, boolean_t exceeded);
int memorystatus_send_note(int event_code, void *data, size_t data_length);
void memorystatus_send_low_swap_note(void);
void consider_vm_pressure_events(void);

#if CONFIG_MEMORYSTATUS

int memorystatus_low_mem_privileged_listener(uint32_t op_flags);
int memorystatus_send_pressure_note(int pid);
boolean_t memorystatus_is_foreground_locked(proc_t p);
boolean_t memorystatus_bg_pressure_eligible(proc_t p);
void memorystatus_proc_flags_unsafe(void * v, boolean_t *is_dirty, boolean_t *is_dirty_tracked, boolean_t *allow_idle_exit);
extern void memorystatus_issue_fg_band_notify(void);

#endif /* CONFIG_MEMORYSTATUS */

#if DEBUG
#define VM_PRESSURE_DEBUG(cond, format, ...)      \
do {                                              \
if (cond) { printf(format, ##__VA_ARGS__); } \
} while(0)
#else
#define VM_PRESSURE_DEBUG(cond, format, ...)
#endif

#endif /* VM_PRESSURE_EVENTS */

#endif /* SYS_MEMORYSTATUS_NOTIFY_H */
