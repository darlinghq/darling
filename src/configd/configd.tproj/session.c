/*
 * Copyright (c) 2000, 2001, 2003-2005, 2007-2019 Apple Inc. All rights reserved.
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

/*
 * Modification History
 *
 * June 1, 2001			Allan Nathanson <ajn@apple.com>
 * - public API conversion
 *
 * March 24, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <SystemConfiguration/SystemConfiguration.h>
#include "configd.h"
#include "configd_server.h"
#include "pattern.h"
#include "session.h"

#include <unistd.h>
#include <bsm/libbsm.h>
#include <os/state_private.h>
#include <sandbox.h>

#if !TARGET_OS_SIMULATOR || (defined(IPHONE_SIMULATOR_HOST_MIN_VERSION_REQUIRED) && (IPHONE_SIMULATOR_HOST_MIN_VERSION_REQUIRED >= 1090))
#define HAVE_MACHPORT_GUARDS
#endif


/* information maintained for the main listener */
static serverSessionRef		server_session		= NULL;

/*
 * information maintained for each active session
 * Note: sync w/sessionQueue()
 */
static CFMutableDictionaryRef	client_sessions		= NULL;


static dispatch_queue_t
sessionQueue(void)
{
	static dispatch_once_t	once;
	static dispatch_queue_t	q;

	dispatch_once(&once, ^{
		// allocate mapping between [client] session mach port and session info
		client_sessions = CFDictionaryCreateMutable(NULL,
							    0,
							    NULL,	// use the actual mach_port_t as the key
							    &kCFTypeDictionaryValueCallBacks);

		// and a queue to synchronize access to the mapping
		q = dispatch_queue_create("SCDynamicStore/sessions", NULL);
	});

	return q;
}


#pragma mark -
#pragma mark __serverSession object

static CFStringRef		__serverSessionCopyDescription	(CFTypeRef cf);
static void			__serverSessionDeallocate	(CFTypeRef cf);

static const CFRuntimeClass	__serverSessionClass = {
	0,					// version
	"serverSession",			// className
	NULL,					// init
	NULL,					// copy
	__serverSessionDeallocate,		// dealloc
	NULL,					// equal
	NULL,					// hash
	NULL,					// copyFormattingDesc
	__serverSessionCopyDescription	// copyDebugDesc
};

static CFTypeID	__serverSessionTypeID	= _kCFRuntimeNotATypeID;


static CFStringRef
__serverSessionCopyDescription(CFTypeRef cf)
{
	CFAllocatorRef		allocator	= CFGetAllocator(cf);
	CFMutableStringRef	result;
	serverSessionRef	session		= (serverSessionRef)cf;

	result = CFStringCreateMutable(allocator, 0);
	CFStringAppendFormat(result, NULL, CFSTR("<serverSession %p [%p]> {"), cf, allocator);

	// add client port
	CFStringAppendFormat(result, NULL, CFSTR("port = 0x%x (%d)"), session->key, session->key);

	// add session info
	if (session->name != NULL) {
		CFStringAppendFormat(result, NULL, CFSTR(", name = %@"), session->name);
	}

	CFStringAppendFormat(result, NULL, CFSTR("}"));
	return result;
}


static void
__serverSessionDeallocate(CFTypeRef cf)
{
#pragma unused(cf)
	serverSessionRef	session		= (serverSessionRef)cf;

	if (session->changedKeys != NULL)	CFRelease(session->changedKeys);
	if (session->name != NULL)		CFRelease(session->name);
	if (session->sessionKeys != NULL)	CFRelease(session->sessionKeys);

	return;
}


