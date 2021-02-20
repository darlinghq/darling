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
 * IPMonitorControl.c
 * - IPC channel to IPMonitor
 * - used to create interface rank assertions
 */

/*
 * Modification History
 *
 * December 16, 2013	Dieter Siegmund (dieter@apple.com)
 * - initial revision
 */

#include "IPMonitorControl.h"
#include "IPMonitorControlPrivate.h"
#include "symbol_scope.h"
#include <CoreFoundation/CFRuntime.h>
#include <net/if.h>
#include <xpc/xpc.h>
#include <xpc/private.h>
#include <SystemConfiguration/SCPrivate.h>

#ifdef TEST_IPMONITOR_CONTROL

#define	my_log(__level, __format, ...)	SCPrint(TRUE, stdout, CFSTR(__format "\n"), ## __VA_ARGS__)

#else /* TEST_IPMONITOR_CONTROL */

#define	my_log(__level, __format, ...)	SC_log(__level, __format, ## __VA_ARGS__)

#endif	/* TEST_IPMONITOR_CONTROL */

/**
 ** IPMonitorControl CF object glue
 **/

struct IPMonitorControl {
    CFRuntimeBase		cf_base;

    dispatch_queue_t		queue;
    xpc_connection_t		connection;

    CFMutableDictionaryRef	assertions; /* ifname<string> = rank<number> */
    CFMutableDictionaryRef	advisories; /* ifname<string> = adv<number> */
};

STATIC CFStringRef	__IPMonitorControlCopyDebugDesc(CFTypeRef cf);
STATIC void		__IPMonitorControlDeallocate(CFTypeRef cf);

STATIC CFTypeID __kIPMonitorControlTypeID = _kCFRuntimeNotATypeID;

STATIC const CFRuntimeClass __IPMonitorControlClass = {
    0,					/* version */
    "IPMonitorControl",			/* className */
    NULL,				/* init */
    NULL,				/* copy */
    __IPMonitorControlDeallocate,	/* deallocate */
    NULL,				/* equal */
    NULL,				/* hash */
    NULL,				/* copyFormattingDesc */
    __IPMonitorControlCopyDebugDesc	/* copyDebugDesc */
};

STATIC CFStringRef
__IPMonitorControlCopyDebugDesc(CFTypeRef cf)
{
    CFAllocatorRef		allocator = CFGetAllocator(cf);
    IPMonitorControlRef		control = (IPMonitorControlRef)cf;

    return (CFStringCreateWithFormat(allocator, NULL,
				     CFSTR("<IPMonitorControl %p>"),
				     control));
}

STATIC void
__IPMonitorControlDeallocate(CFTypeRef cf)
{
    IPMonitorControlRef control = (IPMonitorControlRef)cf;

    if (control->connection != NULL) {
	xpc_release(control->connection);
    }
    if (control->queue != NULL) {
	dispatch_release(control->queue);
    }
    my_CFRelease(&control->advisories);
    my_CFRelease(&control->assertions);
    return;
}

/**
 ** IPMonitorControl support functions
 **/
STATIC void
__IPMonitorControlRegisterClass(void)
{
    STATIC dispatch_once_t	once;

    dispatch_once(&once, ^{
	    __kIPMonitorControlTypeID
		= _CFRuntimeRegisterClass(&__IPMonitorControlClass);
	});
    return;
}

STATIC IPMonitorControlRef
__IPMonitorControlAllocate(CFAllocatorRef allocator)
{
    IPMonitorControlRef		control;
    int				size;

    __IPMonitorControlRegisterClass();
    size = sizeof(*control) - sizeof(CFRuntimeBase);
    control = (IPMonitorControlRef)
	_CFRuntimeCreateInstance(allocator,
				 __kIPMonitorControlTypeID, size, NULL);
    return (control);
}

