/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#ifndef SYS_MEMORYSTATUS_H
#define SYS_MEMORYSTATUS_H

#include <stdint.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/param.h>

#define DEFAULT_JETSAM_PRIORITY -100

enum {
	kMemorystatusFlagsFrontmost =         (1 << 0),
	kMemorystatusFlagsKilled =            (1 << 1),
	kMemorystatusFlagsKilledHiwat =       (1 << 2),
 	kMemorystatusFlagsFrozen     =        (1 << 3),
 	kMemorystatusFlagsKilledVnodes =      (1 << 4),
 	kMemorystatusFlagsKilledSwap =        (1 << 5),
  	kMemorystatusFlagsThawed =            (1 << 6),
  	kMemorystatusFlagsKilledVM =          (1 << 7),
	kMemorystatusFlagsSuspForDiagnosis =  (1 << 8),
	kMemorystatusFlagsActive =            (1 << 9),
	kMemorystatusFlagsSupportsIdleExit =  (1 << 10),
	kMemorystatusFlagsDirty =             (1 << 11)
};

#if TARGET_OS_EMBEDDED || CONFIG_EMBEDDED

/*
 * Define Memory Status event subclass.
 * Subclass of KEV_SYSTEM_CLASS
 */

/*!
	@defined KEV_MEMORYSTATUS_SUBCLASS
	@discussion The kernel event subclass for memory status events.
*/
#define KEV_MEMORYSTATUS_SUBCLASS        3

enum {
	kMemorystatusLevelNote = 1,
	kMemorystatusSnapshotNote = 2,
	kMemorystatusFreezeNote = 3,
	kMemorystatusPressureNote = 4
};

enum {
	kMemorystatusLevelAny = -1,
	kMemorystatusLevelNormal = 0,
	kMemorystatusLevelWarning = 1,
	kMemorystatusLevelUrgent = 2,
	kMemorystatusLevelCritical = 3
};

typedef struct memorystatus_priority_entry {
	pid_t pid;
	uint32_t flags;
	int32_t hiwat_pages;
	int32_t priority;
	int32_t reserved;
	int32_t reserved2;
} memorystatus_priority_entry_t;

/*
** how many processes to snapshot
*/
#define kMaxSnapshotEntries 128 

typedef struct memorystatus_kernel_stats {
	uint32_t free_pages;
	uint32_t active_pages;
	uint32_t inactive_pages;
	uint32_t throttled_pages;
	uint32_t purgeable_pages;
	uint32_t wired_pages;
} memorystatus_kernel_stats_t;

/*
** This is a variable-length struct.
** Allocate a buffer of the size returned by the sysctl, cast to a memorystatus_snapshot_t *
*/

typedef struct jetsam_snapshot_entry {
	pid_t pid;
	char name[MAXCOMLEN+1];
	int32_t priority;
	uint32_t pages;
	uint32_t flags;
	uint8_t uuid[16];
} memorystatus_jetsam_snapshot_entry_t;

typedef struct jetsam_snapshot {
	uint64_t snapshot_time;
	uint64_t notification_time;
	memorystatus_kernel_stats_t stats;
	size_t entry_count;
	memorystatus_jetsam_snapshot_entry_t entries[1];
} memorystatus_jetsam_snapshot_t;

typedef memorystatus_priority_entry_t 		jetsam_priority_entry_t;
typedef memorystatus_jetsam_snapshot_t 		jetsam_snapshot_t;
typedef memorystatus_jetsam_snapshot_entry_t 	jetsam_snapshot_entry_t;

#define kMemoryStatusLevelNote 		kMemorystatusLevelNote
#define kMemoryStatusSnapshotNote	kMemorystatusSnapshotNote
#define kMemoryStatusFreezeNote		kMemorystatusFreezeNote
#define kMemoryStatusPressureNote	kMemorystatusPressureNote

typedef struct memorystatus_freeze_entry {
 	int32_t pid;
 	uint32_t flags;
 	uint32_t pages;
} memorystatus_freeze_entry_t;

#endif /* TARGET_OS_EMBEDDED */

#ifdef XNU_KERNEL_PRIVATE

/* General tunables */

#define DELTA_PERCENT                    5
#define CRITICAL_PERCENT                 5
#define HIGHWATER_PERCENT               10
#define PRESSURE_PERCENT                15
#define FREEZE_PERCENT                  50

#define POLICY_MORE_FREE_OFFSET_PERCENT    5
#define POLICY_DIAGNOSTIC_OFFSET_PERCENT   5

