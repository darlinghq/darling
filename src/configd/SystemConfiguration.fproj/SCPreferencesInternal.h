/*
 * Copyright (c) 2000, 2001, 2003-2005, 2007-2011, 2013, 2014 Apple Inc. All rights reserved.
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

#ifndef _SCPREFERENCESINTERNAL_H
#define _SCPREFERENCESINTERNAL_H

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <SystemConfiguration/SCPreferences.h>
#include <SystemConfiguration/SCDynamicStore.h>
#include <dispatch/dispatch.h>


#define	PREFS_DEFAULT_DIR		CFSTR("/Library/Preferences/SystemConfiguration")
#define	PREFS_DEFAULT_CONFIG		CFSTR("preferences.plist")

#define	PREFS_DEFAULT_DIR_OLD		CFSTR("/var/db/SystemConfiguration")
#define	PREFS_DEFAULT_CONFIG_OLD	CFSTR("preferences.xml")

#define	PREFS_DEFAULT_USER_DIR		CFSTR("Library/Preferences")

#define	NETWORK_INTERFACES_PREFS	CFSTR("NetworkInterfaces.plist")
#define	INTERFACES			CFSTR("Interfaces")


/* Define the per-preference-handle structure */
typedef struct {

	/* base CFType information */
	CFRuntimeBase		cfBase;

	/* lock */
	pthread_mutex_t		lock;

	/* session name */
	CFStringRef		name;

	/* preferences ID */
	CFStringRef		prefsID;

	/* options */
	CFDictionaryRef		options;

	/* configuration file */
	char			*path;
	char			*newPath;

	/* preferences lock, lock file */
	Boolean			locked;
	int			lockFD;
	char			*lockPath;
	struct timeval		lockTime;

	/* configuration file signature */
	CFDataRef		signature;

	/* configd session */
	SCDynamicStoreRef	session;

	/* configd session keys */
	CFStringRef		sessionKeyLock;
	CFStringRef		sessionKeyCommit;
	CFStringRef		sessionKeyApply;

	/* run loop source, callout, context, rl scheduling info */
	Boolean			scheduled;
	CFRunLoopSourceRef      rls;
	SCPreferencesCallBack	rlsFunction;
	SCPreferencesContext	rlsContext;
	CFMutableArrayRef       rlList;
	dispatch_queue_t	dispatchQueue;		// SCPreferencesSetDispatchQueue

	/* preferences */
	CFMutableDictionaryRef	prefs;

	/* flags */
	Boolean			accessed;
	Boolean			changed;
	Boolean			isRoot;
	Boolean			limit_SCNetworkConfiguration;

	/* authorization, helper */
	CFDataRef		authorizationData;
	Boolean			authorizationRequired;
	mach_port_t		helper_port;

} SCPreferencesPrivate, *SCPreferencesPrivateRef;


/* Define signature data */
typedef struct {
	int64_t		st_dev;		/* inode's device */
	uint64_t	st_ino;		/* inode's number */
	uint64_t	tv_sec;		/* time of last data modification */
	uint64_t	tv_nsec;
	off_t		st_size;	/* file size, in bytes */
} SCPSignatureData, *SCPSignatureDataRef;


__BEGIN_DECLS

Boolean
__SCPreferencesCreate_helper		(SCPreferencesRef	prefs);

void
__SCPreferencesAccess			(SCPreferencesRef	prefs);

Boolean
__SCPreferencesAddSession		(SCPreferencesRef       prefs);

CF_RETURNS_RETAINED
CFDataRef
__SCPSignatureFromStatbuf		(const struct stat	*statBuf);

char *
__SCPreferencesPath			(CFAllocatorRef		allocator,
					 CFStringRef		prefsID,
					 Boolean		useNewPrefs);

CF_RETURNS_RETAINED
CFStringRef
_SCPNotificationKey			(CFAllocatorRef		allocator,
					 CFStringRef		prefsID,
					 int			keyType);
Boolean
__SCPreferencesGetLimitSCNetworkConfiguration	(SCPreferencesRef prefs);

void
__SCPreferencesSetLimitSCNetworkConfiguration
					(SCPreferencesRef	prefs,
					 Boolean		limit_SCNetworkConfiguration);

Boolean
__SCPreferencesUsingDefaultPrefs		(SCPreferencesRef	prefs);

SCPreferencesRef
__SCPreferencesCreateNIPrefsFromPrefs	(SCPreferencesRef prefs);

__END_DECLS

#endif /* _SCPREFERENCESINTERNAL_H */
