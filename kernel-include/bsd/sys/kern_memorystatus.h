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

#ifndef SYS_MEMORYSTATUS_H
#define SYS_MEMORYSTATUS_H

#include <sys/time.h>
#include <mach_debug/zone_info.h>
#include <sys/proc.h>

#define MEMORYSTATUS_ENTITLEMENT "com.apple.private.memorystatus"

#define JETSAM_PRIORITY_REVISION                  2

#define JETSAM_PRIORITY_IDLE_HEAD                -2
/* The value -1 is an alias to JETSAM_PRIORITY_DEFAULT */
#define JETSAM_PRIORITY_IDLE                      0
#define JETSAM_PRIORITY_IDLE_DEFERRED             1 /* Keeping this around till all xnu_quick_tests can be moved away from it.*/
#define JETSAM_PRIORITY_AGING_BAND1               JETSAM_PRIORITY_IDLE_DEFERRED
#define JETSAM_PRIORITY_BACKGROUND_OPPORTUNISTIC  2
#define JETSAM_PRIORITY_AGING_BAND2               JETSAM_PRIORITY_BACKGROUND_OPPORTUNISTIC
#define JETSAM_PRIORITY_BACKGROUND                3
#define JETSAM_PRIORITY_ELEVATED_INACTIVE         JETSAM_PRIORITY_BACKGROUND
#define JETSAM_PRIORITY_MAIL                      4
#define JETSAM_PRIORITY_PHONE                     5
#define JETSAM_PRIORITY_UI_SUPPORT                8
#define JETSAM_PRIORITY_FOREGROUND_SUPPORT        9
#define JETSAM_PRIORITY_FOREGROUND               10
#define JETSAM_PRIORITY_AUDIO_AND_ACCESSORY      12
#define JETSAM_PRIORITY_CONDUCTOR                13
#define JETSAM_PRIORITY_DRIVER_APPLE             15
#define JETSAM_PRIORITY_HOME                     16
#define JETSAM_PRIORITY_EXECUTIVE                17
#define JETSAM_PRIORITY_IMPORTANT                18
#define JETSAM_PRIORITY_CRITICAL                 19

#define JETSAM_PRIORITY_MAX                      21

/* TODO - tune. This should probably be lower priority */
#define JETSAM_PRIORITY_DEFAULT                  18
#define JETSAM_PRIORITY_TELEPHONY                19

/* Compatibility */
#define DEFAULT_JETSAM_PRIORITY                  18

/*
 * The deferral time used by default for apps and daemons in all aging
 * policies except kJetsamAgingPolicySysProcsReclaimedFirst is
 * DEFERRED_IDLE_EXIT_TIME_SECS.
 *
 * For kJetsamAgingPolicySysProcsReclaimedFirst,
 *
 * Daemons: The actual idle deferred time for the daemon is based on
 * the relaunch behavior of the daemon. The relaunch behavior determines
 * the scaling factor applied to DEFERRED_IDLE_EXIT_TIME_SECS. See
 * kJetsamSysProcsIdleDelayTime* ratios defined in kern_memorystatus.c
 *
 * Apps: The apps are aged for DEFERRED_IDLE_EXIT_TIME_SECS factored
 * by kJetsamAppsIdleDelayTimeRatio.
 */
#define DEFERRED_IDLE_EXIT_TIME_SECS             10

#define KEV_MEMORYSTATUS_SUBCLASS                 3

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
	int32_t priority;
	uint64_t user_data;
	int32_t limit;  /* MB */
	uint32_t state;
} memorystatus_priority_entry_t;

/*
 * This should be the structure to specify different properties
 * for processes (group or single) from user-space. Unfortunately,
 * we can't move to it completely because the priority_entry structure
 * above has been in use for a while now. We'll have to deprecate it.
 *
 * To support new fields/properties, we will add a new structure with a
 * new version and a new size.
 */
#define MEMORYSTATUS_MPE_VERSION_1              1

