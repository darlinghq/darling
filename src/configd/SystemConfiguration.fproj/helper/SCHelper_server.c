/*
 * Copyright (c) 2005-2014 Apple Inc. All rights reserved.
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

#include <getopt.h>
#include <grp.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <bsm/libbsm.h>
#include <servers/bootstrap.h>
#include <sys/types.h>
#include <sysexits.h>

//#define DEBUG_MACH_PORT_ALLOCATIONS

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <Security/Security.h>
#include <Security/SecTask.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include <SystemConfiguration/SCValidation.h>

#include "SCPreferencesInternal.h"
#include "SCHelper_client.h"
#include "helper_types.h"

#pragma mark -
#pragma mark SCHelper session managment


//
// entitlement used to control write access to a given "prefsID"
//
#define	kSCReadEntitlementName		CFSTR("com.apple.SystemConfiguration.SCPreferences-read-access")
#define	kSCWriteEntitlementName		CFSTR("com.apple.SystemConfiguration.SCPreferences-write-access")

//
// entitlement used to allow limited [VPN configuration] write access to the "preferences.plist"
//
#define kSCVPNFilterEntitlementName	CFSTR("com.apple.networking.vpn.configuration")

typedef	enum { NO = 0, YES, UNKNOWN } lazyBoolean;

typedef const struct __SCHelperSession * SCHelperSessionRef;

typedef struct {

	// base CFType information
	CFRuntimeBase		cfBase;

	// per session lock
	pthread_mutex_t		lock;

	// authorization
	AuthorizationRef	authorization;
	Boolean			use_entitlement;

	// session mach port
	mach_port_t		port;
	CFMachPortRef		mp;

	// Mach security audit trailer for evaluating credentials
	audit_token_t		auditToken;

	// write access entitlement associated with this session
	lazyBoolean		callerReadAccess;
	lazyBoolean		callerWriteAccess;

	// configuration filtering
	lazyBoolean		isSetChange;	// only network "set" changes
	lazyBoolean		isVPNChange;	// only VPN configuration changes
	CFArrayRef		vpnTypes;

	// preferences
	SCPreferencesRef	prefs;

	/* backtraces */
	CFMutableSetRef		backtraces;

} SCHelperSessionPrivate, *SCHelperSessionPrivateRef;


static CFStringRef	__SCHelperSessionCopyDescription	(CFTypeRef cf);
static void		__SCHelperSessionDeallocate		(CFTypeRef cf);


static void		__SCHelperSessionLogBacktrace		(const void *value, void *context);


static CFTypeID		__kSCHelperSessionTypeID	= _kCFRuntimeNotATypeID;
static Boolean		debug				= FALSE;
static pthread_once_t	initialized			= PTHREAD_ONCE_INIT;
static CFRunLoopRef	main_runLoop			= NULL;
static CFMutableSetRef	sessions			= NULL;
static int		sessions_closed			= 0;	// count of sessions recently closed
static int		sessions_generation		= 0;
static pthread_mutex_t	sessions_lock			= PTHREAD_MUTEX_INITIALIZER;


#pragma mark -
#pragma mark Helper session management


#if	!TARGET_OS_IPHONE
static Boolean
 __SCHelperSessionUseEntitlement(SCHelperSessionRef session)
{
	SCHelperSessionPrivateRef       sessionPrivate  = (SCHelperSessionPrivateRef)session;

	return sessionPrivate->use_entitlement;
}
#endif	//!TARGET_OS_IPHONE


static AuthorizationRef
__SCHelperSessionGetAuthorization(SCHelperSessionRef session)
{
	SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)session;

	return sessionPrivate->authorization;
}


static Boolean
__SCHelperSessionSetAuthorization(SCHelperSessionRef session, CFTypeRef authorizationData)
{
	Boolean				ok		= TRUE;
	SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)session;

	/*
	 * On OSX, the authorizationData can either be a CFData-wrapped/externalized
	 * "authorization" or a CFString indicating that we should check entitlements.
	 *
	 * On iOS, the authorizationData is a CFString indicating that we should
	 * check entitlements.
	 */
	pthread_mutex_lock(&sessionPrivate->lock);

	if (sessionPrivate->authorization != NULL) {
#if	!TARGET_OS_IPHONE
		if (!__SCHelperSessionUseEntitlement(session)) {
			OSStatus	status;

			status = AuthorizationFree(sessionPrivate->authorization, kAuthorizationFlagDefaults);
//			status = AuthorizationFree(sessionPrivate->authorization, kAuthorizationFlagDestroyRights);
			if (status != errAuthorizationSuccess) {
				SCLog(TRUE, LOG_DEBUG,
				      CFSTR("AuthorizationFree() failed: status = %d"),
				      (int)status);
			}
		} else {
			CFRelease(sessionPrivate->authorization);
		}
#else	//!TARGET_OS_IPHONE
		CFRelease(sessionPrivate->authorization);
#endif	//!TARGET_OS_IPHONE
		sessionPrivate->authorization = NULL;
		sessionPrivate->use_entitlement = FALSE;
	}

#if	!TARGET_OS_IPHONE
	if (isA_CFData(authorizationData)) {
		AuthorizationExternalForm	extForm;

		if (CFDataGetLength(authorizationData) == sizeof(extForm.bytes)) {
			OSStatus	status;

			bcopy(CFDataGetBytePtr(authorizationData), extForm.bytes, sizeof(extForm.bytes));
			status = AuthorizationCreateFromExternalForm(&extForm,
								     &sessionPrivate->authorization);
			if (status != errAuthorizationSuccess) {
				SCLog(TRUE, LOG_ERR,
				      CFSTR("AuthorizationCreateFromExternalForm() failed: status = %d"),
				      (int)status);
				sessionPrivate->authorization = NULL;
				ok = FALSE;
			}
		}
	} else if (isA_CFString(authorizationData)) {
		sessionPrivate->authorization = (void *)CFRetain(authorizationData);
		sessionPrivate->use_entitlement = TRUE;
	}
#else	//!TARGET_OS_IPHONE
	if (isA_CFString(authorizationData)) {
		sessionPrivate->authorization = (void *)CFRetain(authorizationData);
		sessionPrivate->use_entitlement = TRUE;
	}
#endif	//!TARGET_OS_IPHONE

	pthread_mutex_unlock(&sessionPrivate->lock);

	return ok;
}


static SCPreferencesRef
__SCHelperSessionGetPreferences(SCHelperSessionRef session)
{
	SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)session;

	return sessionPrivate->prefs;
}


static void
__SCHelperSessionSetThreadName(SCHelperSessionRef session)
{
	char				*caller		= NULL;
	char				name[64];
	char				*path		= NULL;
	char				*path_s		= NULL;
	SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)session;


	if (sessionPrivate->mp == NULL) {
		return;
	}

	if (sessionPrivate->prefs != NULL) {
		SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)sessionPrivate->prefs;

		if (prefsPrivate->name != NULL) {
			caller = _SC_cfstring_to_cstring(prefsPrivate->name,
							 NULL,
							 0,
							 kCFStringEncodingUTF8);
		}

		path = (prefsPrivate->newPath != NULL) ? prefsPrivate->newPath : prefsPrivate->path;
		if (path != NULL) {
			path_s = strrchr(path, '/');
			if (path_s != NULL) {
				path = path_s;
			}
		}
	}

	if (caller != NULL) {
		snprintf(name, sizeof(name), "SESSION|%p|%s|%s%s",
			(void *)(uintptr_t)CFMachPortGetPort(sessionPrivate->mp),
			(caller != NULL) ? caller : "?",
			(path_s != NULL) ? "*/"   : "",
			(path   != NULL) ? path   : "?");
		CFAllocatorDeallocate(NULL, caller);
	} else {
		snprintf(name, sizeof(name), "SESSION|%p",
			 (void *)(uintptr_t)CFMachPortGetPort(sessionPrivate->mp));
	}

	pthread_setname_np(name);

	return;
}


static Boolean
__SCHelperSessionSetPreferences(SCHelperSessionRef session, SCPreferencesRef prefs)
{
	SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)session;

	pthread_mutex_lock(&sessionPrivate->lock);

	if (prefs != NULL) {
		CFRetain(prefs);
	}
	if (sessionPrivate->prefs != NULL) {
		SCLog(debug, LOG_DEBUG,
		      CFSTR("%p : close"),
		      session);
		CFRelease(sessionPrivate->prefs);
	}
	if (prefs != NULL) {
		SCLog(debug, LOG_DEBUG,
		      CFSTR("%p : open, prefs = %@"),
		      session,
		      prefs);
	}
	sessionPrivate->prefs = prefs;

	__SCHelperSessionSetThreadName(session);

	pthread_mutex_unlock(&sessionPrivate->lock);

	return TRUE;
}


static void
__SCHelperSessionSetNetworkSetFilter(SCHelperSessionRef session, Boolean setChange)
{
	SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)session;

	pthread_mutex_lock(&sessionPrivate->lock);

	sessionPrivate->isSetChange = setChange ? YES : NO;

	pthread_mutex_unlock(&sessionPrivate->lock);

	return;
}


static Boolean
__SCHelperSessionUseNetworkSetFilter(SCHelperSessionRef session)
{
	SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)session;

	return (sessionPrivate->isSetChange == YES) ? TRUE : FALSE;
}


