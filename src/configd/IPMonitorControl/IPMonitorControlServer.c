/*
 * Copyright (c) 2013-2014 Apple Inc. All rights reserved.
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
 * IPMonitorControlServer.c
 * - IPC channel to IPMonitor
 * - used to create interface rank assertions
 */

/*
 * Modification History
 *
 * December 16, 2013	Dieter Siegmund (dieter@apple.com)
 * - initial revision
 */

#include <CoreFoundation/CoreFoundation.h>
#include <xpc/xpc.h>
#include <xpc/private.h>
#include <sys/queue.h>
#include <CoreFoundation/CFRunLoop.h>
#include <SystemConfiguration/SCNetworkConfigurationPrivate.h>
#include "IPMonitorControlServer.h"
#include "symbol_scope.h"
#include "IPMonitorControlPrivate.h"
#include <SystemConfiguration/SCPrivate.h>

STATIC Boolean *	S_verbose;

#ifdef TEST_IPMONITOR_CONTROL
#define my_log(__level, fmt, ...)	SCPrint(TRUE, stdout, CFSTR(fmt "\n"), ## __VA_ARGS__)

#else /* TEST_IPMONITOR_CONTROL */
#include "ip_plugin.h"
#endif /* TEST_IPMONITOR_CONTROL */

STATIC dispatch_queue_t 	S_IPMonitorControlServerQueue;

typedef struct ControlSession ControlSession, * ControlSessionRef;

#define LIST_HEAD_ControlSession LIST_HEAD(ControlSessionHead, ControlSession)
#define LIST_ENTRY_ControlSession LIST_ENTRY(ControlSession)
LIST_HEAD_ControlSession	S_ControlSessions;

struct ControlSession {
    LIST_ENTRY_ControlSession	link;
    xpc_connection_t		connection;
    CFMutableDictionaryRef	assertions; /* ifname<string> = rank<number> */
};

/**
 ** Support Functions
 **/
STATIC CFMutableArrayRef	S_if_changes;
STATIC CFRange			S_if_changes_range;

STATIC void
InterfaceChangedListAddInterface(CFStringRef ifname)
{
    if (S_if_changes == NULL) {
	S_if_changes = CFArrayCreateMutable(NULL,
					    0, &kCFTypeArrayCallBacks);
	CFArrayAppendValue(S_if_changes, ifname);
	S_if_changes_range.length = 1;
    }
    else if (CFArrayContainsValue(S_if_changes, S_if_changes_range,
				  ifname) == FALSE) {
	CFArrayAppendValue(S_if_changes, ifname);
	S_if_changes_range.length++;
    }
}

STATIC CFArrayRef
InterfaceChangedListCopy(void)
{
    CFArrayRef		current_list;

    current_list = S_if_changes;
    S_if_changes = NULL;
    return (current_list);
}

STATIC void
InterfaceRankAssertionAdd(const void * key, const void * value, void * context)
{
    CFMutableDictionaryRef * 	assertions_p;
    CFNumberRef			existing_rank;
    CFNumberRef			rank = (CFNumberRef)value;

    assertions_p = (CFMutableDictionaryRef *)context;
    if (*assertions_p == NULL) {
	*assertions_p
	    = CFDictionaryCreateMutable(NULL, 0,
					&kCFTypeDictionaryKeyCallBacks,
					&kCFTypeDictionaryValueCallBacks);
	CFDictionarySetValue(*assertions_p, key, value);
	return;
    }
    existing_rank = CFDictionaryGetValue(*assertions_p, key);
    if (existing_rank == NULL
	|| (CFNumberCompare(rank, existing_rank, NULL)
	    == kCFCompareGreaterThan)) {
	CFDictionarySetValue(*assertions_p, key, value);
    }
    return;
}

STATIC CFDictionaryRef
InterfaceRankAssertionsCopy(void)
{
    CFMutableDictionaryRef	assertions = NULL;
    ControlSessionRef		session;

    LIST_FOREACH(session, &S_ControlSessions, link) {
	if (session->assertions == NULL) {
	    continue;
	}
	CFDictionaryApplyFunction(session->assertions,
				  InterfaceRankAssertionAdd,
				  &assertions);
    }
    return (assertions);
}

STATIC CFRunLoopRef		S_runloop;
STATIC CFRunLoopSourceRef	S_signal_source;