#define MEMORYSTATUS_MPE_VERSION_1_SIZE         sizeof(struct memorystatus_properties_entry_v1)

typedef struct memorystatus_properties_entry_v1 {
	int version;
	pid_t pid;
	int32_t priority;
	int use_probability;
	uint64_t user_data;
	int32_t limit;  /* MB */
	uint32_t state;
	char proc_name[MAXCOMLEN + 1];
	char __pad1[3];
} memorystatus_properties_entry_v1_t;

typedef struct memorystatus_kernel_stats {
	uint32_t free_pages;
	uint32_t active_pages;
	uint32_t inactive_pages;
	uint32_t throttled_pages;
	uint32_t purgeable_pages;
	uint32_t wired_pages;
	uint32_t speculative_pages;
	uint32_t filebacked_pages;
	uint32_t anonymous_pages;
	uint32_t compressor_pages;
	uint64_t compressions;
	uint64_t decompressions;
	uint64_t total_uncompressed_pages_in_compressor;
	uint64_t zone_map_size;
	uint64_t zone_map_capacity;
	uint64_t largest_zone_size;
	char     largest_zone_name[MACH_ZONE_NAME_MAX_LEN];
} memorystatus_kernel_stats_t;

/*
** This is a variable-length struct.
** Allocate a buffer of the size returned by the sysctl, cast to a memorystatus_snapshot_t *
*/

typedef struct jetsam_snapshot_entry {
	pid_t    pid;
	char     name[(2 * MAXCOMLEN) + 1];
	int32_t  priority;
	uint32_t state;
	uint32_t fds;
	uint8_t  uuid[16];
	uint64_t user_data;
	uint64_t killed;
	uint64_t pages;
	uint64_t max_pages_lifetime;
	uint64_t purgeable_pages;
	uint64_t jse_internal_pages;
	uint64_t jse_internal_compressed_pages;
	uint64_t jse_purgeable_nonvolatile_pages;
	uint64_t jse_purgeable_nonvolatile_compressed_pages;
	uint64_t jse_alternate_accounting_pages;
	uint64_t jse_alternate_accounting_compressed_pages;
	uint64_t jse_iokit_mapped_pages;
	uint64_t jse_page_table_pages;
	uint64_t jse_memory_region_count;
	uint64_t jse_gencount;                  /* memorystatus_thread generation counter */
	uint64_t jse_starttime;                 /* absolute time when process starts */
	uint64_t jse_killtime;                  /* absolute time when jetsam chooses to kill a process */
	uint64_t jse_idle_delta;                /* time spent in idle band */
	uint64_t jse_coalition_jetsam_id;       /* we only expose coalition id for COALITION_TYPE_JETSAM */
	struct timeval64 cpu_time;
	uint64_t jse_thaw_count;
} memorystatus_jetsam_snapshot_entry_t;

typedef struct jetsam_snapshot {
	uint64_t snapshot_time;                 /* absolute time snapshot was initialized */
	uint64_t notification_time;             /* absolute time snapshot was consumed */
	uint64_t js_gencount;                   /* memorystatus_thread generation counter */
	memorystatus_kernel_stats_t stats;      /* system stat when snapshot is initialized */
	size_t entry_count;
	memorystatus_jetsam_snapshot_entry_t entries[];
} memorystatus_jetsam_snapshot_t;

/* TODO - deprecate; see <rdar://problem/12969599> */
#define kMaxSnapshotEntries 192

/*
 * default jetsam snapshot support
 */
extern memorystatus_jetsam_snapshot_t *memorystatus_jetsam_snapshot;
extern memorystatus_jetsam_snapshot_t *memorystatus_jetsam_snapshot_copy;
extern unsigned int memorystatus_jetsam_snapshot_count;
extern unsigned int memorystatus_jetsam_snapshot_copy_count;
extern unsigned int memorystatus_jetsam_snapshot_max;
extern unsigned int memorystatus_jetsam_snapshot_size;
extern uint64_t memorystatus_jetsam_snapshot_last_timestamp;
extern uint64_t memorystatus_jetsam_snapshot_timeout;
#define memorystatus_jetsam_snapshot_list memorystatus_jetsam_snapshot->entries
#define JETSAM_SNAPSHOT_TIMEOUT_SECS 30