static Boolean
__SCHelperSessionSetVPNFilter(SCHelperSessionRef session, Boolean vpnChange, CFArrayRef vpnTypes)
{
	SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)session;

	pthread_mutex_lock(&sessionPrivate->lock);

	sessionPrivate->isVPNChange = vpnChange ? YES : NO;

	if (vpnTypes != NULL) {
		CFRetain(vpnTypes);
	}
	if (sessionPrivate->vpnTypes != NULL) {
		CFRelease(sessionPrivate->vpnTypes);
	}
	sessionPrivate->vpnTypes = vpnTypes;

	pthread_mutex_unlock(&sessionPrivate->lock);

	return TRUE;
}


static CFArrayRef
__SCHelperSessionUseVPNFilter(SCHelperSessionRef session, CFArrayRef *vpnTypes)
{
	SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)session;

	*vpnTypes = sessionPrivate->vpnTypes;
	return (sessionPrivate->vpnTypes != NULL) ? TRUE : FALSE;
}


static void
__SCHelperSessionLog(const void *value, void *context)
{
	SCHelperSessionRef		session		= (SCHelperSessionRef)value;
	SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)session;
	FILE				**logFile	= (FILE **)context;

	pthread_mutex_lock(&sessionPrivate->lock);

	if ((sessionPrivate->mp != NULL) && (sessionPrivate->prefs != NULL)) {
		SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)sessionPrivate->prefs;

		SCLog(TRUE, LOG_NOTICE,
		      CFSTR("  %p {port = %p, caller = %@, path = %s%s}"),
		      session,
		      (void *)(uintptr_t)CFMachPortGetPort(sessionPrivate->mp),
		      prefsPrivate->name,
		      prefsPrivate->newPath ? prefsPrivate->newPath : prefsPrivate->path,
		      prefsPrivate->locked ? ", locked" : "");

		if ((sessionPrivate->backtraces != NULL) &&
		    (CFSetGetCount(sessionPrivate->backtraces) > 0)) {
			// log/report all collected backtraces
			CFSetApplyFunction(sessionPrivate->backtraces,
					   __SCHelperSessionLogBacktrace,
					   (void *)logFile);

			// to ensure that we don't log the same backtraces multiple
			// times we remove any reported traces
			CFRelease(sessionPrivate->backtraces);
			sessionPrivate->backtraces = NULL;
		}
	}

	pthread_mutex_unlock(&sessionPrivate->lock);

	return;
}


#pragma mark -


static const CFRuntimeClass __SCHelperSessionClass = {
	0,					// version
	"SCHelperSession",			// className
	NULL,					// init
	NULL,					// copy
	__SCHelperSessionDeallocate,		// dealloc
	NULL,					// equal
	NULL,					// hash
	NULL,					// copyFormattingDesc
	__SCHelperSessionCopyDescription	// copyDebugDesc
};


static CFStringRef
__SCHelperSessionCopyDescription(CFTypeRef cf)
{
	CFAllocatorRef			allocator	= CFGetAllocator(cf);
	CFMutableStringRef		result;
	SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)cf;

	pthread_mutex_lock(&sessionPrivate->lock);

	result = CFStringCreateMutable(allocator, 0);
	CFStringAppendFormat(result, NULL, CFSTR("<SCHelperSession %p [%p]> {"), cf, allocator);
	CFStringAppendFormat(result, NULL, CFSTR("authorization = %p"), sessionPrivate->authorization);
	if (sessionPrivate->mp != NULL) {
		CFStringAppendFormat(result, NULL,
				     CFSTR(", mp = %p (port = 0x%x)"),
				     sessionPrivate->mp,
				     CFMachPortGetPort(sessionPrivate->mp));
	}
	if (sessionPrivate->prefs != NULL) {
		CFStringAppendFormat(result, NULL, CFSTR(", prefs = %@"), sessionPrivate->prefs);
	}
	CFStringAppendFormat(result, NULL, CFSTR("}"));

	pthread_mutex_unlock(&sessionPrivate->lock);

	return result;
}


static void
__SCHelperSessionDeallocate(CFTypeRef cf)
{
	SCHelperSessionRef		session		= (SCHelperSessionRef)cf;
	SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)session;

	// we're releasing "a" session so take the global lock...
	pthread_mutex_lock(&sessions_lock);

	// release resources
	__SCHelperSessionSetAuthorization(session, NULL);
	__SCHelperSessionSetPreferences(session, NULL);
	__SCHelperSessionSetNetworkSetFilter(session, FALSE);
	__SCHelperSessionSetVPNFilter(session, FALSE, NULL);
	pthread_mutex_destroy(&sessionPrivate->lock);
	if (sessionPrivate->backtraces != NULL) {
		CFRelease(sessionPrivate->backtraces);
	}

	// we no longer need/want to track this session
	CFSetRemoveValue(sessions, sessionPrivate);
	sessions_generation++;
	sessions_closed++;

	// release the global lock, wake up the main runloop, all done
	pthread_mutex_unlock(&sessions_lock);
	CFRunLoopWakeUp(main_runLoop);

	return;
}


static void
__SCHelperSessionInitialize(void)
{
	__kSCHelperSessionTypeID = _CFRuntimeRegisterClass(&__SCHelperSessionClass);
	return;
}


static SCHelperSessionRef
__SCHelperSessionCreate(CFAllocatorRef allocator)
{
	SCHelperSessionPrivateRef	sessionPrivate;
	uint32_t			size;

	// initialize runtime
	pthread_once(&initialized, __SCHelperSessionInitialize);

	// allocate session
	size           = sizeof(SCHelperSessionPrivate) - sizeof(CFRuntimeBase);
	sessionPrivate = (SCHelperSessionPrivateRef)_CFRuntimeCreateInstance(allocator,
									    __kSCHelperSessionTypeID,
									    size,
									    NULL);
	if (sessionPrivate == NULL) {
		return NULL;
	}

	if (pthread_mutex_init(&sessionPrivate->lock, NULL) != 0) {
		SCLog(TRUE, LOG_ERR, CFSTR("pthread_mutex_init(): failure to initialize per session lock"));
		CFRelease(sessionPrivate);
		return NULL;
	}
	sessionPrivate->authorization		= NULL;
	sessionPrivate->use_entitlement		= FALSE;
	sessionPrivate->port			= MACH_PORT_NULL;
	sessionPrivate->mp			= NULL;
	sessionPrivate->callerReadAccess	= UNKNOWN;
	sessionPrivate->callerWriteAccess	= UNKNOWN;
	sessionPrivate->isSetChange		= UNKNOWN;
	sessionPrivate->isVPNChange		= UNKNOWN;
	sessionPrivate->vpnTypes		= NULL;
	sessionPrivate->prefs			= NULL;
	sessionPrivate->backtraces		= NULL;

	// keep track this session
	pthread_mutex_lock(&sessions_lock);
	if (sessions == NULL) {
		const CFSetCallBacks mySetCallBacks = { 0, NULL, NULL, CFCopyDescription, CFEqual, CFHash };

		// create a non-retaining set
		sessions = CFSetCreateMutable(NULL, 0, &mySetCallBacks);
	}
	CFSetAddValue(sessions, sessionPrivate);
	sessions_generation++;
	pthread_mutex_unlock(&sessions_lock);

	return (SCHelperSessionRef)sessionPrivate;
}


#pragma mark -


static SCHelperSessionRef
__SCHelperSessionFindWithPort(mach_port_t port)
{
	SCHelperSessionRef	session	= NULL;

	// keep track this session
	pthread_mutex_lock(&sessions_lock);
	if (sessions != NULL) {
		CFIndex		i;
		CFIndex		n	= CFSetGetCount(sessions);
		const void *	vals_q[16];
		const void **	vals	= vals_q;

		if (n > (CFIndex)(sizeof(vals_q) / sizeof(SCHelperSessionRef)))
			vals = CFAllocatorAllocate(NULL, n * sizeof(CFStringRef), 0);
		CFSetGetValues(sessions, vals);
		for (i = 0; i < n; i++) {
			SCHelperSessionPrivateRef	sessionPrivate;

			sessionPrivate = (SCHelperSessionPrivateRef)vals[i];
			if (sessionPrivate->port == port) {
				session = (SCHelperSessionRef)sessionPrivate;
				break;
			}
		}
		if (vals != vals_q)
			CFAllocatorDeallocate(NULL, vals);
	}
	pthread_mutex_unlock(&sessions_lock);

	return session;
}


#pragma mark -
#pragma mark Session backtrace logging


static void
__SCHelperSessionAddBacktrace(SCHelperSessionRef session, CFStringRef backtrace, const char * command)
{
	CFStringRef			logEntry;
	SCPreferencesRef		prefs;
	SCPreferencesPrivateRef		prefsPrivate;
	SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)session;

	prefs = __SCHelperSessionGetPreferences((SCHelperSessionRef)sessionPrivate);
	if (prefs == NULL) {
		// if no prefs
		return;
	}
	prefsPrivate = (SCPreferencesPrivateRef)prefs;

	logEntry = CFStringCreateWithFormat(NULL, NULL,
					    CFSTR("%@ [%s]: %s\n\n%@"),
					    prefsPrivate->name,
					    prefsPrivate->newPath ? prefsPrivate->newPath : prefsPrivate->path,
					    command,
					    backtrace);

	pthread_mutex_lock(&sessionPrivate->lock);

	if (sessionPrivate->backtraces == NULL) {
		sessionPrivate->backtraces = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
	}
	CFSetSetValue(sessionPrivate->backtraces, logEntry);

	pthread_mutex_unlock(&sessionPrivate->lock);

	CFRelease(logEntry);
	return;
}


