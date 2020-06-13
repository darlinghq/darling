/*
 * Copyright (c) 2013-2018 Apple Inc. All rights reserved.
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
#include <SystemConfiguration/SCPrivate.h>
#include "IPMonitorControlServer.h"
#include "symbol_scope.h"
#include "IPMonitorControlPrivate.h"
#include "IPMonitorAWDReport.h"

#ifdef TEST_IPMONITOR_CONTROL
#define	my_log(__level, __format, ...)	SCPrint(TRUE, stdout, CFSTR(__format "\n"), ## __VA_ARGS__)

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
    CFMutableDictionaryRef	advisories; /* ifname<string> = advisory<number> */
};

/**
 ** Support Functions
 **/
STATIC CFMutableArrayRef	S_if_changes;
STATIC CFRange			S_if_changes_range;

STATIC CFNumberRef
RankLastNumberGet(void)
{
    STATIC CFNumberRef		rank_last;

    if (rank_last == NULL) {
	SCNetworkServicePrimaryRank	rank;

	rank = kSCNetworkServicePrimaryRankLast;
	rank_last = CFNumberCreate(NULL, kCFNumberSInt32Type, &rank);
    }
    return (rank_last);
}

STATIC void
InterfaceChangedListAddInterface(CFStringRef ifname)
{
    if (S_if_changes == NULL) {
	S_if_changes = CFArrayCreateMutable(NULL,
					    0, &kCFTypeArrayCallBacks);
	CFArrayAppendValue(S_if_changes, ifname);
	S_if_changes_range.length = 1;
    }
    else if (!CFArrayContainsValue(S_if_changes, S_if_changes_range, ifname)) {
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
	CFDictionarySetValue(*assertions_p, key, rank);
	return;
    }
    existing_rank = CFDictionaryGetValue(*assertions_p, key);
    if (existing_rank == NULL
	|| (CFNumberCompare(rank, existing_rank, NULL)
	    == kCFCompareGreaterThan)) {
	CFDictionarySetValue(*assertions_p, key, rank);
    }
    return;
}

STATIC void
InterfaceAdvisoryAdd(const void * key, const void * value, void * context)
{
#pragma unused(value)
    CFMutableDictionaryRef * 	assertions_p;
    CFNumberRef			existing_rank;
    CFNumberRef			rank;

    /* an interface advisory implies RankLast */
    rank = RankLastNumberGet();
    assertions_p = (CFMutableDictionaryRef *)context;
    if (*assertions_p == NULL) {
	*assertions_p
	    = CFDictionaryCreateMutable(NULL, 0,
					&kCFTypeDictionaryKeyCallBacks,
					&kCFTypeDictionaryValueCallBacks);
	CFDictionarySetValue(*assertions_p, key, rank);
	return;
    }
    existing_rank = CFDictionaryGetValue(*assertions_p, key);
    if (existing_rank == NULL
	|| (CFNumberCompare(rank, existing_rank, NULL)
	    == kCFCompareGreaterThan)) {
	CFDictionarySetValue(*assertions_p, key, rank);
    }
    return;
}

STATIC CFDictionaryRef
InterfaceRankAssertionsCopy(void)
{
    CFMutableDictionaryRef	assertions = NULL;
    ControlSessionRef		session;

    LIST_FOREACH(session, &S_ControlSessions, link) {
	if (session->advisories != NULL) {
	    CFDictionaryApplyFunction(session->advisories,
				      InterfaceAdvisoryAdd,
				      &assertions);
	}
	if (session->assertions != NULL) {
	    CFDictionaryApplyFunction(session->assertions,
				      InterfaceRankAssertionAdd,
				      &assertions);
	}
    }
    return (assertions);
}

STATIC Boolean
InterfaceHasAdvisories(CFStringRef ifname)
{
    ControlSessionRef		session;

    LIST_FOREACH(session, &S_ControlSessions, link) {
	if (session->advisories != NULL
	    && CFDictionaryContainsKey(session->advisories, ifname)) {
	    return (TRUE);
	}
    }
    return (FALSE);
}