static serverSessionRef
__serverSessionCreate(CFAllocatorRef allocator, mach_port_t server)
{
	static dispatch_once_t	once;
	serverSessionRef	session;
	uint32_t		size;

	// initialize runtime
	dispatch_once(&once, ^{
		__serverSessionTypeID = _CFRuntimeRegisterClass(&__serverSessionClass);
	});

	// allocate session
	size    = sizeof(serverSession) - sizeof(CFRuntimeBase);
	session = (serverSessionRef)_CFRuntimeCreateInstance(allocator,
							     __serverSessionTypeID,
							     size,
							     NULL);
	if (session == NULL) {
		return NULL;
	}

	// if needed, allocate a mach port for SCDynamicStore client
	if (server == MACH_PORT_NULL) {
		kern_return_t		kr;
		mach_port_t		mp	= MACH_PORT_NULL;
#ifdef	HAVE_MACHPORT_GUARDS
		mach_port_options_t	opts;
#endif	// HAVE_MACHPORT_GUARDS

	    retry_allocate :

#ifdef	HAVE_MACHPORT_GUARDS
		memset(&opts, 0, sizeof(opts));
		opts.flags = MPO_CONTEXT_AS_GUARD;

		kr = mach_port_construct(mach_task_self(), &opts, (mach_port_context_t)session, &mp);
#else	// HAVE_MACHPORT_GUARDS
		kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &mp);
#endif	// HAVE_MACHPORT_GUARDS

		if (kr != KERN_SUCCESS) {
			char	*err	= NULL;

			SC_log(LOG_NOTICE, "could not allocate mach port: %s", mach_error_string(kr));
			if ((kr == KERN_NO_SPACE) || (kr == KERN_RESOURCE_SHORTAGE)) {
				sleep(1);
				goto retry_allocate;
			}

			(void) asprintf(&err, "Could not allocate mach port: %s", mach_error_string(kr));
			_SC_crash(err != NULL ? err : "Could not allocate new session (mach) port",
				  NULL,
				  NULL);
			if (err != NULL) free(err);
			CFRelease(session);
			return NULL;
		}

		// insert send right that will be moved to the client
		kr = mach_port_insert_right(mach_task_self(),
					    mp,
					    mp,
					    MACH_MSG_TYPE_MAKE_SEND);
		if (kr != KERN_SUCCESS) {
			/*
			 * We can't insert a send right into our own port!  This should
			 * only happen if someone stomped on OUR port (so let's leave
			 * the port alone).
			 */
			SC_log(LOG_ERR, "mach_port_insert_right() failed: %s", mach_error_string(kr));
			CFRelease(session);
			return NULL;
		}

		server = mp;
	}

	session->callerEUID		= 1;		/* not "root" */
	session->callerRootAccess	= UNKNOWN;
	session->callerWriteEntitlement	= kCFNull;	/* UNKNOWN */
	session->key			= server;
//	session->store			= NULL;

	return session;
}


#pragma mark -
#pragma mark SCDynamicStore state handler


static void
addSessionReference(const void *key, const void *value, void *context)
{
#pragma unused(key)
	CFMutableDictionaryRef	dict		= (CFMutableDictionaryRef)context;
	serverSessionRef	session		= (serverSessionRef)value;

	if (session->name != NULL) {
		int		cnt;
		CFNumberRef	num;

		if (!CFDictionaryGetValueIfPresent(dict,
						   session->name,
						   (const void **)&num) ||
		    !CFNumberGetValue(num, kCFNumberIntType, &cnt)) {
			// if first session
			cnt = 0;
		}
		cnt++;
		num = CFNumberCreate(NULL, kCFNumberIntType, &cnt);
		CFDictionarySetValue(dict, session->name, num);
		CFRelease(num);
	}

	return;
}