/* General memorystatus stuff */

extern uint64_t memorystatus_sysprocs_idle_delay_time;
extern uint64_t memorystatus_apps_idle_delay_time;

/* State */
#define kMemorystatusSuspended        0x01
#define kMemorystatusFrozen           0x02
#define kMemorystatusWasThawed        0x04
#define kMemorystatusTracked          0x08
#define kMemorystatusSupportsIdleExit 0x10
#define kMemorystatusDirty            0x20
#define kMemorystatusAssertion        0x40

/*
 * Jetsam exit reason definitions - related to memorystatus
 *
 * When adding new exit reasons also update:
 *	JETSAM_REASON_MEMORYSTATUS_MAX
 *	kMemorystatusKilled... Cause enum
 *	memorystatus_kill_cause_name[]
 */
#define JETSAM_REASON_INVALID                                                           0
#define JETSAM_REASON_GENERIC                                                           1
#define JETSAM_REASON_MEMORY_HIGHWATER                                          2
#define JETSAM_REASON_VNODE                                                                     3
#define JETSAM_REASON_MEMORY_VMPAGESHORTAGE                                     4
#define JETSAM_REASON_MEMORY_PROCTHRASHING                                      5
#define JETSAM_REASON_MEMORY_FCTHRASHING                                        6
#define JETSAM_REASON_MEMORY_PERPROCESSLIMIT                            7
#define JETSAM_REASON_MEMORY_DISK_SPACE_SHORTAGE                        8
#define JETSAM_REASON_MEMORY_IDLE_EXIT                                          9
#define JETSAM_REASON_ZONE_MAP_EXHAUSTION                                       10
#define JETSAM_REASON_MEMORY_VMCOMPRESSOR_THRASHING                     11
#define JETSAM_REASON_MEMORY_VMCOMPRESSOR_SPACE_SHORTAGE        12
#define JETSAM_REASON_LOWSWAP                                   13
#define JETSAM_REASON_MEMORYSTATUS_MAX  JETSAM_REASON_LOWSWAP

/*
 * Jetsam exit reason definitions - not related to memorystatus
 */
#define JETSAM_REASON_CPULIMIT                  100

/* Cause */
enum {
	kMemorystatusInvalid                                                    = JETSAM_REASON_INVALID,
	kMemorystatusKilled                                                             = JETSAM_REASON_GENERIC,
	kMemorystatusKilledHiwat                                                = JETSAM_REASON_MEMORY_HIGHWATER,
	kMemorystatusKilledVnodes                                               = JETSAM_REASON_VNODE,
	kMemorystatusKilledVMPageShortage                               = JETSAM_REASON_MEMORY_VMPAGESHORTAGE,
	kMemorystatusKilledProcThrashing                                = JETSAM_REASON_MEMORY_PROCTHRASHING,
	kMemorystatusKilledFCThrashing                                  = JETSAM_REASON_MEMORY_FCTHRASHING,
	kMemorystatusKilledPerProcessLimit                              = JETSAM_REASON_MEMORY_PERPROCESSLIMIT,
	kMemorystatusKilledDiskSpaceShortage                    = JETSAM_REASON_MEMORY_DISK_SPACE_SHORTAGE,
	kMemorystatusKilledIdleExit                                             = JETSAM_REASON_MEMORY_IDLE_EXIT,
	kMemorystatusKilledZoneMapExhaustion                    = JETSAM_REASON_ZONE_MAP_EXHAUSTION,
	kMemorystatusKilledVMCompressorThrashing                = JETSAM_REASON_MEMORY_VMCOMPRESSOR_THRASHING,
	kMemorystatusKilledVMCompressorSpaceShortage    = JETSAM_REASON_MEMORY_VMCOMPRESSOR_SPACE_SHORTAGE,
	kMemorystatusKilledLowSwap                      = JETSAM_REASON_LOWSWAP,
};