STATIC void
SetNotificationInfo(CFRunLoopRef runloop, CFRunLoopSourceRef rls)
{
    S_runloop = runloop;
    S_signal_source = rls;
    return;
}

STATIC void
GenerateNotification(void)
{
    if (S_signal_source != NULL) {
	CFRunLoopSourceSignal(S_signal_source);
	if (S_runloop != NULL) {
	    CFRunLoopWakeUp(S_runloop);
	}
    }
    return;
}

/**
 ** ControlSession
 **/
STATIC void
AddChangedInterface(const void * key, const void * value, void * context)
{
    InterfaceChangedListAddInterface((CFStringRef)key);
    return;
}

STATIC void
ControlSessionInvalidate(ControlSessionRef session)
{
    if (*S_verbose) {
	my_log(LOG_NOTICE, "Invalidating %p", session);
    }
    LIST_REMOVE(session, link);
    if (session->assertions != NULL) {
	my_log(LOG_DEBUG,
	       "IPMonitorControlServer: %p pid %d removing assertions %@",
	       session->connection, 
	       xpc_connection_get_pid(session->connection),
	       session->assertions);
	CFDictionaryApplyFunction(session->assertions, AddChangedInterface,
				  NULL);
	CFRelease(session->assertions);
	session->assertions = NULL;
	GenerateNotification();
    }
    return;
}

STATIC void
ControlSessionRelease(void * p)
{
    if (*S_verbose) {
	my_log(LOG_NOTICE, "Releasing %p", p);
    }
    free(p);
    return;
}

STATIC ControlSessionRef
ControlSessionLookup(xpc_connection_t connection)
{
    return ((ControlSessionRef)xpc_connection_get_context(connection));
}

STATIC ControlSessionRef
ControlSessionCreate(xpc_connection_t connection)
{
    ControlSessionRef	session;

    session = (ControlSessionRef)malloc(sizeof(*session));
    bzero(session, sizeof(*session));
    session->connection = connection;
    xpc_connection_set_finalizer_f(connection, ControlSessionRelease);
    xpc_connection_set_context(connection, session);
    LIST_INSERT_HEAD(&S_ControlSessions, session, link);
    if (*S_verbose) {
	my_log(LOG_NOTICE, "Created %p (connection %p)", session, connection);
    }
    return (session);
}

STATIC ControlSessionRef
ControlSessionGet(xpc_connection_t connection)
{
    ControlSessionRef	session;

    session = ControlSessionLookup(connection);
    if (session != NULL) {
	return (session);
    }
    return (ControlSessionCreate(connection));
}

STATIC void
ControlSessionSetInterfaceRank(ControlSessionRef session,
			       const char * ifname,
			       SCNetworkServicePrimaryRank rank)
{
    CFStringRef		ifname_cf;

    if (session->assertions == NULL) {
	if (rank == kSCNetworkServicePrimaryRankDefault) {
	    /* no assertions, no need to store rank */
	    return;
	}
	session->assertions
	    = CFDictionaryCreateMutable(NULL, 0,
					&kCFTypeDictionaryKeyCallBacks,
					&kCFTypeDictionaryValueCallBacks);
    }
    ifname_cf = CFStringCreateWithCString(NULL, ifname,
					  kCFStringEncodingUTF8);
    
    if (rank == kSCNetworkServicePrimaryRankDefault) {
	CFDictionaryRemoveValue(session->assertions, ifname_cf);
	if (CFDictionaryGetCount(session->assertions) == 0) {
	    CFRelease(session->assertions);
	    session->assertions = NULL;
	}
    }
    else {
	CFNumberRef	rank_cf;

	rank_cf = CFNumberCreate(NULL, kCFNumberSInt32Type, &rank);
	CFDictionarySetValue(session->assertions, ifname_cf, rank_cf);
	CFRelease(rank_cf);
    }
    InterfaceChangedListAddInterface(ifname_cf);
    GenerateNotification();
    CFRelease(ifname_cf);
    return;
}