static void
__SCHelperSessionLogBacktrace(const void *value, void *context)
{
	CFSetRef	backtrace	= (CFSetRef)value;
	FILE		**logFile	= (FILE **)context;

	if (*logFile == NULL) {
		char		path[PATH_MAX];
		struct tm	tm_now;
		struct timeval	tv_now;

		(void)gettimeofday(&tv_now, NULL);
		(void)localtime_r(&tv_now.tv_sec, &tm_now);

		snprintf(path,
			 sizeof(path),
			 "/Library/Logs/CrashReporter/SCHelper-%4d-%02d-%02d-%02d%02d%02d.log",
			 tm_now.tm_year + 1900,
			 tm_now.tm_mon + 1,
			 tm_now.tm_mday,
			 tm_now.tm_hour,
			 tm_now.tm_min,
			 tm_now.tm_sec);

		*logFile = fopen(path, "a");
		if (*logFile == NULL) {
			// if log file could not be created
			return;
		}

		SCLog(TRUE, LOG_INFO, CFSTR("created backtrace log: %s"), path);
	}

	SCPrint(TRUE, *logFile, CFSTR("%@\n"), backtrace);
	return;
}


#pragma mark -
#pragma mark Helpers


/*
 * EXIT
 *   (in)  data   = N/A
 *   (out) status = SCError()
 *   (out) reply  = N/A
 */
static Boolean
do_Exit(SCHelperSessionRef session, void *info, CFDataRef data, uint32_t *status, CFDataRef *reply)
{
	*status = -1;
	return FALSE;
}


/*
 * AUTHORIZE
 *   (in)  data   = authorizationDict (in 2 flavors)
 *		    kSCHelperAuthAuthorization - use provided AuthorizationExternalForm
 *		    kSCHelperAuthCallerInfo    - use entitlement
 *   (out) status = OSStatus
 *   (out) reply  = N/A
 */
static Boolean
do_Auth(SCHelperSessionRef session, void *info, CFDataRef data, uint32_t *status, CFDataRef *reply)
{
	CFDictionaryRef	authorizationDict;
#if	!TARGET_OS_IPHONE
	CFDataRef	authorizationData	= NULL;
#endif
	Boolean		ok			= FALSE;

	if (_SCUnserialize((CFPropertyListRef*)&authorizationDict, data, NULL, 0) == FALSE) {
		return FALSE;
	}

	if (authorizationDict == NULL) {
		return FALSE;
	}

	if (!isA_CFDictionary(authorizationDict)) {
		CFRelease(authorizationDict);
		return FALSE;
	}

#if	!TARGET_OS_IPHONE
	authorizationData = CFDictionaryGetValue(authorizationDict, kSCHelperAuthAuthorization);
	if (authorizationData != NULL && isA_CFData(authorizationData)) {
		ok = __SCHelperSessionSetAuthorization(session, authorizationData);
	} else
#endif
	{
		CFStringRef	authorizationInfo;

		authorizationInfo = CFDictionaryGetValue(authorizationDict, kSCHelperAuthCallerInfo);
		if (authorizationInfo != NULL && isA_CFString(authorizationInfo)) {
			ok = __SCHelperSessionSetAuthorization(session, authorizationInfo);
		}
	}

	CFRelease(authorizationDict);
	*status = ok ? 0 : 1;
	return TRUE;
}


#if	!TARGET_OS_IPHONE


/*
 * SCHELPER_MSG_KEYCHAIN_COPY
 *   (in)  data   = unique_id
 *   (out) status = SCError()
 *   (out) reply  = password
 */
static Boolean
do_keychain_copy(SCHelperSessionRef session, void *info, CFDataRef data, uint32_t *status, CFDataRef *reply)
{
	Boolean			ok		= FALSE;
	SCPreferencesRef	prefs;
	CFStringRef		unique_id	= NULL;

	if ((data != NULL) && !_SCUnserializeString(&unique_id, data, NULL, 0)) {
		return FALSE;
	}

	if (unique_id != NULL) {
		if (isA_CFString(unique_id)) {
			prefs = __SCHelperSessionGetPreferences(session);
			*reply = _SCPreferencesSystemKeychainPasswordItemCopy(prefs, unique_id);
			if (*reply == NULL) {
				*status = SCError();
			}
			ok = TRUE;
		}

		CFRelease(unique_id);
	}

	return ok;
}


/*
 * SCHELPER_MSG_KEYCHAIN_EXISTS
 *   (in)  data   = unique_id
 *   (out) status = SCError()
 *   (out) reply  = N/A
 */
static Boolean
do_keychain_exists(SCHelperSessionRef session, void *info, CFDataRef data, uint32_t *status, CFDataRef *reply)
{
	Boolean			ok		= FALSE;
	SCPreferencesRef	prefs;
	CFStringRef		unique_id	= NULL;

	if ((data != NULL) && !_SCUnserializeString(&unique_id, data, NULL, 0)) {
		return FALSE;
	}

	if (unique_id != NULL) {
		if (isA_CFString(unique_id)) {
			prefs = __SCHelperSessionGetPreferences(session);
			ok = _SCPreferencesSystemKeychainPasswordItemExists(prefs, unique_id);
			if (!ok) {
				*status = SCError();
			}
		}

		CFRelease(unique_id);
	}

	return ok;
}


/*
 * SCHELPER_MSG_KEYCHAIN_REMOVE
 *   (in)  data   = unique_id
 *   (out) status = SCError()
 *   (out) reply  = N/A
 */
static Boolean
do_keychain_remove(SCHelperSessionRef session, void *info, CFDataRef data, uint32_t *status, CFDataRef *reply)
{
	Boolean			ok		= FALSE;
	SCPreferencesRef	prefs;
	CFStringRef		unique_id	= NULL;

	if ((data != NULL) && !_SCUnserializeString(&unique_id, data, NULL, 0)) {
		return FALSE;
	}

	if (unique_id != NULL) {
		if (isA_CFString(unique_id)) {
			prefs = __SCHelperSessionGetPreferences(session);
			ok = _SCPreferencesSystemKeychainPasswordItemRemove(prefs, unique_id);
			if (!ok) {
				*status = SCError();
			}
		}

		CFRelease(unique_id);
	}

	return ok;
}


/*
 * SCHELPER_MSG_KEYCHAIN_SET
 *   (in)  data   = options dictionary
 *   (out) status = SCError()
 *   (out) reply  = N/A
 */
static Boolean
do_keychain_set(SCHelperSessionRef session, void *info, CFDataRef data, uint32_t *status, CFDataRef *reply)
{
	CFStringRef		account;
	CFStringRef		description;
	CFArrayRef		executablePaths	= NULL;
	CFStringRef		label;
	Boolean			ok;
	CFDictionaryRef		options		= NULL;
	CFDataRef		password;
	SCPreferencesRef	prefs;
	CFStringRef		unique_id;

	if ((data != NULL) && !_SCUnserialize((CFPropertyListRef *)&options, data, NULL, 0)) {
		return FALSE;
	}

	if (options != NULL) {
		if (!isA_CFDictionary(options)) {
			CFRelease(options);
			return FALSE;
		}
	} else {
		return FALSE;
	}

	if (CFDictionaryGetValueIfPresent(options,
					  kSCKeychainOptionsAllowedExecutables,
					  (const void **)&executablePaths)) {
		CFMutableArrayRef	executableURLs;
		CFIndex			i;
		CFIndex			n;
		CFMutableDictionaryRef	newOptions;

		executableURLs = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		n = CFArrayGetCount(executablePaths);
		for (i = 0; i < n; i++) {
			CFDataRef	path;
			CFURLRef	url;

			path = CFArrayGetValueAtIndex(executablePaths, i);
			url  = CFURLCreateFromFileSystemRepresentation(NULL,
								       CFDataGetBytePtr(path),
								       CFDataGetLength(path),
								       FALSE);
			if (url != NULL) {
				CFArrayAppendValue(executableURLs, url);
				CFRelease(url);
			}
		}

		newOptions = CFDictionaryCreateMutableCopy(NULL, 0, options);
		CFDictionarySetValue(newOptions, kSCKeychainOptionsAllowedExecutables, executableURLs);
		CFRelease(executableURLs);

		CFRelease(options);
		options = newOptions;
	}

	unique_id   = CFDictionaryGetValue(options, kSCKeychainOptionsUniqueID);
	label       = CFDictionaryGetValue(options, kSCKeychainOptionsLabel);
	description = CFDictionaryGetValue(options, kSCKeychainOptionsDescription);
	account     = CFDictionaryGetValue(options, kSCKeychainOptionsAccount);
	password    = CFDictionaryGetValue(options, kSCKeychainOptionsPassword);

	prefs = __SCHelperSessionGetPreferences(session);
	ok = _SCPreferencesSystemKeychainPasswordItemSet(prefs,
							 unique_id,
							 label,
							 description,
							 account,
							 password,
							 options);
	CFRelease(options);
	if (!ok) {
		*status = SCError();
	}

	return TRUE;
}


#endif	// !TARGET_OS_IPHONE


/*
 * SCHELPER_MSG_INTERFACE_REFRESH
 *   (in)  data   = ifName
 *   (out) status = SCError()
 *   (out) reply  = N/A
 */