STATIC xpc_object_t
create_request_dictionary(void)
{
    const char *	progname;
    xpc_object_t	request;

    request = xpc_dictionary_create(NULL, NULL, 0);
    progname = getprogname();
    if (progname != NULL) {
	xpc_dictionary_set_string(request,
				  kIPMonitorControlRequestKeyProcessName,
				  progname);
    }
    return (request);
}

STATIC Boolean
IPMonitorControlHandleResponse(xpc_object_t event, Boolean async,
			       Boolean * retry_p)
{
    Boolean	retry = FALSE;
    Boolean	success = FALSE;
    xpc_type_t	type;

    type = xpc_get_type(event);
    if (type == XPC_TYPE_DICTIONARY) {
	if (async) {
	    /* we don't expect async responses messages */
	    my_log(LOG_NOTICE, "unexpected message");
	}
	else {
	    int64_t	error;

	    error = xpc_dictionary_get_int64(event,
					     kIPMonitorControlResponseKeyError);
	    if (error != 0) {
		success = FALSE;
#ifdef TEST_IPMONITOR_CONTROL
		my_log(LOG_NOTICE, "failure code %lld", error);
#endif	/* TEST_IPMONITOR_CONTROL */
	    }
	    else {
		success = TRUE;
	    }
	}
    }
    else if (type == XPC_TYPE_ERROR) {
	if (event == XPC_ERROR_CONNECTION_INTERRUPTED) {
#ifdef TEST_IPMONITOR_CONTROL
	    my_log(LOG_NOTICE, "can retry");
#endif	/* TEST_IPMONITOR_CONTROL */
	    retry = TRUE;
	}
	else {
	    const char *	desc;

	    desc = xpc_dictionary_get_string(event, XPC_ERROR_KEY_DESCRIPTION);
	    my_log(LOG_NOTICE, "%s", desc);
	}
    }
    else {
	my_log(LOG_NOTICE, "unknown event type : %p", type);
    }
    if (retry_p != NULL) {
	*retry_p = retry;
    }
    return (success);
}


STATIC void
_IPMonitorControlSetInterfacePrimaryRank(IPMonitorControlRef control,
					 CFStringRef ifname_cf,
					 SCNetworkServicePrimaryRank rank)
{
    if (control->assertions == NULL) {
	if (rank == kSCNetworkServicePrimaryRankDefault) {
	    /* no assertions, no need to store rank */
	    return;
	}
	control->assertions
	    = CFDictionaryCreateMutable(NULL, 0,
					&kCFTypeDictionaryKeyCallBacks,
					&kCFTypeDictionaryValueCallBacks);
    }
    if (rank == kSCNetworkServicePrimaryRankDefault) {
	CFDictionaryRemoveValue(control->assertions, ifname_cf);
	if (CFDictionaryGetCount(control->assertions) == 0) {
	    my_CFRelease(&control->assertions);
	}
    }
    else {
	CFNumberRef	rank_cf;

	rank_cf = CFNumberCreate(NULL, kCFNumberSInt32Type, &rank);
	CFDictionarySetValue(control->assertions, ifname_cf, rank_cf);
	CFRelease(rank_cf);
    }
    return;
}

STATIC void
ApplyInterfaceRank(const void * key, const void * value, void * context)
{
    xpc_connection_t		connection = (xpc_connection_t)context;
    char			ifname[IF_NAMESIZE];
    SCNetworkServicePrimaryRank	rank;
    xpc_object_t		request;

    if (!CFStringGetCString(key, ifname, sizeof(ifname),
			    kCFStringEncodingUTF8)) {
	return;
    }
    if (!CFNumberGetValue(value, kCFNumberSInt32Type, &rank)) {
	return;
    }
    request = create_request_dictionary();
    xpc_dictionary_set_uint64(request,
			      kIPMonitorControlRequestKeyType,
			      kIPMonitorControlRequestTypeSetInterfaceRank);
    xpc_dictionary_set_string(request,
			      kIPMonitorControlRequestKeyInterfaceName,
			      ifname);
    xpc_dictionary_set_uint64(request,
			      kIPMonitorControlRequestKeyPrimaryRank,
			      rank);
    xpc_connection_send_message(connection, request);
    xpc_release(request);
    return;
}


