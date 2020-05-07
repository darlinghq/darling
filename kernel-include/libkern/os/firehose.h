/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __firehose_h
#define __firehose_h

__BEGIN_DECLS

/*!
 * @function __firehose_buffer_push_to_logd
 *
 * @abstract
 * Called by the dispatch firehose apis to notify logd that a chunk is available
 */
    __WATCHOS_AVAILABLE(3.0) __OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_AVAILABLE(10.0)
void __firehose_buffer_push_to_logd(firehose_buffer_t fb, bool for_io);

/*!
 * @function __firehose_allocate
 *
 * @abstract
 * Wrapper to allocate kernel memory
 */
__WATCHOS_AVAILABLE(3.0) __OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_AVAILABLE(10.0)
void __firehose_allocate(vm_offset_t *addr, vm_size_t size);

/*!
 * @function __firehose_critical_region_enter
 *
 * @abstract
 * Function that disables preemption
 */
__WATCHOS_AVAILABLE(3.0) __OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_AVAILABLE(10.0)
extern void __firehose_critical_region_enter(void);

/*!
 * @function __firehose_critical_region_leave
 *
 * @abstract
 * Function that enables preemption
 */
__WATCHOS_AVAILABLE(3.0) __OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_AVAILABLE(10.0)
extern void __firehose_critical_region_leave(void);

extern void oslogwakeup(void);

__END_DECLS

#endif /* __firehose_h */
