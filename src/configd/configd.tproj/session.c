/*
 * Copyright (c) 2000, 2001, 2003-2005, 2007-2014 Apple Inc. All rights reserved.
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
#include <sandbox.h>

#if !TARGET_IPHONE_SIMULATOR || (defined(IPHONE_SIMULATOR_HOST_MIN_VERSION_REQUIRED) && (IPHONE_SIMULATOR_HOST_MIN_VERSION_REQUIRED >= 1090))
#define HAVE_MACHPORT_GUARDS
#endif


/* information maintained for each active session */
static serverSessionRef	*sessions	= NULL;
static int		nSessions	= 0;	/* # of allocated sessions */
static int		lastSession	= -1;	/* # of last used session */

/* CFMachPortInvalidation runloop */
static CFRunLoopRef	sessionRunLoop	= NULL;

/* temp session */
static serverSessionRef	temp_session	= NULL;


__private_extern__
serverSessionRef
getSession(mach_port_t server)
{
	int	i;

	if (server == MACH_PORT_NULL) {
		SCLog(TRUE, LOG_ERR, CFSTR("Excuse me, why is getSession() being called with an invalid port?"));
		return NULL;
	}

	/* look for matching session (note: slot 0 is the "server" port) */
	for (i = 1; i <= lastSession; i++) {
		serverSessionRef	thisSession = sessions[i];

		if (thisSession == NULL) {
			/* found an empty slot, skip it */
			continue;
		}

		if (thisSession->key == server) {
			/* we've seen this server before */
			return thisSession;
		}

		if ((thisSession->store != NULL) &&
		    (((SCDynamicStorePrivateRef)thisSession->store)->notifySignalTask == server)) {
			/* we've seen this task port before */
			return thisSession;
		}
	}

	/* no sessions available */
	return NULL;
}


__private_extern__
serverSessionRef
tempSession(mach_port_t server, CFStringRef name, audit_token_t auditToken)
{
	static dispatch_once_t		once;
	SCDynamicStorePrivateRef	storePrivate;

	if (sessions[0]->key != server) {
		// if not SCDynamicStore "server" port
		return NULL;
	}

	dispatch_once(&once, ^{
		temp_session = sessions[0];	/* use "server" session */
		(void) __SCDynamicStoreOpen(&temp_session->store, NULL);
	});

	/* save audit token, caller entitlements */
	temp_session->auditToken		= auditToken;
	temp_session->callerEUID		= 1;		/* not "root" */
	temp_session->callerRootAccess		= UNKNOWN;
	if ((temp_session->callerWriteEntitlement != NULL) &&
	    (temp_session->callerWriteEntitlement != kCFNull)) {
		CFRelease(temp_session->callerWriteEntitlement);
	}
	temp_session->callerWriteEntitlement	= kCFNull;	/* UNKNOWN */

	/* save name */
	storePrivate = (SCDynamicStorePrivateRef)temp_session->store;
	if (storePrivate->name != NULL) CFRelease(storePrivate->name);
	storePrivate->name = CFRetain(name);

	return temp_session;
}