static Boolean
do_interface_refresh(SCHelperSessionRef session, void *info, CFDataRef data, uint32_t *status, CFDataRef *reply)
{
	CFStringRef	ifName	= NULL;
	Boolean		ok	= FALSE;

	if ((data != NULL) && !_SCUnserializeString(&ifName, data, NULL, 0)) {
		*status = kSCStatusInvalidArgument;
		SCLog(TRUE, LOG_ERR, CFSTR("interface name not valid"));
		return FALSE;
	}

	if (ifName == NULL) {
		*status = kSCStatusInvalidArgument;
		SCLog(TRUE, LOG_ERR, CFSTR("interface name not valid"));
		return FALSE;
	}

	if (isA_CFString(ifName)) {
		ok = _SCNetworkInterfaceForceConfigurationRefresh(ifName);
		if (!ok) {
			*status = SCError();
			SCLog(TRUE, LOG_ERR,
			      CFSTR("interface \"%@\" not refreshed: %s"),
			      ifName,
			      SCErrorString(*status));
		}
	} else {
		*status = kSCStatusInvalidArgument;
		SCLog(TRUE, LOG_ERR, CFSTR("interface name not valid"));
	}

	CFRelease(ifName);

	return ok;
}


/*
 * OPEN
 *   (in)  data   = prefsID
 *   (out) status = SCError()
 *   (out) reply  = N/A
 */
static Boolean
do_prefs_Open(SCHelperSessionRef session, void *info, CFDataRef data, uint32_t *status, CFDataRef *reply)
{
	CFStringRef		name;
	CFDictionaryRef		options;
	CFNumberRef		pid;
	SCPreferencesRef	prefs		= __SCHelperSessionGetPreferences(session);
	CFDictionaryRef		prefsInfo	= NULL;
	CFStringRef		prefsID;
	CFStringRef		prefsName;
	CFStringRef		proc_name;

	if (prefs != NULL) {
		return FALSE;
	}

	if ((data != NULL) && !_SCUnserialize((CFPropertyListRef *)&prefsInfo, data, NULL, 0)) {
		SCLog(TRUE, LOG_ERR, CFSTR("data not valid, %@"), data);
		return FALSE;
	}

	if ((prefsInfo == NULL) || !isA_CFDictionary(prefsInfo)) {
		SCLog(TRUE, LOG_ERR, CFSTR("info not valid"));
		if (prefsInfo != NULL) CFRelease(prefsInfo);
		return FALSE;
	}

	// get [optional] prefsID
	prefsID = CFDictionaryGetValue(prefsInfo, CFSTR("prefsID"));
	prefsID = isA_CFString(prefsID);
	if (prefsID != NULL) {
		if (CFStringHasPrefix(prefsID, CFSTR("/")) ||
		    CFStringHasPrefix(prefsID, CFSTR("../")) ||
		    CFStringHasSuffix(prefsID, CFSTR("/..")) ||
		    (CFStringFind(prefsID, CFSTR("/../"), 0).location != kCFNotFound)) {
			// if we're trying to escape from the preferences directory
			SCLog(TRUE, LOG_ERR, CFSTR("prefsID (%@) not valid"), prefsID);
			CFRelease(prefsInfo);
			*status = kSCStatusInvalidArgument;
			return TRUE;
		}
	}

	// get [optional] options
	options = CFDictionaryGetValue(prefsInfo, CFSTR("options"));
	options = isA_CFDictionary(options);

	// get preferences session "name"
	name = CFDictionaryGetValue(prefsInfo, CFSTR("name"));
	if (!isA_CFString(name)) {
		SCLog(TRUE, LOG_ERR, CFSTR("session \"name\" not valid"));
		CFRelease(prefsInfo);
		return FALSE;
	}

	// get PID of caller
	pid = CFDictionaryGetValue(prefsInfo, CFSTR("PID"));
	if (!isA_CFNumber(pid)) {
		SCLog(TRUE, LOG_ERR, CFSTR("PID not valid"));
		CFRelease(prefsInfo);
		return FALSE;
	}

	// get process name of caller
	proc_name = CFDictionaryGetValue(prefsInfo, CFSTR("PROC_NAME"));
	if (!isA_CFString(proc_name)) {
		SCLog(TRUE, LOG_ERR, CFSTR("process name not valid"));
		CFRelease(prefsInfo);
		return FALSE;
	}

	// build [helper] preferences "name" (used for debugging) and estabish
	// a preferences session.
	prefsName = CFStringCreateWithFormat(NULL, NULL,
					     CFSTR("%@(%@):%@"),
					     proc_name,
					     pid,
					     name);

	prefs = SCPreferencesCreateWithOptions(NULL, prefsName, prefsID, NULL, options);
	CFRelease(prefsName);
	CFRelease(prefsInfo);

	__SCHelperSessionSetPreferences(session, prefs);

	if (prefs != NULL) {
#ifdef	NOTYET
		Boolean	ok;
		CFRunLoopRef	rl	= CFRunLoopGetCurrent();

		// [temporarily] schedule notifications to ensure that we can use
		// the SCDynamicStore to track helper sessions
		ok = SCPreferencesScheduleWithRunLoop(prefs, rl, kCFRunLoopDefaultMode);
		if (ok) {
			(void)SCPreferencesUnscheduleFromRunLoop(prefs, rl, kCFRunLoopDefaultMode);
		}
#endif	// NOTYET

		// the session now holds a references to the SCPreferencesRef
		CFRelease(prefs);
	} else {
		*status = SCError();
	}

	return TRUE;
}


/*
 * ACCESS
 *   (in)  data   = N/A
 *   (out) status = SCError()
 *   (out) reply  = current signature + current preferences
 */
static Boolean
do_prefs_Access(SCHelperSessionRef session, void *info, CFDataRef data, uint32_t *status, CFDataRef *reply)
{
	Boolean			ok;
	SCPreferencesRef	prefs		= __SCHelperSessionGetPreferences(session);
	CFDataRef		signature;

	if (prefs == NULL) {
		return FALSE;
	}

	signature = SCPreferencesGetSignature(prefs);
	if (signature != NULL) {
		const void *		dictKeys[2];
		const void *		dictVals[2];
		SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;
		CFDictionaryRef		replyDict;

		dictKeys[0] = CFSTR("signature");
		dictVals[0] = signature;

		dictKeys[1] = CFSTR("preferences");
		dictVals[1] = prefsPrivate->prefs;

		replyDict = CFDictionaryCreate(NULL,
					       (const void **)&dictKeys,
					       (const void **)&dictVals,
					       sizeof(dictKeys)/sizeof(dictKeys[0]),
					       &kCFTypeDictionaryKeyCallBacks,
					       &kCFTypeDictionaryValueCallBacks);

		ok = _SCSerialize(replyDict, reply, NULL, NULL);
		CFRelease(replyDict);
		if (!ok) {
			return FALSE;
		}
	} else {
		*status = SCError();
	}

	return TRUE;
}


/*
 * LOCK
 *   (in)  data   = client prefs signature (NULL if check not needed)
 *   (out) status = SCError()
 *   (out) reply  = N/A
 */
static Boolean
do_prefs_Lock(SCHelperSessionRef session, void *info, CFDataRef data, uint32_t *status, CFDataRef *reply)
{
	CFDataRef		clientSignature	= (CFDataRef)data;
	Boolean			ok;
	SCPreferencesRef	prefs		= __SCHelperSessionGetPreferences(session);
	Boolean			wait		= (info == (void *)FALSE) ? FALSE : TRUE;

	if (prefs == NULL) {
		return FALSE;
	}

	ok = SCPreferencesLock(prefs, wait);
	if (!ok) {
		*status = SCError();
		return TRUE;
	}

	if (clientSignature != NULL) {
		CFDataRef	serverSignature;

		serverSignature = SCPreferencesGetSignature(prefs);
		if (!CFEqual(clientSignature, serverSignature)) {
			(void)SCPreferencesUnlock(prefs);
			*status = kSCStatusStale;
		}
	}

	return TRUE;
}


/*
 * COMMIT
 *   (in)  data   = new preferences (NULL if commit w/no changes)
 *   (out) status = SCError()
 *   (out) reply  = new signature
 */
