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
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/work_interval.h>

#include <mach/mach.h>
#include <mach/mach_time.h>
#include <mach/port.h>

#include <sys/errno.h>
#include <stdlib.h>
#include <strings.h>

struct work_interval {
	uint64_t thread_id;
	uint64_t work_interval_id;
	uint32_t create_flags;
	mach_port_t wi_port;
};

extern uint64_t __thread_selfid(void);

/* Create a new work interval handle (currently for the current thread only). */
int
work_interval_create(work_interval_t *interval_handle, uint32_t create_flags)
{
	int ret;
	work_interval_t handle;

	if (interval_handle == NULL) {
		errno = EINVAL;
		return -1;
	}

	struct work_interval_create_params create_params = {
		.wicp_create_flags = create_flags,
	};

	ret = __work_interval_ctl(WORK_INTERVAL_OPERATION_CREATE2, 0,
	    &create_params, sizeof(create_params));
	if (ret == -1) {
		return ret;
	}

	handle = malloc(sizeof(*handle));
	if (handle == NULL) {
		errno = ENOMEM;
		return -1;
	}

	handle->thread_id = __thread_selfid();
	handle->work_interval_id = create_params.wicp_id;
	handle->create_flags = create_params.wicp_create_flags;
	handle->wi_port = create_params.wicp_port;

	*interval_handle = handle;
	return 0;
}

int
work_interval_get_flags_from_port(mach_port_t port, uint32_t *flags)
{
	if (!MACH_PORT_VALID(port) || flags == NULL) {
		errno = EINVAL;
		return -1;
	}

	struct work_interval_create_params create_params = { 0 };

	int ret = __work_interval_ctl(WORK_INTERVAL_OPERATION_GET_FLAGS, port,
	    &create_params, sizeof(create_params));
	if (ret == -1) {
		return ret;
	}

	*flags = create_params.wicp_create_flags;
	return 0;
}

int
work_interval_notify(work_interval_t interval_handle, uint64_t start,
    uint64_t finish, uint64_t deadline, uint64_t next_start,
    uint32_t notify_flags)
{
	int ret;
	uint64_t work_interval_id;
	struct work_interval_notification notification = {
		.start = start,
		.finish = finish,
		.deadline = deadline,
		.next_start = next_start,
		.notify_flags = notify_flags
	};

	if (interval_handle == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (interval_handle->create_flags & WORK_INTERVAL_FLAG_IGNORED) {
		return 0;
	}

	notification.create_flags = interval_handle->create_flags;
	work_interval_id = interval_handle->work_interval_id;

	ret = __work_interval_ctl(WORK_INTERVAL_OPERATION_NOTIFY, work_interval_id,
	    &notification, sizeof(notification));
	return ret;
}

int
work_interval_notify_simple(work_interval_t interval_handle, uint64_t start,
    uint64_t deadline, uint64_t next_start)
{
	return work_interval_notify(interval_handle, start, mach_absolute_time(),
	           deadline, next_start, 0);
}


int
work_interval_destroy(work_interval_t interval_handle)
{
	if (interval_handle == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (interval_handle->create_flags & WORK_INTERVAL_FLAG_JOINABLE) {
		mach_port_t wi_port = interval_handle->wi_port;

		/*
		 * A joinable work interval's lifetime is tied to the port lifetime.
		 * When the last port reference is destroyed, the work interval
		 * is destroyed via no-senders notification.
		 *
		 * Note however that after destroy it can no longer be notified
		 * because the userspace token is gone.
		 *
		 * Additionally, this function does not cause the thread to un-join
		 * the interval.
		 */
		kern_return_t kr = mach_port_deallocate(mach_task_self(), wi_port);

		if (kr != KERN_SUCCESS) {
			/*
			 * If the deallocate fails, then someone got their port
			 * lifecycle wrong and over-released a port right.
			 *
			 * Return an error so the client can assert on this,
			 * and still find the port name in the interval handle.
			 */
			errno = EINVAL;
			return -1;
		}

		interval_handle->wi_port = MACH_PORT_NULL;
		interval_handle->work_interval_id = 0;

		free(interval_handle);
		return 0;
	} else {
		uint64_t work_interval_id = interval_handle->work_interval_id;

		int ret = __work_interval_ctl(WORK_INTERVAL_OPERATION_DESTROY,
		    work_interval_id, NULL, 0);

		interval_handle->work_interval_id = 0;

		int saved_errno = errno;
		free(interval_handle);
		errno = saved_errno;
		return ret;
	}
}

int
work_interval_join(work_interval_t interval_handle)
{
	if (interval_handle == NULL) {
		errno = EINVAL;
		return -1;
	}

	if ((interval_handle->create_flags & WORK_INTERVAL_FLAG_JOINABLE) == 0) {
		errno = EINVAL;
		return -1;
	}

	mach_port_t wi_port = interval_handle->wi_port;

	if (!MACH_PORT_VALID(wi_port)) {
		errno = EINVAL;
		return -1;
	}

	return work_interval_join_port(wi_port);
}

int
work_interval_join_port(mach_port_t port)
{
	if (port == MACH_PORT_NULL) {
		errno = EINVAL;
		return -1;
	}

	return __work_interval_ctl(WORK_INTERVAL_OPERATION_JOIN,
	           (uint64_t)port, NULL, 0);
}

int
work_interval_leave(void)
{
	return __work_interval_ctl(WORK_INTERVAL_OPERATION_JOIN,
	           (uint64_t)MACH_PORT_NULL, NULL, 0);
}

int
work_interval_copy_port(work_interval_t interval_handle, mach_port_t *port)
{
	if (port == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (interval_handle == NULL) {
		*port = MACH_PORT_NULL;
		errno = EINVAL;
		return -1;
	}

	if ((interval_handle->create_flags & WORK_INTERVAL_FLAG_JOINABLE) == 0) {
		*port = MACH_PORT_NULL;
		errno = EINVAL;
		return -1;
	}

	mach_port_t wi_port = interval_handle->wi_port;

	kern_return_t kr = mach_port_mod_refs(mach_task_self(), wi_port,
	    MACH_PORT_RIGHT_SEND, 1);

	if (kr != KERN_SUCCESS) {
		*port = MACH_PORT_NULL;
		errno = EINVAL;
		return -1;
	}

	*port = wi_port;

	return 0;
}
