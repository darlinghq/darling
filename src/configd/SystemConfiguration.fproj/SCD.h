/*
 * Copyright (c) 2011, 2016-2018 Apple Inc. All rights reserved.
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

#ifndef _SCD_H
#define _SCD_H

#include <os/availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <SystemConfiguration/SCDynamicStore.h>


typedef struct {
	int			_sc_error;
	SCDynamicStoreRef	_sc_store;
} __SCThreadSpecificData, *__SCThreadSpecificDataRef;


__BEGIN_DECLS


#pragma mark -
#pragma mark [p]thread specific data


__SCThreadSpecificDataRef
__SCGetThreadSpecificData		(void);


#pragma mark -
#pragma mark ScheduleWithRunLoop/UnscheduleFromRunLoop


/*
 * object / CFRunLoop  management
 */
void
_SC_signalRunLoop			(CFTypeRef		obj,
					 CFRunLoopSourceRef     rls,
					 CFArrayRef		rlList);

Boolean
_SC_isScheduled				(CFTypeRef		obj,
					 CFRunLoopRef		runLoop,
					 CFStringRef		runLoopMode,
					 CFMutableArrayRef      rlList);

void
_SC_schedule				(CFTypeRef		obj,
					 CFRunLoopRef		runLoop,
					 CFStringRef		runLoopMode,
					 CFMutableArrayRef      rlList);

Boolean
_SC_unschedule				(CFTypeRef		obj,
					 CFRunLoopRef		runLoop,
					 CFStringRef		runLoopMode,
					 CFMutableArrayRef      rlList,
					 Boolean		all);


#pragma mark -
#pragma mark Misc


char *
_SC_cfstring_to_cstring_ext		(CFStringRef		cfstr,
					 char			*buf,
					 CFIndex		bufLen,
					 CFStringEncoding	encoding,
					 UInt8			lossByte,
					 CFIndex		*usedBufLen);

__END_DECLS

#endif	/* _SCD_H */
