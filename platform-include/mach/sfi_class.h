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

#ifndef	_MACH_SFI_CLASS_H_
#define	_MACH_SFI_CLASS_H_

#include <stdint.h>
#include <mach/mach_types.h>

#define SFI_CLASS_DEFINITIONS 20140526

/*
 * SFI Classes are used to categorized threads, and map
 * them to "off-time" windows.
 *
 * Threads classified approximately in this order, so
 * there's an implicit requirement that lowest duty cycle
 * (largest off-time) are a lower class, and increasing
 * SFI classes increase the duty cycle (if SFI is enabled
 * at all).
 */
typedef uint32_t sfi_class_id_t;

#ifdef XNU_KERNEL_PRIVATE

/*
 * Total number of classes supported including SFI_CLASS_UNSPECIFIED.
 * If new class is defined increase this number.
 */
#define MAX_SFI_CLASS_ID	0x00000011

/*
 * Threads may initially start out unspecified
 */
#define SFI_CLASS_UNSPECIFIED	0x00000000

#endif /* XNU_KERNEL_PRIVATE */

/*
 * Threads are placed in this class as a result of placing threads or
 * processes in a background state using APIs such as setpriority(2),
 * specifying PRIO_DARWIN_THREAD or PRIO_DARWIN_PROCESS.
 */
#define SFI_CLASS_DARWIN_BG		0x00000001

/*
 * Threads are placed in this class as a result of an application
 * entering "Nap mode".
 */
#define SFI_CLASS_APP_NAP		0x00000002

/*
 * Threads are placed in this class by making per coalition (by
 * calling coalition_set_sfi_class(cid, SFI_CLASS_MANAGED)) or per
 * process selection (by calling process_set_sfi_class(pid,
 * SFI_CLASS_MANAGED)). FOCAL/NONFOCAL is a function of a task's
 * role.
 */
#define SFI_CLASS_MANAGED_FOCAL			0x00000003

#define SFI_CLASS_MANAGED_NONFOCAL		0x00000004

#define SFI_CLASS_MANAGED		SFI_CLASS_MANAGED_FOCAL

/*
 * Coalitions/processes that have not been explicitly tagged
 * and are not opted into one of the special classes below
 * fall into the default categories. FOCAL/NONFOCAL is a function
 * of a task's role.
 */
#define SFI_CLASS_DEFAULT_FOCAL			0x00000005

#define SFI_CLASS_DEFAULT_NONFOCAL		0x00000006

#define SFI_CLASS_DEFAULT		SFI_CLASS_DEFAULT_FOCAL

/*
 * Threads that are part of the kernel task should be duty-cycled
 * only as an extreme last resort, since they must be preempted
 * while locks may be held in kernel mode.
 */
#define SFI_CLASS_KERNEL		0x00000007

/*
 * Threads that must not be part of "Selective Forced Idle" are
 * placed in this class. Real time threads, threads of
 * processes such as WindowServer that are critical to good user
 * experience, should be placed in this class.
 */
#define SFI_CLASS_OPTED_OUT		0x00000008

/*
 * Threads running in various QOS classes
 */
#define SFI_CLASS_UTILITY		    0x00000009
#define SFI_CLASS_LEGACY_FOCAL		    0x0000000A
#define SFI_CLASS_LEGACY_NONFOCAL	    0x0000000B
#define SFI_CLASS_USER_INITIATED_FOCAL	    0x0000000C
#define SFI_CLASS_USER_INITIATED_NONFOCAL   0x0000000D
#define SFI_CLASS_USER_INTERACTIVE_FOCAL    0x0000000E
#define SFI_CLASS_USER_INTERACTIVE_NONFOCAL 0x0000000F
#define SFI_CLASS_MAINTENANCE               0x00000010

/*
 * Windows that are specified smaller than MIN_SFI_WINDOW_USEC
 * will be automatically rounded up.
 */
#define MIN_SFI_WINDOW_USEC		500

#endif	/* _MACH_SFI_CLASS_H_ */
