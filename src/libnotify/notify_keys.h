/*
 * Copyright (c) 2007-2009 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Portions Copyright (c) 2007-2009 Apple Inc.  All Rights Reserved.
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

/*
 * This file lists notification keys that are posted using the 
 * notify_post() API by various Mac OS X system services.
 * The exact circumstances under which services post these
 * notifications is controlled by those services, and may change
 * in future software releases.
 */

/*
 * Directory Service notifications
 * These are posted by the DirectoryService daemon to advise clients that
 * cached data should be invalidated.
 */
#define kNotifyDSCacheInvalidation "com.apple.system.DirectoryService.InvalidateCache"
#define kNotifyDSCacheInvalidationGroup "com.apple.system.DirectoryService.InvalidateCache.group"
#define kNotifyDSCacheInvalidationHost "com.apple.system.DirectoryService.InvalidateCache.host"
#define kNotifyDSCacheInvalidationService "com.apple.system.DirectoryService.InvalidateCache.service"
#define kNotifyDSCacheInvalidationUser "com.apple.system.DirectoryService.InvalidateCache.user"

/*
 * File System notifications
 * These advise clients of various filesystem events.
 */
#define kNotifyVFSMount "com.apple.system.kernel.mount"
#define kNotifyVFSUnmount "com.apple.system.kernel.unmount"
#define kNotifyVFSUpdate "com.apple.system.kernel.mountupdate"
#define kNotifyVFSLowDiskSpace "com.apple.system.lowdiskspace"
#define kNotifyVFSLowDiskSpaceRootFS "com.apple.system.lowdiskspace.system"
#define kNotifyVFSLowDiskSpaceOtherFS "com.apple.system.lowdiskspace.user"

/*
 * System Configuration notifications
 * These advise clients of changes in the system configuration
 * managed by the system configuration server (configd).
 * Note that a much richer set of notifications are available to
 * clients using the SCDynamicStore API.
 */
#define kNotifySCHostNameChange "com.apple.system.hostname"
#define kNotifySCNetworkChange "com.apple.system.config.network_change"

/*
 * ASL notifications
 * Sent by syslogd to advise clients that new log messages have been
 * added to the ASL database.
 */
#define kNotifyASLDBUpdate "com.apple.system.logger.message"

/*
 * Time Zone change notification
 * Sent by notifyd when the system's timezone changes.
 */
#define kNotifyTimeZoneChange "com.apple.system.timezone"

/*
 * System clock change notification
 * Sent when a process modifies the system clock using the settimeofday system call.
 */
#define kNotifyClockSet "com.apple.system.clock_set"