static Boolean
do_prefs_Commit(SCHelperSessionRef session, void *info, CFDataRef data, uint32_t *status, CFDataRef *reply)
{
	Boolean			ok;
	SCPreferencesRef	prefs		= __SCHelperSessionGetPreferences(session);
	CFPropertyListRef	prefsData	= NULL;
	SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;
	Boolean			useSetFilter;
	Boolean			useVPNFilter;
	CFArrayRef		vpnTypes	= NULL;

	if (prefs == NULL) {
		return FALSE;
	}

	if (data == NULL) {
		// if commit with no changes
		goto commit;
	}

	ok = _SCUnserialize(&prefsData, data, NULL, 0);
	if (!ok) {
		return FALSE;
	}

	if (!isA_CFDictionary(prefsData)) {
		*status = kSCStatusFailed;
		ok = FALSE;
		goto done;
	}

	useSetFilter = __SCHelperSessionUseNetworkSetFilter(session);
	useVPNFilter = __SCHelperSessionUseVPNFilter(session, &vpnTypes);
	if (useSetFilter || useVPNFilter) {
		ok = FALSE;

		if (prefsPrivate->prefs != NULL) {
			CFIndex			c;
			CFMutableDictionaryRef	prefsNew	= NULL;
			CFMutableDictionaryRef	prefsOld	= NULL;
			CFMutableDictionaryRef	prefsSave	= prefsPrivate->prefs;

			for (c = 0; c < 2; c++) {

				switch (c) {
					case 0 :
						prefsPrivate->prefs = CFDictionaryCreateMutableCopy(NULL, 0, prefsSave);
						break;
					case 1 :
						prefsPrivate->prefs = CFDictionaryCreateMutableCopy(NULL, 0, prefsData);
						break;
				}

				if (useSetFilter) {
					// filter out current network set selection
					(void) SCPreferencesRemoveValue(prefs, kSCPrefCurrentSet);
				} else if (useVPNFilter) {
					CFRange		range		= CFRangeMake(0, CFArrayGetCount(vpnTypes));
					CFArrayRef	services;

					// filter out VPN services of the specified type
					services = SCNetworkServiceCopyAll(prefs);
					if (services != NULL) {
						CFIndex	i;
						CFIndex	n	= CFArrayGetCount(services);

						for (i = 0; i < n; i++) {
							SCNetworkServiceRef	service;

							service = CFArrayGetValueAtIndex(services, i);
							if (_SCNetworkServiceIsVPN(service)) {
								SCNetworkInterfaceRef	child;
								CFStringRef		childType	= NULL;
								SCNetworkInterfaceRef	interface;
								CFStringRef		interfaceType;

								interface     = SCNetworkServiceGetInterface(service);
								interfaceType = SCNetworkInterfaceGetInterfaceType(interface);
								child         = SCNetworkInterfaceGetInterface(interface);
								if (child != NULL) {
									childType = SCNetworkInterfaceGetInterfaceType(child);
								}
								if (CFEqual(interfaceType, kSCNetworkInterfaceTypeVPN) &&
								    (childType != NULL) &&
								    CFArrayContainsValue(vpnTypes, range, childType)) {
									// filter out VPN service
									(void) SCNetworkServiceRemove(service);
								} else {
									// mark all other VPN services "enabled"
									(void) SCNetworkServiceSetEnabled(service, TRUE);
								}
							}
						}

						CFRelease(services);
					}
				}

				switch (c) {
					case 0 :
						prefsOld = prefsPrivate->prefs;
						break;
					case 1 :
						prefsNew = prefsPrivate->prefs;
						break;
				}
			}

			// compare the filtered configurations
			ok = _SC_CFEqual(prefsOld, prefsNew);

			// clean up
			if (prefsOld != NULL) CFRelease(prefsOld);
			if (prefsNew != NULL) CFRelease(prefsNew);
			prefsPrivate->prefs = prefsSave;
		}

		if (!ok) {
			*status = kSCStatusAccessError;
			goto done;
		}
	}

	if (prefsPrivate->prefs != NULL) {
		CFRelease(prefsPrivate->prefs);
	}
	prefsPrivate->prefs    = CFDictionaryCreateMutableCopy(NULL, 0, prefsData);
	prefsPrivate->accessed = TRUE;
	prefsPrivate->changed  = TRUE;

    commit :

	ok = SCPreferencesCommitChanges(prefs);
	if (ok) {
		*reply = SCPreferencesGetSignature(prefs);
		CFRetain(*reply);
	} else {
		*status = SCError();
	}

    done :

	if (prefsData != NULL) CFRelease(prefsData);
	return ok;
}


/*
 * APPLY
 *   (in)  data   = N/A
 *   (out) status = SCError()
 *   (out) reply  = N/A
 */
static Boolean
do_prefs_Apply(SCHelperSessionRef session, void *info, CFDataRef data, uint32_t *status, CFDataRef *reply)
{
	Boolean			ok;
	SCPreferencesRef	prefs	= __SCHelperSessionGetPreferences(session);

	if (prefs == NULL) {
		return FALSE;
	}

	ok = SCPreferencesApplyChanges(prefs);
	if (!ok) {
		*status = SCError();
	}

	return TRUE;
}


/*
 * UNLOCK
 *   (in)  data   = N/A
 *   (out) status = SCError()
 *   (out) reply  = N/A
 */
static Boolean
do_prefs_Unlock(SCHelperSessionRef session, void *info, CFDataRef data, uint32_t *status, CFDataRef *reply)
{
	Boolean			ok;
	SCPreferencesRef	prefs	= __SCHelperSessionGetPreferences(session);

	if (prefs == NULL) {
		return FALSE;
	}

	ok = SCPreferencesUnlock(prefs);
	if (!ok) {
		*status = SCError();
	}

	return TRUE;
}


/*
 * CLOSE
 *   (in)  data   = N/A
 *   (out) status = SCError()
 *   (out) reply  = N/A
 */
static Boolean
do_prefs_Close(SCHelperSessionRef session, void *info, CFDataRef data, uint32_t *status, CFDataRef *reply)
{
	SCPreferencesRef	prefs	= __SCHelperSessionGetPreferences(session);

	if (prefs == NULL) {
		return FALSE;
	}

	__SCHelperSessionSetPreferences(session, NULL);
	*status = -1;
	return TRUE;
}


/*
 * SYNCHRONIZE
 *   (in)  data   = N/A
 *   (out) status = kSCStatusOK
 *   (out) reply  = N/A
 */
static Boolean
do_prefs_Synchronize(SCHelperSessionRef session, void *info, CFDataRef data, uint32_t *status, CFDataRef *reply)
{
	SCPreferencesRef	prefs	= __SCHelperSessionGetPreferences(session);

	if (prefs == NULL) {
		return FALSE;
	}

	SCPreferencesSynchronize(prefs);
	*status = kSCStatusOK;
	return TRUE;
}


#pragma mark -
#pragma mark Process commands


static CFStringRef
sessionName(SCHelperSessionRef session)
{
	CFStringRef		name	= NULL;
	SCPreferencesRef	prefs;

	prefs = __SCHelperSessionGetPreferences(session);
	if (prefs != NULL) {
		SCPreferencesPrivateRef	prefsPrivate	= (SCPreferencesPrivateRef)prefs;

		name = prefsPrivate->name;
	}

	return (name != NULL) ? name : CFSTR("???");
}


static CFStringRef
sessionPrefsID(SCHelperSessionRef session)
{
	CFStringRef			prefsID;
	SCPreferencesPrivateRef		prefsPrivate;
	SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)session;

	prefsPrivate = (SCPreferencesPrivateRef)sessionPrivate->prefs;
	if ((prefsPrivate != NULL) && (prefsPrivate->prefsID != NULL)) {
		prefsID  = prefsPrivate->prefsID;
	} else {
		prefsID = PREFS_DEFAULT_CONFIG;
	}

	return prefsID;
}


static CFTypeRef
copyEntitlement(SCHelperSessionRef session, CFStringRef entitlement)
{
	SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)session;
	SecTaskRef			task;
	CFTypeRef			value		= NULL;

	// Create the security task from the audit token
	task = SecTaskCreateWithAuditToken(NULL, sessionPrivate->auditToken);
	if (task != NULL) {
		CFErrorRef	error	= NULL;

		// Get the value for the entitlement
		value = SecTaskCopyValueForEntitlement(task, entitlement, &error);
		if ((value == NULL) && (error != NULL)) {
			CFIndex		code	= CFErrorGetCode(error);
			CFStringRef	domain	= CFErrorGetDomain(error);

			if (!CFEqual(domain, kCFErrorDomainMach) ||
			    ((code != kIOReturnInvalid) && (code != kIOReturnNotFound))) {
				// if unexpected error
				SCLog(TRUE, LOG_ERR,
				      CFSTR("SecTaskCopyValueForEntitlement(,\"%@\",) failed, error = %@ : %@"),
				      entitlement,
				      error,
				      sessionName(session));
			}
			CFRelease(error);
		}

		CFRelease(task);
	} else {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("SecTaskCreateWithAuditToken() failed: %@"),
		      sessionName(session));
	}

	return value;
}


#if	!TARGET_OS_IPHONE
static Boolean
isSetChange(SCHelperSessionRef session)
{
	SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)session;

	if (sessionPrivate->isSetChange == UNKNOWN) {
		CFBooleanRef			bVal		= NULL;
		CFStringRef			prefsID;
		SCPreferencesPrivateRef		prefsPrivate	= (SCPreferencesPrivateRef)sessionPrivate->prefs;
		Boolean				setFilter	= FALSE;

		prefsID = sessionPrefsID(session);
		if (CFEqual(prefsID, PREFS_DEFAULT_CONFIG) &&
		    isA_CFDictionary(prefsPrivate->options) &&
		    CFDictionaryGetValueIfPresent(prefsPrivate->options,
						  kSCPreferencesOptionChangeNetworkSet,
						  (const void **)&bVal) &&
		    isA_CFBoolean(bVal) &&
		    CFBooleanGetValue(bVal)) {
			setFilter = TRUE;
		}

		// establish network set (location) filter
		__SCHelperSessionSetNetworkSetFilter(session, setFilter);
	}

	return (sessionPrivate->isSetChange == YES) ? TRUE : FALSE;
}
#endif	// !TARGET_OS_IPHONE


