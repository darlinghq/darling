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

#ifndef _SYS_COALITION_H_
#define _SYS_COALITION_H_

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <mach/coalition.h>

__BEGIN_DECLS

#ifndef KERNEL
/* Userspace syscall prototypes */

/* Syscalls */
int coalition_create(uint64_t *cid_out, uint32_t flags);
int coalition_terminate(uint64_t cid, uint32_t flags);
int coalition_reap(uint64_t cid, uint32_t flags);

/* Wrappers around __coalition_info syscall (with proper struct types) */
int coalition_info_resource_usage(uint64_t cid, struct coalition_resource_usage *cru, size_t sz);
int coalition_info_set_name(uint64_t cid, const char *name, size_t size);
int coalition_info_set_efficiency(uint64_t cid, uint64_t flags);
int coalition_ledger_set_logical_writes_limit(uint64_t cid, int64_t limit);

#else /* KERNEL */

#if CONFIG_COALITIONS
/* in-kernel BSD interfaces */

/*
 * coalition_id:
 * Get the unique 64-bit identifier associated with the given coalition
 */
uint64_t coalition_id(coalition_t coal);


/*
 * coalitions_get_list:
 * Get a list of coalitions as procinfo_coalinfo structures
 *
 * This interface is primarily to support libproc.
 *
 * Parameters:
 *      type      : The COALITION_TYPE of the coalitions to investigate.
 *                  Valid types can be found in <mach/coalition.h>
 *      coal_list : Pointer to an array of procinfo_coalinfo structures
 *                  that will be filled with information about each
 *                  coalition whose type matches 'type'
 *                  NOTE: This can be NULL to perform a simple query of
 *                  the total number of coalitions.
 *      list_sz   : The size (in number of structures) of 'coal_list'
 *
 * Returns: 0 if no coalitions matching 'type' are found
 *          Otherwise: the number of coalitions whose type matches
 *                     the 'type' parameter (all coalitions if type == -1)
 */
extern int coalitions_get_list(int type, struct procinfo_coalinfo *coal_list, int list_sz);


/*
 * task_get_coalition:
 * Return the coalition of a task.
 *
 * Parameters:
 *      task      : The task to investigate
 *      coal_type : The COALITION_TYPE of the coalition to investigate.
 *                  Valid types can be found in <mach/coalition.h>
 *
 * Returns: valid coalition_t or COALITION_NULL
 */
extern coalition_t task_get_coalition(task_t task, int coal_type);


/*
 * coalition_is_leader:
 * Determine if a task is a coalition leader.
 *
 * Parameters:
 *      task      : The task to investigate
 *      coal      : The coalition to test against.
 *                  NOTE: This can be COALITION_NULL, in case FALSE is returned.
 *
 * Returns: TRUE if 'task' is the coalition's leader, FALSE otherwise.
 */
extern boolean_t coalition_is_leader(task_t task, coalition_t coal);

/*
 * coalition_get_leader:
 * Get a task reference on the leader of a given coalition
 *
 * Parameters:
 *	coal      : The coalition to investigate
 *
 * Returns: A referenced task pointer of the leader of the given coalition.
 *          This could be TASK_NULL if the coalition doesn't have a leader.
 *          If the return value is non-null, the caller is responsible to call
 *          task_deallocate on the returned value.
 */
extern task_t coalition_get_leader(coalition_t coal);


/*
 * coalition_get_task_count:
 * Sum up the number of tasks in the given coalition
 *
 * Parameters:
 *      coal     : The coalition to investigate
 *
 * Returns: The number of tasks in the coalition
 */
extern int coalition_get_task_count(coalition_t coal);

/*
 * coalition_get_page_count:
 * Sum up the page count for each task in the coalition specified by 'coal'
 *
 * Parameters:
 *      coal     : The coalition to investigate
 *      ntasks   : If non-NULL, this will be filled in with the number of
 *                 tasks in the coalition.
 *
 * Returns: The sum of all pages used by all members of the coalition
 */
extern uint64_t coalition_get_page_count(coalition_t coal, int *ntasks);

/*
 * coalition_get_pid_list:
 * Gather a list of constituent PIDs of tasks within a coalition playing a
 * given role.
 *
 * Parameters:
 *      coal       : The coalition to investigate
 *      rolemask   : The set of coalition task roles used to filter the list
 *                   of PIDs returned in 'pid_list'. Roles can be combined
 *                   using the COALITION_ROLEMASK_* tokens found in
 *                   <mach/coalition.h>. Each PID returned is guaranteed to
 *                   be tagged with one of the task roles specified by this
 *                   mask.
 *      sort_order : The order in which the returned PIDs should be sorted
 *                   by default this is in descending page count.
 *      pid_list   : Pointer to an array of PIDs that will be filled with
 *                   members of the coalition tagged with the given 'taskrole'
 *      list_sz    : The size (in number of PIDs) of 'pid_list'
 *
 * Note:
 * This function will return the list of PIDs in a sorted order. By default
 * the PIDs will be sorted by task page count in descending order. In the
 * future it may be possible for user space to specify a level of importance
 * for each coalition member. If there is a user space specified importance,
 * then the list of PIDs returned will be sorted in _ascending_ importance,
 * i.e., pid_list[0] will be the least important task (or the largest consumer
 * of memory). The desired sort order can be specified using the
 * COALITION_SORT_* definitions in osfmk/mach/coalition.h
 *
 * It is also possible to return an unsorted list of PIDs using the special
 * sort type 'COALITION_SORT_NOSORT'
 *
 * Returns: < 0 on ERROR
 *          0 if 'coal' contains no tasks whose role is 'taskrole'
 *              (or if the coalition is being deallocated)
 *          Otherwise: the number of PIDs in the coalition whose role is
 *                     'taskrole'. NOTE: This may be larger or smaller than
 *                     the 'pid_list' array.
 *
 */
extern int coalition_get_pid_list(coalition_t coal, uint32_t rolemask,
    int sort_order, int *pid_list, int list_sz);

#else /* !CONFIG_COALITIONS */
static inline uint64_t
coalition_id(__unused coalition_t coal)
{
	return 0;
}

static inline int
coalitions_get_list(__unused int type,
    __unused struct procinfo_coalinfo *coal_list,
    __unused int list_sz)
{
	return 0;
}

static inline coalition_t
coalition_get_leader(__unused task_t task,
    __unused int coal_type)
{
	return COALITION_NULL;
}

static inline boolean_t
coalition_is_leader(__unused task_t task,
    __unused coalition_t coal)
{
	return FALSE;
}

static inline int
coalition_get_task_count(__unused coalition_t coal)
{
	return 0;
}

static inline uint64_t
coalition_get_page_count(__unused coalition_t coal,
    __unused int *ntasks)
{
	return 0;
}

static inline int
coalition_get_pid_list(__unused coalition_t coal,
    __unused uint32_t rolemask,
    __unused int sort_order,
    __unused int *pid_list,
    __unused int list_sz)
{
	return 0;
}
#endif

#endif /* KERNEL */

__END_DECLS

#endif /* _SYS_COALITION_H_ */
