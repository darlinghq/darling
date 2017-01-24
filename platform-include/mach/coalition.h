/*
 * Copyright (c) 2014 Apple Inc. All rights reserved.
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

#ifndef _MACH_COALITION_H_
#define _MACH_COALITION_H_

/* code shared by userspace and xnu */

#define COALITION_CREATE_FLAGS_MASK       ((uint32_t)0xF1)
#define COALITION_CREATE_FLAGS_PRIVILEGED ((uint32_t)0x01)

#define COALITION_CREATE_FLAGS_TYPE_MASK  ((uint32_t)0xF0)
#define COALITION_CREATE_FLAGS_TYPE_SHIFT (4)

#define COALITION_CREATE_FLAGS_GET_TYPE(flags) \
	(((flags) & COALITION_CREATE_FLAGS_TYPE_MASK) >> COALITION_CREATE_FLAGS_TYPE_SHIFT)

#define COALITION_CREATE_FLAGS_SET_TYPE(flags, type) \
	do { \
		flags &= ~COALITION_CREATE_FLAGS_TYPE_MASK; \
		flags |= (((type) << COALITION_CREATE_FLAGS_TYPE_SHIFT) \
			   & COALITION_CREATE_FLAGS_TYPE_MASK); \
	} while (0)


#define COALITION_TYPE_RESOURCE  (0)
#define COALITION_TYPE_JETSAM    (1)
#define COALITION_TYPE_MAX       (1)

#define COALITION_NUM_TYPES      (COALITION_TYPE_MAX + 1)

#define COALITION_TASKROLE_UNDEF  (0)
#define COALITION_TASKROLE_LEADER (1)
#define COALITION_TASKROLE_XPC    (2)
#define COALITION_TASKROLE_EXT    (3)

#define COALITION_NUM_TASKROLES   (4)

#define COALITION_ROLEMASK_ALLROLES ((1 << COALITION_NUM_TASKROLES) - 1)
#define COALITION_ROLEMASK_UNDEF    (1 << COALITION_TASKROLE_UNDEF)
#define COALITION_ROLEMASK_LEADER   (1 << COALITION_TASKROLE_LEADER)
#define COALITION_ROLEMASK_XPC      (1 << COALITION_TASKROLE_XPC)
#define COALITION_ROLEMASK_EXT      (1 << COALITION_TASKROLE_EXT)

#define COALITION_SORT_NOSORT     (0)
#define COALITION_SORT_DEFAULT    (1)
#define COALITION_SORT_MEM_ASC    (2)
#define COALITION_SORT_MEM_DEC    (3)
#define COALITION_SORT_USER_ASC   (4)
#define COALITION_SORT_USER_DEC   (5)

#define COALITION_NUM_SORT        (6)

struct coalition_resource_usage {
	uint64_t tasks_started;
	uint64_t tasks_exited;
	uint64_t time_nonempty;
	uint64_t cpu_time;
	uint64_t interrupt_wakeups;
	uint64_t platform_idle_wakeups;
	uint64_t bytesread;
	uint64_t byteswritten;
	uint64_t gpu_time;
	uint64_t cpu_time_billed_to_me;
	uint64_t cpu_time_billed_to_others;
	uint64_t energy;
	uint64_t logical_immediate_writes;
	uint64_t logical_deferred_writes;
	uint64_t logical_invalidated_writes;
	uint64_t logical_metadata_writes;
};

#ifdef PRIVATE
/* definitions shared by only xnu + Libsyscall */

/* Syscall flavors */
#define COALITION_OP_CREATE 1
#define COALITION_OP_TERMINATE 2
#define COALITION_OP_REAP 3

/* coalition_info flavors */
#define COALITION_INFO_RESOURCE_USAGE 1

/* structure returned from libproc coalition listing interface */
struct procinfo_coalinfo {
	uint64_t coalition_id;
	uint32_t coalition_type;
	uint32_t coalition_tasks;
};

#endif /* PRIVATE */

#ifdef XNU_KERNEL_PRIVATE
#if COALITION_DEBUG
#define coal_dbg(fmt, ...) \
	printf("%s: " fmt "\n", __func__, ## __VA_ARGS__)
#else
#define coal_dbg(fmt, ...)
#endif

#endif

#endif /* _MACH_COALITION_H_ */