static void
add_state_handler()
{
	os_state_block_t	state_block;

	state_block = ^os_state_data_t(os_state_hints_t hints) {
#pragma unused(hints)
		CFDataRef		data	= NULL;
		CFIndex			n;
		Boolean			ok;
		os_state_data_t		state_data;
		size_t			state_data_size;
		CFIndex			state_len;

		n = CFDictionaryGetCount(client_sessions);
		if (n < 500) {
			CFStringRef	str;

			str = CFStringCreateWithFormat(NULL, NULL, CFSTR("n = %ld"), n);
			ok = _SCSerialize(str, &data, NULL, NULL);
			CFRelease(str);
		} else {
			CFMutableDictionaryRef	dict;

			dict = CFDictionaryCreateMutable(NULL,
							 0,
							 &kCFTypeDictionaryKeyCallBacks,
							 &kCFTypeDictionaryValueCallBacks);
			CFDictionaryApplyFunction(client_sessions, addSessionReference, dict);
			ok = _SCSerialize(dict, &data, NULL, NULL);
			CFRelease(dict);
		}

		state_len = (ok && (data != NULL)) ? CFDataGetLength(data) : 0;
		state_data_size = OS_STATE_DATA_SIZE_NEEDED(state_len);
		if (state_data_size > MAX_STATEDUMP_SIZE) {
			SC_log(LOG_ERR, "SCDynamicStore/sessions : state data too large (%zd > %zd)",
			       state_data_size,
			       (size_t)MAX_STATEDUMP_SIZE);
			if (data != NULL) CFRelease(data);
			return NULL;
		}

		state_data = calloc(1, state_data_size);
		if (state_data == NULL) {
			SC_log(LOG_ERR, "SCDynamicStore/sessions: could not allocate state data");
			if (data != NULL) CFRelease(data);
			return NULL;
		}

		state_data->osd_type = OS_STATE_DATA_SERIALIZED_NSCF_OBJECT;
		state_data->osd_data_size = (uint32_t)state_len;
		strlcpy(state_data->osd_title, "SCDynamicStore/sessions", sizeof(state_data->osd_title));
		if (state_len > 0) {
			memcpy(state_data->osd_data, CFDataGetBytePtr(data), state_len);
		}
		if (data != NULL) CFRelease(data);

		return state_data;
	};

	(void) os_state_add_handler(sessionQueue(), state_block);
	return;
}


#pragma mark -
#pragma mark SCDynamicStore session management


__private_extern__
serverSessionRef
getSession(mach_port_t server)
{
	__block serverSessionRef	session;

	assert(server != MACH_PORT_NULL);
	dispatch_sync(sessionQueue(), ^{
		session = (serverSessionRef)CFDictionaryGetValue(client_sessions,
								 (const void *)(uintptr_t)server);
	});

	return session;
}


__private_extern__
serverSessionRef
getSessionNum(CFNumberRef serverNum)
{
	union {
		mach_port_t	mp;
		uint64_t	val;
	} server;
	serverSessionRef	session;

	(void) CFNumberGetValue(serverNum, kCFNumberSInt64Type, &server.val);
	session = getSession(server.mp);

	return session;
}


__private_extern__
serverSessionRef
getSessionStr(CFStringRef serverKey)
{
	mach_port_t		server;
	serverSessionRef	session;
	char			str[16];

	(void) _SC_cfstring_to_cstring(serverKey, str, sizeof(str), kCFStringEncodingASCII);
	server = atoi(str);
	session = getSession(server);

	return session;
}


__private_extern__
serverSessionRef
tempSession(mach_port_t server, CFStringRef name, audit_token_t auditToken)
{
	static dispatch_once_t		once;
	SCDynamicStorePrivateRef	storePrivate;	/* temp session */
	static serverSession		temp_session;

	dispatch_once(&once, ^{
		temp_session = *server_session;		/* use "server" session clone */
		(void) __SCDynamicStoreOpen(&temp_session.store, NULL);
	});

	if (temp_session.key != server) {
		// if not SCDynamicStore "server" port
		return NULL;
	}

	/* save audit token, caller entitlements */
	temp_session.auditToken			= auditToken;
	temp_session.callerEUID			= 1;		/* not "root" */
	temp_session.callerRootAccess		= UNKNOWN;
	if ((temp_session.callerWriteEntitlement != NULL) &&
	    (temp_session.callerWriteEntitlement != kCFNull)) {
		CFRelease(temp_session.callerWriteEntitlement);
	}
	temp_session.callerWriteEntitlement	= kCFNull;	/* UNKNOWN */

	/* save name */
	storePrivate = (SCDynamicStorePrivateRef)temp_session.store;
	if (storePrivate->name != NULL) CFRelease(storePrivate->name);
	storePrivate->name = CFRetain(name);

	return &temp_session;
}


