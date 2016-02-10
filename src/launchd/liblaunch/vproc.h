/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#ifndef __VPROC_H__
#define __VPROC_H__

#include <sys/cdefs.h>
#include <sys/types.h>
#include <Availability.h>

#ifndef VPROC_HAS_TRANSACTIONS
	#define VPROC_HAS_TRANSACTIONS
#endif

__BEGIN_DECLS

#pragma GCC visibility push(default)

typedef void * vproc_err_t;

typedef struct vproc_s * vproc_t;
typedef void * vprocmgr_t;

const char *vproc_strerror(vproc_err_t r);

/*!
 * @header      vproc
 *
 * Processes have two reference counts associated with them:
 *
 * Transactions Tracks unfinished work. For example: saving a modified
 *		document.
 * Standby	Tracks outstanding callbacks from external subsystems.
 *
 * Descriptive aliases:
 *
 * A process with no outstanding transactions is called "clean."
 * A process with outstanding transactions is called "dirty."
 * A process with no standby work is called "idle."
 *
 * Sometimes, the operating system needs processes to exit. Unix has two
 * primary signals to kill applications:
 *
 * SIGKILL	Not catchable by the application.
 * SIGTERM	Catchable by the application.
 *
 * If a process is clean, the operating system is free to SIGKILL it at
 * shutdown or logout. This behavior is opt in.
 *
 * If a process is clean and idle, the operating system may send SIGKILL after
 * a application specified timeout. This behavior is opt in.
 *
 * If a process is dirty and idle, the operating system may send SIGTERM after
 * a application specified timeout. This behavior is opt in.
 *
 *
 * launchd jobs should update their property lists accordingly.
 *
 * We plan to have LaunchServices use private methods to coordinate
 * whether GUI applications have opted into this design.
 */

/*!
 * @typedef	vproc_transaction_t
 *
 * @abstract
 * An opaque handle used to track outstanding transactions.
 */
typedef struct vproc_transaction_s *vproc_transaction_t;

/*!
 * @function vproc_transaction_begin
 *
 * @param	virtual_proc
 * This is meant for future API improvements. Pass NULL for now.
 *
 * @result
 * Returns an opaque handle to be passed to vproc_transaction_end().
 *
 * @abstract
 * Call this API before creating data that needs to be saved via I/O later.
 */
vproc_transaction_t
vproc_transaction_begin(vproc_t virtual_proc) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_5_0);

/*!
 * @function vproc_transaction_end
 *
 * @param	virtual_proc
 * This is meant for future API improvements. Pass NULL for now.
 *
 * @param	handle
 * The handle previously created with vproc_transaction_begin().
 *
 * @abstract
 * Call this API after the data has either been flushed or otherwise resolved.
 *
 * @discussion
 * Calling this API with the same handle more than once is undefined.
 */
void
vproc_transaction_end(vproc_t virtual_proc, vproc_transaction_t handle) __OSX_AVAILABLE_STARTING(__MAC_10_6, __IPHONE_5_0);

/*!
 * @typedef	vproc_standby_t
 *
 * @abstract
 * An opaque handle used to track outstanding standby requests.
 */
typedef struct vproc_standby_s *vproc_standby_t;

/*!
 * @function vproc_standby_begin
 *
 * @param	virtual_proc
 * This is meant for future API improvements. Pass NULL for now.
 *
 * @result
 * Returns an opaque handle to be passed to vproc_standby_end().
 *
 * @abstract
 * Call this API before registering notifications. For example: timers network
 * state change, or when monitoring keyboard/mouse events.
 * 
 * @discussion
 * This API is undefined and is currently a no-op.
 */
vproc_standby_t
vproc_standby_begin(vproc_t virtual_proc) __OSX_AVAILABLE_STARTING(__MAC_NA, __IPHONE_NA);

/*!
 * @function vproc_standby_end
 *
 * @param	virtual_proc
 * This is meant for future API improvements. Pass NULL for now.
 *
 * @param	handle
 * The handle previously created with vproc_standby_begin().
 *
 * @abstract
 * Call this API when deregistering notifications.
 *
 * @discussion
 * Calling this API with the same handle more than once is undefined.
 * This API is undefined and is currently a no-op.
 */
void
vproc_standby_end(vproc_t virtual_proc, vproc_standby_t handle) __OSX_AVAILABLE_STARTING(__MAC_NA, __IPHONE_NA);

#pragma GCC visibility pop

__END_DECLS

#endif /* __VPROC_H__ */