STATIC SCNetworkServicePrimaryRank
ControlSessionGetInterfaceRank(ControlSessionRef session,
			       const char * ifname)
{
    SCNetworkServicePrimaryRank	rank = kSCNetworkServicePrimaryRankDefault;

    if (session->assertions != NULL) {
	CFStringRef		ifname_cf;
	CFNumberRef		rank_cf;

	ifname_cf = CFStringCreateWithCString(NULL, ifname,
					      kCFStringEncodingUTF8);
	rank_cf = CFDictionaryGetValue(session->assertions, ifname_cf);
	CFRelease(ifname_cf);
	if (rank_cf != NULL) {
	    (void)CFNumberGetValue(rank_cf, kCFNumberSInt32Type, &rank);
	}
    }
    return (rank);
}

/**
 ** IPMonitorControlServer
 **/
STATIC Boolean
IPMonitorControlServerValidateConnection(xpc_connection_t connection)
{
    uid_t		uid;

    uid = xpc_connection_get_euid(connection);
    return (uid == 0);
}

STATIC int
IPMonitorControlServerHandleSetInterfaceRank(xpc_connection_t connection,
					     xpc_object_t request,
					     xpc_object_t reply)
{
    const char *		ifname;
    SCNetworkServicePrimaryRank	rank;
    ControlSessionRef		session;

    if (IPMonitorControlServerValidateConnection(connection) == FALSE) {
	my_log(LOG_DEBUG,
	       "IPMonitorControlServer: %p pid %d permission denied",
	       connection, xpc_connection_get_pid(connection));
	return (EPERM);
    }
    ifname 
	= xpc_dictionary_get_string(request,
				    kIPMonitorControlRequestKeyInterfaceName);
    if (ifname == NULL) {
	return (EINVAL);
    }
    rank = (SCNetworkServicePrimaryRank)
	xpc_dictionary_get_uint64(request,
				  kIPMonitorControlRequestKeyPrimaryRank);
    switch (rank) {
    case kSCNetworkServicePrimaryRankDefault:
    case kSCNetworkServicePrimaryRankFirst:
    case kSCNetworkServicePrimaryRankLast:
    case kSCNetworkServicePrimaryRankNever:
    case kSCNetworkServicePrimaryRankScoped:
	break;
    default:
	return (EINVAL);
    }
    session = ControlSessionGet(connection);
    ControlSessionSetInterfaceRank(session, ifname, rank);
    my_log(LOG_DEBUG,
	   "IPMonitorControlServer: %p pid %d set %s %u",
	   connection, xpc_connection_get_pid(connection), ifname, rank);
    return (0);
}

STATIC int
IPMonitorControlServerHandleGetInterfaceRank(xpc_connection_t connection,
					     xpc_object_t request,
					     xpc_object_t reply)
{
    const char *		ifname;
    SCNetworkServicePrimaryRank	rank;
    ControlSessionRef		session;

    if (reply == NULL) {
	/* no point in processing the request if we can't provide an answer */
	return (EINVAL);
    }
    session = ControlSessionLookup(connection);
    if (session == NULL) {
	/* no session, no rank assertion */
	return (ENOENT);
    }
    ifname 
	= xpc_dictionary_get_string(request,
				    kIPMonitorControlRequestKeyInterfaceName);
    if (ifname == NULL) {
	return (EINVAL);
    }
    rank = ControlSessionGetInterfaceRank(session, ifname);
    xpc_dictionary_set_uint64(reply, kIPMonitorControlResponseKeyPrimaryRank,
			      rank);
    return (0);
}

STATIC void
IPMonitorControlServerHandleDisconnect(xpc_connection_t connection)
{
    ControlSessionRef	session;

    if (*S_verbose) {
	my_log(LOG_NOTICE, "IPMonitorControlServer: client %p went away",
	       connection);
    }
    session = ControlSessionLookup(connection);
    if (session == NULL) {
	/* never asserted anything */
	return;
    }
    ControlSessionInvalidate(session);
    return;
}