static Boolean
isVPNChange(SCHelperSessionRef session)
{
	SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)session;

	if (sessionPrivate->isVPNChange == UNKNOWN) {
		CFArrayRef	entitlement;
		Boolean		vpnChange	= FALSE;
		CFArrayRef	vpnTypes	= NULL;

		entitlement = copyEntitlement(session, kSCVPNFilterEntitlementName);
		if (entitlement != NULL) {
			if (isA_CFArray(entitlement)) {
				CFStringRef	prefsID;

				prefsID = sessionPrefsID(session);
				if (CFEqual(prefsID, PREFS_DEFAULT_CONFIG)) {
					// save the VPN type identifiers
					vpnTypes = CFRetain(entitlement);

					// grant an exception
					vpnChange = TRUE;
				} else if (CFStringHasPrefix(prefsID, CFSTR("VPN-")) &&
					   CFStringHasSuffix(prefsID, CFSTR(".plist"))) {
					CFRange		range;
					CFStringRef	vpnID;

					range.location = sizeof("VPN-") - 1;
					range.length   = CFStringGetLength(prefsID)
							 - (sizeof("VPN-") - 1)		// trim VPN-
							 - (sizeof(".plist") - 1);	// trim .plist
					vpnID = CFStringCreateWithSubstring(NULL, prefsID, range);
					if (CFArrayContainsValue(entitlement,
								 CFRangeMake(0, CFArrayGetCount(entitlement)),
								 vpnID)) {
						// grant an exception
						vpnChange = TRUE;
					}
					CFRelease(vpnID);
				}
			}

			CFRelease(entitlement);
		}

		__SCHelperSessionSetVPNFilter(session, vpnChange, vpnTypes);
		if (vpnTypes != NULL) {
			CFRelease(vpnTypes);
		}
	}

	return (sessionPrivate->isVPNChange == YES) ? TRUE : FALSE;
}


static Boolean
checkEntitlement(SCHelperSessionRef session, CFStringRef prefsID, CFStringRef entitlement_name)
{
	CFArrayRef			entitlement;
	Boolean				hasEntitlement	= FALSE;

	entitlement = copyEntitlement(session, entitlement_name);
	if (entitlement != NULL) {
		if (isA_CFArray(entitlement)) {
			if (CFArrayContainsValue(entitlement,
						 CFRangeMake(0, CFArrayGetCount(entitlement)),
						 prefsID)) {
				// if client DOES have entitlement
				hasEntitlement = TRUE;
			}
		} else {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("hasAuthorization() session=%@: entitlement=%@: not valid"),
			      sessionName(session),
			      entitlement_name);
		}

		CFRelease(entitlement);
	}

#if	TARGET_OS_IPHONE
	// make an exception for VPN configuration management
	if (!hasEntitlement) {
		if (isVPNChange(session)) {
			// grant a "filtered" exception
			hasEntitlement = TRUE;
		}
	}
#endif	// TARGET_OS_IPHONE

	return hasEntitlement;
}


static Boolean
hasAuthorization(SCHelperSessionRef session, Boolean needWrite)
{
	AuthorizationRef		authorization		= __SCHelperSessionGetAuthorization(session);
	CFStringRef			prefsID;
	SCHelperSessionPrivateRef	sessionPrivate		= (SCHelperSessionPrivateRef)session;

	if (authorization == NULL) {
		return FALSE;
	}

#if	!TARGET_OS_IPHONE
	if (!__SCHelperSessionUseEntitlement(session)) {
		AuthorizationFlags	flags;
		AuthorizationItem	items[1];
		AuthorizationRights	rights;
		OSStatus		status;

		if (isSetChange(session)) {
			items[0].name        = kSCPreferencesAuthorizationRight_network_set;
			items[0].value       = NULL;
			items[0].valueLength = 0;
			items[0].flags       = 0;
		} else if (isVPNChange(session)) {
			items[0].name        = kSCPreferencesAuthorizationRight_write;
			items[0].value       = NULL;
			items[0].valueLength = 0;
			items[0].flags       = 0;
		} else {
			items[0].name        = kSCPreferencesAuthorizationRight_write;
			items[0].value       = NULL;
			items[0].valueLength = 0;
			items[0].flags       = 0;
		}

		rights.count = sizeof(items) / sizeof(items[0]);
		rights.items = items;

		flags = kAuthorizationFlagDefaults;
		flags |= kAuthorizationFlagInteractionAllowed;
		flags |= kAuthorizationFlagExtendRights;
//		flags |= kAuthorizationFlagPartialRights;
//		flags |= kAuthorizationFlagPreAuthorize;

		status = AuthorizationCopyRights(authorization,
						 &rights,
						 kAuthorizationEmptyEnvironment,
						 flags,
						 NULL);
		if (status != errAuthorizationSuccess) {
			SCLog(TRUE, LOG_DEBUG,
			      CFSTR("AuthorizationCopyRights() failed: status = %d"),
			      (int)status);
			return FALSE;
		}

		return TRUE;
	}
#endif	// !TARGET_OS_IPHONE

	prefsID = sessionPrefsID(session);

	if (sessionPrivate->callerWriteAccess == UNKNOWN) {
		if (checkEntitlement(session, prefsID, kSCWriteEntitlementName)) {
			sessionPrivate->callerWriteAccess = YES;
			sessionPrivate->callerReadAccess  = YES;	// implied
		} else {
			sessionPrivate->callerWriteAccess = NO;
		}
	}

	if (needWrite) {
		if (sessionPrivate->callerWriteAccess == YES) {
			return TRUE;
		} else {
			SCLog(TRUE, LOG_ERR,
			      CFSTR("SCPreferences write access to \"%@\" denied, no entitlement for \"%@\""),
			      prefsID,
			      sessionName(session));
			return FALSE;
		}
	}

	if (sessionPrivate->callerReadAccess == UNKNOWN) {
		if (checkEntitlement(session, prefsID, kSCReadEntitlementName)) {
			sessionPrivate->callerReadAccess  = YES;
		} else {
			sessionPrivate->callerWriteAccess = NO;
		}
	}

	if (sessionPrivate->callerReadAccess == YES) {
		return TRUE;
	}

	SCLog(TRUE, LOG_ERR,
	      CFSTR("SCPreferences access to \"%@\" denied, no entitlement for \"%@\""),
	      prefsID,
	      sessionName(session));
	return FALSE;
}


typedef Boolean (*helperFunction)	(SCHelperSessionRef	session,
					 void			*info,
					 CFDataRef		data,
					 uint32_t		*status,
					 CFDataRef		*reply);


static const struct helper {
	int		command;
	const char	*commandName;
	Boolean		needsAuthorization;
	Boolean		needsWrite;
	helperFunction	func;
	void		*info;
} helpers[] = {
	{ SCHELPER_MSG_AUTH,			"AUTH",			FALSE,	FALSE,	do_Auth			, NULL		},

	{ SCHELPER_MSG_PREFS_OPEN,		"PREFS open",		FALSE,	FALSE,	do_prefs_Open		, NULL		},
	{ SCHELPER_MSG_PREFS_ACCESS,		"PREFS access",		TRUE,	FALSE,	do_prefs_Access		, NULL		},
	{ SCHELPER_MSG_PREFS_LOCK,		"PREFS lock",		TRUE,	TRUE,	do_prefs_Lock		, (void *)FALSE	},
	{ SCHELPER_MSG_PREFS_LOCKWAIT,		"PREFS lock/wait",	TRUE,	TRUE,	do_prefs_Lock		, (void *)TRUE	},
	{ SCHELPER_MSG_PREFS_COMMIT,		"PREFS commit",		TRUE,	TRUE,	do_prefs_Commit		, NULL		},
	{ SCHELPER_MSG_PREFS_APPLY,		"PREFS apply",		TRUE,	TRUE,	do_prefs_Apply		, NULL		},
	{ SCHELPER_MSG_PREFS_UNLOCK,		"PREFS unlock",		FALSE,	TRUE,	do_prefs_Unlock		, NULL		},
	{ SCHELPER_MSG_PREFS_CLOSE,		"PREFS close",		FALSE,	FALSE,	do_prefs_Close		, NULL		},
	{ SCHELPER_MSG_PREFS_SYNCHRONIZE,	"PREFS synchronize",	FALSE,	FALSE,	do_prefs_Synchronize	, NULL		},

	{ SCHELPER_MSG_INTERFACE_REFRESH,	"INTERFACE refresh",	TRUE,	TRUE,	do_interface_refresh	, NULL		},

#if	!TARGET_OS_IPHONE
	{ SCHELPER_MSG_KEYCHAIN_COPY,		"KEYCHAIN copy",	TRUE,	FALSE,	do_keychain_copy	, NULL		},
	{ SCHELPER_MSG_KEYCHAIN_EXISTS,		"KEYCHAIN exists",	TRUE,	FALSE,	do_keychain_exists	, NULL		},
	{ SCHELPER_MSG_KEYCHAIN_REMOVE,		"KEYCHAIN remove",	TRUE,	TRUE,	do_keychain_remove	, NULL		},
	{ SCHELPER_MSG_KEYCHAIN_SET,		"KEYCHAIN set",		TRUE,	TRUE,	do_keychain_set		, NULL		},
#endif	// !TARGET_OS_IPHONE

	{ SCHELPER_MSG_EXIT,			"EXIT",			FALSE,	FALSE,	do_Exit			, NULL		}
};
#define nHELPERS (sizeof(helpers)/sizeof(struct helper))


static int
findCommand(uint32_t command)
{
	int	i;

	for (i = 0; i < (int)nHELPERS; i++) {
		if (helpers[i].command == command) {
			return i;
		}
	}

	return -1;
}


static void *
newHelper(void *arg)
{
	CFRunLoopSourceRef		rls		= NULL;
	SCHelperSessionRef		session		= (SCHelperSessionRef)arg;
	SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)session;

	assert(session != NULL);
	assert(sessionPrivate->mp != NULL);

	__SCHelperSessionSetThreadName(session);

	rls = CFMachPortCreateRunLoopSource(NULL, sessionPrivate->mp, 0);
	CFRelease(sessionPrivate->mp);

	if (rls != NULL) {
		CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
		CFRelease(rls);

		SCLog(debug, LOG_DEBUG, CFSTR("%p : start"), session);
		CFRunLoopRun();
		SCLog(debug, LOG_DEBUG, CFSTR("%p : stop"), session);
	}

	return NULL;
}