__private_extern__
void
addSession(serverSessionRef session, Boolean isMain)
{
	session->serverChannel = dispatch_mach_create_f("configd/SCDynamicStore",
							server_queue(),
							(void *)session,
							server_mach_channel_handler);
	if (!isMain) {
		// if not main SCDynamicStore port, watch for exit
		dispatch_mach_request_no_senders(session->serverChannel);
	}
#if	TARGET_OS_SIMULATOR
	// simulators don't support MiG QoS propagation yet
	dispatch_set_qos_class_fallback(session->serverChannel, QOS_CLASS_USER_INITIATED);
#else
	dispatch_set_qos_class_fallback(session->serverChannel, QOS_CLASS_BACKGROUND);
#endif
	dispatch_mach_connect(session->serverChannel, session->key, MACH_PORT_NULL, NULL);
	return;
}


__private_extern__
serverSessionRef
addClient(mach_port_t server, audit_token_t audit_token)
{

	__block serverSessionRef	newSession	= NULL;

	dispatch_sync(sessionQueue(), ^{
		Boolean		ok;

		// check to see if we already have an open session
		ok = CFDictionaryContainsKey(client_sessions,
					     (const void *)(uintptr_t)server);
		if (ok) {
			// if we've already added a session for this port
			return;
		}

		// allocate a new session for "the" server
		newSession = __serverSessionCreate(NULL, MACH_PORT_NULL);
		if (newSession != NULL) {
			// and add a port --> session mapping
			CFDictionarySetValue(client_sessions,
					     (const void *)(uintptr_t)newSession->key,
					     newSession);

			// save the audit_token in case we need to check the callers credentials
			newSession->auditToken = audit_token;

			CFRelease(newSession);	// reference held by dictionary
		}
	});

	if (newSession != NULL) {
		addSession(newSession, FALSE);
	}

	return newSession;
}


__private_extern__
serverSessionRef
addServer(mach_port_t server)
{
	// allocate a session for "the" server
	server_session = __serverSessionCreate(NULL, server);
	addSession(server_session, TRUE);

	// add a state dump handler
	add_state_handler();

	return server_session;
}


__private_extern__
void
cleanupSession(serverSessionRef session)
{
	mach_port_t	server		= session->key;

	SC_trace("cleanup : %5d", server);

	/*
	 * Close any open connections including cancelling any outstanding
	 * notification requests and releasing any locks.
	 */
	__MACH_PORT_DEBUG(TRUE, "*** cleanupSession", server);
	(void) __SCDynamicStoreClose(&session->store);
	__MACH_PORT_DEBUG(TRUE, "*** cleanupSession (after __SCDynamicStoreClose)", server);

	/*
	 * Our send right has already been removed. Remove our receive right.
	 */
#ifdef	HAVE_MACHPORT_GUARDS
	(void) mach_port_destruct(mach_task_self(), server, 0, (mach_port_context_t)session);
#else	// HAVE_MACHPORT_GUARDS
	(void) mach_port_mod_refs(mach_task_self(), server, MACH_PORT_RIGHT_RECEIVE, -1);
#endif	// HAVE_MACHPORT_GUARDS

	/*
	 * release any entitlement info
	 */
	if ((session->callerWriteEntitlement != NULL) &&
	    (session->callerWriteEntitlement != kCFNull)) {
		CFRelease(session->callerWriteEntitlement);
	}

	/*
	 * get rid of the per-session structure.
	 */
	dispatch_sync(sessionQueue(), ^{
		CFDictionaryRemoveValue(client_sessions,
					(const void *)(uintptr_t)server);
	});

	return;
}


__private_extern__
void
closeSession(serverSessionRef session)
{
	/*
	 * cancel and release the mach channel
	 */
	if (session->serverChannel != NULL) {
		dispatch_mach_cancel(session->serverChannel);
		dispatch_release(session->serverChannel);
		session->serverChannel = NULL;
	}

	return;
}


typedef struct ReportSessionInfo {
	FILE	*f;
	int	n;
} ReportSessionInfo, *ReportSessionInfoRef;