__private_extern__
serverSessionRef
addSession(mach_port_t server, CFStringRef (*copyDescription)(const void *info))
{
	CFMachPortContext	context		= { 0, NULL, NULL, NULL, NULL };
	kern_return_t		kr;
	mach_port_t		mp		= server;
	int			n		= -1;
	serverSessionRef	newSession	= NULL;

	/* save current (SCDynamicStore) runloop */
	if (sessionRunLoop == NULL) {
		sessionRunLoop = CFRunLoopGetCurrent();
	}

	if (nSessions <= 0) {
		/* if first session (the "server" port) */
		n = 0;		/* use slot "0" */
		lastSession = 0;	/* last used slot */

		nSessions = 64;
		sessions = malloc(nSessions * sizeof(serverSessionRef));

		// allocate a new session for "the" server
		newSession = calloc(1, sizeof(serverSession));
	} else {
		int			i;
#ifdef	HAVE_MACHPORT_GUARDS
		mach_port_options_t	opts;
#endif	// HAVE_MACHPORT_GUARDS

		/* check to see if we already have an open session (note: slot 0 is the "server" port) */
		for (i = 1; i <= lastSession; i++) {
			serverSessionRef	thisSession	= sessions[i];

			if (thisSession == NULL) {
				/* found an empty slot */
				if (n < 0) {
					/* keep track of the first [empty] slot */
					n = i;
				}

				/* and keep looking for a matching session */
				continue;
			}

			if (thisSession->key == server) {
				/* we've seen this server before */
				return NULL;
			}

			if ((thisSession->store != NULL) &&
				   (((SCDynamicStorePrivateRef)thisSession->store)->notifySignalTask == server)) {
				/* we've seen this task port before */
				return NULL;
			}
		}

		/* add a new session */
		if (n < 0) {
			/* if no empty slots */
			n = ++lastSession;
			if (lastSession >= nSessions) {
				/* expand the session list */
				nSessions *= 2;
				sessions = reallocf(sessions, (nSessions * sizeof(serverSessionRef)));
			}
		}

		// allocate a session for this client
		newSession = calloc(1, sizeof(serverSession));

		// create mach port for SCDynamicStore client
		mp = MACH_PORT_NULL;

	    retry_allocate :

#ifdef	HAVE_MACHPORT_GUARDS
		bzero(&opts, sizeof(opts));
		opts.flags = MPO_CONTEXT_AS_GUARD;

		kr = mach_port_construct(mach_task_self(), &opts, newSession, &mp);
#else	// HAVE_MACHPORT_GUARDS
		kr = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &mp);
#endif	// HAVE_MACHPORT_GUARDS

		if (kr != KERN_SUCCESS) {
			char	*err	= NULL;

			SCLog(TRUE, LOG_ERR, CFSTR("addSession: could not allocate mach port: %s"), mach_error_string(kr));
			if ((kr == KERN_NO_SPACE) || (kr == KERN_RESOURCE_SHORTAGE)) {
				sleep(1);
				goto retry_allocate;
			}

			(void) asprintf(&err, "addSession: could not allocate mach port: %s", mach_error_string(kr));
			_SC_crash(err != NULL ? err : "addSession: could not allocate mach port",
				  NULL,
				  NULL);
			if (err != NULL) free(err);

			free(newSession);
			return NULL;
		}
	}

	// create server port
	context.info		= newSession;
	context.copyDescription = copyDescription;

	//
	// Note: we create the CFMachPort *before* we insert a send
	//       right present to ensure that CF does not establish
	//       its dead name notification.
	//
	newSession->serverPort = _SC_CFMachPortCreateWithPort("SCDynamicStore/session",
							      mp,
							      configdCallback,
							      &context);

	if (n > 0) {
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
			SCLog(TRUE, LOG_ERR, CFSTR("addSession mach_port_insert_right(): %s"), mach_error_string(kr));

			free(newSession);
			return NULL;
		}
	}

	sessions[n] = newSession;
	sessions[n]->key			= mp;
//	sessions[n]->serverRunLoopSource	= NULL;
//	sessions[n]->store			= NULL;
	sessions[n]->callerEUID			= 1;		/* not "root" */
	sessions[n]->callerRootAccess		= UNKNOWN;
	sessions[n]->callerWriteEntitlement	= kCFNull;	/* UNKNOWN */

	return newSession;
}


__private_extern__
void
cleanupSession(mach_port_t server)
{
	int		i;

	for (i = 1; i <= lastSession; i++) {
		CFStringRef		sessionKey;
		serverSessionRef	thisSession = sessions[i];

		if (thisSession == NULL) {
			/* found an empty slot, skip it */
			continue;
		}

		if (thisSession->key == server) {
			/*
			 * session entry still exists.
			 */

			if (_configd_trace) {
				SCTrace(TRUE, _configd_trace, CFSTR("cleanup : %5d\n"), server);
			}

			/*
			 * Close any open connections including cancelling any outstanding
			 * notification requests and releasing any locks.
			 */
			__MACH_PORT_DEBUG(TRUE, "*** cleanupSession", server);
			(void) __SCDynamicStoreClose(&thisSession->store);
			__MACH_PORT_DEBUG(TRUE, "*** cleanupSession (after __SCDynamicStoreClose)", server);

			/*
			 * Our send right has already been removed. Remove our receive right.
			 */
#ifdef	HAVE_MACHPORT_GUARDS
			(void) mach_port_destruct(mach_task_self(), server, 0, thisSession);
#else	// HAVE_MACHPORT_GUARDS
			(void) mach_port_mod_refs(mach_task_self(), server, MACH_PORT_RIGHT_RECEIVE, -1);
#endif	// HAVE_MACHPORT_GUARDS

			/*
			 * release any entitlement info
			 */
			if ((thisSession->callerWriteEntitlement != NULL) &&
			    (thisSession->callerWriteEntitlement != kCFNull)) {
				CFRelease(thisSession->callerWriteEntitlement);
			}

			/*
			 * We don't need any remaining information in the
			 * sessionData dictionary, remove it.
			 */
			sessionKey = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), server);
			CFDictionaryRemoveValue(sessionData, sessionKey);
			CFRelease(sessionKey);

			/*
			 * get rid of the per-session structure.
			 */
			free(thisSession);
			sessions[i] = NULL;

			if (i == lastSession) {
				/* we are removing the last session, update last used slot */
				while (--lastSession > 0) {
					if (sessions[lastSession] != NULL) {
						break;
					}
				}
			}

			return;
		}
	}

	SCLog(TRUE, LOG_ERR, CFSTR("MACH_NOTIFY_NO_SENDERS w/no session, port = %d"), server);
	__MACH_PORT_DEBUG(TRUE, "*** cleanupSession w/no session", server);
	return;
}