/*
 * For backwards compatibility
 * Keeping these around for external users (e.g. ReportCrash, Ariadne).
 * TODO: Remove once they stop using these.
 */
#define kMemorystatusKilledDiagnostic           kMemorystatusKilledDiskSpaceShortage
#define kMemorystatusKilledVMThrashing          kMemorystatusKilledVMCompressorThrashing
#define JETSAM_REASON_MEMORY_VMTHRASHING        JETSAM_REASON_MEMORY_VMCOMPRESSOR_THRASHING

/* Memorystatus control */
#define MEMORYSTATUS_BUFFERSIZE_MAX 65536

#ifndef KERNEL
int memorystatus_get_level(user_addr_t level);
int memorystatus_control(uint32_t command, int32_t pid, uint32_t flags, void *buffer, size_t buffersize);
#endif

/* Commands */
#define MEMORYSTATUS_CMD_GET_PRIORITY_LIST            1
#define MEMORYSTATUS_CMD_SET_PRIORITY_PROPERTIES      2
#define MEMORYSTATUS_CMD_GET_JETSAM_SNAPSHOT          3
#define MEMORYSTATUS_CMD_GET_PRESSURE_STATUS          4
#define MEMORYSTATUS_CMD_SET_JETSAM_HIGH_WATER_MARK   5    /* Set active memory limit = inactive memory limit, both non-fatal	*/
#define MEMORYSTATUS_CMD_SET_JETSAM_TASK_LIMIT        6    /* Set active memory limit = inactive memory limit, both fatal	*/
#define MEMORYSTATUS_CMD_SET_MEMLIMIT_PROPERTIES      7    /* Set memory limits plus attributes independently			*/
#define MEMORYSTATUS_CMD_GET_MEMLIMIT_PROPERTIES      8    /* Get memory limits plus attributes					*/
#define MEMORYSTATUS_CMD_PRIVILEGED_LISTENER_ENABLE   9    /* Set the task's status as a privileged listener w.r.t memory notifications  */
#define MEMORYSTATUS_CMD_PRIVILEGED_LISTENER_DISABLE  10   /* Reset the task's status as a privileged listener w.r.t memory notifications  */
#define MEMORYSTATUS_CMD_AGGRESSIVE_JETSAM_LENIENT_MODE_ENABLE  11   /* Enable the 'lenient' mode for aggressive jetsam. See comments in kern_memorystatus.c near the top. */
#define MEMORYSTATUS_CMD_AGGRESSIVE_JETSAM_LENIENT_MODE_DISABLE 12   /* Disable the 'lenient' mode for aggressive jetsam. */
#define MEMORYSTATUS_CMD_GET_MEMLIMIT_EXCESS          13   /* Compute how much a process's phys_footprint exceeds inactive memory limit */
#define MEMORYSTATUS_CMD_ELEVATED_INACTIVEJETSAMPRIORITY_ENABLE         14 /* Set the inactive jetsam band for a process to JETSAM_PRIORITY_ELEVATED_INACTIVE */
#define MEMORYSTATUS_CMD_ELEVATED_INACTIVEJETSAMPRIORITY_DISABLE        15 /* Reset the inactive jetsam band for a process to the default band (0)*/
#define MEMORYSTATUS_CMD_SET_PROCESS_IS_MANAGED       16   /* (Re-)Set state on a process that marks it as (un-)managed by a system entity e.g. assertiond */
#define MEMORYSTATUS_CMD_GET_PROCESS_IS_MANAGED       17   /* Return the 'managed' status of a process */
#define MEMORYSTATUS_CMD_SET_PROCESS_IS_FREEZABLE     18   /* Is the process eligible for freezing? Apps and extensions can pass in FALSE to opt out of freezing, i.e.,
	                                                    *  if they would prefer being jetsam'ed in the idle band to being frozen in an elevated band. */
