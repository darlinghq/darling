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
#define my_log(__level, fmt, ...)	SCPrint(TRUE, stdout, CFSTR(fmt "\n"), ## __VA_ARGS__)

#else /* TEST_IPMONITOR_CONTROL */

#define my_log(__level, fmt, ...)	SCLog(TRUE, __level, CFSTR(fmt), ## __VA_ARGS__)
#endif /* TEST_IPMONITOR_CONTROL */

/**
 ** IPMonitorControl CF object glue
 **/

struct IPMonitorControl {
    CFRuntimeBase		cf_base;

    dispatch_queue_t		queue;
    xpc_connection_t		connection;
    CFMutableDictionaryRef	assertions; /* ifname<string> = rank<number> */
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
	xpc_release(control->queue);
    }
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
    bzero(((void *)control) + sizeof(CFRuntimeBase), size);
    return (control);
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
	    my_log(LOG_NOTICE, "IPMonitorControl: unexpected message");
	}
	else {
	    int64_t	error;
	    
	    error = xpc_dictionary_get_int64(event,
					     kIPMonitorControlResponseKeyError);
	    if (error != 0) {
		success = FALSE;
#ifdef TEST_IPMONITOR_CONTROL
		my_log(LOG_NOTICE,
		       "IPMonitorControl: failure code %lld", error);
#endif /* TEST_IPMONITOR_CONTROL */
	    }
	    else {
		success = TRUE;
	    }
	}
    }
    else if (type == XPC_TYPE_ERROR) {
	if (event == XPC_ERROR_CONNECTION_INTERRUPTED) {
#ifdef TEST_IPMONITOR_CONTROL
	    my_log(LOG_NOTICE, "IPMonitorControl: can retry");
#endif /* TEST_IPMONITOR_CONTROL */
	    retry = TRUE;
	}
	else {
	    const char *	desc;

	    desc = xpc_dictionary_get_string(event, XPC_ERROR_KEY_DESCRIPTION);
	    my_log(LOG_NOTICE, "IPMonitorControl: %s", desc);
	}
    }
    else {
	my_log(LOG_NOTICE, "IPMonitorControl: unknown event type : %p", type);
    }
    if (retry_p != NULL) {
	*retry_p = retry;
    }
    return (success);
}


STATIC void
IPMonitorControlSetInterfaceRank(IPMonitorControlRef control,
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
	    CFRelease(control->assertions);
	    control->assertions = NULL;
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

    if (CFStringGetCString(key, ifname, sizeof(ifname), 
			   kCFStringEncodingUTF8) == FALSE) {
	return;
    }
    if (CFNumberGetValue(value, kCFNumberSInt32Type, &rank) == FALSE) {
	return;
    }
    request = xpc_dictionary_create(NULL, NULL, 0);
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
	Boolean			retry;

	(void)IPMonitorControlHandleResponse(event, TRUE, &retry);
	if (retry && control->assertions != NULL) {
	    CFDictionaryApplyFunction(control->assertions,
				      ApplyInterfaceRank,
				      control->connection);
	}
    };
    xpc_connection_set_event_handler(connection, handler);
    control->connection = connection;
    control->queue = queue;
    xpc_connection_resume(connection);
    return (control);
}

PRIVATE_EXTERN Boolean
IPMonitorControlSetInterfacePrimaryRank(IPMonitorControlRef control,
					CFStringRef ifname_cf,
					SCNetworkServicePrimaryRank rank)
{
    char		ifname[IF_NAMESIZE];
    xpc_object_t	request;
    Boolean		success = FALSE;

    if (CFStringGetCString(ifname_cf, ifname, sizeof(ifname), 
			   kCFStringEncodingUTF8) == FALSE) {
	return (FALSE);
    }
    request = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_uint64(request,
			      kIPMonitorControlRequestKeyType,
			      kIPMonitorControlRequestTypeSetInterfaceRank);
    xpc_dictionary_set_string(request,
			      kIPMonitorControlRequestKeyInterfaceName,
			      ifname);
    xpc_dictionary_set_uint64(request,
			      kIPMonitorControlRequestKeyPrimaryRank,
			      rank);
    while (TRUE) {
	xpc_object_t	reply;
	Boolean		retry_on_error = FALSE;

	reply = xpc_connection_send_message_with_reply_sync(control->connection,
							    request);
	if (reply == NULL) {
	    my_log(LOG_NOTICE, "IPMonitorControl: failed to send message");
	    break;
	}
	success = IPMonitorControlHandleResponse(reply, FALSE,
						 &retry_on_error);
	xpc_release(reply);
	if (success) {
	    break;
	}
	if (retry_on_error) {
	    continue;
	}
	my_log(LOG_NOTICE, "IPMonitorControl: fatal error");
	break;
    }
    xpc_release(request);
    if (success) {
	/* sync our state */
	CFRetain(ifname_cf);
	CFRetain(control);
	dispatch_async(control->queue,
		       ^{
			   IPMonitorControlSetInterfaceRank(control,
							    ifname_cf,
							    rank);
			   CFRelease(ifname_cf);
			   CFRelease(control);
		       });
    }
    return (success);
}

SCNetworkServicePrimaryRank
IPMonitorControlGetInterfacePrimaryRank(IPMonitorControlRef control,
					CFStringRef ifname_cf)
{
    char			ifname[IF_NAMESIZE];
    SCNetworkServicePrimaryRank rank;
    xpc_object_t		request;

    rank = kSCNetworkServicePrimaryRankDefault;
    if (CFStringGetCString(ifname_cf, ifname, sizeof(ifname), 
			   kCFStringEncodingUTF8) == FALSE) {
	goto done;
    }
    request = xpc_dictionary_create(NULL, NULL, 0);
    xpc_dictionary_set_uint64(request,
			      kIPMonitorControlRequestKeyType,
			      kIPMonitorControlRequestTypeGetInterfaceRank);
    xpc_dictionary_set_string(request,
			      kIPMonitorControlRequestKeyInterfaceName,
			      ifname);
    while (TRUE) {
	xpc_object_t	reply;
	Boolean		retry_on_error = FALSE;
	Boolean		success;

	reply = xpc_connection_send_message_with_reply_sync(control->connection,
							    request);
	if (reply == NULL) {
	    my_log(LOG_NOTICE, "IPMonitorControl: failed to send message");
	    break;
	}
	success = IPMonitorControlHandleResponse(reply, FALSE, &retry_on_error);
	if (success) {
	    rank = (SCNetworkServicePrimaryRank)
		xpc_dictionary_get_uint64(reply,
					  kIPMonitorControlResponseKeyPrimaryRank);
	}
	xpc_release(reply);
	if (success) {
	    break;
	}
	if (retry_on_error) {
	    continue;
	}
	break;
    }
    xpc_release(request);

 done:
    return (rank);
}