static void
printOne(const void *key, const void *value, void *context)
{
#pragma unused(key)
	ReportSessionInfoRef	reportInfo	= (ReportSessionInfoRef)context;
	serverSessionRef	session		= (serverSessionRef)value;

	SCPrint(TRUE, reportInfo->f, CFSTR("  %d : port = 0x%x"), ++reportInfo->n, session->key);
	SCPrint(TRUE, reportInfo->f, CFSTR(", name = %@"), session->name);
	if (session->changedKeys != NULL) {
		SCPrint(TRUE, reportInfo->f, CFSTR("\n    changedKeys = %@"), session->changedKeys);
	}
	if (session->sessionKeys != NULL) {
		SCPrint(TRUE, reportInfo->f, CFSTR("\n    sessionKeys = %@"), session->sessionKeys);
	}
	SCPrint(TRUE, reportInfo->f, CFSTR("\n"));
	return;
}


__private_extern__
void
listSessions(FILE *f)
{
	dispatch_sync(sessionQueue(), ^{
		ReportSessionInfo	reportInfo	= { .f = f, .n = 0 };

		SCPrint(TRUE, f, CFSTR("Current sessions :\n"));
		CFDictionaryApplyFunction(client_sessions,
					  printOne,
					  (void *)&reportInfo);
		SCPrint(TRUE, f, CFSTR("\n"));
	});
	return;
}


#include <Security/Security.h>
#include <Security/SecTask.h>

static CFTypeRef
copyEntitlement(serverSessionRef session, CFStringRef entitlement)
{
	SecTaskRef	task;
	CFTypeRef	value	= NULL;

	// Create the security task from the audit token
	task = SecTaskCreateWithAuditToken(NULL, session->auditToken);
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
				SC_log(LOG_NOTICE, "SecTaskCopyValueForEntitlement(,\"%@\",) failed, error = %@ : %@",
				       entitlement,
				       error,
				       session->name);
			}
			CFRelease(error);
		}

		CFRelease(task);
	} else {
		SC_log(LOG_NOTICE, "SecTaskCreateWithAuditToken() failed: %@",
		       session->name);
	}

	return value;
}


static pid_t
sessionPid(serverSessionRef session)
{
	pid_t	caller_pid;

#if     (__MAC_OS_X_VERSION_MIN_REQUIRED >= 1080) && !TARGET_OS_IPHONE
	caller_pid = audit_token_to_pid(session->auditToken);
#else	// (__MAC_OS_X_VERSION_MIN_REQUIRED >= 1080) && !TARGET_OS_IPHONE
	audit_token_to_au32(session->auditToken,
			    NULL,		// auidp
			    NULL,		// euid
			    NULL,		// egid
			    NULL,		// ruid
			    NULL,		// rgid
			    &caller_pid,	// pid
			    NULL,		// asid
			    NULL);		// tid
#endif	// (__MAC_OS_X_VERSION_MIN_REQUIRED >= 1080) && !TARGET_OS_IPHONE

	return caller_pid;
}


__private_extern__
Boolean
hasRootAccess(serverSessionRef session)
{
#if	!TARGET_OS_SIMULATOR

	if (session->callerRootAccess == UNKNOWN) {
#if     (__MAC_OS_X_VERSION_MIN_REQUIRED >= 1080) && !TARGET_OS_IPHONE
		session->callerEUID = audit_token_to_euid(session->auditToken);
#else	// (__MAC_OS_X_VERSION_MIN_REQUIRED >= 1080) && !TARGET_OS_IPHONE
		audit_token_to_au32(session->auditToken,
				    NULL,			// auidp
				    &session->callerEUID,	// euid
				    NULL,			// egid
				    NULL,			// ruid
				    NULL,			// rgid
				    NULL,			// pid
				    NULL,			// asid
				    NULL);			// tid
#endif	// (__MAC_OS_X_VERSION_MIN_REQUIRED >= 1080) && !TARGET_OS_IPHONE
		session->callerRootAccess = (session->callerEUID == 0) ? YES : NO;
	}

	return (session->callerRootAccess == YES) ? TRUE : FALSE;

#else	// !TARGET_OS_SIMULATOR
#pragma unused(session)

	/*
	 * assume that all processes interacting with
	 * the iOS Simulator "configd" are OK.
	 */
	return TRUE;

#endif	// !TARGET_OS_SIMULATOR
}