#define MEMORYSTATUS_CMD_GET_PROCESS_IS_FREEZABLE     19   /* Return the freezable state of a process. */

#if CONFIG_FREEZE
#if DEVELOPMENT || DEBUG
#define MEMORYSTATUS_CMD_FREEZER_CONTROL              20
#endif /* DEVELOPMENT || DEBUG */
#endif /* CONFIG_FREEZE */

#define MEMORYSTATUS_CMD_GET_AGGRESSIVE_JETSAM_LENIENT_MODE      21   /* Query if the lenient mode for aggressive jetsam is enabled. */

#define MEMORYSTATUS_CMD_INCREASE_JETSAM_TASK_LIMIT   22   /* Used by DYLD to increase the jetsam active and inactive limits, when using roots */

/* Commands that act on a group of processes */
#define MEMORYSTATUS_CMD_GRP_SET_PROPERTIES           100

#if PRIVATE
/* Test commands */

/* Trigger forced jetsam */
#define MEMORYSTATUS_CMD_TEST_JETSAM            1000
#define MEMORYSTATUS_CMD_TEST_JETSAM_SORT       1001

/* Panic on jetsam options */
typedef struct memorystatus_jetsam_panic_options {
	uint32_t data;
	uint32_t mask;
} memorystatus_jetsam_panic_options_t;

#define MEMORYSTATUS_CMD_SET_JETSAM_PANIC_BITS        1002

/* Select priority band sort order */
#define JETSAM_SORT_NOSORT      0
#define JETSAM_SORT_DEFAULT     1

#endif /* PRIVATE */

/* memorystatus_control() flags */

#define MEMORYSTATUS_FLAGS_SNAPSHOT_ON_DEMAND           0x1     /* A populated snapshot buffer is returned on demand */
#define MEMORYSTATUS_FLAGS_SNAPSHOT_AT_BOOT             0x2     /* Returns a snapshot with memstats collected at boot */
#define MEMORYSTATUS_FLAGS_SNAPSHOT_COPY                0x4     /* Returns the previously populated snapshot created by the system */
#define MEMORYSTATUS_FLAGS_GRP_SET_PRIORITY             0x8     /* Set jetsam priorities for a group of pids */
#define MEMORYSTATUS_FLAGS_GRP_SET_PROBABILITY          0x10    /* Set probability of use for a group of processes */

/*
 * For use with memorystatus_control:
 * MEMORYSTATUS_CMD_GET_JETSAM_SNAPSHOT
 *
 * A jetsam snapshot is initialized when a non-idle
 * jetsam event occurs.  The data is held in the
 * buffer until it is reaped. This is the default
 * behavior.
 *
 * Flags change the default behavior:
 *	Demand mode - this is an on_demand snapshot,
 *	meaning data is populated upon request.
 *
 *	Boot mode - this is a snapshot of
 *	memstats collected before loading the
 *	init program.  Once collected, these
 *	stats do not change.  In this mode,
 *	the snapshot entry_count is always 0.
 *
 *	Copy mode - this returns the previous snapshot
 *      collected by the system. The current snaphshot
 *	might be only half populated.
 *
 * Snapshots are inherently racey between request
 * for buffer size and actual data compilation.
 */

/* These definitions are required for backwards compatibility */
#define MEMORYSTATUS_SNAPSHOT_ON_DEMAND         MEMORYSTATUS_FLAGS_SNAPSHOT_ON_DEMAND
#define MEMORYSTATUS_SNAPSHOT_AT_BOOT           MEMORYSTATUS_FLAGS_SNAPSHOT_AT_BOOT
#define MEMORYSTATUS_SNAPSHOT_COPY              MEMORYSTATUS_FLAGS_SNAPSHOT_COPY

/*
 * For use with memorystatus_control:
 * MEMORYSTATUS_CMD_SET_PRIORITY_PROPERTIES
 */
typedef struct memorystatus_priority_properties {
	int32_t  priority;
	uint64_t user_data;
} memorystatus_priority_properties_t;