STATIC AWDIPMonitorInterfaceAdvisoryReport_Flags
advisory_to_flags(SCNetworkInterfaceAdvisory advisory)
{
    AWDIPMonitorInterfaceAdvisoryReport_Flags	flags;

    switch (advisory) {
    case kSCNetworkInterfaceAdvisoryNone:
    default:
	flags = 0;
	break;
    case kSCNetworkInterfaceAdvisoryLinkLayerIssue:
	flags = AWDIPMonitorInterfaceAdvisoryReport_Flags_LINK_LAYER_ISSUE;
	break;
    case kSCNetworkInterfaceAdvisoryUplinkIssue:
	flags = AWDIPMonitorInterfaceAdvisoryReport_Flags_UPLINK_ISSUE;
	break;
    }
    return (flags);
}

STATIC AWDIPMonitorInterfaceAdvisoryReport_Flags
InterfaceGetAdvisoryFlags(CFStringRef ifname,
			  ControlSessionRef exclude_session,
			  uint32_t * ret_count)
{
    uint32_t					count;
    AWDIPMonitorInterfaceAdvisoryReport_Flags	flags = 0;
    ControlSessionRef				session;

    count = 0;
    LIST_FOREACH(session, &S_ControlSessions, link) {
	SCNetworkInterfaceAdvisory 	advisory = 0;
	CFNumberRef			advisory_cf;

	if (session->advisories == NULL) {
	    continue;
	}
	if (exclude_session != NULL && exclude_session == session) {
	    continue;
	}
	advisory_cf = CFDictionaryGetValue(session->advisories, ifname);
	if (advisory_cf == NULL) {
	    /* session has no advisories for this interface */
	    continue;
	}
	(void)CFNumberGetValue(advisory_cf, kCFNumberSInt32Type, &advisory);
	flags |= advisory_to_flags(advisory);
	count++;
    }
    *ret_count = count;
    return (flags);
}

