/*
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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

#ifndef _MACH_RESOURCE_MONITORS_H_
#define _MACH_RESOURCE_MONITORS_H_

#include <sys/syslimits.h>      /* PATH_MAX */
#ifndef XNU_KERNEL_PRIVATE
#include <TargetConditionals.h>
#endif

__BEGIN_DECLS

/*
 * resource_notify_flags_t
 * The top 32 bits are common flags, the bottom for per-call flags.
 */
typedef uint64_t resource_notify_flags_t;
#define kRNFlagsNone                0

/* Flags applicable to any monitors. */
#define kRNFatalLimitFlag           (1ULL << 32)

/* For the disk writes I/O monitor.
 *  The default is logical writes.  */
#define kRNPhysicalWritesFlag       (1ULL < 1)

/* TEMPORARY compatibility, to be removed */
#define kCPUTriggerFatalFlag kRNFatalLimitFlag




/*
 * Process name types for proc_internal.h.
 * proc_name_t is used by resource_notify.defs clients in user space.
 *
 * MAXCOMLEN is defined in bsd/sys/param.h which we can neither include
 * (type conflicts) nor modify (POSIX).
 */
#define MAXCOMLEN 16

typedef char command_t[MAXCOMLEN + 1];
typedef char proc_name_t[2*MAXCOMLEN + 1];
typedef char posix_path_t[PATH_MAX];

__END_DECLS

#endif /* _MACH_RESOURCE_MONITORS_H_ */