STATIC void
IPMonitorControlServerHandleRequest(xpc_connection_t connection,
				    xpc_object_t request)
{
    xpc_type_t	type;
    
    type = xpc_get_type(request);
    if (type == XPC_TYPE_DICTIONARY) {
	int			error = 0;
	uint64_t		request_type;
	xpc_connection_t	remote;
	xpc_object_t		reply;
	
	request_type 
	    = xpc_dictionary_get_uint64(request,
					kIPMonitorControlRequestKeyType);
	reply = xpc_dictionary_create_reply(request);
	switch (request_type) {
	case kIPMonitorControlRequestTypeSetInterfaceRank:
	    error = IPMonitorControlServerHandleSetInterfaceRank(connection,
								 request,
								 reply);
	    break;
	case kIPMonitorControlRequestTypeGetInterfaceRank:
	    error = IPMonitorControlServerHandleGetInterfaceRank(connection,
								 request,
								 reply);
	    break;
	default:
	    error = EINVAL;
	    break;
	}
	if (reply == NULL) {
	    /* client didn't want a reply */
	    return;
	}
	xpc_dictionary_set_int64(reply, kIPMonitorControlResponseKeyError,
				 error);
	remote = xpc_dictionary_get_remote_connection(request);
	xpc_connection_send_message(remote, reply);
	xpc_release(reply);
    }
    else if (type == XPC_TYPE_ERROR) {
	if (request == XPC_ERROR_CONNECTION_INVALID) {
	    IPMonitorControlServerHandleDisconnect(connection);
	}
	else if (request == XPC_ERROR_CONNECTION_INTERRUPTED) {
	    my_log(LOG_NOTICE,
		   "IPMonitorControlServer: connection interrupted");
	}
    }
    else {
	my_log(LOG_NOTICE, "IPMonitorControlServer: unexpected event");
    }
    return;
}

STATIC void
IPMonitorControlServerHandleNewConnection(xpc_connection_t connection)
{
    xpc_handler_t	handler;

    handler = ^(xpc_object_t event) {
	IPMonitorControlServerHandleRequest(connection, event);
    };
    xpc_connection_set_event_handler(connection, handler);
    xpc_connection_resume(connection);
    return;
}

STATIC xpc_connection_t
IPMonitorControlServerCreate(dispatch_queue_t queue, const char * name)
{
    uint64_t		flags = XPC_CONNECTION_MACH_SERVICE_LISTENER;
    xpc_connection_t	connection;
    xpc_handler_t	handler;

    connection = xpc_connection_create_mach_service(name, queue, flags);
    if (connection == NULL) {
	return (NULL);
    }
    handler = ^(xpc_object_t event) {
	xpc_type_t	type;
	
	type = xpc_get_type(event);
	if (type == XPC_TYPE_CONNECTION) {
	    IPMonitorControlServerHandleNewConnection(event);
	}
	else if (type == XPC_TYPE_ERROR) {
	    const char	*	desc;
	    
	    desc = xpc_dictionary_get_string(event, XPC_ERROR_KEY_DESCRIPTION);
	    if (event == XPC_ERROR_CONNECTION_INVALID) {
		my_log(LOG_NOTICE, "IPMonitorControlServer: %s", desc);
		xpc_release(connection);
	    }
	    else {
		my_log(LOG_NOTICE, "IPMonitorControlServer: %s", desc);
	    } 
	}
	else {
	    my_log(LOG_NOTICE, "IPMonitorControlServer: unknown event %p",
		   type);
	}
    };
    S_IPMonitorControlServerQueue = queue;
    xpc_connection_set_event_handler(connection, handler);
    xpc_connection_resume(connection);
    return (connection);
}

PRIVATE_EXTERN Boolean
IPMonitorControlServerStart(CFRunLoopRef runloop, CFRunLoopSourceRef rls,
			    Boolean * verbose)
{
    dispatch_queue_t	q;
    xpc_connection_t	connection;

    S_verbose = verbose;
    SetNotificationInfo(runloop, rls);
    q = dispatch_queue_create("IPMonitorControlServer", NULL);
    connection = IPMonitorControlServerCreate(q, kIPMonitorControlServerName);
    if (connection == NULL) {
	my_log(LOG_ERR,
	       "IPMonitorControlServer: failed to create server");
	dispatch_release(q);
	return (FALSE);
    }
    return (TRUE);
}

PRIVATE_EXTERN CFArrayRef
IPMonitorControlServerCopyInterfaceRankInformation(CFDictionaryRef * info)
{
    __block CFArrayRef		changed;
    __block CFDictionaryRef	dict;

    dispatch_sync(S_IPMonitorControlServerQueue,
		  ^{
		      dict = InterfaceRankAssertionsCopy();
		      changed = InterfaceChangedListCopy();
		  });
    *info = dict;
    return (changed);
}
