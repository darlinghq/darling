/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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

#ifndef _KERN_BTLOG_H_
#define _KERN_BTLOG_H_

#include <kern/kern_types.h>
#include <kern/debug.h>
#include <sys/cdefs.h>
#include <stdint.h>
#include <mach_debug/zone_info.h>

#ifdef  XNU_KERNEL_PRIVATE

/*
 * The btlog subsystem allows for fast unobtrusive backtraces
 * to be recorded and maintained in chronological order.
 *
 * Each backtrace is associated with an element/object,
 * and an operation. For example, memory allocations and
 * frees can be tracked with this infrastructure. So
 * can refcounts. The "operation" namespace is maintained
 * by the caller.
 *
 * When the event buffer fills, records are reused in FIFO
 * order.
 *
 * When a btlog_t is created, callbacks can be provided
 * to ensure proper locking of the datastructures. If these
 * are not provided, the caller is responsible for
 * preventing simultaneous modification.
 */

/*
 * BTLOG_MAX_DEPTH configures how deep of a stack trace is stored.  15
 * levels is usually enough to get past all the layers of code in
 * kalloc and IOKit and see who the actual caller is up above these
 * lower levels, when used by the zone allocator logging code.
 */

#define BTLOG_MAX_DEPTH 15

struct btlog;
typedef struct btlog btlog_t;

extern btlog_t *btlog_create(size_t numrecords,
    size_t record_btdepth,
    boolean_t caller_will_remove_entries_for_element);

extern void btlog_add_entry(btlog_t *btlog,
    void *element,
    uint8_t operation,
    void *bt[],
    size_t btcount);

extern void btlog_remove_entries_for_element(btlog_t *btlog,
    void *element);

#if DEBUG || DEVELOPMENT
void btlog_copy_backtraces_for_elements(btlog_t      * btlog,
    uintptr_t    * instances,
    uint32_t     * count,
    uint32_t       zoneSize,
    leak_site_proc proc,
    void         * refCon);

size_t get_btlog_records_count(btlog_t *btlog);

void get_btlog_records(btlog_t *btlog,
    zone_btrecord_t *records,
    unsigned int *numrecs);

#endif  /* DEBUG || DEVELOPMENT */

#endif  /* XNU_KERNEL_PRIVATE */

#endif  /* _KERN_BTLOG_H_ */
