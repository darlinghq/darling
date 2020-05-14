/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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

/*!
 * @header
 * Darwin-specific interfaces for manipulating Mach exception handling
 * properties of a task.
 */
#ifndef __DARWIN_EXCEPTION_H
#define __DARWIN_EXCEPTION_H

#include <os/base.h>
#include <os/api.h>

#include <sys/cdefs.h>
#include <spawn.h>

#include <mach/mach.h>
#include <mach/port.h>
#include <mach/mach_port.h>
#include <mach/kern_return.h>

#if DARWIN_TAPI
#include "tapi.h"
#endif

__BEGIN_DECLS;

/*!
 * @typedef os_crash_flags_t
 * A flagset describing crash handling behaviors.
 *
 * @param OS_CRASH_FLAG_INIT
 * Handle no exceptions.
 *
 * @param OS_CRASH_FLAG_CRASH
 * Handle exceptions due to crashing conditions (e.g. segmentation violations,
 * signals raise(3)ed manually, etc.). These exceptions will suspend the subject
 * task until the exception handler has returned.
 *
 * @param OS_CRASH_FLAG_GUARD
 * Handle exceptions due to the subject task violating guarded kernel handles
 * (e.g. guarded mach port rights and file descriptors).
 *
 * @param OS_CRASH_FLAG_RESOURCE
 * Handle exceptions due to the subject task exceeding resource usage limits
 * (e.g. CPU spins, memory growth, etc.).
 *
 * @param OS_CRASH_FLAG_CORPSE
 * Handle exceptions which create corpses. The subject task of these exceptions
 * is a corpse of the original task, which is torn down asynchronously. Corpses
 * are a limited representation of the original task that is suitable for most
 * introspection needs.
 *
 * This flag is mutually exclusive with {@link OS_CRASH_FLAG_CRASH}, and if both
 * are present, this flag will be preferred.
 *
 * @discussion
 * There are other Mach exception types than the ones enumerated by these flags,
 * but they are almost exclusively handled internally by the kernel, and
 * therefore are of little interest to userspace. Those that are not handled by
 * the kernel are only relevant to debuggers.
 */
OS_ENUM(os_crash_flags, uint32_t,
	OS_CRASH_FLAG_INIT = 0,
	OS_CRASH_FLAG_CRASH = (1 << 0),
	OS_CRASH_FLAG_GUARD = (1 << 1),
	OS_CRASH_FLAG_RESOURCE = (1 << 2),
	OS_CRASH_FLAG_CORPSE = (1 << 3),
);

/*!
 * @enum os_crash_type_t
 * A type describing exception types relevant to userspace crash reporters.
 * These values serve as indexes into a {@link os_crash_port_array_t}.
 *
 * @const OS_CRASH_NONE
 * No exception type.
 *
 * @const OS_CRASH_CRASH
 * A crashing exception with the behavior described in {@link OS_CRASH_FLAG_CRASH}.
 *
 * @const OS_CRASH_GUARD
 * A guard exception with the behavior described in {@link OS_CRASH_FLAG_GUARD}.
 *
 * @const OS_CRASH_RESOURCE
 * A resource exception with the behavior described in {@link OS_CRASH_FLAG_RESOURCE}.
 *
 * @const OS_CRASH_CORPSE
 * A corpse exception with the behavior described in {@link OS_CRASH_FLAG_CORPSE}.
 */
OS_ENUM(os_crash_type, uint32_t,
	OS_CRASH_NONE,
	OS_CRASH_CRASH,
	OS_CRASH_GUARD,
	OS_CRASH_RESOURCE,
	OS_CRASH_CORPSE,
	_OS_CRASH_LAST,
);

/*!
 * struct os_crash_port_t
 * A type describing an exception port and the exception it handles.
 *
 * @property oep_type
 * The type of exception handled by the port.
 *
 * @property oep_port
 * A handle representing a send right to the exception port.
 */
DARWIN_API_AVAILABLE_20170407
typedef struct _os_crash_port {
	os_crash_type_t oep_type;
	mach_port_t oep_port;
} os_crash_port_t;

/*!
 * @define DARWIN_EXCEPTION_PORT_ARRAY_COUNT
 * The maximum number of exception ports that an exception port array can
 * accommodate.
 */
#define OS_CRASH_PORT_ARRAY_COUNT (16lu)

/*!
 * @typedef os_crash_port_array_t
 * An array of exception ports. This array can accommodate all exception types
 * described in the {@link os_crash_type_t} type and is sized to
 * accommodate future exception types.
 */
DARWIN_API_AVAILABLE_20170407
typedef os_crash_port_t os_crash_port_array_t[OS_CRASH_PORT_ARRAY_COUNT];

/*!
 * @function os_crash_set_reporter_port
 * Routine to set the exception handler port for the exceptions given.
 *
 * @param where
 * The task port or host port for which to set the exception handler. This
 * routine will internally choose the proper method of setting the exception
 * port based on whether this parameter represents the host or not.
 *
 * @param flags
 * Flags describing which exceptions are to be handled by the port.
 *
 * @param p
 * A send right to the desired exception handler port.
 *
 * @result
 * A kernel return code.
 *
 * @discussion
 * This routine automatically chooses the most expressive thread state to
 * deliver in the exception message based on the current architecture.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_WARN_RESULT
kern_return_t
os_crash_set_reporter_port(mach_port_t where,
		os_crash_flags_t flags, mach_port_t p);

/*!
 * @function os_crash_get_reporter_port_array
 * Routine to get the ports which handle exceptions for all enumerated exception
 * types in {@link os_crash_flags_t}.
 *
 * @param where
 * The task port or host port for which to retrieve exception handler ports.
 * This routine will internally choose the proper method of obtaining the
 * exception ports based on whether this parameter represents the host or not.
 *
 * @param array
 * An array describing the exception ports for the target host or task. This
 * array must be disposed of with {@link os_crash_port_array_deallocate}
 * when it is no longer needed.
 *
 * @result
 * A kernel return code.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_WARN_RESULT OS_NONNULL2
kern_return_t
os_crash_get_reporter_port_array(mach_port_t where,
		os_crash_port_array_t array);

/*!
 * @function os_crash_port_array_deallocate
 * Routine to deallocate an array of exception port descriptors.
 *
 * @param array
 * The array which is to be disposed of.
 *
 * @discussion
 * This routine disposes of the resources represented by the kernel handles in
 * the array. It does not manipulate the array's memory. The expectation is that
 * the caller allocates {@link os_crash_port_array_t} from the stack.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_NONNULL1
void
os_crash_port_array_deallocate(os_crash_port_array_t array);

/*!
 * @function os_crash_spawnattr_set_reporter_port
 * Routine to set the exception handler port for the exceptions given in a
 * posix_spawn(2) attributes structure.
 *
 * @param psattr
 * The spawn attributes for which the exception port should be set.
 *
 * @param flags
 * Flags describing which exceptions are to be handled by the port.
 *
 * @param p
 * A send right to the desired exception handler port.
 *
 * @result
 * A kernel return code. This routine will only fail if the port name given was
 * invalid.
 */
DARWIN_API_AVAILABLE_20170407
OS_EXPORT OS_WARN_RESULT OS_NONNULL1
kern_return_t
os_crash_spawnattr_set_reporter_port(posix_spawnattr_t *psattr,
		os_crash_flags_t flags, mach_port_t p);

__END_DECLS;

#endif // __DARWIN_EXCEPTION_H
