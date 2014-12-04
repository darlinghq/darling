/*
 * Copyright (c) 2014 Apple Inc. All rights reserved.
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

#ifndef _PTHREAD_SPAWN_H
#define _PTHREAD_SPAWN_H

/*!
 * @group posix_spawn QOS class support
 * Apple extensions to posix_spawn(2) and posix_spawnp(2)
 */

#include <pthread/pthread.h>
#include <spawn.h>

__BEGIN_DECLS

/*!
 * @function posix_spawnattr_set_qos_class_np
 *
 * @abstract
 * Sets the QOS class property of a posix_spawn attributes object, which may be
 * used to specify the QOS class a process should be spawned with.
 *
 * @discussion
 * The QOS class specified at the time of process spawn determines both the
 * initial requested QOS class of the main thread in the new process, and the
 * interpretation by the system of all QOS class values requested by threads in
 * the process.
 *
 * @param __attr
 * The spawn attributes object to modify.
 *
 * @param __qos_class
 * A QOS class value:
 *	- QOS_CLASS_UTILITY
 *	- QOS_CLASS_BACKGROUND
 * EINVAL will be returned if any other value is provided.
 *
 * @return
 * Zero if successful, otherwise an errno value.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_8_0)
int
posix_spawnattr_set_qos_class_np(posix_spawnattr_t * __restrict __attr,
                                 qos_class_t __qos_class);

/*!
 * @function posix_spawnattr_get_qos_class_np
 *
 * @abstract
 * Gets the QOS class property of a posix_spawn attributes object.
 *
 * @param __attr
 * The spawn attributes object to inspect.
 *
 * @param __qos_class
 * On output, a QOS class value:
 *	- QOS_CLASS_UTILITY
 *	- QOS_CLASS_BACKGROUND
 *	- QOS_CLASS_UNSPECIFIED
 *
 * @return
 * Zero if successful, otherwise an errno value.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_10, __IPHONE_8_0)
int
posix_spawnattr_get_qos_class_np(const posix_spawnattr_t *__restrict __attr,
                                 qos_class_t * __restrict __qos_class);

__END_DECLS

#endif // _PTHREAD_SPAWN_H