#pragma mark -
#pragma mark Main loop


// MiG generated externals and functions
extern struct mig_subsystem	_helper_subsystem;
extern boolean_t		helper_server(mach_msg_header_t *, mach_msg_header_t *);


static
boolean_t
notify_server(mach_msg_header_t *request, mach_msg_header_t *reply)
{
	mach_no_senders_notification_t	*Request = (mach_no_senders_notification_t *)request;
	mig_reply_error_t		*Reply   = (mig_reply_error_t *)reply;

	reply->msgh_bits        = MACH_MSGH_BITS(MACH_MSGH_BITS_REMOTE(request->msgh_bits), 0);
	reply->msgh_remote_port = request->msgh_remote_port;
	reply->msgh_size        = sizeof(mig_reply_error_t);	/* Minimal size: update as needed */
	reply->msgh_local_port  = MACH_PORT_NULL;
	reply->msgh_id          = request->msgh_id + 100;

	if ((Request->not_header.msgh_id > MACH_NOTIFY_LAST) ||
	    (Request->not_header.msgh_id < MACH_NOTIFY_FIRST)) {
		Reply->NDR     = NDR_record;
		Reply->RetCode = MIG_BAD_ID;
		return FALSE;	/* if this is not a notification message */
	}

	switch (Request->not_header.msgh_id) {
		case MACH_NOTIFY_NO_SENDERS : {
			SCHelperSessionRef		session;

			__MACH_PORT_DEBUG(TRUE, "*** notify_server MACH_NOTIFY_NO_SENDERS", Request->not_header.msgh_local_port);

			// clean up session
			session = __SCHelperSessionFindWithPort(Request->not_header.msgh_local_port);
			if (session != NULL) {
				SCHelperSessionPrivateRef	sessionPrivate	= (SCHelperSessionPrivateRef)session;

				// release CFMachPort *and* SCHelperSession
				CFMachPortInvalidate(sessionPrivate->mp);
			}

			__MACH_PORT_DEBUG(TRUE, "*** notify_server after invalidate", Request->not_header.msgh_local_port);

			// and, lastly, remove our receive right.
			(void) mach_port_mod_refs(mach_task_self(),
						  Request->not_header.msgh_local_port,
						  MACH_PORT_RIGHT_RECEIVE, -1);

			Reply->Head.msgh_bits		= 0;
			Reply->Head.msgh_remote_port	= MACH_PORT_NULL;
			Reply->RetCode			= KERN_SUCCESS;
			return TRUE;
		}

		default :
			break;
	}

	SCLog(TRUE, LOG_ERR, CFSTR("HELP!, Received notification: port=%d, msgh_id=%d"),
	      Request->not_header.msgh_local_port,
	      Request->not_header.msgh_id);

	Reply->NDR     = NDR_record;
	Reply->RetCode = MIG_BAD_ID;
	return FALSE;	/* if this is not a notification we are handling */
}


__private_extern__
boolean_t
helper_demux(mach_msg_header_t *request, mach_msg_header_t *reply)
{
	Boolean				processed = FALSE;

	/*
	 * (attempt to) process SCHelper requests.
	 */
	processed = helper_server(request, reply);
	if (processed) {
		return TRUE;
	}

	/*
	 * (attempt to) process (NO MORE SENDERS) notification messages.
	 */
	processed = notify_server(request, reply);
	if (processed) {
		return TRUE;
	}

	/*
	 * unknown message ID, log and return an error.
	 */
	SCLog(TRUE, LOG_ERR, CFSTR("helper_demux(): unknown message ID (%d) received"), request->msgh_id);
	reply->msgh_bits        = MACH_MSGH_BITS(MACH_MSGH_BITS_REMOTE(request->msgh_bits), 0);
	reply->msgh_remote_port = request->msgh_remote_port;
	reply->msgh_size        = sizeof(mig_reply_error_t);	/* Minimal size */
	reply->msgh_local_port  = MACH_PORT_NULL;
	reply->msgh_id          = request->msgh_id + 100;
	((mig_reply_error_t *)reply)->NDR = NDR_record;
	((mig_reply_error_t *)reply)->RetCode = MIG_BAD_ID;

	return FALSE;
}


#define	MACH_MSG_BUFFER_SIZE	128


static void
helperCallback(CFMachPortRef port, void *msg, CFIndex size, void *info)
{
	mig_reply_error_t *	bufRequest	= msg;
	uint32_t		bufReply_q[MACH_MSG_BUFFER_SIZE/sizeof(uint32_t)];
	mig_reply_error_t *	bufReply	= (mig_reply_error_t *)bufReply_q;
	static CFIndex		bufSize		= 0;
	mach_msg_return_t	mr;
	int			options;

	if (bufSize == 0) {
		// get max size for MiG reply buffers
		bufSize = _helper_subsystem.maxsize;

		// check if our on-the-stack reply buffer will be big enough
		if (bufSize > sizeof(bufReply_q)) {
			SCLog(TRUE, LOG_NOTICE,
			      CFSTR("helperCallback(): buffer size should be increased > %d"),
			      _helper_subsystem.maxsize);
		}
	}

	if (bufSize > sizeof(bufReply_q)) {
		bufReply = CFAllocatorAllocate(NULL, _helper_subsystem.maxsize, 0);
	}
	bufReply->RetCode = 0;

	/* we have a request message */
	(void) helper_demux(&bufRequest->Head, &bufReply->Head);

	if (!(bufReply->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX)) {
		if (bufReply->RetCode == MIG_NO_REPLY) {
			bufReply->Head.msgh_remote_port = MACH_PORT_NULL;
		} else if ((bufReply->RetCode != KERN_SUCCESS) &&
			   (bufRequest->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX)) {
			/*
			 * destroy the request - but not the reply port
			 */
			bufRequest->Head.msgh_remote_port = MACH_PORT_NULL;
			mach_msg_destroy(&bufRequest->Head);
		}
	}

	if (bufReply->Head.msgh_remote_port != MACH_PORT_NULL) {
		/*
		 * send reply.
		 *
		 * We don't want to block indefinitely because the client
		 * isn't receiving messages from the reply port.
		 * If we have a send-once right for the reply port, then
		 * this isn't a concern because the send won't block.
		 * If we have a send right, we need to use MACH_SEND_TIMEOUT.
		 * To avoid falling off the kernel's fast RPC path unnecessarily,
		 * we only supply MACH_SEND_TIMEOUT when absolutely necessary.
		 */

		options = MACH_SEND_MSG;
		if (MACH_MSGH_BITS_REMOTE(bufReply->Head.msgh_bits) != MACH_MSG_TYPE_MOVE_SEND_ONCE) {
			options |= MACH_SEND_TIMEOUT;
		}
		mr = mach_msg(&bufReply->Head,		/* msg */
			      options,			/* option */
			      bufReply->Head.msgh_size,	/* send_size */
			      0,			/* rcv_size */
			      MACH_PORT_NULL,		/* rcv_name */
			      MACH_MSG_TIMEOUT_NONE,	/* timeout */
			      MACH_PORT_NULL);		/* notify */

		/* Has a message error occurred? */
		switch (mr) {
			case MACH_SEND_INVALID_DEST:
			case MACH_SEND_TIMED_OUT:
				break;
			default :
				/* Includes success case.  */
				goto done;
		}
	}

	if (bufReply->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) {
		mach_msg_destroy(&bufReply->Head);
	}

    done :

	if (bufReply != (mig_reply_error_t *)bufReply_q)
		CFAllocatorDeallocate(NULL, bufReply);
	return;
}


static CFStringRef
initMPCopyDescription(const void *info)
{
	return CFStringCreateWithFormat(NULL, NULL, CFSTR("<SCHelper MP>"));
}