STATIC void
_IPMonitorControlSetInterfaceAdvisory(IPMonitorControlRef control,
				      CFStringRef ifname_cf,
				      SCNetworkInterfaceAdvisory advisory)
{
    if (control->advisories == NULL) {
	if (advisory == kSCNetworkInterfaceAdvisoryNone) {
	    /* no advisories, no need to store advisory */
	    return;
	}
	control->advisories
	    = CFDictionaryCreateMutable(NULL, 0,
					&kCFTypeDictionaryKeyCallBacks,
					&kCFTypeDictionaryValueCallBacks);
    }
    if (advisory == kSCNetworkInterfaceAdvisoryNone) {
	CFDictionaryRemoveValue(control->advisories, ifname_cf);
	if (CFDictionaryGetCount(control->advisories) == 0) {
	    my_CFRelease(&control->advisories);
	}
    }
    else {
	CFNumberRef	advisory_cf;

	advisory_cf = CFNumberCreate(NULL, kCFNumberSInt32Type, &advisory);
	CFDictionarySetValue(control->advisories, ifname_cf, advisory_cf);
	CFRelease(advisory_cf);
    }
    return;
}

STATIC void
ApplyInterfaceAdvisory(const void * key, const void * value, void * context)
{
    xpc_connection_t		connection = (xpc_connection_t)context;
    char			ifname[IF_NAMESIZE];
    SCNetworkInterfaceAdvisory	advisory;
    xpc_object_t		request;

    if (!CFStringGetCString(key, ifname, sizeof(ifname),
			    kCFStringEncodingUTF8)) {
	return;
    }
    if (!CFNumberGetValue(value, kCFNumberSInt32Type, &advisory)) {
	return;
    }
    request = create_request_dictionary();
    xpc_dictionary_set_uint64(request,
			      kIPMonitorControlRequestKeyType,
			      kIPMonitorControlRequestTypeSetInterfaceAdvisory);
    xpc_dictionary_set_string(request,
			      kIPMonitorControlRequestKeyInterfaceName,
			      ifname);
    xpc_dictionary_set_uint64(request,
			      kIPMonitorControlRequestKeyAdvisory,
			      advisory);
    xpc_connection_send_message(connection, request);
    xpc_release(request);
    return;
}


/**
 ** IPMonitorControl SPI
 **/
PRIVATE_EXTERN IPMonitorControlRef
IPMonitorControlCreate(void)
{
    xpc_connection_t	connection;
    IPMonitorControlRef	control;
    uint64_t		flags = XPC_CONNECTION_MACH_SERVICE_PRIVILEGED;
    xpc_handler_t	handler;
    dispatch_queue_t	queue;

    control = __IPMonitorControlAllocate(NULL);
    queue = dispatch_queue_create("IPMonitorControl", NULL);
    connection
	= xpc_connection_create_mach_service(kIPMonitorControlServerName,
					     queue, flags);
    handler = ^(xpc_object_t event) {
	Boolean		retry;

	(void)IPMonitorControlHandleResponse(event, TRUE, &retry);
	if (retry) {
	    if (control->assertions != NULL) {
		CFDictionaryApplyFunction(control->assertions,
					  ApplyInterfaceRank,
					  control->connection);
	    }
	    if (control->advisories != NULL) {
		CFDictionaryApplyFunction(control->advisories,
					  ApplyInterfaceAdvisory,
					  control->connection);
	    }
	}
    };
    xpc_connection_set_event_handler(connection, handler);
    control->connection = connection;
    control->queue = queue;
    xpc_connection_resume(connection);
    return (control);
}

