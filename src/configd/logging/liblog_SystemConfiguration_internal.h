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

#ifndef	_LIBLOG_SYSTEMCONFIGURATION_INTERNAL_H
#define	_LIBLOG_SYSTEMCONFIGURATION_INTERNAL_H

#include <os/availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <os/log_private.h>
#include <os/state_private.h>

// <os/log_private.h>
#ifdef __OBJC__
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSAttributedString.h>
typedef NSAttributedString *(*os_log_copy_formatted_fn_t)(const char *type,
							  id value, os_log_type_info_t info);
OS_EXPORT NS_RETURNS_RETAINED
NSAttributedString *
OSLogCopyFormattedString(const char *type, id value, os_log_type_info_t info);
#endif

// <os/state_private.h>
#ifdef __OBJC__
#import <Foundation/NSString.h>
typedef NSString *
(*os_state_create_string_fn_t)(const char *data_type,
			       uint32_t data_size, void *data);
OS_EXPORT NS_RETURNS_RETAINED
NSString *
OSStateCreateStringWithData(const char *data_type, uint32_t data_size, void *data);
#endif

__BEGIN_DECLS

__END_DECLS

#endif	// _LIBLOG_SYSTEMCONFIGURATION_INTERNAL_H
