/*
 * Copyright (c) 2012-2014 Apple Inc. All rights reserved.
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

#ifndef _SCPREFS_OBSERVER_H
#define _SCPREFS_OBSERVER_H

#include <Availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <dispatch/dispatch.h>

typedef enum {
#if	!TARGET_OS_IPHONE
		scprefs_observer_type_mcx	= 1,
#else	// !TARGET_OS_IPHONE
		scprefs_observer_type_global	= 2,
		scprefs_observer_type_profile	= 3,
#endif	// !TARGET_OS_IPHONE
} _scprefs_observer_type;

typedef struct _scprefs_observer_t * scprefs_observer_t;

__BEGIN_DECLS

/*!
 @function prefs_observer_watch
 @discussion Sends a notification to interested configuration agents
 when a particular preference file has changed.
 @param type the type of preference (MCX on OSX, Global/Profiles on iOS) to watch.
 @param plist the name of the plist file to watch.
 @param queue the queue to be called back on.
 @param block the block to be called back on.
 @result Returns the created preferences observer
 */
scprefs_observer_t
_scprefs_observer_watch(_scprefs_observer_type type, const char *plist_name,
			dispatch_queue_t queue, dispatch_block_t block);

/*!
 @function prefs_observer_watch
 @discussion Cancells/deregisters the given preferences watcher.
 @param observer the watcher to be cancelled.
 */
void
_scprefs_observer_cancel(scprefs_observer_t observer);

__END_DECLS

#endif /* _SCPREFS_OBSERVER_H */
