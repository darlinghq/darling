/*
 * Copyright (c) 2005, 2015 Apple Computer, Inc. All rights reserved.
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

#ifndef _SYS_BSDTASK_INFO_H
#define _SYS_BSDTASK_INFO_H

#include <vm/vm_map.h>

struct proc_taskinfo_internal {
	uint64_t                pti_virtual_size;   /* virtual memory size (bytes) */
	uint64_t                pti_resident_size;  /* resident memory size (bytes) */
	uint64_t                pti_total_user;         /* total time */
	uint64_t                pti_total_system;
	uint64_t                pti_threads_user;       /* existing threads only */
	uint64_t                pti_threads_system;
	int32_t                 pti_policy;             /* default policy for new threads */
	int32_t                 pti_faults;             /* number of page faults */
	int32_t                 pti_pageins;    /* number of actual pageins */
	int32_t                 pti_cow_faults; /* number of copy-on-write faults */
	int32_t                 pti_messages_sent;      /* number of messages sent */
	int32_t                 pti_messages_received; /* number of messages received */
	int32_t                 pti_syscalls_mach;  /* number of mach system calls */
	int32_t                 pti_syscalls_unix;  /* number of unix system calls */
	int32_t                 pti_csw;            /* number of context switches */
	int32_t                 pti_threadnum;          /* number of threads in the task */
	int32_t                 pti_numrunning;         /* number of running threads */
	int32_t                 pti_priority;           /* task priority*/
};

#define MAXTHREADNAMESIZE 64

struct proc_threadinfo_internal {
	uint64_t                pth_user_time;      /* user run time */
	uint64_t                pth_system_time;    /* system run time */
	int32_t                 pth_cpu_usage;      /* scaled cpu usage percentage */
	int32_t                 pth_policy;             /* scheduling policy in effect */
	int32_t                 pth_run_state;      /* run state (see below) */
	int32_t                 pth_flags;          /* various flags (see below) */
	int32_t                 pth_sleep_time;     /* number of seconds that thread */
	int32_t                 pth_curpri;             /* cur priority*/
	int32_t                 pth_priority;           /*  priority*/
	int32_t                 pth_maxpriority;                /* max priority*/
	char                    pth_name[MAXTHREADNAMESIZE];            /* thread name, if any */
};



struct proc_regioninfo_internal {
	uint32_t                pri_protection;
	uint32_t                pri_max_protection;
	uint32_t                pri_inheritance;
	uint32_t                pri_flags;              /* shared, external pager, is submap */
	uint64_t                pri_offset;
	uint32_t                pri_behavior;
	uint32_t                pri_user_wired_count;
	uint32_t                pri_user_tag;
	uint32_t                pri_pages_resident;
	uint32_t                pri_pages_shared_now_private;
	uint32_t                pri_pages_swapped_out;
	uint32_t                pri_pages_dirtied;
	uint32_t                pri_ref_count;
	uint32_t                pri_shadow_depth;
	uint32_t                pri_share_mode;
	uint32_t                pri_private_pages_resident;
	uint32_t                pri_shared_pages_resident;
	uint32_t                pri_obj_id;
	uint32_t                pri_depth;
	uint64_t                pri_address;
	uint64_t                pri_size;
};

#ifdef  MACH_KERNEL_PRIVATE

#define PROC_REGION_SUBMAP      1
#define PROC_REGION_SHARED      2

extern uint32_t vnode_vid(void *vp);

#if CONFIG_IOSCHED
kern_return_t vnode_pager_get_object_devvp(memory_object_t mem_obj, uintptr_t *devvp);
extern struct vnode *vnode_mountdevvp(struct vnode *);
#endif

extern boolean_t vnode_isonexternalstorage(void *vp);

#endif /* MACH_KERNEL_PRIVATE */

extern int fill_procregioninfo(task_t t, uint64_t arg, struct proc_regioninfo_internal *pinfo, uintptr_t *vp, uint32_t *vid);
extern int fill_procregioninfo_onlymappedvnodes(task_t t, uint64_t arg, struct proc_regioninfo_internal *pinfo, uintptr_t *vp, uint32_t *vid);
extern int find_region_details(task_t task, vm_map_offset_t offset, uintptr_t *vnodeaddr, uint32_t *vid, uint64_t *start, uint64_t *len);
void fill_taskprocinfo(task_t task, struct proc_taskinfo_internal * ptinfo);
int fill_taskthreadinfo(task_t task, uint64_t thaddr, bool thuniqueid, struct proc_threadinfo_internal * ptinfo, void *, int *);
int fill_taskthreadlist(task_t task, void * buffer, int thcount, bool thuniqueid);
int get_numthreads(task_t);
boolean_t bsd_hasthreadname(void *uth);
void bsd_getthreadname(void *uth, char* buffer);
void bsd_setthreadname(void *uth, const char* buffer);
void bsd_threadcdir(void * uth, void *vptr, int *vidp);
extern void bsd_copythreadname(void *dst_uth, void *src_uth);
int fill_taskipctableinfo(task_t task, uint32_t *table_size, uint32_t *table_free);

#endif /*_SYS_BSDTASK_INFO_H */