__private_extern__
kern_return_t
_helperinit(mach_port_t			server,
	    mach_port_t			*newSession,
	    uint32_t			*status,
	    audit_token_t		audit_token)
{
	CFMachPortContext		context		= { 0
							  , NULL
							  , CFRetain
							  , CFRelease
							  , initMPCopyDescription
							  };
	kern_return_t			kr;
	mach_port_t			oldNotify;
	SCHelperSessionRef		session;
	SCHelperSessionPrivateRef	sessionPrivate;
	pthread_attr_t			tattr;
	pthread_t			tid;

	session = __SCHelperSessionFindWithPort(server);
	if (session != NULL) {
#ifdef	DEBUG
		SCLog(TRUE, LOG_DEBUG, CFSTR("_helperinit(): session is already open."));
#endif	/* DEBUG */
		*status = kSCStatusFailed;	/* you can't re-open an "open" session */
		return KERN_SUCCESS;
	}

	session = __SCHelperSessionCreate(NULL);
	assert(session != NULL);
	sessionPrivate = (SCHelperSessionPrivateRef)session;

	// create per-session port
	kr = mach_port_allocate(mach_task_self(),
				MACH_PORT_RIGHT_RECEIVE,
				&sessionPrivate->port);
	if (kr != KERN_SUCCESS) {
		SCLog(TRUE, LOG_ERR, CFSTR("_helperinit(): mach_port_allocate() failed: %s"), mach_error_string(kr));
		*status = kr;
		goto done;
	}

	*newSession = sessionPrivate->port;

	(void) mach_port_set_attributes(mach_task_self(),
					*newSession,
					MACH_PORT_IMPORTANCE_RECEIVER,
					NULL,
					0);

	//
	// Note: we create the CFMachPort *before* we insert a send
	//       right present to ensure that CF does not establish
	//       its dead name notification.
	//
	context.info = (void *)session;
	sessionPrivate->mp = _SC_CFMachPortCreateWithPort("SCHelper/session",
							  *newSession,
							  helperCallback,
							  &context);

	/* Request a notification when/if the client dies */
	kr = mach_port_request_notification(mach_task_self(),
					    *newSession,
					    MACH_NOTIFY_NO_SENDERS,
					    1,
					    *newSession,
					    MACH_MSG_TYPE_MAKE_SEND_ONCE,
					    &oldNotify);
	if (kr != KERN_SUCCESS) {
		SCLog(TRUE, LOG_ERR, CFSTR("_helperinit() mach_port_request_notification() failed: %s"), mach_error_string(kr));

		// clean up CFMachPort, mach port rights
		CFMachPortInvalidate(sessionPrivate->mp);
		CFRelease(sessionPrivate->mp);
		sessionPrivate->mp = NULL;
		(void) mach_port_mod_refs(mach_task_self(), *newSession, MACH_PORT_RIGHT_RECEIVE, -1);
		*newSession = MACH_PORT_NULL;
		*status = kSCStatusFailed;
		goto done;
	}

	if (oldNotify != MACH_PORT_NULL) {
		SCLog(TRUE, LOG_ERR, CFSTR("_helperinit(): oldNotify != MACH_PORT_NULL"));
	}

	// add send right (that will be passed back to the client)
	(void) mach_port_insert_right(mach_task_self(),
				      *newSession,
				      *newSession,
				      MACH_MSG_TYPE_MAKE_SEND);

	// save audit token
	sessionPrivate->auditToken = audit_token;

	//
	// Note: at this time we should be holding ONE send right and
	//       ONE receive right to the server.  The send right is
	//       moved to the caller.
	//

	// start per-session thread
	pthread_attr_init(&tattr);
	pthread_attr_setscope(&tattr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&tattr, 96 * 1024);	// each thread gets a 96K stack
	pthread_create(&tid, &tattr, newHelper, (void *)session);
	pthread_attr_destroy(&tattr);

	*status = kSCStatusOK;

    done :

	CFRelease(session);
	return KERN_SUCCESS;
}


__private_extern__
kern_return_t
_helperexec(mach_port_t			server,
	    uint32_t			msgID,
	    xmlData_t			dataRef,	/* raw XML bytes */
	    mach_msg_type_number_t	dataLen,
	    xmlData_t			traceRef,	/* raw XML bytes */
	    mach_msg_type_number_t	traceLen,
	    uint32_t			*status,
	    xmlDataOut_t		*replyRef,	/* raw XML bytes */
	    mach_msg_type_number_t	*replyLen)
{
	CFStringRef			backtrace	= NULL;
	CFDataRef			data		= NULL;
	int				i;
	CFDataRef			reply		= NULL;
	SCHelperSessionRef		session;

	*status   = kSCStatusOK;
	*replyRef = NULL;
	*replyLen = 0;

	if ((dataRef != NULL) && (dataLen > 0)) {
		if (!_SCUnserializeData(&data, (void *)dataRef, dataLen)) {
			*status = SCError();
		}
	}

	if ((traceRef != NULL) && (traceLen > 0)) {
		if (!_SCUnserializeString(&backtrace, NULL, (void *)traceRef, traceLen)) {
			*status = SCError();
		}
	}

	if (*status != kSCStatusOK) {
		goto done;
	}

	session = __SCHelperSessionFindWithPort(server);
	if (session == NULL) {
		*status = kSCStatusFailed;	/* you must have an open session to play */
		goto done;
	}

	i = findCommand(msgID);
	if (i == -1) {
		SCLog(TRUE, LOG_ERR, CFSTR("received unknown command : %u"), msgID);
		*status = kSCStatusInvalidArgument;
		goto done;
	}

	SCLog(debug, LOG_DEBUG,
	      CFSTR("%p : processing command \"%s\"%s"),
	      session,
	      helpers[i].commandName,
	      (data != NULL) ? " w/data" : "");

	if (helpers[i].needsAuthorization &&
	    !hasAuthorization(session, helpers[i].needsWrite)) {
		SCLog(debug, LOG_DEBUG,
		      CFSTR("%p : command \"%s\" : not authorized"),
		      session,
		      helpers[i].commandName);
		*status = kSCStatusAccessError;
	}

	if (*status == kSCStatusOK) {
		if (backtrace != NULL) {
			__SCHelperSessionAddBacktrace(session, backtrace, helpers[i].commandName);
		}
		(*helpers[i].func)(session, helpers[i].info, data, status, &reply);
	}

	if ((*status != -1) || (reply != NULL)) {
		Boolean	ok;

		SCLog(debug, LOG_DEBUG,
		      CFSTR("%p : sending status %u%s"),
		      session,
		      *status,
		      (reply != NULL) ? " w/reply" : "");

		/* serialize the data */
		if (reply != NULL) {
			CFIndex		len;

			ok = _SCSerializeData(reply, (void **)replyRef, &len);
			*replyLen = (mach_msg_type_number_t)len;
			CFRelease(reply);
			reply = NULL;
			if (!ok) {
				*status = SCError();
				goto done;
			}
		}
	}

    done :

	if (data  != NULL) CFRelease(data);
	if (backtrace != NULL) CFRelease(backtrace);
	if (reply != NULL) CFRelease(reply);
	return KERN_SUCCESS;
}


static CFStringRef
helperMPCopyDescription(const void *info)
{
	return CFStringCreateWithFormat(NULL, NULL, CFSTR("<main SCHelper MP>"));
}


static int
init_MiG(const char *service_name, int *n_listeners)
{
	CFMachPortContext	context	= { 0
					  , (void *)1
					  , NULL
					  , NULL
					  , helperMPCopyDescription
					  };
	kern_return_t		kr;
	CFMachPortRef		mp;
	CFRunLoopSourceRef	rls;
	mach_port_t		service_port	= MACH_PORT_NULL;

	kr = bootstrap_check_in(bootstrap_port, service_name, &service_port);
	if (kr != BOOTSTRAP_SUCCESS) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("SCHelper: bootstrap_check_in() failed: %s"),
		      bootstrap_strerror(kr));
		return 1;
	}

	// add a run loop source to listen for new requests
	mp = _SC_CFMachPortCreateWithPort("SCHelper/server",
					  service_port,
					  helperCallback,
					  &context);
	rls = CFMachPortCreateRunLoopSource(NULL, mp, 0);
	CFRelease(mp);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

	*n_listeners = *n_listeners + 1;

	return 0;
}


#pragma mark -
#pragma mark Main


static const struct option longopts[] = {
	{ "debug",	no_argument,	0,	'd'	},
	{ 0,		0,		0,	0	}
};


int
main(int argc, char **argv)
{
	Boolean			done		= FALSE;
	int			err		= 0;
	int			gen_reported	= 0;
	int			idle		= 0;
	int			n_listeners	= 0;
//	extern int		optind;
	int			opt;
	int			opti;

	openlog("SCHelper", LOG_CONS|LOG_PID, LOG_DAEMON);

	// process any arguments
	while ((opt = getopt_long(argc, argv, "d", longopts, &opti)) != -1) {
		switch(opt) {
			case 'd':
				debug = TRUE;
				break;
			case 0 :
//				if (strcmp(longopts[opti].name, "debug") == 1) {
//				}
				break;
			case '?':
			default :
				SCLog(TRUE, LOG_ERR,
				      CFSTR("ignoring unknown or ambiguous command line option"));
				break;
		}
	}
//	argc -= optind;
//	argv += optind;

	if (geteuid() != 0) {
		SCLog(TRUE, LOG_ERR, CFSTR("%s"), strerror(EACCES));
		exit(EACCES);
	}

	main_runLoop = CFRunLoopGetCurrent();

	err = init_MiG("com.apple.SystemConfiguration.helper", &n_listeners);
	if ((err != 0) || (n_listeners == 0)) {
		exit(err);
	}

	pthread_setname_np("SCHelper main thread");

	while (!done) {
		SInt32	rlStatus;
		int	gen_current;

		rlStatus = CFRunLoopRunInMode(kCFRunLoopDefaultMode, 15.0, TRUE);

		pthread_mutex_lock(&sessions_lock);

		if (sessions != NULL) {
			if (rlStatus == kCFRunLoopRunTimedOut) {
				idle++;

				if ((CFSetGetCount(sessions) == 0) && (sessions_closed == 0)) {
					// if we don't have any open sessions and no
					// sessions have recently been closed
					done = TRUE;
				}
			} else {
				idle = 0;
			}
		}
		gen_current = sessions_generation;
		sessions_closed = 0;

		if (!done && (idle >= (2 * 60 / 15))) {
			if (gen_reported != gen_current) {
				FILE	*logFile	= NULL;

				SCLog(TRUE, LOG_NOTICE, CFSTR("active (but IDLE) sessions"));
				CFSetApplyFunction(sessions, __SCHelperSessionLog, (void *)&logFile);
				gen_reported = gen_current;

				if (logFile != NULL) {
					(void) fclose(logFile);
				}
			}
			idle = 0;
		}

		pthread_mutex_unlock(&sessions_lock);
	}

	exit(EX_OK);
}