/*
 * Inform the kernel that setting the priority property is driven by assertions.
 */
#define MEMORYSTATUS_SET_PRIORITY_ASSERTION     0x1

/*
 * For use with memorystatus_control:
 * MEMORYSTATUS_CMD_SET_MEMLIMIT_PROPERTIES
 * MEMORYSTATUS_CMD_GET_MEMLIMIT_PROPERTIES
 */
typedef struct memorystatus_memlimit_properties {
	int32_t memlimit_active;                /* jetsam memory limit (in MB) when process is active */
	uint32_t memlimit_active_attr;
	int32_t memlimit_inactive;              /* jetsam memory limit (in MB) when process is inactive */
	uint32_t memlimit_inactive_attr;
} memorystatus_memlimit_properties_t;

typedef struct memorystatus_memlimit_properties2 {
	memorystatus_memlimit_properties_t v1;
	uint32_t memlimit_increase;             /* jetsam memory limit increase (in MB) for active and inactive states */
	uint32_t memlimit_increase_bytes;       /* bytes used to determine the jetsam memory limit increase, for active and inactive states */
} memorystatus_memlimit_properties2_t;

#define MEMORYSTATUS_MEMLIMIT_ATTR_FATAL        0x1     /* if set, exceeding the memlimit is fatal */

#ifdef XNU_KERNEL_PRIVATE

/*
 * A process will be killed immediately if it crosses a memory limit marked as fatal.
 * Fatal limit types are the
 *	- default system-wide task limit
 *	- per-task custom memory limit
 *
 * A process with a non-fatal memory limit can exceed that limit, but becomes an early
 * candidate for jetsam when the device is under memory pressure.
 * Non-fatal limit types are the
 *	- high-water-mark limit
 *
 * Processes that opt into dirty tracking are evaluated
 * based on clean vs dirty state.
 *      dirty ==> active
 *      clean ==> inactive
 *
 * Processes that do not opt into dirty tracking are
 * evalulated based on priority level.
 *      Foreground or above ==> active
 *      Below Foreground    ==> inactive
 */

/*
 * p_memstat_state flag holds
 *	- in kernel process state and memlimit state
 */

#define P_MEMSTAT_SUSPENDED            0x00000001 /* Process is suspended and likely in the IDLE band */
#define P_MEMSTAT_FROZEN               0x00000002 /* Process has some state on disk. It should be suspended */
#define P_MEMSTAT_FREEZE_DISABLED      0x00000004 /* Process isn't freeze-eligible and will not be frozen */
#define P_MEMSTAT_ERROR                0x00000008 /* Process couldn't be jetsammed for some reason. Transient state so jetsam can skip it next time it sees it */
#define P_MEMSTAT_LOCKED               0x00000010 /* Process is being actively worked on behind the proc_list_lock */
#define P_MEMSTAT_TERMINATED           0x00000020 /* Process is exiting */
#define P_MEMSTAT_FREEZE_IGNORE        0x00000040 /* Process was evaluated by freezer and will be ignored till the next time it goes active and does something */
#define P_MEMSTAT_PRIORITYUPDATED      0x00000080 /* Process had its jetsam priority updated */
#define P_MEMSTAT_FOREGROUND           0x00000100 /* Process is in the FG jetsam band...unused??? */
#define P_MEMSTAT_REFREEZE_ELIGIBLE    0x00000400 /* Process was once thawed i.e. its state was brought back from disk. It is now refreeze eligible.*/
#define P_MEMSTAT_MANAGED              0x00000800 /* Process is managed by assertiond i.e. is either application or extension */
#define P_MEMSTAT_INTERNAL             0x00001000 /* Process is a system-critical-not-be-jetsammed process i.e. launchd */
#define P_MEMSTAT_FATAL_MEMLIMIT                  0x00002000   /* current fatal state of the process's memlimit */
#define P_MEMSTAT_MEMLIMIT_ACTIVE_FATAL           0x00004000   /* if set, exceeding limit is fatal when the process is active   */
#define P_MEMSTAT_MEMLIMIT_INACTIVE_FATAL         0x00008000   /* if set, exceeding limit is fatal when the process is inactive */
#define P_MEMSTAT_USE_ELEVATED_INACTIVE_BAND      0x00010000   /* if set, the process will go into this band & stay there when in the background instead
	                                                        *  of the aging bands and/or the IDLE band. */
