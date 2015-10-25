/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#include <errno.h>
#include <libkern/OSMemoryNotification.h>
#include <mach/mach.h>
#include <notify.h>
#include <stdlib.h>
#include <sys/time.h>
#include <dispatch/dispatch.h>
#include <dispatch/private.h>

const char *kOSMemoryNotificationName = "com.apple.system.memorystatus";

struct _OSMemoryNotification {
	int token;
	mach_port_t port;
};

int
OSMemoryNotificationCreate(OSMemoryNotificationRef *note)
{
	OSMemoryNotificationRef ref = malloc(sizeof(struct _OSMemoryNotification));

	if (NULL == ref) {
		return ENOMEM;
	}
	
	if (NOTIFY_STATUS_OK != notify_register_mach_port(kOSMemoryNotificationName, &ref->port, 0, &ref->token))
		return ENOMEM;

	*note = ref;

	return 0;
}

int
OSMemoryNotificationDestroy(OSMemoryNotificationRef note)
{
	if (NOTIFY_STATUS_OK != notify_cancel(note->token))
		return EINVAL;

	if (KERN_SUCCESS != mach_port_deallocate(mach_task_self(), note->port))
		return EINVAL;

	free(note);

	return 0;
}
		
int
OSMemoryNotificationWait(OSMemoryNotificationRef note, OSMemoryNotificationLevel *level)
{
	return OSMemoryNotificationTimedWait(note, level, NULL);
}

static mach_msg_timeout_t
abs_to_timeout(const struct timeval *abstime)
{
	struct timeval rel, now;

	gettimeofday(&now, NULL);

	rel.tv_usec = abstime->tv_usec - now.tv_usec;
	rel.tv_sec = abstime->tv_sec - now.tv_sec;
	if (rel.tv_usec < 0) {
		rel.tv_usec += 1000000;
		rel.tv_sec--;
	}
	if (((int)rel.tv_sec < 0) || ((rel.tv_sec == 0) && (rel.tv_usec == 0))) {
		return 0;
	}
	return rel.tv_sec * 1000 + rel.tv_usec / 1000;
}

int
OSMemoryNotificationTimedWait(OSMemoryNotificationRef note, OSMemoryNotificationLevel *level, const struct timeval *abstime)
{
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);
    dispatch_retain(sema);
    
    dispatch_source_t memoryNotificationSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_VM, 0, DISPATCH_VM_PRESSURE, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0));
    dispatch_source_set_event_handler(memoryNotificationSource, ^{
        dispatch_semaphore_signal(sema);
        dispatch_release(sema);
    });

    dispatch_resume(memoryNotificationSource);
    
    dispatch_time_t waitTime = DISPATCH_TIME_FOREVER;
    if (abstime) {
        mach_msg_timeout_t timeout = abs_to_timeout(abstime);
        waitTime = dispatch_time(DISPATCH_TIME_NOW, timeout * 1e6);
    }
    
    long ret = dispatch_semaphore_wait(sema, waitTime);

    dispatch_release(sema);
    dispatch_release(memoryNotificationSource);

    if (ret != 0) {
        return ETIMEDOUT;
    }
    
    if (level) {
        *level = OSMemoryNotificationLevelUrgent;
    }
    
    return 0;
}

OSMemoryNotificationLevel OSMemoryNotificationCurrentLevel(void)
{
	return OSMemoryNotificationLevelNormal;
}