__private_extern__
Boolean
hasWriteAccess(serverSessionRef session, const char *op, CFStringRef key)
{
	Boolean	isSetup;

	// need to special case writing "Setup:" keys
	isSetup = CFStringHasPrefix(key, kSCDynamicStoreDomainSetup);

	if (hasRootAccess(session)) {
		pid_t	pid;

		// grant write access to eUID==0 processes

		pid = sessionPid(session);
		if (isSetup && (pid != getpid())) {
			/*
			 * WAIT!!!
			 *
			 * This is NOT configd (or a plugin) trying to
			 * write to an SCDynamicStore "Setup:" key.  In
			 * general, this is unwise and we should at the
			 * very least complain.
			 */
			SC_log(LOG_NOTICE, "*** Non-configd process (pid=%d) attempting to %s \"%@\" ***",
			       pid,
			       op,
			       key);
		}

		return TRUE;
	}

	if (isSetup) {
		/*
		 * STOP!!!
		 *
		 * This is a non-root process trying to write to
		 * an SCDynamicStore "Setup:" key.  This is not
		 * something we should ever allow (regardless of
		 * any entitlements).
		 */
		SC_log(LOG_NOTICE, "*** Non-root process (pid=%d) attempting to modify \"%@\" ***",
		       sessionPid(session),
		       key);

		return FALSE;
	}

	if (session->callerWriteEntitlement == kCFNull) {
		session->callerWriteEntitlement = copyEntitlement(session,
								  kSCWriteEntitlementName);
	}

	if (session->callerWriteEntitlement == NULL) {
		return FALSE;
	}

	if (isA_CFBoolean(session->callerWriteEntitlement) &&
	    CFBooleanGetValue(session->callerWriteEntitlement)) {
		// grant write access to "entitled" processes
		return TRUE;
	}

	if (isA_CFDictionary(session->callerWriteEntitlement)) {
		CFArrayRef	keys;
		CFArrayRef	patterns;

		keys = CFDictionaryGetValue(session->callerWriteEntitlement, CFSTR("keys"));
		if (isA_CFArray(keys)) {
			if (CFArrayContainsValue(keys,
						 CFRangeMake(0, CFArrayGetCount(keys)),
						 key)) {
				// if key matches one of the entitlement "keys", grant
				// write access
				return TRUE;
			}
		}

		patterns = CFDictionaryGetValue(session->callerWriteEntitlement, CFSTR("patterns"));
		if (isA_CFArray(patterns)) {
			CFIndex		i;
			CFIndex		n	= CFArrayGetCount(patterns);

			for (i = 0; i < n; i++) {
				CFStringRef	pattern;

				pattern = CFArrayGetValueAtIndex(patterns, i);
				if (isA_CFString(pattern)) {
					if (patternKeyMatches(pattern, key)) {
						// if key matches one of the entitlement
						// "patterns", grant write access
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}


__private_extern__
Boolean
hasPathAccess(serverSessionRef session, const char *path)
{
	pid_t	pid;
	char	realPath[PATH_MAX];

	if (realpath(path, realPath) == NULL) {
		SC_log(LOG_INFO, "realpath() failed: %s", strerror(errno));
		return FALSE;
	}

#if	(__MAC_OS_X_VERSION_MIN_REQUIRED >= 1080) && !TARGET_OS_IPHONE
	pid = audit_token_to_pid(session->auditToken);
#else	// (__MAC_OS_X_VERSION_MIN_REQUIRED >= 1080) && !TARGET_OS_IPHONE
	audit_token_to_au32(session->auditToken,
			    NULL,		// auidp
			    NULL,		// euid
			    NULL,		// egid
			    NULL,		// ruid
			    NULL,		// rgid
			    &pid,		// pid
			    NULL,		// asid
			    NULL);		// tid
#endif	// (__MAC_OS_X_VERSION_MIN_REQUIRED >= 1080) && !TARGET_OS_IPHONE
	if (sandbox_check(pid,							// pid
			  "file-write-data",					// operation
			  SANDBOX_FILTER_PATH | SANDBOX_CHECK_NO_REPORT,	// sandbox_filter_type
			  realPath) > 0) {					// ...
		SC_log(LOG_INFO, "sandbox access denied: %s", strerror(errno));
		return FALSE;
	}

	return TRUE;
}