#define P_MEMSTAT_PRIORITY_ASSERTION              0x00020000   /* jetsam priority is being driven by an assertion */


/*
 * p_memstat_relaunch_flags holds
 *      - relaunch behavior when jetsammed
 */
#define P_MEMSTAT_RELAUNCH_UNKNOWN      0x0
#define P_MEMSTAT_RELAUNCH_LOW          0x1
#define P_MEMSTAT_RELAUNCH_MED          0x2
#define P_MEMSTAT_RELAUNCH_HIGH         0x4

/*
 * Checking the p_memstat_state almost always requires the proc_list_lock
 * because the jetsam thread could be on the other core changing the state.
 *
 * App -- almost always managed by a system process. Always have dirty tracking OFF. Can include extensions too.
 * System Processes -- not managed by anybody. Always have dirty tracking ON. Can include extensions (here) too.
 */
#define isApp(p)            ((p->p_memstat_state & P_MEMSTAT_MANAGED) || ! (p->p_memstat_dirty & P_DIRTY_TRACK))
#define isSysProc(p)            ( ! (p->p_memstat_state & P_MEMSTAT_MANAGED) || (p->p_memstat_dirty & P_DIRTY_TRACK))

#define MEMSTAT_BUCKET_COUNT (JETSAM_PRIORITY_MAX + 1)

typedef struct memstat_bucket {
	TAILQ_HEAD(, proc) list;
	int count;
	int relaunch_high_count;
} memstat_bucket_t;

extern memstat_bucket_t memstat_bucket[MEMSTAT_BUCKET_COUNT];

/*
 * Table that expresses the probability of a process
 * being used in the next hour.
 */
typedef struct memorystatus_internal_probabilities {
	char proc_name[MAXCOMLEN + 1];
	int use_probability;
} memorystatus_internal_probabilities_t;

extern memorystatus_internal_probabilities_t *memorystatus_global_probabilities_table;
extern size_t memorystatus_global_probabilities_size;


extern void memorystatus_init(void) __attribute__((section("__TEXT, initcode")));

extern void memorystatus_init_at_boot_snapshot(void);

extern int memorystatus_add(proc_t p, boolean_t locked);
extern int memorystatus_update(proc_t p, int priority, uint64_t user_data, boolean_t is_assertion, boolean_t effective,
    boolean_t update_memlimit, int32_t memlimit_active, boolean_t memlimit_active_is_fatal,
    int32_t memlimit_inactive, boolean_t memlimit_inactive_is_fatal);

/* Remove this process from jetsam bands for killing or freezing.
 * The proc_list_lock is held by the caller.
 * @param p: The process to remove.
 * @return: 0 if successful. EAGAIN if the process can't be removed right now (because it's being frozen) or ESRCH.
 */
extern int memorystatus_remove(proc_t p);

int memorystatus_update_inactive_jetsam_priority_band(pid_t pid, uint32_t opflags, int priority, boolean_t effective_now);
int memorystatus_relaunch_flags_update(proc_t p, int relaunch_flags);

extern int memorystatus_dirty_track(proc_t p, uint32_t pcontrol);
extern int memorystatus_dirty_set(proc_t p, boolean_t self, uint32_t pcontrol);
extern int memorystatus_dirty_get(proc_t p, boolean_t locked);
extern int memorystatus_dirty_clear(proc_t p, uint32_t pcontrol);

extern int memorystatus_on_terminate(proc_t p);

extern void memorystatus_on_suspend(proc_t p);
extern void memorystatus_on_resume(proc_t p);
extern void memorystatus_on_inactivity(proc_t p);

