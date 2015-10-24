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

#ifndef _OSMEMORYNOTIFICATION_H_
#define _OSMEMORYNOTIFICATION_H_

#include <sys/cdefs.h>
#include <Availability.h>

/*
**  OSMemoryNotification.h
**  
**  Kernel-generated notification mechanism to alert registered tasks when physical memory
**  pressure reaches certain thresholds. Notifications are triggered in both directions
**  so clients can manage their memory usage more and less aggressively.
**
**  All of these functions and data types are iOS-only and deprecated in iOS 5.0. Applications
**  should listen to UIKit memory warnings (didReceiveMemoryWarning or
**  UIApplicationDidReceiveMemoryWarningNotification) instead.
*/

__BEGIN_DECLS

struct timeval;

/*
** Opaque type for notification object
**
** DEPRECATED. Use UIKit memory notification system.
*/
 
typedef struct _OSMemoryNotification * OSMemoryNotificationRef;

/*
** Threshold values for notifications
**
** DEPRECATED. Use UIKit memory notification system.
*/
 
typedef enum {
	OSMemoryNotificationLevelAny      = -1,
	OSMemoryNotificationLevelNormal   =  0,
	OSMemoryNotificationLevelWarning  =  1,
	OSMemoryNotificationLevelUrgent   =  2,
	OSMemoryNotificationLevelCritical =  3
} OSMemoryNotificationLevel;

/*
** Creation routines. Returns the created OSMemoryNotificationRef in the note param.
** returns: 0 on success
**          ENOMEM if insufficient memory or resources exists to create the notification object
**          EINVAL if the threshold is not a valid notification level
**
** DEPRECATED. Use UIKit memory notification system.
*/

int OSMemoryNotificationCreate(OSMemoryNotificationRef *note)   __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_NA, __MAC_NA, __IPHONE_2_0, __IPHONE_5_0);

/*
** returns: 0 on success
**          EINVAL if the notification is not an initialized notification object
 **
 ** DEPRECATED. Use UIKit memory notification system.
*/

int OSMemoryNotificationDestroy(OSMemoryNotificationRef note)   __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_NA, __MAC_NA, __IPHONE_2_0, __IPHONE_5_0);

/*
** Block waiting for notification
** returns: 0 on success, with the level that triggered the notification in the level param
**          EINVAL if the notification object is invalid
**          ETIMEDOUT if abstime passes before notification occurs
**
**  'note' is now ignored.
**
** DEPRECATED. Use UIKit memory notification system.
*/
int OSMemoryNotificationWait(OSMemoryNotificationRef note, OSMemoryNotificationLevel *level)    __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_NA, __MAC_NA, __IPHONE_2_0, __IPHONE_5_0);
int OSMemoryNotificationTimedWait(OSMemoryNotificationRef note, OSMemoryNotificationLevel *level, const struct timeval *abstime) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_NA, __MAC_NA, __IPHONE_2_0, __IPHONE_5_0);

/*
** Simple polling interface to detect current memory pressure level
**
** DEPRECATED. This is not a reliable way to discover the system's memory condition and
**             the level is not meaningful in iOS 5.0 and later. Use UIKit memory notification
**             system instead, with no need to check the level.
*/

OSMemoryNotificationLevel OSMemoryNotificationCurrentLevel(void)    __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_NA, __MAC_NA, __IPHONE_2_0, __IPHONE_5_0);

/*
** External notify(3) string for manual notification setup
**
** DEPRECATED. Use UIKit memory notification system.
*/

extern const char *kOSMemoryNotificationName    __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_NA, __MAC_NA, __IPHONE_2_0, __IPHONE_5_0);

__END_DECLS

#endif /* _OSMEMORYNOTIFICATION_H_ */
