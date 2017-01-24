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
#include <mach/mach_time.h>
#include <sys/errno.h>
#include <stdlib.h>

struct work_interval {
	uint64_t thread_id;
	uint64_t work_interval_id;
};

extern uint64_t __thread_selfid(void);

/* Create a new work interval handle (currently for the current thread only). Flags is unused */
int
work_interval_create(work_interval_t *interval_handle, uint32_t flags __unused)
{
	int ret;
	uint64_t work_interval_id;
	work_interval_t handle;

	ret = __work_interval_ctl(WORK_INTERVAL_OPERATION_CREATE, 0, &work_interval_id, sizeof(work_interval_id));
	if (ret == -1) {
		return ret;
	}

	handle = malloc(sizeof(*handle));
	if (handle == NULL) {
		errno = ENOMEM;
		return -1;
	}

	handle->thread_id = __thread_selfid();
	handle->work_interval_id = work_interval_id;

	*interval_handle = handle;
	return 0;
}

int
work_interval_notify(work_interval_t interval_handle, uint64_t start, uint64_t finish, uint64_t deadline, uint64_t next_start, uint32_t flags)
{
	int ret;
	uint64_t work_interval_id;
	struct work_interval_notification notification = {
		.start = start,
		.finish = finish,
		.deadline = deadline,
		.next_start = next_start,
		.flags = flags,
		.unused1 = 0
	};

	if (interval_handle == NULL) {
		errno = EINVAL;
		return -1;
	}

	work_interval_id = interval_handle->work_interval_id;

	ret = __work_interval_ctl(WORK_INTERVAL_OPERATION_NOTIFY, work_interval_id, &notification, sizeof(notification));
	return ret;
}

int
work_interval_notify_simple(work_interval_t interval_handle, uint64_t start, uint64_t deadline, uint64_t next_start)
{
	return work_interval_notify(interval_handle, start, mach_absolute_time(), deadline, next_start, 0);
}

int
work_interval_destroy(work_interval_t interval_handle)
{
	int ret, saved_errno;
	uint64_t work_interval_id;

	if (interval_handle == NULL) {
		errno = EINVAL;
		return -1;
	}

	work_interval_id = interval_handle->work_interval_id;

	ret = __work_interval_ctl(WORK_INTERVAL_OPERATION_DESTROY, work_interval_id, NULL, 0);
	saved_errno = errno;
	free(interval_handle);
	errno = saved_errno;

	return ret;
}
