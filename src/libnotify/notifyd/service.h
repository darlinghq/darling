/*
 * Copyright (c) 2003-2010 Apple Inc. All rights reserved.
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

#ifndef _NOTIFY_SERVICE_H_
#define _NOTIFY_SERVICE_H_

#define SERVICE_TYPE_NONE 0
#define SERVICE_TYPE_PATH_PUBLIC 1
#define SERVICE_TYPE_PATH_PRIVATE 2
#define SERVICE_TYPE_TIMER_PUBLIC 3
#define SERVICE_TYPE_TIMER_PRIVATE 4

#define SERVICE_PREFIX "com.apple.system.notify.service."
#define SERVICE_PREFIX_LEN 32

typedef struct
{
	uint32_t type;
	void *private;
} svc_info_t;

int service_open(const char *name, client_t *client, uint32_t uid, uint32_t gid);
int service_open_path(const char *name, const char *path, uid_t uid, gid_t gid);
int service_open_path_private(const char *name, client_t *client, const char *path, uid_t uid, gid_t gid, uint32_t flags);
int service_open_timer(const char *name, const char *args);
int service_open_timer_private(const char *name, client_t *client, const char *args);
void service_close(svc_info_t *info);

#endif /* _NOTIFY_SERVICE_H_ */
