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

#ifndef _SYS_PROC_UUID_POLICY_H
#define _SYS_PROC_UUID_POLICY_H

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/types.h>
#include <stdint.h>
#include <uuid/uuid.h>

__BEGIN_DECLS

/*
 * The proc_uuid_policy subsystem allows a privileged client to
 * upload policies to the kernel keyed by Mach-O executable
 * UUID. In-kernel clients can query this policy table cheaply
 * to determine if a resource or process should by governed
 * by the policy flags. During early boot, the policy table
 * may be empty or sparse, which in-kernel clients should
 * have a specified behavior for.
 */

#define PROC_UUID_POLICY_OPERATION_CLEAR        0x00000000
#define PROC_UUID_POLICY_OPERATION_ADD          0x00000001
#define PROC_UUID_POLICY_OPERATION_REMOVE       0x00000002

/* The namespace of flags are managed by in-kernel clients */
#define PROC_UUID_POLICY_FLAGS_NONE                     0x00000000
#define PROC_UUID_NO_CELLULAR                           0x00000001
#define PROC_UUID_NECP_APP_POLICY                       0x00000002
#define PROC_UUID_ALT_DYLD_POLICY                       0x00000004

/* To be removed, replaced by PROC_UUID_NECP_APP_POLICY */
#define PROC_UUID_FLOW_DIVERT                           0x00000002

#ifdef BSD_KERNEL_PRIVATE
/*
 * Look up a policy indexed by UUID.
 *
 * Paramaters:
 *     uuid          UUID to look up, must be not the zero-uuid
 *     flags         Flags that have been associated with the UUID on successful
 *                   lookup.
 *     gencount      The generation count of the internal policy table representation.
 *
 *     Initial lookups by an in-kernel subsystem should pass 0 for flags/gencount.
 *     Subsequent lookups for the same UUID with the same flags and gencount passed
 *     in can short-circuit the lookup if the generation count has not changed.
 *
 * Return:
 *     0        Success, UUID was found, flags and gencount are returned
 *     EINVAL   Bad UUID or other pointer parameter
 *     ENOENT   UUID not found
 *
 */
extern int proc_uuid_policy_lookup(uuid_t uuid, uint32_t *flags, int32_t *gencount);

extern void proc_uuid_policy_init(void);

extern int proc_uuid_policy_kernel(uint32_t operation, uuid_t uuid, uint32_t flags);
#endif /* BSD_KERNEL_PRIVATE */

#ifndef KERNEL
/*
 * Upload a policy indexed by UUID.
 *
 * Parameters:
 *     operation     CLEAR    Clear specified flags for all entries.
 *                            Entries are removed if they have no remaining flags.
 *                   ADD      Add the specified UUID and flags to the policy table.
 *                            Flags are ORed  with existing entries for the UUID.
 *                   REMOVE   Mask out flags in the entry for the specified UUID.
 *                            Entry is removed if it has no remaining flags.
 *     uuid          Pointer to UUID for Mach-O executable
 *     uuidlen       sizeof(uuid_t)
 *     flags         Flags to be stored in the policy table. See operation notes above.
 *
 * Return:
 *     0        Success, operation completed without error.
 *     -1       Failure, errno can contain:
 *         ENOENT   REMOVE operation specified a UUID not in the policy table.
 *         EPERM    Call is not privileged to call this system call
 *         EINVAL   Invalid parameter
 *         ERANGE   Invalid uuidlen
 *         ENOMEM   Too many entries exist
 */
extern int proc_uuid_policy(uint32_t operation, uuid_t uuid, size_t uuidlen, uint32_t flags);
#endif /* !KERNEL */

__END_DECLS

#endif /*_SYS_PROC_UUID_POLICY_H */