extern void memorystatus_on_pageout_scan_end(void);

/* Memorystatus kevent */

void memorystatus_kevent_init(lck_grp_t *grp, lck_attr_t *attr);

int memorystatus_knote_register(struct knote *kn);
void memorystatus_knote_unregister(struct knote *kn);

#if CONFIG_MEMORYSTATUS
void memorystatus_log_exception(const int max_footprint_mb, boolean_t memlimit_is_active, boolean_t memlimit_is_fatal);
void memorystatus_on_ledger_footprint_exceeded(int warning, boolean_t memlimit_is_active, boolean_t memlimit_is_fatal);
void proc_memstat_terminated(proc_t p, boolean_t set);
void memorystatus_proc_flags_unsafe(void * v, boolean_t *is_dirty, boolean_t *is_dirty_tracked, boolean_t *allow_idle_exit);

#if __arm64__
void memorystatus_act_on_legacy_footprint_entitlement(proc_t p, boolean_t footprint_increase);
void memorystatus_act_on_ios13extended_footprint_entitlement(proc_t p);
#endif /* __arm64__ */

#endif /* CONFIG_MEMORYSTATUS */

int memorystatus_get_pressure_status_kdp(void);

#if CONFIG_JETSAM

typedef enum memorystatus_policy {
	kPolicyDefault        = 0x0,
	kPolicyMoreFree       = 0x1,
} memorystatus_policy_t;

boolean_t memorystatus_kill_on_VM_page_shortage(boolean_t async);
boolean_t memorystatus_kill_on_FC_thrashing(boolean_t async);
boolean_t memorystatus_kill_on_VM_compressor_thrashing(boolean_t async);
boolean_t memorystatus_kill_on_vnode_limit(void);

void jetsam_on_ledger_cpulimit_exceeded(void);
void memorystatus_fast_jetsam_override(boolean_t enable_override);

#endif /* CONFIG_JETSAM */

/* These are very verbose printfs(), enable with
 * MEMORYSTATUS_DEBUG_LOG
 */
#if MEMORYSTATUS_DEBUG_LOG
#define MEMORYSTATUS_DEBUG(cond, format, ...)      \
do {                                              \
if (cond) { printf(format, ##__VA_ARGS__); } \
} while(0)
#else
#define MEMORYSTATUS_DEBUG(cond, format, ...)
#endif

boolean_t memorystatus_kill_on_zone_map_exhaustion(pid_t pid);
boolean_t memorystatus_kill_on_VM_compressor_space_shortage(boolean_t async);
void memorystatus_pages_update(unsigned int pages_avail);
boolean_t memorystatus_idle_exit_from_VM(void);
proc_t memorystatus_get_first_proc_locked(unsigned int *bucket_index, boolean_t search);
proc_t memorystatus_get_next_proc_locked(unsigned int *bucket_index, proc_t p, boolean_t search);
void memorystatus_get_task_page_counts(task_t task, uint32_t *footprint, uint32_t *max_footprint_lifetime, uint32_t *purgeable_pages);
void memorystatus_invalidate_idle_demotion_locked(proc_t p, boolean_t clean_state);
void memorystatus_update_priority_locked(proc_t p, int priority, boolean_t head_insert, boolean_t skip_demotion_check);

#if VM_PRESSURE_EVENTS

extern kern_return_t memorystatus_update_vm_pressure(boolean_t);

#if CONFIG_MEMORYSTATUS
/* Flags */
extern int memorystatus_low_mem_privileged_listener(uint32_t op_flags);
extern int memorystatus_send_pressure_note(int pid);
extern boolean_t memorystatus_is_foreground_locked(proc_t p);
extern boolean_t memorystatus_bg_pressure_eligible(proc_t p);
#endif /* CONFIG_MEMORYSTATUS */

#endif /* VM_PRESSURE_EVENTS */

#endif /* XNU_KERNEL_PRIVATE */

#endif /* SYS_MEMORYSTATUS_H */