STATIC xpc_object_t
IPMonitorControlSendRequest(IPMonitorControlRef control,
			    xpc_object_t request)
{
    xpc_object_t	reply;

    while (TRUE) {
	Boolean		retry_on_error = FALSE;
	Boolean		success;

	reply = xpc_connection_send_message_with_reply_sync(control->connection,
							    request);
	if (reply == NULL) {
	    my_log(LOG_NOTICE, "failed to send message");
	    break;
	}
	success = IPMonitorControlHandleResponse(reply, FALSE,
						 &retry_on_error);
	if (success) {
	    break;
	}
	xpc_release(reply);
	reply = NULL;
	if (retry_on_error) {
	    continue;
	}
	my_log(LOG_NOTICE, "fatal error");
	break;
    }
    return (reply);
}

PRIVATE_EXTERN Boolean
IPMonitorControlSetInterfacePrimaryRank(IPMonitorControlRef control,
					CFStringRef ifname_cf,
					SCNetworkServicePrimaryRank rank)
{
    char		ifname[IF_NAMESIZE];
    xpc_object_t	reply;
    xpc_object_t	request;
    Boolean		success = FALSE;

    if (!CFStringGetCString(ifname_cf, ifname, sizeof(ifname),
			    kCFStringEncodingUTF8)) {
	return (FALSE);
    }

    request = create_request_dictionary();
    xpc_dictionary_set_uint64(request,
			      kIPMonitorControlRequestKeyType,
			      kIPMonitorControlRequestTypeSetInterfaceRank);
    xpc_dictionary_set_string(request,
			      kIPMonitorControlRequestKeyInterfaceName,
			      ifname);
    xpc_dictionary_set_uint64(request,
			      kIPMonitorControlRequestKeyPrimaryRank,
			      rank);
    reply = IPMonitorControlSendRequest(control, request);
    xpc_release(request);
    if (reply != NULL) {
	success = TRUE;
	xpc_release(reply);

	/* sync our state */
	CFRetain(ifname_cf);
	CFRetain(control);
	dispatch_async(control->queue,
		       ^{
			   _IPMonitorControlSetInterfacePrimaryRank(control,
								    ifname_cf,
								    rank);
			   CFRelease(ifname_cf);
			   CFRelease(control);
		       });
    }
    return (success);
}

PRIVATE_EXTERN SCNetworkServicePrimaryRank
IPMonitorControlGetInterfacePrimaryRank(IPMonitorControlRef control,
					CFStringRef ifname_cf)
{
    char				ifname[IF_NAMESIZE];
    SCNetworkServicePrimaryRank		rank;
    xpc_object_t			reply;
    xpc_object_t			request;

    rank = kSCNetworkServicePrimaryRankDefault;
    if (!CFStringGetCString(ifname_cf, ifname, sizeof(ifname),
			   kCFStringEncodingUTF8)) {
	return rank;
    }

    request = create_request_dictionary();
    xpc_dictionary_set_uint64(request,
			      kIPMonitorControlRequestKeyType,
			      kIPMonitorControlRequestTypeGetInterfaceRank);
    xpc_dictionary_set_string(request,
			      kIPMonitorControlRequestKeyInterfaceName,
			      ifname);
    reply = IPMonitorControlSendRequest(control, request);
    if (reply != NULL) {
	rank = (SCNetworkServicePrimaryRank)
	    xpc_dictionary_get_uint64(reply,
				      kIPMonitorControlResponseKeyPrimaryRank);
	xpc_release(reply);
    }
    xpc_release(request);
    return (rank);
}

