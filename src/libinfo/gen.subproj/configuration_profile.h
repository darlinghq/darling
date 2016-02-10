/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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

#ifndef __CONFIGURATION_PROFILE_H__
#define __CONFIGURATION_PROFILE_H__

#include <xpc/xpc.h>
#include <Availability.h>

char *configuration_profile_create_notification_key(const char *ident) __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_7_0);
xpc_object_t configuration_profile_copy_property_list(const char *ident) __OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_7_0);

#endif /* __CONFIGURATION_PROFILE_H__ */
