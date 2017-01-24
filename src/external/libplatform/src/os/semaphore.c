/*
 * Copyright (c) 2008-2013 Apple Inc. All rights reserved.
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

#include "os/internal.h"
#include <mach/mach_init.h>
#include <mach/semaphore.h>
#include <mach/task.h>
#include <mach/thread_switch.h>

#define OS_VERIFY_MIG(x, msg) do { \
		if (unlikely((x) == MIG_REPLY_MISMATCH)) { \
			__LIBPLATFORM_CLIENT_CRASH__(x, msg); \
		} \
	} while (0)

#define OS_SEMAPHORE_VERIFY_KR(x, msg) do { \
		if (unlikely(x)) { \
			__LIBPLATFORM_CLIENT_CRASH__(x, msg); \
		} \
	} while (0)

os_semaphore_t
_os_semaphore_create(void)
{
	semaphore_t s4;
	kern_return_t kr;
	while (unlikely(kr = semaphore_create(mach_task_self(), &s4,
			SYNC_POLICY_FIFO, 0))) {
		OS_VERIFY_MIG(kr, "Allocating semaphore failed with MIG_REPLY_MISMATCH");
		thread_switch(MACH_PORT_NULL, SWITCH_OPTION_WAIT, 100);
	}
	return (os_semaphore_t)s4;
}

void
_os_semaphore_dispose(os_semaphore_t sema)
{
	semaphore_t s4 = (semaphore_t)sema;
	kern_return_t kr = semaphore_destroy(mach_task_self(), s4);
	OS_SEMAPHORE_VERIFY_KR(kr, "Destroying semaphore failed");
}

void
_os_semaphore_signal(os_semaphore_t sema)
{
	semaphore_t s4 = (semaphore_t)sema;
	kern_return_t kr = semaphore_signal(s4);
	OS_SEMAPHORE_VERIFY_KR(kr, "Signaling semaphore failed");
}

void
_os_semaphore_wait(os_semaphore_t sema)
{
	semaphore_t s4 = (semaphore_t)sema;
	kern_return_t kr;
	do {
		kr = semaphore_wait(s4);
	} while (unlikely(kr == KERN_ABORTED));
	OS_SEMAPHORE_VERIFY_KR(kr, "Waiting on semaphore failed");
}