PRIVATE_EXTERN Boolean
IPMonitorControlSetInterfaceAdvisory(IPMonitorControlRef control,
				     CFStringRef ifname_cf,
				     SCNetworkInterfaceAdvisory advisory,
				     CFStringRef reason)
{
    char		ifname[IF_NAMESIZE];
    char *		reason_str = NULL;
    xpc_object_t	reply;
    xpc_object_t	request;
    Boolean		success = FALSE;

    if (!CFStringGetCString(ifname_cf, ifname, sizeof(ifname),
			    kCFStringEncodingUTF8)) {
	return (FALSE);
    }
    if (reason != NULL) {
	reason_str
	    = _SC_cfstring_to_cstring(reason, NULL, 0, kCFStringEncodingUTF8);
    }
    request = create_request_dictionary();
    xpc_dictionary_set_uint64(request,
			      kIPMonitorControlRequestKeyType,
			      kIPMonitorControlRequestTypeSetInterfaceAdvisory);
    xpc_dictionary_set_string(request,
			      kIPMonitorControlRequestKeyInterfaceName,
			      ifname);
    xpc_dictionary_set_uint64(request,
			      kIPMonitorControlRequestKeyAdvisory,
			      advisory);
    if (reason_str != NULL) {
	xpc_dictionary_set_string(request,
				  kIPMonitorControlRequestKeyReason,
				  reason_str);
	CFAllocatorDeallocate(NULL, reason_str);
    }
    reply = IPMonitorControlSendRequest(control, request);
    xpc_release(request);
    if (reply != NULL) {
	xpc_release(reply);
	success = TRUE;

	/* sync our state */
	CFRetain(ifname_cf);
	CFRetain(control);
	dispatch_async(control->queue,
		       ^{
			   _IPMonitorControlSetInterfaceAdvisory(control,
								 ifname_cf,
								 advisory);
			   CFRelease(ifname_cf);
			   CFRelease(control);
		       });
    }
    return (success);
}

PRIVATE_EXTERN Boolean
IPMonitorControlInterfaceAdvisoryIsSet(IPMonitorControlRef control,
				       CFStringRef ifname_cf)
{
    char		ifname[IF_NAMESIZE];
    xpc_object_t	reply;
    xpc_object_t	request;
    Boolean		is_set = FALSE;

    if (!CFStringGetCString(ifname_cf, ifname, sizeof(ifname),
			    kCFStringEncodingUTF8)) {
	return (FALSE);
    }
    request = create_request_dictionary();
    xpc_dictionary_set_uint64(request,
			      kIPMonitorControlRequestKeyType,
			      kIPMonitorControlRequestTypeInterfaceAdvisoryIsSet);
    xpc_dictionary_set_string(request,
			      kIPMonitorControlRequestKeyInterfaceName,
			      ifname);
    reply = IPMonitorControlSendRequest(control, request);
    xpc_release(request);
    if (reply != NULL) {
	if (xpc_dictionary_get_bool(reply,
				    kIPMonitorControlResponseKeyAdvisoryIsSet)) {
	    is_set = TRUE;
	}
	xpc_release(reply);
    }
    return (is_set);
}

PRIVATE_EXTERN Boolean
IPMonitorControlAnyInterfaceAdvisoryIsSet(IPMonitorControlRef control)
{
    xpc_object_t	reply;
    xpc_object_t	request;
    Boolean		is_set = FALSE;

    request = create_request_dictionary();
    xpc_dictionary_set_uint64(request,
			      kIPMonitorControlRequestKeyType,
			      kIPMonitorControlRequestTypeAnyInterfaceAdvisoryIsSet);
    reply = IPMonitorControlSendRequest(control, request);
    xpc_release(request);
    if (reply != NULL) {
	if (xpc_dictionary_get_bool(reply,
				    kIPMonitorControlResponseKeyAdvisoryIsSet)) {
	    is_set = TRUE;
	}
	xpc_release(reply);
    }
    return (is_set);
}

PRIVATE_EXTERN CFStringRef
IPMonitorControlCopyInterfaceAdvisoryNotificationKey(CFStringRef ifname)
{
    return (_IPMonitorControlCopyInterfaceAdvisoryNotificationKey(ifname));
}