#define IDLE_EXIT_TIME_SECS 			10

enum {
	kProcessSuspended             = (1 << 0), 
	kProcessFrozen                = (1 << 1),
	kProcessNoReclaimWorth        = (1 << 2),
	kProcessIgnored               = (1 << 3),
	kProcessLocked                = (1 << 4),
	kProcessKilled                =  (1 << 5),
	kProcessNotifiedForPressure   = (1 << 6),
	kProcessPriorityUpdated       = (1 << 7),
	kProcessActive                = (1 << 8),
	kProcessForeground            = (1 << 9),
	kProcessSuspendedForDiag      = (1 << 10),
	kProcessSupportsIdleExit      = (1 << 11),
	kProcessDirty                 = (1 << 12),
	kProcessIgnoreIdleExit        = (1 << 13)
};

typedef struct memorystatus_node {
	TAILQ_ENTRY(memorystatus_node) link;
	pid_t pid;
	int32_t priority;
	uint32_t state;
#if CONFIG_JETSAM
	int32_t hiwat_pages;
#endif
#if CONFIG_FREEZE
	uint32_t resident_pages;
#endif
	uint64_t clean_time;
} memorystatus_node;

extern int memorystatus_wakeup;
extern unsigned int memorystatus_running;

extern unsigned int memorystatus_available_pages;
extern unsigned int memorystatus_available_pages_critical;
extern unsigned int memorystatus_level;
extern unsigned int memorystatus_delta;

extern void memorystatus_init(void) __attribute__((section("__TEXT, initcode")));

extern kern_return_t memorystatus_list_add(int pid, int priority, int high_water_mark);
extern kern_return_t memorystatus_list_change(boolean_t effective, int pid, int priority, int state_flags, int high_water_mark);
extern kern_return_t memorystatus_list_remove(int pid);

extern kern_return_t memorystatus_on_track_dirty(int pid, boolean_t track);
extern kern_return_t memorystatus_on_dirty(int pid, boolean_t dirty);

extern void memorystatus_on_suspend(int pid);
extern void memorystatus_on_resume(int pid);
extern void memorystatus_on_inactivity(int pid);

#if CONFIG_JETSAM

typedef enum memorystatus_policy_t {
	kPolicyDefault        = 0x0, 
	kPolicyMoreFree       = 0x1,
	kPolicyDiagnoseAll    = 0x2,
	kPolicyDiagnoseFirst  = 0x4,
	kPolicyDiagnoseActive = (kPolicyDiagnoseAll | kPolicyDiagnoseFirst),
} memorystatus_policy_t;

extern int memorystatus_jetsam_wakeup;
extern unsigned int memorystatus_jetsam_running;

extern int memorystatus_kill_top_proc(boolean_t any, uint32_t reason);
extern int memorystatus_kill_top_proc_from_VM(void);

extern void memorystatus_update(unsigned int pages_avail);

#if VM_PRESSURE_EVENTS

#define MEMORYSTATUS_SUSPENDED_THRESHOLD  4

extern int memorystatus_request_vm_pressure_candidate(void);
extern void memorystatus_send_pressure_note(int pid);

#endif /* VM_PRESSURE_EVENTS */

#endif /* CONFIG_JETSAM */

#ifdef CONFIG_FREEZE

#define FREEZE_PAGES_MIN   ( 1 * 1024 * 1024 / PAGE_SIZE)
#define FREEZE_PAGES_MAX   (16 * 1024 * 1024 / PAGE_SIZE)

#define FREEZE_SUSPENDED_THRESHOLD_LOW     2
#define FREEZE_SUSPENDED_THRESHOLD_DEFAULT 4

#define FREEZE_DAILY_MB_MAX 	  1024
#define FREEZE_DAILY_PAGEOUTS_MAX (FREEZE_DAILY_MB_MAX * (1024 * 1024 / PAGE_SIZE))

typedef struct throttle_interval_t {
	uint32_t mins;
	uint32_t burst_multiple;
	uint32_t pageouts;
	uint32_t max_pageouts;
	mach_timespec_t ts;
	boolean_t throttle;
} throttle_interval_t;

extern boolean_t memorystatus_freeze_enabled;
extern int memorystatus_freeze_wakeup;

extern void memorystatus_freeze_init(void) __attribute__((section("__TEXT, initcode")));

#endif /* CONFIG_FREEZE */

#endif /* XNU_KERNEL_PRIVATE */

#endif /* SYS_MEMORYSTATUS_H */