STATIC Boolean
AnyInterfaceHasAdvisories(void)
{
    ControlSessionRef		session;

    LIST_FOREACH(session, &S_ControlSessions, link) {
	if (session->advisories != NULL) {
	    return (TRUE);
	}
    }
    return (FALSE);
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
NotifyIPMonitor(void)
{
    if (S_signal_source != NULL) {
	CFRunLoopSourceSignal(S_signal_source);
	if (S_runloop != NULL) {
	    CFRunLoopWakeUp(S_runloop);
	}
    }
    return;
}

STATIC void
NotifyInterfaceAdvisory(CFStringRef ifname)
{
    CFStringRef		key;

    key = _IPMonitorControlCopyInterfaceAdvisoryNotificationKey(ifname);
    SCDynamicStoreNotifyValue(NULL, key);
    CFRelease(key);
    return;
}

STATIC void
SubmitInterfaceAdvisoryMetric(CFStringRef ifname,
			      AWDIPMonitorInterfaceAdvisoryReport_Flags flags,
			      uint32_t count)
{
    InterfaceAdvisoryReportRef	report;
    AWDIPMonitorInterfaceType	type;

    /* XXX need to actually figure out what the interface type is */
    if (CFStringHasPrefix(ifname, CFSTR("pdp"))) {
	type = AWDIPMonitorInterfaceType_IPMONITOR_INTERFACE_TYPE_CELLULAR;
    }
    else {
	type = AWDIPMonitorInterfaceType_IPMONITOR_INTERFACE_TYPE_WIFI;
    }
    report = InterfaceAdvisoryReportCreate(type);
    if (report == NULL) {
	return;
    }
    InterfaceAdvisoryReportSetFlags(report, flags);
    InterfaceAdvisoryReportSetAdvisoryCount(report, count);
    InterfaceAdvisoryReportSubmit(report);
    my_log(LOG_NOTICE, "%@: submitted AWD report %@", ifname, report);
    CFRelease(report);
}

/**
 ** ControlSession
 **/
STATIC void
AddChangedInterface(const void * key, const void * value, void * context)
{
#pragma unused(value)
#pragma unused(context)
    InterfaceChangedListAddInterface((CFStringRef)key);
    return;
}

STATIC void
AddChangedInterfaceNotify(const void * key, const void * value, void * context)
{
#pragma unused(value)
#pragma unused(context)
    InterfaceChangedListAddInterface((CFStringRef)key);
    NotifyInterfaceAdvisory((CFStringRef)key);
    return;
}

STATIC void
GenerateMetricForInterfaceAtSessionClose(const void * key, const void * value,
					 void * context)
{
    uint32_t		count_after;
    uint32_t		count_before;
    AWDIPMonitorInterfaceAdvisoryReport_Flags flags_after;
    AWDIPMonitorInterfaceAdvisoryReport_Flags flags_before;
    CFStringRef		ifname = (CFStringRef)key;
    ControlSessionRef	session = (ControlSessionRef)context;

#pragma unused(value)
    /*
     * Get the flags and count including this session, then again
     * excluding this session. If either flags or count are different,
     * generate the metric.
     */
    flags_before = InterfaceGetAdvisoryFlags(ifname, NULL, &count_before);
    flags_after	= InterfaceGetAdvisoryFlags(ifname, session, &count_after);
    if (flags_before != flags_after || count_before != count_after) {
	SubmitInterfaceAdvisoryMetric(ifname, flags_after, count_after);
    }
    return;
}

STATIC void
ControlSessionGenerateMetricsAtClose(ControlSessionRef session)
{
    if (session->advisories == NULL) {
	return;
    }
    CFDictionaryApplyFunction(session->advisories,
			      GenerateMetricForInterfaceAtSessionClose,
			      session);
}

STATIC void
ControlSessionInvalidate(ControlSessionRef session)
{
    my_log(LOG_DEBUG, "Invalidating %p", session);
    ControlSessionGenerateMetricsAtClose(session);
    LIST_REMOVE(session, link);
    if (session->assertions != NULL || session->advisories != NULL) {
	if (session->advisories != NULL) {
	    my_log(LOG_NOTICE,
		   "pid %d removing advisories %@",
		   xpc_connection_get_pid(session->connection),
		   session->advisories);
	    CFDictionaryApplyFunction(session->advisories,
				      AddChangedInterfaceNotify,
				      NULL);
	    my_CFRelease(&session->advisories);
	}
	if (session->assertions != NULL) {
	    my_log(LOG_NOTICE,
		   "pid %d removing assertions %@",
		   xpc_connection_get_pid(session->connection),
		   session->assertions);
	    CFDictionaryApplyFunction(session->assertions, AddChangedInterface,
				      NULL);
	    my_CFRelease(&session->assertions);
	}
	NotifyIPMonitor();
    }
    return;
}

STATIC void
ControlSessionRelease(void * p)
{
    my_log(LOG_DEBUG, "Releasing %p", p);
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
    memset(session, 0, sizeof(*session));
    session->connection = connection;
    xpc_connection_set_finalizer_f(connection, ControlSessionRelease);
    xpc_connection_set_context(connection, session);
    LIST_INSERT_HEAD(&S_ControlSessions, session, link);
    my_log(LOG_DEBUG, "Created %p (connection %p)", session, connection);
    return (session);
}

STATIC ControlSessionRef
ControlSessionForConnection(xpc_connection_t connection)
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
    NotifyIPMonitor();
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

STATIC void
ControlSessionSetInterfaceAdvisory(ControlSessionRef session,
				   const char * ifname,
				   SCNetworkInterfaceAdvisory advisory)
{
    uint32_t		count_after;
    uint32_t		count_before;
    AWDIPMonitorInterfaceAdvisoryReport_Flags flags_after;
    AWDIPMonitorInterfaceAdvisoryReport_Flags flags_before;
    CFStringRef		ifname_cf;

    if (session->advisories == NULL) {
	if (advisory == kSCNetworkInterfaceAdvisoryNone) {
	    /* no advisories, no need to store advisory */
	    return;
	}
	session->advisories
	    = CFDictionaryCreateMutable(NULL, 0,
					&kCFTypeDictionaryKeyCallBacks,
					&kCFTypeDictionaryValueCallBacks);
    }
    ifname_cf = CFStringCreateWithCString(NULL, ifname,
					  kCFStringEncodingUTF8);
    flags_before = InterfaceGetAdvisoryFlags(ifname_cf, NULL, &count_before);
    if (advisory == kSCNetworkInterfaceAdvisoryNone) {
	CFDictionaryRemoveValue(session->advisories, ifname_cf);
	if (CFDictionaryGetCount(session->advisories) == 0) {
	    CFRelease(session->advisories);
	    session->advisories = NULL;
	}
    }
    else {
	CFNumberRef			advisory_cf;

	advisory_cf = CFNumberCreate(NULL, kCFNumberSInt32Type, &advisory);
	CFDictionarySetValue(session->advisories, ifname_cf, advisory_cf);
	CFRelease(advisory_cf);
    }
    flags_after = InterfaceGetAdvisoryFlags(ifname_cf, NULL, &count_after);
    if (flags_before != flags_after || count_before != count_after) {
	SubmitInterfaceAdvisoryMetric(ifname_cf, flags_after, count_after);
    }
    InterfaceChangedListAddInterface(ifname_cf);
    NotifyInterfaceAdvisory(ifname_cf);
    NotifyIPMonitor();
    CFRelease(ifname_cf);
    return;
}

/**
 ** IPMonitorControlServer
 **/

STATIC const char *
get_process_name(xpc_object_t request)
{
    const char *	process_name;

    process_name
	= xpc_dictionary_get_string(request,
				    kIPMonitorControlRequestKeyProcessName);
    if (process_name == NULL) {
	process_name = "<unknown>";
    }
    return (process_name);
}

STATIC Boolean
IPMonitorControlServerConnectionIsRoot(xpc_connection_t connection)
{
    uid_t		uid;

    uid = xpc_connection_get_euid(connection);
    return (uid == 0);
}

STATIC Boolean
IPMonitorControlServerConnectionHasEntitlement(xpc_connection_t connection,
					       const char * entitlement)
{
    Boolean 		entitled = FALSE;
    xpc_object_t 	val;

    val = xpc_connection_copy_entitlement_value(connection, entitlement);
    if (val != NULL) {
	if (xpc_get_type(val) == XPC_TYPE_BOOL) {
	    entitled = xpc_bool_get_value(val);
	}
	xpc_release(val);
    }
    return (entitled);
}

STATIC const char *
get_rank_str(SCNetworkServicePrimaryRank rank)
{
    const char *	str = NULL;

    switch (rank) {
    case kSCNetworkServicePrimaryRankDefault:
	str = "Default";
	break;
    case kSCNetworkServicePrimaryRankFirst:
	str = "First";
	break;
    case kSCNetworkServicePrimaryRankLast:
	str = "Last";
	break;
    case kSCNetworkServicePrimaryRankNever:
	str = "Never";
	break;
    case kSCNetworkServicePrimaryRankScoped:
	str = "Scoped";
	break;
    default:
	break;
    }
    return (str);
}

STATIC int
HandleSetInterfaceRank(xpc_connection_t connection,
		       xpc_object_t request,
		       xpc_object_t reply)
{
#pragma unused(reply)
    const char *		ifname;
    SCNetworkServicePrimaryRank	rank;
    const char *		rank_str;
    ControlSessionRef		session;

    if (!IPMonitorControlServerConnectionIsRoot(connection)) {
	my_log(LOG_INFO, "connection %p pid %d permission denied",
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
    rank_str = get_rank_str(rank);
    if (rank_str == NULL) {
	return (EINVAL);
    }
    session = ControlSessionForConnection(connection);
    ControlSessionSetInterfaceRank(session, ifname, rank);
    my_log(LOG_NOTICE, "%s[%d] SetInterfaceRank(%s) = %s (%u)",
	   get_process_name(request),
	   xpc_connection_get_pid(connection), ifname, rank_str, rank);
    return (0);
}

STATIC int
HandleGetInterfaceRank(xpc_connection_t connection,
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

STATIC const char *
get_advisory_str(SCNetworkInterfaceAdvisory advisory)
{
    const char *	str = NULL;

    switch (advisory) {
    case kSCNetworkInterfaceAdvisoryNone:
	str = "None";
	break;
    case kSCNetworkInterfaceAdvisoryLinkLayerIssue:
	str = "LinkLayerIssue";
	break;
    case kSCNetworkInterfaceAdvisoryUplinkIssue:
	str = "UplinkIssue";
	break;
    default:
	break;
    }
    return (str);
}

STATIC int
HandleSetInterfaceAdvisory(xpc_connection_t connection,
			   xpc_object_t request,
			   xpc_object_t reply)
{
#pragma unused(reply)
    SCNetworkInterfaceAdvisory	advisory;
    const char *		advisory_str;
    const char *		ifname;
    const char *		reason;
    ControlSessionRef		session;

#define ENTITLEMENT "com.apple.SystemConfiguration.SCNetworkInterfaceSetAdvisory"
    if (!IPMonitorControlServerConnectionIsRoot(connection)
	&& !IPMonitorControlServerConnectionHasEntitlement(connection,
							   ENTITLEMENT)) {
	my_log(LOG_INFO, "connection %p pid %d permission denied",
	       connection, xpc_connection_get_pid(connection));
	return (EPERM);
    }
    ifname
	= xpc_dictionary_get_string(request,
				    kIPMonitorControlRequestKeyInterfaceName);
    if (ifname == NULL) {
	return (EINVAL);
    }
    reason
	= xpc_dictionary_get_string(request,
				    kIPMonitorControlRequestKeyReason);
    advisory = (SCNetworkInterfaceAdvisory)
	xpc_dictionary_get_uint64(request, kIPMonitorControlRequestKeyAdvisory);

    /* validate the advisory code */
    advisory_str = get_advisory_str(advisory);
    if (advisory_str == NULL) {
	return (EINVAL);
    }
    session = ControlSessionForConnection(connection);
    ControlSessionSetInterfaceAdvisory(session, ifname, advisory);
    my_log(LOG_NOTICE, "%s[%d] SetInterfaceAdvisory(%s) = %s (%u) reason='%s'",
	   get_process_name(request),
	   xpc_connection_get_pid(connection), ifname, advisory_str, advisory,
	   reason != NULL ? reason : "" );
    return (0);
}

STATIC int
HandleInterfaceAdvisoryIsSet(xpc_connection_t connection,
			     xpc_object_t request,
			     xpc_object_t reply)
{
#pragma unused(connection)
    const char *		ifname;
    CFStringRef			ifname_cf;

    if (reply == NULL) {
	/* no point in processing the request if we can't provide an answer */
	return (EINVAL);
    }
    ifname
	= xpc_dictionary_get_string(request,
				    kIPMonitorControlRequestKeyInterfaceName);
    if (ifname == NULL) {
	return (EINVAL);
    }
    ifname_cf = CFStringCreateWithCString(NULL, ifname,
					  kCFStringEncodingUTF8);
    xpc_dictionary_set_bool(reply,
			    kIPMonitorControlResponseKeyAdvisoryIsSet,
			    InterfaceHasAdvisories(ifname_cf));
    CFRelease(ifname_cf);
    return (0);
}

STATIC int
HandleAnyInterfaceAdvisoryIsSet(xpc_connection_t connection,
				xpc_object_t request,
				xpc_object_t reply)
{
#pragma unused(connection)
#pragma unused(request)
    if (reply == NULL) {
	/* no point in processing the request if we can't provide an answer */
	return (EINVAL);
    }
    xpc_dictionary_set_bool(reply,
			    kIPMonitorControlResponseKeyAdvisoryIsSet,
			    AnyInterfaceHasAdvisories());
    return (0);
}

STATIC void
IPMonitorControlServerHandleDisconnect(xpc_connection_t connection)
{
    ControlSessionRef	session;

    my_log(LOG_DEBUG, "IPMonitorControlServer: client %p went away",
	   connection);
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
	    error = HandleSetInterfaceRank(connection, request, reply);
	    break;
	case kIPMonitorControlRequestTypeGetInterfaceRank:
	    error = HandleGetInterfaceRank(connection, request, reply);
	    break;
	case kIPMonitorControlRequestTypeSetInterfaceAdvisory:
	    error = HandleSetInterfaceAdvisory(connection, request, reply);
	    break;
	case kIPMonitorControlRequestTypeInterfaceAdvisoryIsSet:
	    error = HandleInterfaceAdvisoryIsSet(connection, request, reply);
	    break;
	case kIPMonitorControlRequestTypeAnyInterfaceAdvisoryIsSet:
	    error = HandleAnyInterfaceAdvisoryIsSet(connection, request, reply);
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
	    my_log(LOG_INFO, "connection interrupted");
	}
    }
    else {
	my_log(LOG_NOTICE, "unexpected event");
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
    xpc_connection_set_target_queue(connection, S_IPMonitorControlServerQueue);
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
		my_log(LOG_NOTICE, "%s", desc);
		xpc_release(connection);
	    }
	    else {
		my_log(LOG_NOTICE, "%s", desc);
	    }
	}
	else {
	    my_log(LOG_NOTICE, "unknown event %p", type);
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
#pragma unused(verbose)
    dispatch_queue_t	q;
    xpc_connection_t	connection;

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
