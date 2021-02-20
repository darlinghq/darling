/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
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

#ifndef __OS_PROC__
#define __OS_PROC__

#include <stddef.h>
#include <sys/cdefs.h>
#include <os/availability.h>

/*!
 * @header
 *
 * @preprocinfo
 * This is for functions that operate on the calling process alone.
 */

__BEGIN_DECLS

/*!
 * @function os_proc_available_memory
 *
 * @abstract
 * Return the number of bytes remaining, at the time of the call, before the
 * current process will hit its current dirty memory limit.
 *
 * @discussion
 * Developers can query this value efficiently whenever it is needed. The return
 * value is only a snapshot at the time of the call. Caching the result is not
 * advised. The result may be instantaneously invalidated by actions taken in
 * another thread or another framework.
 *
 * Memory limits can change during the app life cycle. Make sure to check accordingly.
 *
 * The size returned is not representative of the total memory of the device, it
 * is the current dirty memory limit minus the dirty memory footprint used at the
 * time of the query.
 *
 * This interface allows an app to efficiently consume all available memory resources.
 * Significant memory use, even under the current memory limit, may still cause
 * system-wide performance including the termination of other apps and system
 * processes. Take care to use the minimum amount of memory needed to satisfy the
 * user’s need.
 *
 * If you need more information than just the available memory, you can use task_info().
 * The information returned is equivalent to the task_vm_info.limit_bytes_remaining
 * field. task_info() is a more expensive call, and will return information such
 * as your phys_footprint, which is used to calculate the return of this function.
 *
 * Dirty memory contains data that must be kept in RAM (or the equivalent) even
 * when unused. It is memory that has been modified.
 *
 * @param none
 *
 * @result
 * The remaining bytes. 0 is returned if the calling process is not an app, or
 * the calling process exceeds its memory limit.
 */

    API_UNAVAILABLE(macos) API_AVAILABLE(ios(13.0), tvos(13.0), watchos(6.0), bridgeos(4.0))
extern
size_t os_proc_available_memory(void);

__END_DECLS

#endif
