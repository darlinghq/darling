/*
 * Copyright (c) 2009-2010 Apple Inc. All rights reserved.
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

#ifndef __NOTIFY_PRIVATE_H__
#define __NOTIFY_PRIVATE_H__

#include <stdint.h>
#include <sys/types.h>
#include <os/base.h>
#include <Availability.h>

#define NOTIFY_OPT_DISPATCH 0x00000001
#define NOTIFY_OPT_REGEN    0x00000002
#define NOTIFY_OPT_ENABLE   0x04000000
#define NOTIFY_OPT_DISABLE  0x08000000

#define NOTIFY_NO_DISPATCH  0x80000000

#define ROOT_ENTITLEMENT_KEY "com.apple.notify.root_access"

OS_EXPORT uint32_t notify_suspend_pid(pid_t pid)
__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0);

OS_EXPORT uint32_t notify_resume_pid(pid_t pid)
__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_0);

OS_EXPORT uint32_t notify_simple_post(const char *name)
__API_DEPRECATED("No longer supported, use notify_post", macos(10.7, 10.15), ios(4.3, 13.0), watchos(1.0, 6.0), tvos(1.0, 13.0));

OS_EXPORT void notify_set_options(uint32_t opts)
__OSX_AVAILABLE_STARTING(__MAC_10_8,__IPHONE_6_0);

OS_EXPORT void _notify_fork_child(void)
__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_3);

OS_EXPORT uint32_t notify_peek(int token, uint32_t *val)
__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_3);

OS_EXPORT uint32_t notify_monitor_file(int token, char *path, int flags)
__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_3);

OS_EXPORT uint32_t notify_get_event(int token, int *ev, char *buf, int *len)
__API_DEPRECATED("No longer supported", macos(10.7, 10.15), ios(4.3, 13.0), watchos(1.0, 6.0), tvos(1.0, 13.0));

OS_EXPORT uint32_t notify_register_plain(const char *name, int *out_token)
__OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_4_3);

OS_EXPORT uint32_t notify_dump_status(const char *filepath);

#endif /* __NOTIFY_PRIVATE_H__ */