__private_extern__
void
listSessions(FILE *f)
{
	int	i;

	SCPrint(TRUE, f, CFSTR("Current sessions :\n"));
	for (i = 0; i <= lastSession; i++) {
		serverSessionRef	thisSession = sessions[i];

		if (thisSession == NULL) {
			continue;
		}

		SCPrint(TRUE, f, CFSTR("\t%d : port = 0x%x"), i, thisSession->key);

		if (thisSession->store != NULL) {
			SCDynamicStorePrivateRef	storePrivate	= (SCDynamicStorePrivateRef)thisSession->store;

			if (storePrivate->notifySignalTask != TASK_NULL) {
			       SCPrint(TRUE, f, CFSTR(", task = %d"), storePrivate->notifySignalTask);
			}
		}

		if (sessionData != NULL) {
			CFDictionaryRef	info;
			CFStringRef	key;

			key = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), thisSession->key);
			info = CFDictionaryGetValue(sessionData, key);
			CFRelease(key);
			if (info != NULL) {
				CFStringRef	name;

				name = CFDictionaryGetValue(info, kSCDName);
				if (name != NULL) {
					SCPrint(TRUE, f, CFSTR(", name = %@"), name);
				}
			}
		}

		if (thisSession->serverPort != NULL) {
			SCPrint(TRUE, f, CFSTR("\n\t\t%@"), thisSession->serverPort);
		}

		if (thisSession->serverRunLoopSource != NULL) {
			SCPrint(TRUE, f, CFSTR("\n\t\t%@"), thisSession->serverRunLoopSource);
		}

		SCPrint(TRUE, f, CFSTR("\n"));
	}

	SCPrint(TRUE, f, CFSTR("\n"));
	return;
}


#include <Security/Security.h>
#include <Security/SecTask.h>

static CFStringRef
sessionName(serverSessionRef session)
{
	CFDictionaryRef	info;
	CFStringRef	name	= NULL;
	CFStringRef	sessionKey;

	sessionKey = CFStringCreateWithFormat(NULL, NULL, CFSTR("%d"), session->key);
	info = CFDictionaryGetValue(sessionData, sessionKey);
	CFRelease(sessionKey);

	if (info != NULL) {
		name = CFDictionaryGetValue(info, kSCDName);
	}

	return (name != NULL) ? name : CFSTR("???");
}

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
#if	!TARGET_IPHONE_SIMULATOR

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

#else	// !TARGET_IPHONE_SIMULATOR

	/*
	 * assume that all processes interacting with
	 * the iOS Simulator "configd" are OK.
	 */
	return TRUE;

#endif	// !TARGET_IPHONE_SIMULATOR
}


__private_extern__
Boolean
hasWriteAccess(serverSessionRef session, CFStringRef key)
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
			SCLog(TRUE, LOG_ERR,
			      CFSTR("*** Non-configd process (pid=%d) attempting to modify \"%@\" ***"),
			      pid,
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
		SCLog(TRUE, LOG_ERR,
		      CFSTR("*** Non-root process (pid=%d) attempting to modify \"%@\" ***"),
		      sessionPid(session),
		      key);

		//return FALSE;		// return FALSE when rdar://9811832 has beed fixed
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
		SCLog(TRUE, LOG_DEBUG, CFSTR("hasPathAccess realpath() failed: %s"), strerror(errno));
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
		SCLog(TRUE, LOG_DEBUG, CFSTR("hasPathAccess sandbox access denied: %s"), strerror(errno));
		return FALSE;
	}

	return TRUE;
}
