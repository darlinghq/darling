/*
 * Copyright (c) 2003-2014 Apple Inc. All rights reserved.
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
 * April 14, 2004		Christophe Allie <callie@apple.com>
 * - use mach messages

 * December 20, 2002		Christophe Allie <callie@apple.com>
 * - initial revision
 */


//#define DEBUG_MACH_PORT_ALLOCATIONS


#include <Availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <dispatch/dispatch.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <CoreFoundation/CFXPCBridge.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include <SystemConfiguration/SCValidation.h>
#include <SystemConfiguration/VPNAppLayerPrivate.h>
#include <SystemConfiguration/VPNTunnel.h>

#if	!TARGET_OS_IPHONE
#include <Security/Security.h>
#include "dy_framework.h"
#endif	// !TARGET_OS_IPHONE

#include <bootstrap.h>

#include <pthread.h>
#include <notify.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <mach/mach.h>
#include <bsm/audit.h>
#include <bsm/libbsm.h>
#include <sandbox.h>
#include <sys/proc_info.h>
#include <libproc.h>

#include <ppp/ppp_msg.h>
#include "pppcontroller.h"
#include <ppp/pppcontroller_types.h>

#ifndef	PPPCONTROLLER_SERVER_PRIV
#define	PPPCONTROLLER_SERVER_PRIV	PPPCONTROLLER_SERVER
#endif	// !PPPCONTROLLER_SERVER_PRIV


#include "SCNetworkConnectionInternal.h"

static int		debug			= 0;
static pthread_once_t	initialized		= PTHREAD_ONCE_INIT;
static pthread_mutex_t	scnc_lock		= PTHREAD_MUTEX_INITIALIZER;
static mach_port_t	scnc_server		= MACH_PORT_NULL;
static char		*scnc_server_name	= NULL;


typedef struct {

	/* base CFType information */
	CFRuntimeBase			cfBase;

	/* lock */
	pthread_mutex_t			lock;

	/* service */
	SCNetworkServiceRef		service;

	/* client info (if we are proxying for another process */
	mach_port_t			client_audit_session;
	audit_token_t			client_audit_token;
	mach_port_t			client_bootstrap_port;
	uid_t				client_uid;
	gid_t				client_gid;
	pid_t				client_pid;
	uuid_t				client_uuid;
	CFStringRef			client_bundle_id;

	/* ref to PPP controller for control messages */
	mach_port_t			session_port;

	/* ref to PPP controller for notification messages */
	CFMachPortRef			notify_port;

	/* keep track of whether we're acquired the initial status */
	Boolean				haveStatus;

	/* run loop source, callout, context, rl scheduling info */
	Boolean				scheduled;
	CFRunLoopSourceRef		rls;
	SCNetworkConnectionCallBack	rlsFunction;
	SCNetworkConnectionContext	rlsContext;
	CFMutableArrayRef		rlList;

	/* SCNetworkConnectionSetDispatchQueue */
	dispatch_group_t		dispatchGroup;
	dispatch_queue_t		dispatchQueue;
	dispatch_source_t		dispatchSource;

	SCNetworkConnectionType		type;
	Boolean				on_demand;
	CFDictionaryRef			on_demand_info;
	CFDictionaryRef			on_demand_user_options;
	CFStringRef			on_demand_required_probe;

	/* Flow Divert support info */
	CFDictionaryRef			flow_divert_token_params;

#if !TARGET_IPHONE_SIMULATOR
	/* NetworkExtension data structures */
	ne_session_t			ne_session;
#endif /* !TARGET_IPHONE_SIMULATOR */
} SCNetworkConnectionPrivate, *SCNetworkConnectionPrivateRef;


static __inline__ CFTypeRef
isA_SCNetworkConnection(CFTypeRef obj)
{
	return (isA_CFType(obj, SCNetworkConnectionGetTypeID()));
}


#if !TARGET_IPHONE_SIMULATOR
Boolean
__SCNetworkConnectionUseNetworkExtension(SCNetworkConnectionPrivateRef connectionPrivate)
{
	Boolean result = FALSE;

	if (ne_session_use_as_system_vpn() && connectionPrivate->service != NULL) {
		_SCErrorSet(kSCStatusOK);
		result = _SCNetworkServiceIsVPN(connectionPrivate->service);
		/*
		 * SCNetworkServiceGetInterface (called by _SCNetworkServiceIsVPN) will set the SC error to kSCStatusInvalidArgument if the service does not have an associated prefs object.
		 * In that case, we try to get the service type/subtype from the dynamic store.
		 */
		if (!result && SCError() == kSCStatusInvalidArgument) {
			CFStringRef interfaceKey = SCDynamicStoreKeyCreateNetworkServiceEntity(kCFAllocatorDefault,
											     kSCDynamicStoreDomainSetup,
											     SCNetworkServiceGetServiceID(connectionPrivate->service),
											     kSCEntNetInterface);
			CFDictionaryRef interfaceDict = SCDynamicStoreCopyValue(NULL, interfaceKey);
			if (isA_CFDictionary(interfaceDict)) {
				CFStringRef interfaceType = CFDictionaryGetValue(interfaceDict, kSCPropNetInterfaceType);
				if (isA_CFString(interfaceType)) {
					if (CFEqual(interfaceType, kSCNetworkInterfaceTypePPP)) {
						CFStringRef interfaceSubType = CFDictionaryGetValue(interfaceDict, kSCPropNetInterfaceSubType);
						result = (isA_CFString(interfaceSubType) && (CFEqual(interfaceSubType, kSCValNetInterfaceSubTypePPTP) || CFEqual(interfaceSubType, kSCValNetInterfaceSubTypeL2TP)));
					} else {
						result = (CFEqual(interfaceType, kSCNetworkInterfaceTypeVPN) || CFEqual(interfaceType, kSCNetworkInterfaceTypeIPSec));
					}
				}
			}
			if (interfaceDict != NULL) {
			    CFRelease(interfaceDict);
			}
			CFRelease(interfaceKey);
		}
	}

	return result;
}
#endif /* !TARGET_IPHONE_SIMULATOR */


Boolean
__SCNetworkConnectionUsingNetworkExtension(SCNetworkConnectionPrivateRef connectionPrivate)
{
#if !TARGET_IPHONE_SIMULATOR
    return (connectionPrivate->ne_session != NULL);
#else
	return FALSE;
#endif /* !TARGET_IPHONE_SIMULATOR */
}


static CFStringRef
__SCNetworkConnectionCopyDescription(CFTypeRef cf)
{
	CFAllocatorRef			allocator		= CFGetAllocator(cf);
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)cf;
	CFMutableStringRef		result;

	result = CFStringCreateMutable(allocator, 0);
	CFStringAppendFormat(result, NULL, CFSTR("<SCNetworkConnection, %p [%p]> {"), cf, allocator);
	CFStringAppendFormat(result, NULL, CFSTR("service = %p"), connectionPrivate->service);
	if (connectionPrivate->session_port != MACH_PORT_NULL) {
		CFStringAppendFormat(result, NULL, CFSTR(", server port = 0x%x"), connectionPrivate->session_port);
	}
	CFStringAppendFormat(result, NULL, CFSTR("using NetworkExtension = %s"), (__SCNetworkConnectionUsingNetworkExtension(connectionPrivate) ? "yes" : "no"));
	CFStringAppendFormat(result, NULL, CFSTR("}"));

	return result;
}


static void
__SCNetworkConnectionDeallocate(CFTypeRef cf)
{
	SCNetworkConnectionPrivateRef	connectionPrivate = (SCNetworkConnectionPrivateRef)cf;

	/* release resources */
	pthread_mutex_destroy(&connectionPrivate->lock);

	if (connectionPrivate->client_audit_session != MACH_PORT_NULL) {
		mach_port_mod_refs(mach_task_self(),
				   connectionPrivate->client_audit_session,
				   MACH_PORT_RIGHT_SEND,
				   -1);
	}

	if (connectionPrivate->client_bootstrap_port != MACH_PORT_NULL) {
		mach_port_mod_refs(mach_task_self(),
				   connectionPrivate->client_bootstrap_port,
				   MACH_PORT_RIGHT_SEND,
				   -1);
	}

	if (connectionPrivate->client_bundle_id != NULL) {
		CFRelease(connectionPrivate->client_bundle_id);
	}

	if (connectionPrivate->rls != NULL) {
		CFRunLoopSourceInvalidate(connectionPrivate->rls);
		CFRelease(connectionPrivate->rls);
	}

	if (connectionPrivate->rlList != NULL) {
		CFRelease(connectionPrivate->rlList);
	}

	if (connectionPrivate->notify_port != NULL) {
		mach_port_t	mp	= CFMachPortGetPort(connectionPrivate->notify_port);

		__MACH_PORT_DEBUG(TRUE, "*** __SCNetworkConnectionDeallocate notify_port", mp);
		CFMachPortInvalidate(connectionPrivate->notify_port);
		CFRelease(connectionPrivate->notify_port);
		mach_port_mod_refs(mach_task_self(), mp, MACH_PORT_RIGHT_RECEIVE, -1);
	}

	if (connectionPrivate->session_port != MACH_PORT_NULL) {
		__MACH_PORT_DEBUG(TRUE, "*** __SCNetworkConnectionDeallocate session_port", connectionPrivate->session_port);
		(void) mach_port_deallocate(mach_task_self(), connectionPrivate->session_port);
	}

	if (connectionPrivate->rlsContext.release != NULL)
		(*connectionPrivate->rlsContext.release)(connectionPrivate->rlsContext.info);

	if (connectionPrivate->service != NULL) {
		CFRelease(connectionPrivate->service);
	}

	if (connectionPrivate->on_demand_info != NULL) {
		CFRelease(connectionPrivate->on_demand_info);
	}

	if (connectionPrivate->on_demand_user_options != NULL) {
		CFRelease(connectionPrivate->on_demand_user_options);
	}

	if (connectionPrivate->on_demand_required_probe != NULL) {
		CFRelease(connectionPrivate->on_demand_required_probe);
	}

	if (connectionPrivate->flow_divert_token_params != NULL) {
		CFRelease(connectionPrivate->flow_divert_token_params);
	}

#if !TARGET_IPHONE_SIMULATOR
	if (connectionPrivate->ne_session != NULL) {
		ne_session_set_event_handler(connectionPrivate->ne_session, NULL, NULL);
		ne_session_release(connectionPrivate->ne_session);
	}
#endif /* !TARGET_IPHONE_SIMULATOR */

	return;
}


static CFTypeID __kSCNetworkConnectionTypeID	= _kCFRuntimeNotATypeID;

static const CFRuntimeClass __SCNetworkConnectionClass = {
	0,					// version
	"SCNetworkConnection",			// className
	NULL,					// init
	NULL,					// copy
	__SCNetworkConnectionDeallocate,	// dealloc
	NULL,					// equal
	NULL,					// hash
	NULL,					// copyFormattingDesc
	__SCNetworkConnectionCopyDescription	// copyDebugDesc
};


static void
childForkHandler()
{
	/* the process has forked (and we are the child process) */

	scnc_server = MACH_PORT_NULL;
	scnc_server_name = NULL;
	return;
}


static void
__SCNetworkConnectionInitialize(void)
{
	char	*env;

	/* get the debug environment variable */
	env = getenv("PPPDebug");
	if (env != NULL) {
		if (sscanf(env, "%d", &debug) != 1) {
			/* PPPDebug value is not valid (or non-numeric), set debug to 1 */
			debug = 1;
		}
	}

	/* register with CoreFoundation */
	__kSCNetworkConnectionTypeID = _CFRuntimeRegisterClass(&__SCNetworkConnectionClass);

	/* add handler to cleanup after fork() */
	(void) pthread_atfork(NULL, NULL, childForkHandler);

	return;
}


static Boolean
__SCNetworkConnectionReconnectNotifications(SCNetworkConnectionRef connection);

#define SC_NETWORK_CONNECTION_QUEUE "SCNetworkConnectionQueue"

static dispatch_queue_t
__SCNetworkConnectionQueue()
{
	static dispatch_once_t	once;
	static dispatch_queue_t	q;

	dispatch_once(&once, ^{
		q = dispatch_queue_create(SC_NETWORK_CONNECTION_QUEUE, NULL);
	});

	return q;
}


static void
__SCNetworkConnectionCallBackRunLoopPerform(SCNetworkConnectionRef connection,
					    CFRunLoopRef rl,
					    CFStringRef rl_mode,
					    SCNetworkConnectionCallBack rlsFunction,
					    void (*context_release)(const void *),
					    void *context_info)
{
	SCNetworkConnectionStatus	nc_status	= kSCNetworkConnectionInvalid;

	nc_status = SCNetworkConnectionGetStatus(connection);
	CFRunLoopPerformBlock(rl, rl_mode,
			      ^{
				      (*rlsFunction)(connection, nc_status, context_info);
				      if ((context_release != NULL) && (context_info != NULL)) {
					      (*context_release)(context_info);
				      }
				      CFRelease(rl);
				      CFRelease(rl_mode);
				      CFRelease(connection);
			      });
	CFRunLoopWakeUp(rl);
	return;
}

static void
__SCNetworkConnectionCallBackDispatchPerform(SCNetworkConnectionRef connection,
					     dispatch_queue_t q,
					     SCNetworkConnectionCallBack rlsFunction,
					     void (*context_release)(const void *),
					     void *context_info)
{
	SCNetworkConnectionStatus	nc_status	= kSCNetworkConnectionInvalid;

	nc_status = SCNetworkConnectionGetStatus(connection);
	dispatch_async(q,
		       ^{
			       (*rlsFunction)(connection, nc_status, context_info);
			       if ((context_release != NULL) && (context_info != NULL)) {
				       (*context_release)(context_info);
			       }
			       dispatch_release(q);
			       CFRelease(connection);
		       });
	return;
}


static void
__SCNetworkConnectionCallBack(void *connection)
{
	boolean_t			exec_async		= FALSE;
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	void				*context_info;
	void				(*context_release)(const void *);
	CFRunLoopRef			rl			= NULL;
	CFStringRef			rl_mode;
	SCNetworkConnectionCallBack	rlsFunction		= NULL;
	dispatch_queue_t		q			= NULL;
	SCNetworkConnectionStatus	nc_status		= kSCNetworkConnectionInvalid;

	pthread_mutex_lock(&connectionPrivate->lock);

	if (!connectionPrivate->scheduled) {
		// if not currently scheduled
		pthread_mutex_unlock(&connectionPrivate->lock);
		return;
	}

	rlsFunction = connectionPrivate->rlsFunction;
	if (rlsFunction == NULL) {
		pthread_mutex_unlock(&connectionPrivate->lock);
		return;
	}

	if ((connectionPrivate->rlsContext.retain != NULL) && (connectionPrivate->rlsContext.info != NULL)) {
		context_info	= (void *)(*connectionPrivate->rlsContext.retain)(connectionPrivate->rlsContext.info);
		context_release	= connectionPrivate->rlsContext.release;
	} else {
		context_info	= connectionPrivate->rlsContext.info;
		context_release	= NULL;
	}

#if !TARGET_IPHONE_SIMULATOR
	if (__SCNetworkConnectionUsingNetworkExtension(connectionPrivate)) {
		pthread_mutex_unlock(&connectionPrivate->lock);

		nc_status = SCNetworkConnectionGetStatus(connection);
		(*rlsFunction)(connection, nc_status, context_info);
		if ((context_release != NULL) && (context_info != NULL)) {
			(*context_release)(context_info);
		}
		CFRelease(connection); /* This releases the reference that we took in the NESessionEventStatusChanged event handler */
		return;
	}
#endif /* !TARGET_IPHONE_SIMULATOR */

	// Do we need to spin a new thread? (either we are running on the main
	// dispatch queue or main runloop)
	if (connectionPrivate->rlList == NULL) {
		// if we are performing the callback on a dispatch queue
		q = connectionPrivate->dispatchQueue;
		if (q == dispatch_get_main_queue()) {
			exec_async = TRUE;
		}
	} else {
		rl = CFRunLoopGetCurrent();
		if (rl == CFRunLoopGetMain()) {
			exec_async = TRUE;
		}
	}

	CFRetain(connection);
	pthread_mutex_unlock(&connectionPrivate->lock);

	if (!exec_async) {
		nc_status = SCNetworkConnectionGetStatus(connection);
		(*rlsFunction)(connection, nc_status, context_info);
		if ((context_release != NULL) && (context_info != NULL)) {
			(*context_release)(context_info);
		}
		CFRelease(connection);
		return;
	}

	if (connectionPrivate->rlList == NULL) {
		assert(q != NULL);
		dispatch_retain(q);
		dispatch_async(__SCNetworkConnectionQueue(), ^{
				       __SCNetworkConnectionCallBackDispatchPerform(connection,
										    q,
										    rlsFunction,
										    context_release,
										    context_info);
		});
	} else {
		assert(rl != NULL);
		CFRetain(rl);
		rl_mode = CFRunLoopCopyCurrentMode(rl);
		dispatch_async(__SCNetworkConnectionQueue(), ^{
			__SCNetworkConnectionCallBackRunLoopPerform(connection,
								    rl,
								    rl_mode,
								    rlsFunction,
								    context_release,
								    context_info);
		});
	}

	return;
}


static void
__SCNetworkConnectionMachCallBack(CFMachPortRef port, void * msg, CFIndex size, void * info)
{
	mach_no_senders_notification_t	*buf			= msg;
	mach_msg_id_t			msgid			= buf->not_header.msgh_id;
	SCNetworkConnectionRef		connection		= (SCNetworkConnectionRef)info;

	if (msgid == MACH_NOTIFY_NO_SENDERS) {
		// re-establish notification
		SCLog(_sc_verbose, LOG_DEBUG, CFSTR("__SCNetworkConnectionMachCallBack: PPPController server died"));
		(void)__SCNetworkConnectionReconnectNotifications(connection);
	}

	__SCNetworkConnectionCallBack(info);
}


#pragma mark -
#pragma mark SCNetworkConnection APIs


static CFStringRef
pppMPCopyDescription(const void *info)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)info;

	return CFStringCreateWithFormat(NULL,
					NULL,
					CFSTR("<SCNetworkConnection MP %p> {service = %@, callout = %p}"),
					connectionPrivate,
					connectionPrivate->service,
					connectionPrivate->rlsFunction);
}


static SCNetworkConnectionPrivateRef
__SCNetworkConnectionCreatePrivate(CFAllocatorRef		allocator,
				   SCNetworkServiceRef		service,
				   SCNetworkConnectionCallBack	callout,
				   SCNetworkConnectionContext	*context)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= NULL;
	uint32_t			size;


	/* initialize runtime */
	pthread_once(&initialized, __SCNetworkConnectionInitialize);

	/* allocate NetworkConnection */
	size = sizeof(SCNetworkConnectionPrivate) - sizeof(CFRuntimeBase);
	connectionPrivate = (SCNetworkConnectionPrivateRef)_CFRuntimeCreateInstance(allocator, __kSCNetworkConnectionTypeID, size, NULL);
	if (connectionPrivate == NULL) {
		goto fail;
	}

	/* zero the data structure */
	bzero(((u_char*)connectionPrivate)+sizeof(CFRuntimeBase), size);

	pthread_mutex_init(&connectionPrivate->lock, NULL);

	/* save the service */
	if (service != NULL) {
		connectionPrivate->service = CFRetain(service);
	}

	connectionPrivate->client_audit_session = MACH_PORT_NULL;
	connectionPrivate->client_bootstrap_port = MACH_PORT_NULL;
	connectionPrivate->client_uid = geteuid();
	connectionPrivate->client_gid = getegid();
	connectionPrivate->client_pid = getpid();
	connectionPrivate->client_bundle_id = NULL;
	uuid_clear(connectionPrivate->client_uuid);

	connectionPrivate->rlsFunction = callout;

	if (context) {
		bcopy(context, &connectionPrivate->rlsContext, sizeof(SCNetworkConnectionContext));
		if (context->retain != NULL) {
			connectionPrivate->rlsContext.info = (void *)(*context->retain)(context->info);
		}
	}

	connectionPrivate->type = kSCNetworkConnectionTypeUnknown;

#if !TARGET_IPHONE_SIMULATOR
	if (__SCNetworkConnectionUseNetworkExtension(connectionPrivate)) {
		CFStringRef serviceID = SCNetworkServiceGetServiceID(connectionPrivate->service);
		if (serviceID != NULL) {
			uuid_string_t service_uuid_str;
			if (CFStringGetCString(serviceID, service_uuid_str, sizeof(service_uuid_str), kCFStringEncodingUTF8)) {
				uuid_t config_id;
				if (uuid_parse(service_uuid_str, config_id) == 0) {
					connectionPrivate->ne_session = ne_session_create(config_id, NESessionTypeVPN);
				}
			}
		}

		if (connectionPrivate->ne_session == NULL) {
			SCLog(TRUE, LOG_ERR, CFSTR("SCNetworkConnection failed to create an ne_session: service ID %@ is not a valid UUID"), serviceID);
			goto fail;
		}
	}
#endif /* !TARGET_IPHONE_SIMULATOR */

	/* success, return the connection reference */
	return connectionPrivate;

    fail:

	/* failure, clean up and leave */
	if (connectionPrivate != NULL) {
		CFRelease(connectionPrivate);
	}

	_SCErrorSet(kSCStatusFailed);
	return NULL;
}


static mach_port_t
__SCNetworkConnectionServerPort(kern_return_t *status)
{
	mach_port_t	server	= MACH_PORT_NULL;

#ifdef	BOOTSTRAP_PRIVILEGED_SERVER
	*status = bootstrap_look_up2(bootstrap_port,
				     __SCNetworkConnectionGetControllerPortName(),
				     &server,
				     0,
				     BOOTSTRAP_PRIVILEGED_SERVER);
#else	// BOOTSTRAP_PRIVILEGED_SERVER
	*status = bootstrap_look_up(bootstrap_port, __SCNetworkConnectionGetControllerPortName(), &server);
#endif	// BOOTSTRAP_PRIVILEGED_SERVER

	switch (*status) {
		case BOOTSTRAP_SUCCESS :
			// service currently registered, "a good thing" (tm)
			return server;
		case BOOTSTRAP_NOT_PRIVILEGED :
			// the service is not privileged
			break;
		case BOOTSTRAP_UNKNOWN_SERVICE :
			// service not currently registered, try again later
			break;
		default :
#ifdef	DEBUG
			SCLog(_sc_verbose, LOG_DEBUG,
			      CFSTR("SCNetworkConnection bootstrap_look_up() failed: status=%s"),
			      bootstrap_strerror(*status));
#endif	// DEBUG
			break;
	}

	scnc_server_name = NULL;		/* reset pppcontroller server */
	return MACH_PORT_NULL;
}

static mach_port_t
__SCNetworkConnectionGetCurrentServerPort(void)
{
	return scnc_server;
}

static mach_port_t
__SCNetworkConnectionRefreshServerPort(mach_port_t current_server, int *mach_result)
{
	mach_port_t new_server;

	pthread_mutex_lock(&scnc_lock);
	if (scnc_server != MACH_PORT_NULL) {
		if (current_server == scnc_server) {
			scnc_server_name = NULL;
			// if the server we tried returned the error
			(void)mach_port_deallocate(mach_task_self(), scnc_server);
			scnc_server = __SCNetworkConnectionServerPort(mach_result);
		} else {
			// another thread has refreshed the server port
		}
	} else {
		scnc_server = __SCNetworkConnectionServerPort(mach_result);
	}
	new_server = scnc_server;
	pthread_mutex_unlock(&scnc_lock);

	return new_server;
}

#if	((__MAC_OS_X_VERSION_MIN_REQUIRED >= 1070) || (__IPHONE_OS_VERSION_MIN_REQUIRED >= 50000)) && (!TARGET_IPHONE_SIMULATOR || (__IPHONE_OS_VERSION_MIN_REQUIRED >= 70000))
#define	HAVE_PPPCONTROLLER_ATTACHWITHAUDITSESSION
#endif


#if	((__MAC_OS_X_VERSION_MIN_REQUIRED >= 1080) || (__IPHONE_OS_VERSION_MIN_REQUIRED >= 50000)) && !TARGET_IPHONE_SIMULATOR
#define	HAVE_PPPCONTROLLER_ATTACHWITHPROXY
#endif

static mach_port_t
__SCNetworkConnectionSessionPort(SCNetworkConnectionPrivateRef connectionPrivate)
{
	void		*data		= NULL;
	CFIndex		dataLen		= 0;
	CFDataRef	dataRef		= NULL;
	mach_port_t	notify_port	= MACH_PORT_NULL;
	mach_port_t	oldNotify	= MACH_PORT_NULL;
	int		retry		= 0;
	int		sc_status	= kSCStatusFailed;
	mach_port_t	server		= __SCNetworkConnectionGetCurrentServerPort();
	kern_return_t	status		= KERN_SUCCESS;

#ifdef	HAVE_PPPCONTROLLER_ATTACHWITHAUDITSESSION
	mach_port_t	au_session	= MACH_PORT_NULL;
#endif	// HAVE_PPPCONTROLLER_ATTACHWITHAUDITSESSION

	if (connectionPrivate->session_port != MACH_PORT_NULL) {
		return connectionPrivate->session_port;
	}

	if (connectionPrivate->service == NULL) {
		sc_status = kSCStatusConnectionNoService;
		goto done;
	}

	if (!_SCSerializeString(SCNetworkServiceGetServiceID(connectionPrivate->service), &dataRef, &data, &dataLen)) {
		goto done;
	}

	if (connectionPrivate->notify_port != NULL) {
		mach_port_t	mp	= CFMachPortGetPort(connectionPrivate->notify_port);

		__MACH_PORT_DEBUG(TRUE, "*** __SCNetworkConnectionSessionPort mp", mp);
		CFMachPortInvalidate(connectionPrivate->notify_port);
		CFRelease(connectionPrivate->notify_port);
		connectionPrivate->notify_port = NULL;
		mach_port_mod_refs(mach_task_self(), mp, MACH_PORT_RIGHT_RECEIVE, -1);
	}

#ifdef	HAVE_PPPCONTROLLER_ATTACHWITHAUDITSESSION
	au_session = audit_session_self();
#endif	// HAVE_PPPCONTROLLER_ATTACHWITHAUDITSESSION

	// open a new session with the server
	while (TRUE) {
		if ((connectionPrivate->rlsFunction != NULL) && (notify_port == MACH_PORT_NULL)) {
			// allocate port (for server response)
			status = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &notify_port);
			if (status != KERN_SUCCESS) {
				SCLog(TRUE, LOG_ERR, CFSTR("__SCNetworkConnectionSessionPort mach_port_allocate(): %s"), mach_error_string(status));
				sc_status = status;
				goto done;
			}

			// add send right (passed to the server)
			status = mach_port_insert_right(mach_task_self(),
							notify_port,
							notify_port,
							MACH_MSG_TYPE_MAKE_SEND);
			if (status != KERN_SUCCESS) {
				SCLog(TRUE, LOG_ERR, CFSTR("__SCNetworkConnectionSessionPort mach_port_insert_right(): %s"), mach_error_string(status));
				mach_port_mod_refs(mach_task_self(), notify_port, MACH_PORT_RIGHT_RECEIVE, -1);
				sc_status = status;
				goto done;
			}
		}

		if (server != MACH_PORT_NULL) {
#ifdef	HAVE_PPPCONTROLLER_ATTACHWITHPROXY
			if ((connectionPrivate->client_audit_session == MACH_PORT_NULL) &&
			    (connectionPrivate->client_bootstrap_port == MACH_PORT_NULL) &&
			    (connectionPrivate->client_uid == geteuid()) &&
			    (connectionPrivate->client_gid == getegid()) &&
			    (connectionPrivate->client_pid == getpid())
			   ) {
#endif	// HAVE_PPPCONTROLLER_ATTACHWITHPROXY
				status = pppcontroller_attach(server,
							      data,
							       (mach_msg_type_number_t)dataLen,
							      bootstrap_port,
							      notify_port,
#ifdef	HAVE_PPPCONTROLLER_ATTACHWITHAUDITSESSION
							      au_session,
#endif	// HAVE_PPPCONTROLLER_ATTACHWITHAUDITSESSION
							      &connectionPrivate->session_port,
							      &sc_status);
#ifdef	HAVE_PPPCONTROLLER_ATTACHWITHPROXY
			} else {
				mach_port_t	client_au_session;
				mach_port_t	client_bootstrap_port;

				if (connectionPrivate->client_audit_session == MACH_PORT_NULL) {
					client_au_session = au_session;
				} else {
					client_au_session = connectionPrivate->client_audit_session;
				}

				if (connectionPrivate->client_bootstrap_port == MACH_PORT_NULL) {
					client_bootstrap_port = bootstrap_port;
				} else {
					client_bootstrap_port = connectionPrivate->client_bootstrap_port;
				}

				status = pppcontroller_attach_proxy(server,
								    data,
								    (mach_msg_type_number_t)dataLen,
								    client_bootstrap_port,
								    notify_port,
								    client_au_session,
								    connectionPrivate->client_uid,
								    connectionPrivate->client_gid,
								    connectionPrivate->client_pid,
								    &connectionPrivate->session_port,
								    &sc_status);
			}
#endif	// HAVE_PPPCONTROLLER_ATTACHWITHPROXY
			if (status == KERN_SUCCESS) {
				if (sc_status != kSCStatusOK) {
					SCLog(TRUE, LOG_DEBUG,
					      CFSTR("__SCNetworkConnectionSessionPort : attach w/error, sc_status=%s%s"),
					      SCErrorString(sc_status),
					      (connectionPrivate->session_port != MACH_PORT_NULL) ? ", w/session_port!=MACH_PORT_NULL" : "");

					if (connectionPrivate->session_port != MACH_PORT_NULL) {
						__MACH_PORT_DEBUG(TRUE,
								  "*** __SCNetworkConnectionSessionPort session_port (attach w/error, cleanup)",
								  connectionPrivate->session_port);
						mach_port_deallocate(mach_task_self(), connectionPrivate->session_port);
						connectionPrivate->session_port = MACH_PORT_NULL;
					}

					if (notify_port != MACH_PORT_NULL) {
						__MACH_PORT_DEBUG(TRUE,
								  "*** __SCNetworkConnectionSessionPort notify_port (attach w/error, cleanup)",
								  notify_port);
						(void) mach_port_mod_refs(mach_task_self(), notify_port, MACH_PORT_RIGHT_RECEIVE, -1);
						notify_port = MACH_PORT_NULL;
					}
				}
				break;
			}

			// our [cached] server port is not valid
			SCLog(TRUE, LOG_DEBUG, CFSTR("__SCNetworkConnectionSessionPort : !attach: %s"), SCErrorString(status));
			if (status == MACH_SEND_INVALID_DEST) {
				// the server is not yet available
				__MACH_PORT_DEBUG(TRUE, "*** __SCNetworkConnectionSessionPort notify_port (!dest)", notify_port);
			} else if (status == MIG_SERVER_DIED) {
				__MACH_PORT_DEBUG(TRUE, "*** __SCNetworkConnectionSessionPort notify_port (!mig)", notify_port);
				// the server we were using is gone and we've lost our send right
				mach_port_mod_refs(mach_task_self(), notify_port, MACH_PORT_RIGHT_RECEIVE, -1);
				notify_port = MACH_PORT_NULL;
			} else {
				// if we got an unexpected error, don't retry
				sc_status = status;
				break;
			}
		}

		server = __SCNetworkConnectionRefreshServerPort(server, &sc_status);
		if (server == MACH_PORT_NULL) {
			// if server not available
			if (sc_status == BOOTSTRAP_UNKNOWN_SERVICE) {
				// if first retry attempt, wait for SCDynamicStore server
				if (retry == 0) {
					SCDynamicStoreRef	store;

					store = SCDynamicStoreCreate(NULL,
								     CFSTR("SCNetworkConnection connect"),
								     NULL,
								     NULL);
					if (store != NULL) {
						CFRelease(store);
					}
				}

				// wait up to 2.5 seconds for the [SCNetworkConnection] server
				// to startup
				if ((retry += 50) < 2500) {
					usleep(50 * 1000);	// sleep 50ms between attempts
					continue;
				}
			}
			break;
		}
	}

	if (notify_port != MACH_PORT_NULL) {
		if (connectionPrivate->session_port != MACH_PORT_NULL) {
			CFMachPortContext	context	= { 0
							  , (void *)connectionPrivate
							  , NULL
							  , NULL
							  , pppMPCopyDescription
			};

			// request a notification when/if the server dies
			status = mach_port_request_notification(mach_task_self(),
								notify_port,
								MACH_NOTIFY_NO_SENDERS,
								1,
								notify_port,
								MACH_MSG_TYPE_MAKE_SEND_ONCE,
								&oldNotify);
			if (status != KERN_SUCCESS) {
				SCLog(TRUE, LOG_ERR, CFSTR("__SCNetworkConnectionSessionPort mach_port_request_notification(): %s"), mach_error_string(status));
				mach_port_mod_refs(mach_task_self(), notify_port, MACH_PORT_RIGHT_RECEIVE, -1);
				sc_status = status;
				goto done;
			}

			if (oldNotify != MACH_PORT_NULL) {
				SCLog(TRUE, LOG_ERR, CFSTR("__SCNetworkConnectionSessionPort(): oldNotify != MACH_PORT_NULL"));
			}

			// create CFMachPort for SCNetworkConnection notification callback
			connectionPrivate->notify_port = _SC_CFMachPortCreateWithPort("SCNetworkConnection",
										      notify_port,
										      __SCNetworkConnectionMachCallBack,
										      &context);

			// we need to try a bit harder to acquire the initial status
			connectionPrivate->haveStatus = FALSE;
		} else {
			// with no server port, release the notification port we allocated
			__MACH_PORT_DEBUG(TRUE,
					  "*** __SCNetworkConnectionSessionPort notify_port (!server)",
					  notify_port);
			(void) mach_port_mod_refs  (mach_task_self(), notify_port, MACH_PORT_RIGHT_RECEIVE, -1);
			(void) mach_port_deallocate(mach_task_self(), notify_port);
			notify_port = MACH_PORT_NULL;
		}
	}

    done :

	// clean up

#ifdef	HAVE_PPPCONTROLLER_ATTACHWITHAUDITSESSION
	if (au_session != MACH_PORT_NULL) {
		(void)mach_port_deallocate(mach_task_self(), au_session);
	}
#endif	// HAVE_PPPCONTROLLER_ATTACHWITHAUDITSESSION

	if (dataRef != NULL)	CFRelease(dataRef);

	switch (sc_status) {
		case kSCStatusOK :
			__MACH_PORT_DEBUG(connectionPrivate->session_port != MACH_PORT_NULL,
					  "*** __SCNetworkConnectionSessionPort session_port",
					  connectionPrivate->session_port);
			__MACH_PORT_DEBUG(notify_port != MACH_PORT_NULL,
					  "*** __SCNetworkConnectionSessionPort notify_port",
					  notify_port);
			break;
		case BOOTSTRAP_UNKNOWN_SERVICE :
			SCLog(TRUE,
			      (status == KERN_SUCCESS) ? LOG_DEBUG : LOG_ERR,
			      CFSTR("PPPController not available"));
			break;
		default :
			SCLog(TRUE,
			      (status == KERN_SUCCESS) ? LOG_DEBUG : LOG_ERR,
			      CFSTR("__SCNetworkConnectionSessionPort pppcontroller_attach(): %s"),
			      SCErrorString(sc_status));
			break;
	}

	if (sc_status != kSCStatusOK) {
		_SCErrorSet(sc_status);
	}

	return connectionPrivate->session_port;
}


static Boolean
__SCNetworkConnectionReconnect(SCNetworkConnectionRef connection)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	mach_port_t			port;

	port = __SCNetworkConnectionSessionPort(connectionPrivate);
	return (port != MACH_PORT_NULL);
}


static Boolean
__SCNetworkConnectionReconnectNotifications(SCNetworkConnectionRef connection)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	dispatch_group_t		dispatchGroup		= NULL;
	dispatch_queue_t		dispatchQueue		= NULL;
	Boolean				ok			= TRUE;
	CFArrayRef			rlList			= NULL;

	// Before we fully tearing down our [old] notifications, make sure
	// we have retained any information that is needed to re-register the
	// [new] notifications.

	pthread_mutex_lock(&connectionPrivate->lock);

	// save and cancel [old] notifications
	if (connectionPrivate->rlList != NULL) {
		rlList = connectionPrivate->rlList;
		connectionPrivate->rlList = NULL;
	}
	if (connectionPrivate->rls != NULL) {
		CFRunLoopSourceInvalidate(connectionPrivate->rls);
		CFRelease(connectionPrivate->rls);
		connectionPrivate->rls = NULL;
	}
	if (connectionPrivate->dispatchSource != NULL) {
		dispatch_source_cancel(connectionPrivate->dispatchSource);
		connectionPrivate->dispatchSource = NULL;
	}

	// make sure dispatchSource is cancelled before removing group/queue
	if (connectionPrivate->dispatchQueue != NULL) {
		// save dispatchQueue, release reference when we've queue'd blocks
		// complete, allow re-scheduling
		dispatchGroup = connectionPrivate->dispatchGroup;
		connectionPrivate->dispatchGroup = NULL;
		dispatchQueue = connectionPrivate->dispatchQueue;
		connectionPrivate->dispatchQueue = NULL;

		// and take an extra reference for rescheduling
		dispatch_retain(dispatchQueue);
	}

	connectionPrivate->scheduled = FALSE;

	pthread_mutex_unlock(&connectionPrivate->lock);

	if (dispatchGroup != NULL) {
		dispatch_group_notify(dispatchGroup, dispatchQueue, ^{
			// release group/queue references
			dispatch_release(dispatchQueue);
			dispatch_release(dispatchGroup);	// releases our connection reference
		});
	}

	// re-schedule
	if (rlList != NULL) {
		CFIndex	i;
		CFIndex	n;

		n = CFArrayGetCount(rlList);
		for (i = 0; i < n; i += 3) {
			CFRunLoopRef	rl	= (CFRunLoopRef)CFArrayGetValueAtIndex(rlList, i+1);
			CFStringRef	rlMode	= (CFStringRef) CFArrayGetValueAtIndex(rlList, i+2);

			ok = SCNetworkConnectionScheduleWithRunLoop(connection, rl, rlMode);
			if (!ok) {
				SCLog((SCError() != BOOTSTRAP_UNKNOWN_SERVICE),
				      LOG_ERR,
				      CFSTR("__SCNetworkConnectionReconnectNotifications: SCNetworkConnectionScheduleWithRunLoop() failed"));
				goto done;
			}
		}
	} else if (dispatchQueue != NULL) {
		ok = SCNetworkConnectionSetDispatchQueue(connection, dispatchQueue);
		if (!ok) {
			SCLog((SCError() != BOOTSTRAP_UNKNOWN_SERVICE),
			      LOG_ERR,
			      CFSTR("__SCNetworkConnectionReconnectNotifications: SCNetworkConnectionSetDispatchQueue() failed"));
			goto done;
		}
	} else {
		ok = FALSE;
	}

    done :

	// cleanup
	if (rlList != NULL) {
		CFRelease(rlList);
	}
	if (dispatchQueue != NULL) {
		dispatch_release(dispatchQueue);
	}

	if (!ok) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("SCNetworkConnection server %s, notification not restored"),
		      (SCError() == BOOTSTRAP_UNKNOWN_SERVICE) ? "shutdown" : "failed");
	}

	return ok;
}


static Boolean
__SCNetworkConnectionNeedsRetry(SCNetworkConnectionRef	connection,
				const char		*error_label,
				kern_return_t		status,
				int			*sc_status)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;

	if (status == KERN_SUCCESS) {
		return FALSE;
	}

	if ((status == MACH_SEND_INVALID_DEST) || (status == MIG_SERVER_DIED)) {
		// the server's gone and our session port's dead, remove the dead name right
		(void) mach_port_deallocate(mach_task_self(), connectionPrivate->session_port);
	} else {
		// we got an unexpected error, leave the [session] port alone
		SCLog(TRUE, LOG_ERR, CFSTR("%s: %s"), error_label, mach_error_string(status));
	}
	connectionPrivate->session_port = MACH_PORT_NULL;
	if ((status == MACH_SEND_INVALID_DEST) || (status == MIG_SERVER_DIED)) {
		if (__SCNetworkConnectionReconnect(connection)) {
			return TRUE;
		}
	}
	*sc_status = status;

	return FALSE;
}


CFTypeID
SCNetworkConnectionGetTypeID(void) {
	pthread_once(&initialized, __SCNetworkConnectionInitialize);	/* initialize runtime */
	return __kSCNetworkConnectionTypeID;
}


CFArrayRef /* of SCNetworkServiceRef's */
SCNetworkConnectionCopyAvailableServices(SCNetworkSetRef set)
{
	CFMutableArrayRef	available;
	Boolean			tempSet	= FALSE;

	if (set == NULL) {
		SCPreferencesRef	prefs;

		prefs = SCPreferencesCreate(NULL, CFSTR("SCNetworkConnectionCopyAvailableServices"), NULL);
		if (prefs != NULL) {
			set = SCNetworkSetCopyCurrent(prefs);
			CFRelease(prefs);
		}
		tempSet = TRUE;
	}

	available = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	if (set != NULL) {
		CFArrayRef	services;

		services = SCNetworkSetCopyServices(set);
		if (services != NULL) {
			CFIndex		i;
			CFIndex		n;

			n = CFArrayGetCount(services);
			for (i = 0; i < n; i++) {
				SCNetworkInterfaceRef	interface;
				CFStringRef		interfaceType;
				SCNetworkServiceRef	service;

				service       = CFArrayGetValueAtIndex(services, i);
				interface     = SCNetworkServiceGetInterface(service);
				if (interface == NULL) {
					continue;
				}

				interfaceType = SCNetworkInterfaceGetInterfaceType(interface);
				if (CFEqual(interfaceType, kSCNetworkInterfaceTypePPP) ||
				    CFEqual(interfaceType, kSCNetworkInterfaceTypeVPN) ||
				    CFEqual(interfaceType, kSCNetworkInterfaceTypeIPSec)) {
					CFArrayAppendValue(available, service);
				}
			}

			CFRelease(services);
		}
	}

	if (tempSet && (set != NULL)) {
		CFRelease(set);
	}
	return available;
}


SCNetworkConnectionRef
SCNetworkConnectionCreateWithService(CFAllocatorRef			allocator,
				     SCNetworkServiceRef		service,
				     SCNetworkConnectionCallBack	callout,
				     SCNetworkConnectionContext		*context)
{
	SCNetworkConnectionPrivateRef	connectionPrivate;

	if (!isA_SCNetworkService(service)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	connectionPrivate = __SCNetworkConnectionCreatePrivate(allocator, service, callout, context);
	return (SCNetworkConnectionRef)connectionPrivate;
}


SCNetworkConnectionRef
SCNetworkConnectionCreateWithServiceID(CFAllocatorRef			allocator,
				       CFStringRef			serviceID,
				       SCNetworkConnectionCallBack	callout,
				       SCNetworkConnectionContext	*context)
{
	SCNetworkConnectionRef	connection;
	SCNetworkServiceRef	service;

	if (!isA_CFString(serviceID)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	service = _SCNetworkServiceCopyActive(NULL, serviceID);
	if (service == NULL) {
		return NULL;
	}

	connection = SCNetworkConnectionCreateWithService(allocator, service, callout, context);
	CFRelease(service);

	return connection;
}


SCNetworkConnectionRef
SCNetworkConnectionCreate(CFAllocatorRef		allocator,
			  SCNetworkConnectionCallBack	callout,
			  SCNetworkConnectionContext	*context)
{
	SCNetworkConnectionPrivateRef	connectionPrivate = __SCNetworkConnectionCreatePrivate(allocator, NULL, callout, context);
	return (SCNetworkConnectionRef)connectionPrivate;
}


CFStringRef
SCNetworkConnectionCopyServiceID(SCNetworkConnectionRef connection)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	CFStringRef			serviceID;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	if (connectionPrivate->service == NULL) {
		_SCErrorSet(kSCStatusConnectionNoService);
		return NULL;
	}

	serviceID = SCNetworkServiceGetServiceID(connectionPrivate->service);
	return CFRetain(serviceID);
}


Boolean
SCNetworkConnectionSetClientInfo(SCNetworkConnectionRef	connection,
				 mach_port_t		client_audit_session,
				 uid_t			client_uid,
				 gid_t			client_gid,
				 pid_t			client_pid)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	// save client audit session port
	if (connectionPrivate->client_audit_session != MACH_PORT_NULL) {
		mach_port_mod_refs(mach_task_self(),
				   connectionPrivate->client_audit_session,
				   MACH_PORT_RIGHT_SEND,
				   -1);
		connectionPrivate->client_audit_session = MACH_PORT_NULL;
	}
	connectionPrivate->client_audit_session = client_audit_session;
	if (connectionPrivate->client_audit_session != MACH_PORT_NULL) {
		mach_port_mod_refs(mach_task_self(),
				   connectionPrivate->client_audit_session,
				   MACH_PORT_RIGHT_SEND,
				   1);
	}

	// save client UID, GID, and PID
	connectionPrivate->client_uid = client_uid;
	connectionPrivate->client_gid = client_gid;
	connectionPrivate->client_pid = client_pid;

	return TRUE;
}


Boolean
SCNetworkConnectionSetClientAuditInfo(SCNetworkConnectionRef	connection,
				      audit_token_t		client_audit_token,
				      mach_port_t		audit_session,
				      mach_port_t		bootstrap_port,
				      pid_t			client_pid,
				      const uuid_t		uuid,
				      const char		*bundle_id)
{
	const audit_token_t		null_audit		= KERNEL_AUDIT_TOKEN_VALUE;
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	gid_t				gid			= 0;
	pid_t				pid			= 0;
	uid_t				uid			= 0;

	if (memcmp(&client_audit_token, &null_audit, sizeof(client_audit_token))) {
#if	TARGET_OS_IPHONE
		audit_token_to_au32(client_audit_token, NULL, &uid, &gid, NULL, NULL, &pid, NULL, NULL);
#else	// TARGET_OS_IPHONE
		uid = audit_token_to_euid(client_audit_token);
		gid = audit_token_to_egid(client_audit_token);
		pid = audit_token_to_pid(client_audit_token);
#endif	// TARGET_OS_IPHONE
	} else {
		pid = client_pid;
	}

	if (!SCNetworkConnectionSetClientInfo(connection, audit_session, uid, gid, pid)) {
		return FALSE;
	}

	if (connectionPrivate->client_bootstrap_port != MACH_PORT_NULL) {
		mach_port_mod_refs(mach_task_self(),
				   connectionPrivate->client_bootstrap_port,
				   MACH_PORT_RIGHT_SEND,
				   -1);
		connectionPrivate->client_bootstrap_port = MACH_PORT_NULL;
	}

	connectionPrivate->client_bootstrap_port = bootstrap_port;
	if (connectionPrivate->client_bootstrap_port != MACH_PORT_NULL) {
		mach_port_mod_refs(mach_task_self(),
				   connectionPrivate->client_bootstrap_port,
				   MACH_PORT_RIGHT_SEND,
				   1);
	}

	memcpy(&connectionPrivate->client_audit_token, &client_audit_token, sizeof(connectionPrivate->client_audit_token));

	if (uuid != NULL && !uuid_is_null(uuid)) {
		uuid_copy(connectionPrivate->client_uuid, uuid);
	}

	if (connectionPrivate->client_bundle_id != NULL) {
		CFRelease(connectionPrivate->client_bundle_id);
		connectionPrivate->client_bundle_id = NULL;
	}

	if (bundle_id != NULL) {
		connectionPrivate->client_bundle_id = CFStringCreateWithCString(kCFAllocatorDefault, bundle_id, kCFStringEncodingUTF8);
	}

	return TRUE;
}


CFDictionaryRef
SCNetworkConnectionCopyStatistics(SCNetworkConnectionRef connection)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	xmlDataOut_t			data			= NULL;
	mach_msg_type_number_t		datalen			= 0;
	int				sc_status		= kSCStatusFailed;
	mach_port_t			session_port;
	CFPropertyListRef		statistics		= NULL;
	kern_return_t			status;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	pthread_mutex_lock(&connectionPrivate->lock);

#if !TARGET_IPHONE_SIMULATOR
	if (__SCNetworkConnectionUsingNetworkExtension(connectionPrivate)) {
		__block xpc_object_t xstats = NULL;
		ne_session_t ne_session = connectionPrivate->ne_session;

		ne_session_retain(ne_session);
		pthread_mutex_unlock(&connectionPrivate->lock);

		dispatch_semaphore_t ne_sema = dispatch_semaphore_create(0);
		ne_session_get_info(ne_session, NESessionInfoTypeStatistics, __SCNetworkConnectionQueue(), ^(xpc_object_t result) {
			if (result != NULL) {
				xstats = xpc_retain(result);
			}
			ne_session_release(ne_session);
			dispatch_semaphore_signal(ne_sema);
		});
		dispatch_semaphore_wait(ne_sema, DISPATCH_TIME_FOREVER);
		dispatch_release(ne_sema);

		if (xstats != NULL) {
			statistics = _CFXPCCreateCFObjectFromXPCObject(xstats);
			xpc_release(xstats);
		} else {
			_SCErrorSet(kSCStatusFailed);
		}

		return statistics;
	}
#endif /* !TARGET_IPHONE_SIMULATOR */

    retry :

	session_port = __SCNetworkConnectionSessionPort(connectionPrivate);
	if (session_port == MACH_PORT_NULL) {
		goto done;
	}

	status = pppcontroller_copystatistics(session_port, &data, &datalen, &sc_status);
	if (__SCNetworkConnectionNeedsRetry(connection,
					    "SCNetworkConnectionCopyStatistics()",
					    status,
					    &sc_status)) {
		goto retry;
	}

	if (data != NULL) {
		if (!_SCUnserialize(&statistics, NULL, data, datalen)) {
			if (sc_status != kSCStatusOK) sc_status = SCError();
		}
		if ((sc_status == kSCStatusOK) && !isA_CFDictionary(statistics)) {
			sc_status = kSCStatusFailed;
		}
	}

	if (sc_status != kSCStatusOK) {
		if (statistics != NULL)	{
			CFRelease(statistics);
			statistics = NULL;
		}
		_SCErrorSet(sc_status);
	}

    done :

	pthread_mutex_unlock(&connectionPrivate->lock);
	return statistics;
}


SCNetworkServiceRef
SCNetworkConnectionGetService(SCNetworkConnectionRef connection)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	return connectionPrivate->service;
}


SCNetworkConnectionStatus
SCNetworkConnectionGetStatus(SCNetworkConnectionRef connection)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	SCNetworkConnectionStatus	nc_status		= kSCNetworkConnectionInvalid;
	int				retry			= 0;
	int				sc_status		= kSCStatusFailed;
	mach_port_t			session_port;
	kern_return_t			status;
	CFStringRef			serviceID;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return kSCNetworkConnectionInvalid;
	}

	if (connectionPrivate->service == NULL) {
		_SCErrorSet(kSCStatusConnectionNoService);
		return kSCNetworkConnectionInvalid;
	}

	// skip retry and return immediately if we know no service is to be found.
	serviceID = SCNetworkServiceGetServiceID(connectionPrivate->service);
	if (CFStringGetLength(serviceID) == 0) {
		_SCErrorSet(kSCStatusConnectionNoService);
		return kSCNetworkConnectionInvalid;
	}

	pthread_mutex_lock(&connectionPrivate->lock);

#if !TARGET_IPHONE_SIMULATOR
	if (__SCNetworkConnectionUsingNetworkExtension(connectionPrivate)) {
		__block ne_session_status_t ne_status;
		ne_session_t ne_session = connectionPrivate->ne_session;

		ne_session_retain(ne_session);
		pthread_mutex_unlock(&connectionPrivate->lock);

		dispatch_semaphore_t ne_sema = dispatch_semaphore_create(0);
		ne_session_get_status(ne_session, __SCNetworkConnectionQueue(), ^(ne_session_status_t status) {
			ne_status = status;
			ne_session_release(ne_session);
			dispatch_semaphore_signal(ne_sema);
		});
		dispatch_semaphore_wait(ne_sema, DISPATCH_TIME_FOREVER);
		dispatch_release(ne_sema);

		return SCNetworkConnectionGetStatusFromNEStatus(ne_status);
	}
#endif /* !TARGET_IPHONE_SIMULATOR */

    retry :

	session_port = __SCNetworkConnectionSessionPort(connectionPrivate);
	if (session_port == MACH_PORT_NULL) {
		nc_status = kSCNetworkConnectionInvalid;
		goto done;
	}

	status = pppcontroller_getstatus(session_port, &nc_status, &sc_status);
	if (__SCNetworkConnectionNeedsRetry(connection,
					    "SCNetworkConnectionGetStatus()",
					    status,
					    &sc_status)) {
		goto retry;
	}

	// wait up to 250 ms for the network service to become available
	if (!connectionPrivate->haveStatus &&
	    (sc_status == kSCStatusConnectionNoService) &&
	    ((retry += 10) < 250)) {
		usleep(10 * 1000);	// sleep 10ms between attempts
		goto retry;
	}

	if (sc_status == kSCStatusOK) {
		connectionPrivate->haveStatus = TRUE;
	} else {
		_SCErrorSet(sc_status);
		nc_status = kSCNetworkConnectionInvalid;
	}

    done :

	pthread_mutex_unlock(&connectionPrivate->lock);
	return nc_status;
}


CFDictionaryRef
SCNetworkConnectionCopyExtendedStatus(SCNetworkConnectionRef connection)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	xmlDataOut_t			data			= NULL;
	mach_msg_type_number_t		datalen			= 0;
	CFPropertyListRef		extstatus		= NULL;
	int				retry			= 0;
	int				sc_status		= kSCStatusFailed;
	mach_port_t			session_port;
	kern_return_t			status;
	CFStringRef			serviceID;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	if (connectionPrivate->service == NULL) {
		_SCErrorSet(kSCStatusConnectionNoService);
		return NULL;
	}

	// skip retry and return immediately if we know no service is to be found.
	serviceID = SCNetworkServiceGetServiceID(connectionPrivate->service);
	if (CFStringGetLength(serviceID) == 0) {
		_SCErrorSet(kSCStatusConnectionNoService);
		return NULL;
	}

	pthread_mutex_lock(&connectionPrivate->lock);

#if !TARGET_IPHONE_SIMULATOR
	if (__SCNetworkConnectionUsingNetworkExtension(connectionPrivate)) {
		__block CFDictionaryRef statusDictionary = NULL;
		ne_session_t ne_session = connectionPrivate->ne_session;

		ne_session_retain(ne_session);
		pthread_mutex_unlock(&connectionPrivate->lock);

		dispatch_semaphore_t ne_sema = dispatch_semaphore_create(0);
		ne_session_get_info(ne_session, NESessionInfoTypeExtendedStatus, __SCNetworkConnectionQueue(), ^(xpc_object_t extended_status) {
			if (extended_status != NULL) {
				statusDictionary = _CFXPCCreateCFObjectFromXPCObject(extended_status);
				ne_session_release(ne_session);
				dispatch_semaphore_signal(ne_sema);
			} else {
				ne_session_get_status(ne_session, __SCNetworkConnectionQueue(), ^(ne_session_status_t ne_status) {
					SCNetworkConnectionStatus status = SCNetworkConnectionGetStatusFromNEStatus(ne_status);
					if (status != kSCNetworkConnectionInvalid) {
						CFStringRef keys[1] = { kSCNetworkConnectionStatus };
						CFNumberRef values[1] = { NULL };
						values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &status);
						statusDictionary = CFDictionaryCreate(kCFAllocatorDefault, (const void **)keys, (const void **)values, sizeof(values) / sizeof(values[0]), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
						CFRelease(values[0]);
					}
					ne_session_release(ne_session);
					dispatch_semaphore_signal(ne_sema);
				});
			}
		});
		dispatch_semaphore_wait(ne_sema, DISPATCH_TIME_FOREVER);
		dispatch_release(ne_sema);

		if (statusDictionary != NULL) {
			extstatus = (CFPropertyListRef)statusDictionary;
		} else {
			_SCErrorSet(kSCStatusFailed);
		}

		return extstatus;
	}
#endif

    retry :

	session_port = __SCNetworkConnectionSessionPort(connectionPrivate);
	if (session_port == MACH_PORT_NULL) {
		goto done;
	}

	status = pppcontroller_copyextendedstatus(session_port, &data, &datalen, &sc_status);
	if (__SCNetworkConnectionNeedsRetry(connection,
					    "SCNetworkConnectionCopyExtendedStatus()",
					    status,
					    &sc_status)) {
		goto retry;
	}

	if (data != NULL) {
		if (!_SCUnserialize(&extstatus, NULL, data, datalen)) {
			if (sc_status != kSCStatusOK) sc_status = SCError();
		}
		if ((sc_status == kSCStatusOK) && !isA_CFDictionary(extstatus)) {
			sc_status = kSCStatusFailed;
		}
	}

	// wait up to 250 ms for the network service to become available
	if (!connectionPrivate->haveStatus &&
	    (sc_status == kSCStatusConnectionNoService) &&
	    ((retry += 10) < 250)) {
		usleep(10 * 1000);	// sleep 10ms between attempts
		goto retry;
	}

	if (sc_status == kSCStatusOK) {
		connectionPrivate->haveStatus = TRUE;
	} else {
		if (extstatus != NULL)	{
			CFRelease(extstatus);
			extstatus = NULL;
		}
		_SCErrorSet(sc_status);
	}

    done :

	pthread_mutex_unlock(&connectionPrivate->lock);
	return extstatus;
}


static void
_SCNetworkConnectionMergeDictionaries (const void *key, const void *value, void *context)
{
	/* Add value only if not present */
	CFDictionaryAddValue((CFMutableDictionaryRef)context, (CFStringRef)key, (CFTypeRef)value);
}


Boolean
SCNetworkConnectionStart(SCNetworkConnectionRef	connection,
			 CFDictionaryRef	userOptions,
			 Boolean		linger)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	CFDataRef			dataref			= NULL;
	void				*data			= NULL;
	CFIndex				datalen			= 0;
	Boolean				ok			= FALSE;
	int				sc_status		= kSCStatusFailed;
	mach_port_t			session_port;
	kern_return_t			status;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if ((userOptions != NULL) && !isA_CFDictionary(userOptions)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (userOptions == NULL) {
		userOptions = connectionPrivate->on_demand_user_options;
	} else if (connectionPrivate->on_demand_user_options != NULL) {
		CFDictionaryRef	localUserOptions	= NULL;

		localUserOptions = CFDictionaryCreateMutableCopy(NULL, 0, userOptions);
		if (localUserOptions) {
			CFDictionaryApplyFunction(connectionPrivate->on_demand_user_options,
						  _SCNetworkConnectionMergeDictionaries,
						  (void *)localUserOptions);
			CFRelease(connectionPrivate->on_demand_user_options);
			userOptions = connectionPrivate->on_demand_user_options = localUserOptions;
		}
	}

	if (debug > 0) {
		CFMutableDictionaryRef	mdict = NULL;

		SCLog(TRUE, LOG_DEBUG, CFSTR("SCNetworkConnectionStart (%p)"), connectionPrivate);

		if (userOptions != NULL) {
			CFDictionaryRef		dict;
			CFStringRef		encryption;
			CFMutableDictionaryRef	new_dict;

			/* special code to remove secret information */
			mdict = CFDictionaryCreateMutableCopy(NULL, 0, userOptions);

			dict = CFDictionaryGetValue(mdict, kSCEntNetPPP);
			if (isA_CFDictionary(dict)) {
				encryption = CFDictionaryGetValue(dict, kSCPropNetPPPAuthPasswordEncryption);
				if (!isA_CFString(encryption) ||
				    !CFEqual(encryption, kSCValNetPPPAuthPasswordEncryptionKeychain)) {
					new_dict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
					CFDictionaryReplaceValue(new_dict, kSCPropNetPPPAuthPassword, CFSTR("******"));
					CFDictionarySetValue(mdict, kSCEntNetPPP, new_dict);
					CFRelease(new_dict);
				}
			}

			dict = CFDictionaryGetValue(mdict, kSCEntNetL2TP);
			if (isA_CFDictionary(dict)) {
				encryption = CFDictionaryGetValue(dict, kSCPropNetL2TPIPSecSharedSecretEncryption);
				if (!isA_CFString(encryption) ||
				    !CFEqual(encryption, kSCValNetL2TPIPSecSharedSecretEncryptionKeychain)) {
					new_dict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
					CFDictionaryReplaceValue(new_dict, kSCPropNetL2TPIPSecSharedSecret, CFSTR("******"));
					CFDictionarySetValue(mdict, kSCEntNetL2TP, new_dict);
					CFRelease(new_dict);
				}
			}

			dict = CFDictionaryGetValue(mdict, kSCEntNetIPSec);
			if (isA_CFDictionary(dict)) {
				encryption = CFDictionaryGetValue(dict, kSCPropNetIPSecSharedSecretEncryption);
				if (!isA_CFString(encryption) ||
				    !CFEqual(encryption, kSCValNetIPSecSharedSecretEncryptionKeychain)) {
					new_dict = CFDictionaryCreateMutableCopy(NULL, 0, dict);
					CFDictionaryReplaceValue(new_dict, kSCPropNetIPSecSharedSecret, CFSTR("******"));
					CFDictionarySetValue(mdict, kSCEntNetIPSec, new_dict);
					CFRelease(new_dict);
				}
			}
		}

		SCLog(TRUE, LOG_DEBUG, CFSTR("User options: %@"), mdict);
		if (mdict != NULL) CFRelease(mdict);
	}

	pthread_mutex_lock(&connectionPrivate->lock);

	/* Clear out any cached flow divert token parameters */
	if (connectionPrivate->flow_divert_token_params != NULL) {
	    CFRelease(connectionPrivate->flow_divert_token_params);
	    connectionPrivate->flow_divert_token_params = NULL;
	}

#if !TARGET_IPHONE_SIMULATOR
	if (__SCNetworkConnectionUsingNetworkExtension(connectionPrivate)) {
		xpc_object_t xuser_options = NULL;

		if (userOptions != NULL) {
			xuser_options = _CFXPCCreateXPCObjectFromCFObject(userOptions);
		}

		if (connectionPrivate->client_bootstrap_port != MACH_PORT_NULL) {
#if NE_SESSION_VERSION > 2
			ne_session_start_on_behalf_of(connectionPrivate->ne_session,
						      xuser_options,
						      connectionPrivate->client_bootstrap_port,
						      connectionPrivate->client_audit_session,
						      connectionPrivate->client_uid,
						      connectionPrivate->client_gid,
						      connectionPrivate->client_pid);
#else
			ne_session_start_on_behalf_of(connectionPrivate->ne_session,
						      xuser_options,
						      connectionPrivate->client_bootstrap_port,
						      connectionPrivate->client_audit_session,
						      connectionPrivate->client_uid,
						      connectionPrivate->client_gid);
#endif
		} else {
			ne_session_start_with_options(connectionPrivate->ne_session, xuser_options);
		}

		if (xuser_options != NULL) {
			xpc_release(xuser_options);
		}

		ok = TRUE;
		goto done;
	}
#endif /* !TARGET_IPHONE_SIMULATOR */

	if (userOptions && !_SCSerialize(userOptions, &dataref, &data, &datalen)) {
		return FALSE;
	}

    retry :

	session_port = __SCNetworkConnectionSessionPort(connectionPrivate);
	if (session_port == MACH_PORT_NULL) {
		if (dataref)	CFRelease(dataref);
		goto done;
	}

	status = pppcontroller_start(session_port,
				     data,
				     (mach_msg_type_number_t)datalen,
				     linger,
				     &sc_status);
	if (__SCNetworkConnectionNeedsRetry(connection,
					    "SCNetworkConnectionStart()",
					    status,
					    &sc_status)) {
		goto retry;
	}

	if (dataref)	CFRelease(dataref);

	if (debug > 0) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("SCNetworkConnectionStart (%p), return: %d"), connectionPrivate, sc_status);
	}

	if (sc_status != kSCStatusOK) {
		_SCErrorSet(sc_status);
		goto done;
	}

	/* connection is now started */
	ok = TRUE;

    done:
	pthread_mutex_unlock(&connectionPrivate->lock);
	return ok;
}


Boolean
SCNetworkConnectionStop(SCNetworkConnectionRef	connection,
			Boolean			forceDisconnect)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	Boolean				ok			= FALSE;
	int				sc_status		= kSCStatusFailed;
	mach_port_t			session_port;
	kern_return_t			status;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (debug > 0) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("SCNetworkConnectionStop (%p)"), connectionPrivate);
	}

	pthread_mutex_lock(&connectionPrivate->lock);

#if !TARGET_IPHONE_SIMULATOR
	if (__SCNetworkConnectionUsingNetworkExtension(connectionPrivate)) {
		ne_session_stop(connectionPrivate->ne_session);
		ok = TRUE;
		goto done;
	}
#endif /* !TARGET_IPHONE_SIMULATOR */

    retry :

	session_port = __SCNetworkConnectionSessionPort(connectionPrivate);
	if (session_port == MACH_PORT_NULL) {
		goto done;
	}

	status = pppcontroller_stop(session_port, forceDisconnect, &sc_status);
	if (__SCNetworkConnectionNeedsRetry(connection,
					    "SCNetworkConnectionStop()",
					    status,
					    &sc_status)) {
		goto retry;
	}

	if (debug > 0) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("SCNetworkConnectionStop (%p), return: %d"), connectionPrivate, sc_status);
	}

	if (sc_status != kSCStatusOK) {
		_SCErrorSet(sc_status);
		goto done;
	}

	/* connection is now disconnecting */
	ok = TRUE;

    done :

	pthread_mutex_unlock(&connectionPrivate->lock);
	return ok;
}


Boolean
SCNetworkConnectionSuspend(SCNetworkConnectionRef connection)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	Boolean				ok			= FALSE;
	int				sc_status		= kSCStatusFailed;
	mach_port_t			session_port;
	kern_return_t			status;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (debug > 0) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("SCNetworkConnectionSuspend (%p)"), connectionPrivate);
	}

	pthread_mutex_lock(&connectionPrivate->lock);

#if !!TARGET_IPHONE_SIMULATOR
	if (__SCNetworkConnectionUsingNetworkExtension(connectionPrivate)) {
		/* Suspend only applies to PPPSerial and PPPoE */
		ok = TRUE;
		goto done;
	}
#endif /* !TARGET_IPHONE_SIMULATOR */

    retry :

	session_port = __SCNetworkConnectionSessionPort(connectionPrivate);
	if (session_port == MACH_PORT_NULL) {
		goto done;
	}

	status = pppcontroller_suspend(session_port, &sc_status);
	if (__SCNetworkConnectionNeedsRetry(connection,
					    "SCNetworkConnectionSuspend()",
					    status,
					    &sc_status)) {
		goto retry;
	}

	if (debug > 0) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("SCNetworkConnectionSuspend (%p), return: %d"), connectionPrivate, sc_status);
	}

	if (sc_status != kSCStatusOK) {
		_SCErrorSet(sc_status);
		goto done;
	}

	/* connection is now suspended */
	ok = TRUE;

    done :

	pthread_mutex_unlock(&connectionPrivate->lock);
	return ok;
}


Boolean
SCNetworkConnectionResume(SCNetworkConnectionRef connection)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	Boolean				ok			= FALSE;
	int				sc_status		= kSCStatusFailed;
	mach_port_t			session_port;
	kern_return_t			status;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (debug > 0) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("SCNetworkConnectionResume (%p)"), connectionPrivate);
	}

	pthread_mutex_lock(&connectionPrivate->lock);

#if !TARGET_IPHONE_SIMULATOR
	if (__SCNetworkConnectionUsingNetworkExtension(connectionPrivate)) {
		/* Resume only applies to PPPSerial and PPPoE */
		ok = TRUE;
		goto done;
	}
#endif /* !TARGET_IPHONE_SIMULATOR */

    retry :

	session_port = __SCNetworkConnectionSessionPort(connectionPrivate);
	if (session_port == MACH_PORT_NULL) {
		goto done;
	}

	status = pppcontroller_resume(session_port, &sc_status);
	if (__SCNetworkConnectionNeedsRetry(connection,
					    "SCNetworkConnectionResume()",
					    status,
					    &sc_status)) {
		goto retry;
	}

	if (debug > 0) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("SCNetworkConnectionResume (%p), return: %d"), connectionPrivate, sc_status);
	}

	if (sc_status != kSCStatusOK) {
		_SCErrorSet(sc_status);
		goto done;
	}

	/* connection is now resume */
	ok = TRUE;

    done :

	pthread_mutex_unlock(&connectionPrivate->lock);
	return ok;
}


#if	!TARGET_IPHONE_SIMULATOR
Boolean
SCNetworkConnectionRefreshOnDemandState(SCNetworkConnectionRef connection)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	Boolean				ok			= FALSE;
	uint32_t			retry			= 0;
	int				sc_status		= kSCStatusFailed;
	mach_port_t			server_port		= __SCNetworkConnectionGetCurrentServerPort();
	kern_return_t			status			= KERN_SUCCESS;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (debug > 0) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("SCNetworkConnectionRefreshOnDemandState (%p)"), connectionPrivate);
	}

	pthread_mutex_lock(&connectionPrivate->lock);

	while (TRUE) {
		if (server_port == MACH_PORT_NULL) {
			server_port = __SCNetworkConnectionRefreshServerPort(server_port, &sc_status);
			if (server_port == MACH_PORT_NULL) {
				// if server not available
				if (sc_status == BOOTSTRAP_UNKNOWN_SERVICE) {
					// wait up to 2.5 seconds for the [SCNetworkConnection] server
					// to startup
					if ((retry += 50) < 2500) {
						usleep(50 * 1000);	// sleep 50ms between attempts
						continue;
					}
				}
				break;
			}
		}

		status = pppcontroller_ondemand_refresh_state(server_port, &sc_status);
		if (status == KERN_SUCCESS)
			break;

		if (status == MACH_SEND_INVALID_DEST) {
			// the server is not yet available
			SCLog(TRUE, LOG_ERR, CFSTR("SCNetworkConnectionRefreshOnDemandState (!dest) (%p)"), connectionPrivate);
		} else if (status == MIG_SERVER_DIED) {
			// the server we were using is gone
			SCLog(TRUE, LOG_ERR, CFSTR("SCNetworkConnectionRefreshOnDemandState (!mig) (%p)"), connectionPrivate);
		} else {
			// if we got an unexpected error, don't retry
			sc_status = status;
			break;
		}
	}

	if (debug > 0) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("SCNetworkConnectionRefreshOnDemandState (%p), return: %d/%d"), connectionPrivate, status, sc_status);
	}

	if (sc_status != kSCStatusOK) {
		_SCErrorSet(sc_status);
		goto done;
	}

	ok = TRUE;

    done :

	pthread_mutex_unlock(&connectionPrivate->lock);
	return ok;
}
#endif	/* !TARGET_IPHONE_SIMULATOR */


CFDictionaryRef
SCNetworkConnectionCopyUserOptions(SCNetworkConnectionRef connection)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	xmlDataOut_t			data			= NULL;
	mach_msg_type_number_t		datalen			= 0;
	int				sc_status		= kSCStatusFailed;
	mach_port_t			session_port;
	kern_return_t			status;
	CFPropertyListRef 		userOptions		= NULL;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	pthread_mutex_lock(&connectionPrivate->lock);

#if !TARGET_IPHONE_SIMULATOR
	if (__SCNetworkConnectionUsingNetworkExtension(connectionPrivate)) {
		__block xpc_object_t config = NULL;
		ne_session_t ne_session = connectionPrivate->ne_session;

		ne_session_retain(ne_session);
		pthread_mutex_unlock(&connectionPrivate->lock);

		dispatch_semaphore_t ne_sema = dispatch_semaphore_create(0);
		ne_session_get_info(ne_session, NESessionInfoTypeConfiguration, __SCNetworkConnectionQueue(), ^(xpc_object_t result) {
			if (result != NULL) {
				config = xpc_retain(result);
			}
			ne_session_release(ne_session);
			dispatch_semaphore_signal(ne_sema);
		});
		dispatch_semaphore_wait(ne_sema, DISPATCH_TIME_FOREVER);
		dispatch_release(ne_sema);

		if (config != NULL) {
			xpc_object_t xoptions = xpc_dictionary_get_value(config, NESMSessionLegacyUserConfigurationKey);
			if (xoptions != NULL) {
				userOptions = _CFXPCCreateCFObjectFromXPCObject(xoptions);
			}
			xpc_release(config);
		}
		return userOptions;
	}
#endif /* !TARGET_IPHONE_SIMULATOR */

    retry :

	session_port = __SCNetworkConnectionSessionPort(connectionPrivate);
	if (session_port == MACH_PORT_NULL) {
		goto done;
	}

	status = pppcontroller_copyuseroptions(session_port, &data, &datalen, &sc_status);
	if (__SCNetworkConnectionNeedsRetry(connection,
					    "SCNetworkConnectionCopyUserOptions()",
					    status,
					    &sc_status)) {
		goto retry;
	}

	if (data != NULL) {
		if (!_SCUnserialize(&userOptions, NULL, data, datalen)) {
			if (sc_status != kSCStatusOK) sc_status = SCError();
		}
		if ((sc_status == kSCStatusOK) && (userOptions != NULL) && !isA_CFDictionary(userOptions)) {
			sc_status = kSCStatusFailed;
		}
	}

	if (sc_status == kSCStatusOK) {
		if (userOptions == NULL) {
			// if no user options, return an empty dictionary
			userOptions = CFDictionaryCreate(NULL,
							 NULL,
							 NULL,
							 0,
							 &kCFTypeDictionaryKeyCallBacks,
							 &kCFTypeDictionaryValueCallBacks);
		}
	} else {
		if (userOptions) {
			CFRelease(userOptions);
			userOptions = NULL;
		}
		_SCErrorSet(sc_status);
	}

    done :

	pthread_mutex_unlock(&connectionPrivate->lock);
	return userOptions;
}


static Boolean
__SCNetworkConnectionScheduleWithRunLoop(SCNetworkConnectionRef	connection,
					 CFRunLoopRef		runLoop,
					 CFStringRef		runLoopMode,
					 dispatch_queue_t	queue)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	Boolean				ok			= FALSE;
	int				sc_status		= kSCStatusFailed;
	mach_port_t			session_port;
	kern_return_t			status;

	pthread_mutex_lock(&connectionPrivate->lock);

	if (connectionPrivate->rlsFunction == NULL) {
		_SCErrorSet(kSCStatusInvalidArgument);
		goto done;
	}

	if ((connectionPrivate->dispatchQueue != NULL) ||		// if we are already scheduled on a dispatch queue
	    ((queue != NULL) && connectionPrivate->scheduled)) {	// if we are already scheduled on a CFRunLoop
		_SCErrorSet(kSCStatusInvalidArgument);
		goto done;
	}

	if (!connectionPrivate->scheduled) {

	    retry :

		if (!__SCNetworkConnectionUsingNetworkExtension(connectionPrivate)) {
			session_port = __SCNetworkConnectionSessionPort(connectionPrivate);
			if (session_port == MACH_PORT_NULL) {
				goto done;
			}

			status = pppcontroller_notification(session_port, 1, &sc_status);
			if (__SCNetworkConnectionNeedsRetry(connection,
							    "__SCNetworkConnectionScheduleWithRunLoop()",
							    status,
							    &sc_status)) {
				goto retry;
			}

			if (sc_status != kSCStatusOK) {
				_SCErrorSet(sc_status);
				goto done;
			}

			if (runLoop != NULL) {
				connectionPrivate->rls = CFMachPortCreateRunLoopSource(NULL, connectionPrivate->notify_port, 0);
				connectionPrivate->rlList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
			}
		} else if (runLoop != NULL) {
			CFRunLoopSourceContext rlsContext = {
			    0,					// version
			    (void *)connection,			// info
			    NULL,				// retain
			    NULL,				// release
			    NULL,				// copy description
			    NULL,				// equal
			    NULL,				// hash
			    NULL,				// schedule
			    NULL,				// cancel
			    __SCNetworkConnectionCallBack,	// perform
			};

			connectionPrivate->rls = CFRunLoopSourceCreate(kCFAllocatorDefault, 0, &rlsContext);
			connectionPrivate->rlList = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		}

		connectionPrivate->scheduled = TRUE;
	}

	if (queue != NULL) {
		// retain the dispatch queue
		connectionPrivate->dispatchQueue = queue;
		dispatch_retain(connectionPrivate->dispatchQueue);

		if (!__SCNetworkConnectionUsingNetworkExtension(connectionPrivate)) {
			dispatch_group_t	group	= NULL;
			mach_port_t		mp;
			dispatch_source_t	source;

			//
			// We've taken a reference to the caller's dispatch_queue and we
			// want to hold on to that reference until we've processed any/all
			// notifications.  To facilitate this we create a group, dispatch
			// any notification blocks via that group, and when the caller
			// has told us to stop the notifications (unschedule) we wait for
			// the group to empty and use the group's finalizer to release
			// our reference to the SCNetworkConnection.
			//
			group = dispatch_group_create();
			connectionPrivate->dispatchGroup = group;
			CFRetain(connection);
			dispatch_set_context(connectionPrivate->dispatchGroup, (void *)connection);
			dispatch_set_finalizer_f(connectionPrivate->dispatchGroup, (dispatch_function_t)CFRelease);

			mp = CFMachPortGetPort(connectionPrivate->notify_port);
			source = dispatch_source_create(DISPATCH_SOURCE_TYPE_MACH_RECV, mp, 0, queue);
			if (source == NULL) {
				SCLog(TRUE, LOG_ERR, CFSTR("SCNetworkConnection dispatch_source_create() failed"));
				_SCErrorSet(kSCStatusFailed);
				goto done;
			}

			// have our dispatch source hold a reference to the notification CFMachPort
			CFRetain(connectionPrivate->notify_port);
			dispatch_set_context(source, (void *)connectionPrivate->notify_port);
			dispatch_set_finalizer_f(source, (dispatch_function_t)CFRelease);

			dispatch_source_set_event_handler(source, ^{
				kern_return_t	kr;
				typedef union {
					u_int8_t			buf[sizeof(mach_msg_empty_t) + MAX_TRAILER_SIZE];
					mach_msg_empty_rcv_t		msg;
					mach_no_senders_notification_t	no_senders;
				} *notify_message_t;
				CFMachPortRef		notify_port;
				notify_message_t	notify_msg;

				notify_msg = (notify_message_t)malloc(sizeof(*notify_msg));

				kr = mach_msg(&notify_msg->msg.header,	// msg
					      MACH_RCV_MSG,		// options
					      0,			// send_size
					      sizeof(*notify_msg),	// rcv_size
					      mp,			// rcv_name
					      MACH_MSG_TIMEOUT_NONE,	// timeout
					      MACH_PORT_NULL);		// notify
				if (kr != KERN_SUCCESS) {
					SCLog(TRUE, LOG_ERR,
					      CFSTR("SCDynamicStore notification handler, kr=0x%x"),
					      kr);
					return;
				}

				CFRetain(connection);
				notify_port = dispatch_get_context(source);

				dispatch_group_async(group, queue, ^{
					__SCNetworkConnectionMachCallBack(notify_port,
									  (void *)notify_msg,
									  sizeof(*notify_msg),
									  (void *)connection);
					free(notify_msg);
					CFRelease(connection);
				});
			});

			dispatch_source_set_cancel_handler(source, ^{
				dispatch_release(source);
			});

			connectionPrivate->dispatchSource = source;
			dispatch_resume(source);
		}
	} else {
		if (!_SC_isScheduled(NULL, runLoop, runLoopMode, connectionPrivate->rlList)) {
			/*
			 * if we do not already have notifications scheduled with
			 * this runLoop / runLoopMode
			 */
			CFRunLoopAddSource(runLoop, connectionPrivate->rls, runLoopMode);
		}

		_SC_schedule(connection, runLoop, runLoopMode, connectionPrivate->rlList);
	}

#if !TARGET_IPHONE_SIMULATOR
	if (__SCNetworkConnectionUsingNetworkExtension(connectionPrivate)) {
		CFRetain(connection);
		ne_session_set_event_handler(connectionPrivate->ne_session, __SCNetworkConnectionQueue(), ^(ne_session_event_t event, void *event_data) {
			#pragma unused(event_data)
			if (event == NESessionEventStatusChanged) {
				CFRetain(connection); /* Released in __SCNetworkConnectionCallBack */
				pthread_mutex_lock(&connectionPrivate->lock);
				if (connectionPrivate->rls != NULL) {
					CFRunLoopSourceSignal(connectionPrivate->rls);
					_SC_signalRunLoop(connection, connectionPrivate->rls, connectionPrivate->rlList);
				} else if (connectionPrivate->dispatchQueue != NULL) {
					dispatch_async(connectionPrivate->dispatchQueue, ^{ __SCNetworkConnectionCallBack((void *)connection); });
				}
				pthread_mutex_unlock(&connectionPrivate->lock);
			} else if (event == NESessionEventCanceled) {
				CFRelease(connection);
			}
		});
	}
#endif /* !TARGET_IPHONE_SIMULATOR */

	ok = TRUE;

    done :

	pthread_mutex_unlock(&connectionPrivate->lock);
	return ok;
}


static Boolean
__SCNetworkConnectionUnscheduleFromRunLoop(SCNetworkConnectionRef	connection,
					   CFRunLoopRef			runLoop,
					   CFStringRef			runLoopMode,
					   dispatch_queue_t		queue)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	dispatch_group_t		drainGroup		= NULL;
	dispatch_queue_t		drainQueue		= NULL;
	int				sc_status		= kSCStatusFailed;
	CFIndex				n			= 0;
	Boolean				ok			= FALSE;
	kern_return_t			status;

	// hold a reference while we unschedule
	CFRetain(connection);

	pthread_mutex_lock(&connectionPrivate->lock);

	if ((runLoop != NULL) && !connectionPrivate->scheduled) {			// if we should be scheduled (but are not)
		_SCErrorSet(kSCStatusInvalidArgument);
		goto done;
	}

	if (((runLoop == NULL) && (connectionPrivate->dispatchQueue == NULL)) ||	// if we should be scheduled on a dispatch queue (but are not)
	    ((runLoop != NULL) && (connectionPrivate->dispatchQueue != NULL))) {	// if we should be scheduled on a CFRunLoop (but are scheduled on a dispatch queue)
		_SCErrorSet(kSCStatusInvalidArgument);
		goto done;
	}

	if (connectionPrivate->dispatchQueue != NULL) {
		if (!__SCNetworkConnectionUsingNetworkExtension(connectionPrivate)) {
			// cancel dispatchSource
			if (connectionPrivate->dispatchSource != NULL) {
				dispatch_source_cancel(connectionPrivate->dispatchSource);
				connectionPrivate->dispatchSource = NULL;
			}

			// save dispatchQueue/group, release reference when all queue'd blocks
			// have been processed, allow re-scheduling
			drainGroup = connectionPrivate->dispatchGroup;
			connectionPrivate->dispatchGroup = NULL;
			drainQueue = connectionPrivate->dispatchQueue;
			connectionPrivate->dispatchQueue = NULL;
		} else {
			dispatch_release(connectionPrivate->dispatchQueue);
			connectionPrivate->dispatchQueue = NULL;
		}
	} else {
		if (!_SC_unschedule(connection, runLoop, runLoopMode, connectionPrivate->rlList, FALSE)) {
			// if not currently scheduled on this runLoop / runLoopMode
			_SCErrorSet(kSCStatusFailed);
			goto done;
		}

		n = CFArrayGetCount(connectionPrivate->rlList);
		if (n == 0 || !_SC_isScheduled(NULL, runLoop, runLoopMode, connectionPrivate->rlList)) {
			/*
			 * if we are no longer scheduled to receive notifications for
			 * this runLoop / runLoopMode
			 */
			CFRunLoopRemoveSource(runLoop, connectionPrivate->rls, runLoopMode);

			if (n == 0) {
				// if *all* notifications have been unscheduled
				CFRelease(connectionPrivate->rlList);
				connectionPrivate->rlList = NULL;
				CFRunLoopSourceInvalidate(connectionPrivate->rls);
				CFRelease(connectionPrivate->rls);
				connectionPrivate->rls = NULL;
			}
		}
	}

	if (n == 0) {
		// if *all* notifications have been unscheduled
		connectionPrivate->scheduled = FALSE;

		if (__SCNetworkConnectionUsingNetworkExtension(connectionPrivate)) {
#if !TARGET_IPHONE_SIMULATOR
			ne_session_cancel(connectionPrivate->ne_session);
#endif /* !TARGET_IPHONE_SIMULATOR */
		} else {
			mach_port_t session_port = __SCNetworkConnectionSessionPort(connectionPrivate);
			if (session_port == MACH_PORT_NULL) {
				goto done;
			}

			status = pppcontroller_notification(session_port, 0, &sc_status);
			if (__SCNetworkConnectionNeedsRetry(connection,
							    "__SCNetworkConnectionUnscheduleFromRunLoop pppcontroller_notification()",
							    status,
							    &sc_status)) {
				sc_status = kSCStatusOK;
				status = KERN_SUCCESS;
			}

			if ((status != KERN_SUCCESS) || (sc_status != kSCStatusOK)) {
				_SCErrorSet(sc_status);
				goto done;
			}
		}
	}

	ok = TRUE;

    done :

	pthread_mutex_unlock(&connectionPrivate->lock);

	if (drainGroup != NULL) {
		dispatch_group_notify(drainGroup, drainQueue, ^{
			// release group/queue references
			dispatch_release(drainQueue);
			dispatch_release(drainGroup);	// releases our connection reference
		});
	}

	// release our reference
	CFRelease(connection);

	return ok;
}


Boolean
SCNetworkConnectionScheduleWithRunLoop(SCNetworkConnectionRef	connection,
				       CFRunLoopRef		runLoop,
				       CFStringRef		runLoopMode)
{
	if (!isA_SCNetworkConnection(connection) || (runLoop == NULL) || (runLoopMode == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	return __SCNetworkConnectionScheduleWithRunLoop(connection, runLoop, runLoopMode, NULL);
}


Boolean
SCNetworkConnectionUnscheduleFromRunLoop(SCNetworkConnectionRef		connection,
					 CFRunLoopRef			runLoop,
					 CFStringRef			runLoopMode)
{
	if (!isA_SCNetworkConnection(connection) || (runLoop == NULL) || (runLoopMode == NULL)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	return __SCNetworkConnectionUnscheduleFromRunLoop(connection, runLoop, runLoopMode, NULL);
}


Boolean
SCNetworkConnectionSetDispatchQueue(SCNetworkConnectionRef	connection,
				    dispatch_queue_t		queue)
{
	Boolean	ok	= FALSE;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (queue != NULL) {
		ok = __SCNetworkConnectionScheduleWithRunLoop(connection, NULL, NULL, queue);
	} else {
		ok = __SCNetworkConnectionUnscheduleFromRunLoop(connection, NULL, NULL, NULL);
	}

	return ok;
}


/* Requires having called SCNetworkConnectionSelectServiceWithOptions previously */
Boolean
SCNetworkConnectionIsOnDemandSuspended(SCNetworkConnectionRef connection)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (connectionPrivate->on_demand_info != NULL) {
		uint32_t	isSuspended	= 0;
		CFNumberRef	num		= NULL;

		num = CFDictionaryGetValue(connectionPrivate->on_demand_info, kSCPropNetVPNOnDemandSuspended);
		if (isA_CFNumber(num) &&
		    CFNumberGetValue(num, kCFNumberSInt32Type, &isSuspended) &&
		    (isSuspended != 0)) {
			return TRUE;
		}
	}

	_SCErrorSet(kSCStatusOK);
	return FALSE;
}

Boolean
SCNetworkConnectionTriggerOnDemandIfNeeded	(CFStringRef			hostName,
						 Boolean			afterDNSFail,
						 int				timeout,
						 int				trafficClass)
{
#if !TARGET_IPHONE_SIMULATOR
	__block Boolean triggeredOnDemand = FALSE;
	struct proc_uniqidentifierinfo procu;
	void *policy_match = NULL;
	char *hostname = NULL;
	CFIndex hostnameSize = 0;
	pid_t pid = getpid();
	uid_t uid = geteuid();
	
	/* Require hostName, require non-root user */
	if (hostName == NULL || geteuid() == 0) {
		goto done;
	}
	
	hostnameSize = CFStringGetLength(hostName);
	if (hostnameSize == 0) {
		goto done;
	}
	
	hostname = malloc(hostnameSize + 1);
	CFStringGetCString(hostName, hostname, hostnameSize + 1, kCFStringEncodingUTF8);
	
	if (proc_pidinfo(pid, PROC_PIDUNIQIDENTIFIERINFO, 1, &procu, sizeof(procu)) != sizeof(procu)) {
		goto done;
	}
	
	policy_match = ne_session_copy_policy_match(hostname, NULL, NULL, procu.p_uuid, procu.p_uuid, pid, uid, 0, trafficClass);
	
	NEPolicyServiceActionType action_type = ne_session_policy_match_get_service_action(policy_match);
	if (action_type == NESessionPolicyActionTrigger ||
	    (afterDNSFail && action_type == NESessionPolicyActionTriggerIfNeeded)) {
		uuid_t config_id;
		if (ne_session_policy_match_get_service(policy_match, config_id)) {
			xpc_object_t start_options = xpc_dictionary_create(NULL, NULL, 0);
			if (start_options != NULL) {
				xpc_dictionary_set_bool(start_options, NESessionStartOptionIsOnDemandKey, true);
				xpc_dictionary_set_string(start_options, NESessionStartOptionMatchHostnameKey, hostname);
				
				ne_session_t new_session = ne_session_create(config_id, ne_session_policy_match_get_service_type(policy_match));
				if (new_session != NULL) {
					dispatch_semaphore_t wait_for_session = dispatch_semaphore_create(0);
					dispatch_retain(wait_for_session);
					xpc_retain(start_options);
					ne_session_get_status(new_session, __SCNetworkConnectionQueue(),
						^(ne_session_status_t status) {
							if (status == NESessionStatusDisconnected) {
								dispatch_retain(wait_for_session);
								ne_session_set_event_handler(new_session, __SCNetworkConnectionQueue(),
									^(ne_session_event_t event, void *event_data) {
										if (event == NESessionEventStatusChanged) {
											dispatch_retain(wait_for_session);
											ne_session_get_status(new_session, __SCNetworkConnectionQueue(),
												^(ne_session_status_t new_status) {
													if (new_status != NESessionStatusConnecting) {
														if (status == NESessionStatusConnected) {
															triggeredOnDemand = TRUE;
														}
														ne_session_cancel(new_session);
													}
													dispatch_release(wait_for_session);
												});
										} else if (event == NESessionEventCanceled) {
											dispatch_semaphore_signal(wait_for_session);
											dispatch_release(wait_for_session);
										}
									});
								ne_session_start_with_options(new_session, start_options);
							} else {
								dispatch_semaphore_signal(wait_for_session);
							}
							dispatch_release(wait_for_session);
							xpc_release(start_options);
						});
					dispatch_semaphore_wait(wait_for_session, timeout ? dispatch_time(DISPATCH_TIME_NOW, (int64_t)timeout * NSEC_PER_SEC) : DISPATCH_TIME_FOREVER);
					dispatch_release(wait_for_session);
					ne_session_release(new_session);
				}
				
				xpc_release(start_options);
			}
		}
	}
done:
	if (hostname) {
		free(hostname);
	}
	
	if (policy_match) {
		free(policy_match);
	}

	return triggeredOnDemand;
#else
#pragma unused(hostName, afterDNSFail, timeout, trafficClass)
	return FALSE;
#endif
}


Boolean
SCNetworkConnectionCopyOnDemandInfo(SCNetworkConnectionRef	connection,
				    CFStringRef			*onDemandRemoteAddress,
				    SCNetworkConnectionStatus	*onDemandConnectionStatus)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (connectionPrivate->service == NULL) {
		_SCErrorSet(kSCStatusConnectionNoService);
		return FALSE;
	}

	if (onDemandRemoteAddress != NULL) {
		*onDemandRemoteAddress = NULL;
	}

	if (onDemandConnectionStatus != NULL) {
		*onDemandConnectionStatus = kSCNetworkConnectionInvalid;
	}

	if (connectionPrivate->on_demand_info != NULL) {
		if (onDemandRemoteAddress != NULL) {
			CFStringRef address =
			    CFDictionaryGetValue(connectionPrivate->on_demand_info, kSCNetworkConnectionOnDemandRemoteAddress);
			if (isA_CFString(address)) {
				*onDemandRemoteAddress = address;
				CFRetain(*onDemandRemoteAddress);
			}
		}

		if (onDemandConnectionStatus != NULL) {
			int num;
			CFNumberRef status_num =
			    CFDictionaryGetValue(connectionPrivate->on_demand_info, kSCNetworkConnectionOnDemandStatus);
			if (isA_CFNumber(status_num) && CFNumberGetValue(status_num, kCFNumberIntType, &num)) {
				*onDemandConnectionStatus = num;
			}
		}
	}

	return connectionPrivate->on_demand;
}


Boolean
SCNetworkConnectionGetReachabilityInfo(SCNetworkConnectionRef		connection,
				       SCNetworkReachabilityFlags	*reach_flags,
				       unsigned int			*reach_if_index)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (connectionPrivate->service == NULL) {
		_SCErrorSet(kSCStatusConnectionNoService);
		return FALSE;
	}

	if (reach_flags != NULL) {
		*reach_flags = 0;
	}

	if (reach_if_index != NULL) {
		*reach_if_index = 0;
	}

	if (connectionPrivate->on_demand_info != NULL) {
		if (reach_flags != NULL) {
			int num;
			CFNumberRef flags_num =
			    CFDictionaryGetValue(connectionPrivate->on_demand_info, kSCNetworkConnectionOnDemandReachFlags);
			if (isA_CFNumber(flags_num) && CFNumberGetValue(flags_num, kCFNumberIntType, &num)) {
				*reach_flags = num;
			}
		}

		if (reach_if_index != NULL) {
			int num;
			CFNumberRef if_index_num =
			    CFDictionaryGetValue(connectionPrivate->on_demand_info, kSCNetworkConnectionOnDemandReachInterfaceIndex);
			if (isA_CFNumber(if_index_num) && CFNumberGetValue(if_index_num, kCFNumberIntType, &num)) {
				*reach_if_index = num;
			}
		}
	}

	return TRUE;
}


SCNetworkConnectionType
SCNetworkConnectionGetType(SCNetworkConnectionRef connection)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return kSCNetworkConnectionTypeUnknown;
	}

	if (connectionPrivate->service == NULL) {
		_SCErrorSet(kSCStatusConnectionNoService);
		return kSCNetworkConnectionTypeUnknown;
	}

	_SCErrorSet(kSCStatusOK);

	return connectionPrivate->type;
}


static Boolean
validate_flow_properties(CFDictionaryRef flowProperties)
{
	CFStringRef			host_name_str;
	CFDataRef			host_address_data;
	CFNumberRef			host_port_num;

	if (!isA_CFDictionary(flowProperties)) {
		return FALSE;
	}

	/* Validate the host name if one was given */
	host_name_str = CFDictionaryGetValue(flowProperties, kSCNetworkConnectionFlowPropertyHostName);
	if (host_name_str != NULL && (!isA_CFString(host_name_str) || CFStringGetLength(host_name_str) == 0)) {
		return FALSE;
	}

	/* Validate the address if one was given */
	host_address_data = CFDictionaryGetValue(flowProperties, kSCNetworkConnectionFlowPropertyHostAddress);
	if (host_address_data != NULL) {
		struct sockaddr *sock_addr;

		if (!isA_CFData(host_address_data) || CFDataGetLength(host_address_data) < sizeof(struct sockaddr)) {
			return FALSE;
		}

		sock_addr = (struct sockaddr *)CFDataGetBytePtr(host_address_data);
		if (CFDataGetLength(host_address_data) < sock_addr->sa_len) {
			return FALSE;
		}

		if (sock_addr->sa_family == AF_INET) {
			if (sock_addr->sa_len >= sizeof(struct sockaddr_in)) {
				struct sockaddr_in *sa_in = (struct sockaddr_in *)(void *)sock_addr;
				in_addr_t any = { INADDR_ANY };
				if (memcmp(&sa_in->sin_addr, &any, sizeof(any)) == 0) {
					return FALSE;
				}
			} else {
				return FALSE;
			}
		} else if (sock_addr->sa_family == AF_INET6) {
			if (sock_addr->sa_len >= sizeof(struct sockaddr_in6)) {
				struct sockaddr_in6 *sa_in6 = (struct sockaddr_in6 *)(void *)sock_addr;
				struct in6_addr any = IN6ADDR_ANY_INIT;
				if (memcmp(&sa_in6->sin6_addr, &any, sizeof(any)) == 0) {
					return FALSE;
				}
			}
		}
	}

	/* We must have either a host name or an address */
	if (host_name_str == NULL && host_address_data == NULL) {
		return FALSE;
	}

	/* Validate the port */
	host_port_num = CFDictionaryGetValue(flowProperties, kSCNetworkConnectionFlowPropertyHostPort);
	if (host_port_num != NULL) {
		int num;
		if (!isA_CFNumber(host_port_num) || !CFNumberGetValue(host_port_num, kCFNumberIntType, &num)) {
			return FALSE;
		}

		if (num == 0) {
			return FALSE;
		}
	} else {
		return FALSE;
	}

	return TRUE;
}


CFDataRef
SCNetworkConnectionCopyFlowDivertToken(SCNetworkConnectionRef	connection,
				       CFDictionaryRef		flowProperties)
{
	CFDictionaryRef			app_properties		= NULL;
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	CFDataRef			token			= NULL;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		goto done;
	}

	if (connectionPrivate->service == NULL) {
		_SCErrorSet(kSCStatusConnectionNoService);
		goto done;
	}

	if (connectionPrivate->type != kSCNetworkConnectionTypeAppLayerVPN) {
		_SCErrorSet(kSCStatusInvalidArgument);
		goto done;
	}

	if (!validate_flow_properties(flowProperties)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		goto done;
	}

	app_properties = VPNAppLayerCopyCachedAppProperties(connectionPrivate->client_audit_token,
							    connectionPrivate->client_pid,
							    connectionPrivate->client_uuid,
							    connectionPrivate->client_bundle_id);
	if (app_properties == NULL) {
		SCLog(TRUE, LOG_ERR, CFSTR("SCNetworkConnectionCopyFlowDivertToken: no cached app properties available"));
		_SCErrorSet(kSCStatusFailed);
		goto done;
	}

	token = VPNAppLayerCreateFlowDivertToken(connection, app_properties, flowProperties);

done:
	if (app_properties != NULL) {
		CFRelease(app_properties);
	}

	return token;
}


int
SCNetworkConnectionGetServiceIdentifier		(SCNetworkConnectionRef		connection)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	int				service_identifier	= -1;

	if (connectionPrivate->service != NULL) {
		service_identifier = 0;
		if (connectionPrivate->on_demand_info != NULL) {
			CFNumberRef id_num = CFDictionaryGetValue(connectionPrivate->on_demand_info, kSCPropNetDNSServiceIdentifier);

			if (isA_CFNumber(id_num)) {
				CFNumberGetValue(id_num, kCFNumberIntType, &service_identifier);
			}
		}
	}

	return service_identifier;
}


#if !TARGET_IPHONE_SIMULATOR
SCNetworkConnectionStatus
SCNetworkConnectionGetStatusFromNEStatus(ne_session_status_t status)
{
	switch (status) {
		case NESessionStatusInvalid:
			return kSCNetworkConnectionInvalid;
		case NESessionStatusDisconnected:
			return kSCNetworkConnectionDisconnected;
		case NESessionStatusConnecting:
		case NESessionStatusReasserting:
			return kSCNetworkConnectionConnecting;
		case NESessionStatusConnected:
			return kSCNetworkConnectionConnected;
		case NESessionStatusDisconnecting:
			return kSCNetworkConnectionDisconnecting;
	}

	return kSCNetworkConnectionInvalid;
}
#endif /* !TARGET_IPHONE_SIMULATOR */


#pragma mark -
#pragma mark User level "dial" API


#define k_NetworkConnect_Notification	"com.apple.networkConnect"
#define k_NetworkConnect_Pref_File	CFSTR("com.apple.networkConnect")
#define k_InterentConnect_Pref_File	CFSTR("com.apple.internetconnect")

#define k_Dial_Default_Key		CFSTR("ConnectByDefault") // needs to go into SC
#define k_Last_Service_Id_Key		CFSTR("ServiceID")
#define k_Unique_Id_Key	 		CFSTR("UniqueIdentifier")


/* Private Prototypes */
static Boolean SCNetworkConnectionPrivateCopyDefaultServiceIDForDial	(CFStringRef *serviceID);
static Boolean SCNetworkConnectionPrivateGetPPPServiceFromDynamicStore	(CFStringRef *serviceID);
static Boolean SCNetworkConnectionPrivateCopyDefaultUserOptionsFromArray(CFArrayRef userOptionsArray, CFDictionaryRef *userOptions);
static Boolean SCNetworkConnectionPrivateIsPPPService			(CFStringRef serviceID, CFStringRef subType1, CFStringRef subType2);
static void addPasswordFromKeychain					(CFStringRef serviceID, CFDictionaryRef *userOptions);
static CFStringRef copyPasswordFromKeychain				(CFStringRef uniqueID);

static int		notify_userprefs_token	= -1;

static CFDictionaryRef	onDemand_configuration	= NULL;
static Boolean		onDemand_force_refresh	= FALSE;
static pthread_mutex_t	onDemand_notify_lock	= PTHREAD_MUTEX_INITIALIZER;
static int		onDemand_notify_token	= -1;


/*
 *	return TRUE if domain1 ends with domain2, and will check for trailing "."
 */
#define WILD_CARD_MATCH_STR CFSTR("*")
Boolean
_SC_domainEndsWithDomain(CFStringRef compare_domain, CFStringRef match_domain)
{
	CFRange		range;
	Boolean		ret		= FALSE;
	CFStringRef	s1		= NULL;
	Boolean		s1_created	= FALSE;
	CFStringRef	s2		= NULL;
	Boolean		s2_created	= FALSE;
	CFStringRef	s3		= NULL;

	if (CFEqual(match_domain, WILD_CARD_MATCH_STR)) {
		return TRUE;
	}

	if (CFStringHasSuffix(compare_domain, CFSTR("."))) {
		range.location = 0;
		range.length = CFStringGetLength(compare_domain) - 1;
		s1 = CFStringCreateWithSubstring(NULL, compare_domain, range);
		if (s1 == NULL) {
			goto done;
		}
		s1_created = TRUE;
	} else {
		s1 = compare_domain;
	}

	if (CFStringHasSuffix(match_domain, CFSTR("."))) {
		range.location = 0;
		range.length = CFStringGetLength(match_domain) - 1;
		s2 = CFStringCreateWithSubstring(NULL, match_domain, range);
		if (s2 == NULL) {
			goto done;
		}
		s2_created = TRUE;
	} else {
		s2 = match_domain;
	}

	if (CFStringHasPrefix(s2, CFSTR("*."))) {
		range.location = 2;
		range.length = CFStringGetLength(s2)-2;
		s3 = CFStringCreateWithSubstring(NULL, s2, range);
		if (s3 == NULL) {
			goto done;
		}
		if (s2_created) {
			CFRelease(s2);
		}
		s2 = s3;
		s2_created = TRUE;
	}

	ret = CFStringHasSuffix(s1, s2);

    done :

	if (s1_created)	CFRelease(s1);
	if (s2_created)	CFRelease(s2);
	return ret;
}

static CFCharacterSetRef
_SC_getNotDotOrStarCharacterSet (void)
{
	static CFCharacterSetRef notDotOrStar = NULL;
	if (notDotOrStar == NULL) {
		CFCharacterSetRef dotOrStar = CFCharacterSetCreateWithCharactersInString(kCFAllocatorDefault, CFSTR(".*"));
		if (dotOrStar) {
			notDotOrStar = CFCharacterSetCreateInvertedSet(kCFAllocatorDefault, dotOrStar);
			CFRelease(dotOrStar);
		}
	}
	return notDotOrStar;
}

static CFMutableStringRef
_SC_createStringByTrimmingDotsAndStars (CFStringRef string)
{
	CFCharacterSetRef notDotOrStar = _SC_getNotDotOrStarCharacterSet();
	CFRange entireString = CFRangeMake(0, CFStringGetLength(string));
	CFMutableStringRef result = CFStringCreateMutableCopy(kCFAllocatorDefault, entireString.length, string);
	CFRange start;
	CFRange end = CFRangeMake(entireString.length, 0);

	if (CFStringFindCharacterFromSet(string, notDotOrStar, entireString, 0, &start) &&
	    CFStringFindCharacterFromSet(string, notDotOrStar, entireString, kCFCompareBackwards, &end)) {
		if (start.location == kCFNotFound || end.location == kCFNotFound || start.location > end.location) {
			CFRelease(result);
			return NULL;
		}
	}

	if ((end.location + 1) < entireString.length) {
		CFStringReplace(result, CFRangeMake(end.location + 1, entireString.length - (end.location + 1)), CFSTR(""));
	}
	if (start.location > 0) {
		CFStringReplace(result, CFRangeMake(0, start.location), CFSTR(""));
	}

	return result;
}

static CFIndex
_SC_getCountOfStringInString (CFStringRef string, CFStringRef substring)
{
	CFIndex count = 0;
	CFArrayRef ranges = CFStringCreateArrayWithFindResults(kCFAllocatorDefault, string, substring, CFRangeMake(0, CFStringGetLength(string)), 0);
	if (ranges != NULL) {
		count = CFArrayGetCount(ranges);
		CFRelease(ranges);
	}
	return count;
}

Boolean
_SC_hostMatchesDomain(CFStringRef hostname, CFStringRef domain)
{
	Boolean			result		= FALSE;
	CFMutableStringRef	trimmedHostname	= NULL;
	CFMutableStringRef	trimmedDomain	= NULL;

	if (!isA_CFString(hostname) || !isA_CFString(domain)) {
		goto done;
	}

	trimmedHostname = _SC_createStringByTrimmingDotsAndStars(hostname);
	trimmedDomain = _SC_createStringByTrimmingDotsAndStars(domain);

	if (!isA_CFString(trimmedHostname) || !isA_CFString(trimmedDomain)) {
		goto done;
	}

	CFIndex numHostnameDots = _SC_getCountOfStringInString(trimmedHostname, CFSTR("."));
	CFIndex numDomainDots = _SC_getCountOfStringInString(trimmedDomain, CFSTR("."));
	if (numHostnameDots == numDomainDots) {
		result = CFEqual(trimmedHostname, trimmedDomain);
	} else if (numDomainDots > 0 && numDomainDots < numHostnameDots) {
		CFStringReplace(trimmedDomain, CFRangeMake(0, 0), CFSTR("."));
		result = CFStringHasSuffix(trimmedHostname, trimmedDomain);
	} else {
		result = FALSE;
	}

done:
	if (trimmedHostname) {
		CFRelease(trimmedHostname);
	}
	if (trimmedDomain) {
		CFRelease(trimmedDomain);
	}
	return result;
}

/* VPN On Demand */

static CFDictionaryRef
__SCNetworkConnectionCopyOnDemandConfiguration(void)
{
	int			changed		= 1;
	int			status;
	uint64_t		triggersCount	= 0;
	CFDictionaryRef		configuration;

	pthread_mutex_lock(&onDemand_notify_lock);
	if (onDemand_notify_token == -1) {
		status = notify_register_check(kSCNETWORKCONNECTION_ONDEMAND_NOTIFY_KEY, &onDemand_notify_token);
		if (status != NOTIFY_STATUS_OK) {
			SCLog(TRUE, LOG_ERR, CFSTR("notify_register_check() failed, status=%d"), status);
			onDemand_notify_token = -1;
		}
	}

	if (onDemand_notify_token != -1) {
		status = notify_check(onDemand_notify_token, &changed);
		if (status != NOTIFY_STATUS_OK) {
			SCLog(TRUE, LOG_ERR, CFSTR("notify_check() failed, status=%d"), status);
			(void)notify_cancel(onDemand_notify_token);
			onDemand_notify_token = -1;
		}
	}

	if (changed && (onDemand_notify_token != -1)) {
		status = notify_get_state(onDemand_notify_token, &triggersCount);
		if (status != NOTIFY_STATUS_OK) {
			SCLog(TRUE, LOG_ERR, CFSTR("notify_get_state() failed, status=%d"), status);
			(void)notify_cancel(onDemand_notify_token);
			onDemand_notify_token = -1;
		}
	}

	if (changed || onDemand_force_refresh) {
		CFStringRef	key;

		if (_sc_debug || (debug > 0)) {
			SCLog(TRUE, LOG_INFO,
			      CFSTR("OnDemand information %s"),
			      (onDemand_configuration == NULL) ? "fetched" : "updated");
		}

		if (onDemand_configuration != NULL) {
			CFRelease(onDemand_configuration);
			onDemand_configuration = NULL;
		}

		if ((triggersCount > 0) || onDemand_force_refresh) {
			key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL, kSCDynamicStoreDomainState, kSCEntNetOnDemand);
			onDemand_configuration = SCDynamicStoreCopyValue(NULL, key);
			CFRelease(key);
			if ((onDemand_configuration != NULL) && !isA_CFDictionary(onDemand_configuration)) {
				CFRelease(onDemand_configuration);
				onDemand_configuration = NULL;
			}
		}

		onDemand_force_refresh = FALSE;
	}

	configuration = (onDemand_configuration != NULL) ? CFRetain(onDemand_configuration) : NULL;
	pthread_mutex_unlock(&onDemand_notify_lock);

	return configuration;
}


__private_extern__
void
__SCNetworkConnectionForceOnDemandConfigurationRefresh(void)
{
	pthread_mutex_lock(&onDemand_notify_lock);
	onDemand_force_refresh = TRUE;
	pthread_mutex_unlock(&onDemand_notify_lock);

	return;
}


static Boolean
__SCNetworkConnectionShouldNeverMatch(CFDictionaryRef trigger, CFStringRef hostName, pid_t client_pid)
{
	CFArrayRef	exceptedProcesses;
	CFIndex		exceptedProcessesCount;
	CFIndex		exceptedProcessesIndex;
	CFArrayRef	exceptions;
	CFIndex		exceptionsCount;
	int		exceptionsIndex;

	// we have a matching domain, check against exception list
	exceptions = CFDictionaryGetValue(trigger, kSCNetworkConnectionOnDemandMatchDomainsNever);
	exceptionsCount = isA_CFArray(exceptions) ? CFArrayGetCount(exceptions) : 0;
	for (exceptionsIndex = 0; exceptionsIndex < exceptionsCount; exceptionsIndex++) {
		CFStringRef	exception;

		exception = CFArrayGetValueAtIndex(exceptions, exceptionsIndex);
		if (isA_CFString(exception) && _SC_domainEndsWithDomain(hostName, exception)) {
			// found matching exception
			if (_sc_debug || (debug > 0)) {
				SCLog(TRUE, LOG_INFO, CFSTR("OnDemand match exception"));
			}
			return TRUE;
		}
	}

	if (client_pid != 0) {
		exceptedProcesses = CFDictionaryGetValue(trigger, kSCNetworkConnectionOnDemandPluginPIDs);
		exceptedProcessesCount = isA_CFArray(exceptedProcesses) ? CFArrayGetCount(exceptedProcesses) : 0;
		for (exceptedProcessesIndex = 0; exceptedProcessesIndex < exceptedProcessesCount; exceptedProcessesIndex++) {
			int		pid;
			CFNumberRef	pidRef;

			pidRef = CFArrayGetValueAtIndex(exceptedProcesses, exceptedProcessesIndex);
			if (isA_CFNumber(pidRef) && CFNumberGetValue(pidRef, kCFNumberIntType, &pid)) {
				if (pid == client_pid) {
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

static CFStringRef
__SCNetworkConnectionDomainGetMatchWithParameters(CFStringRef action, CFPropertyListRef actionParameters, CFStringRef hostName, CFStringRef *probeString)
{
	CFArrayRef	actionArray = NULL;
	CFIndex		actionArraySize = 0;
	CFIndex		i;
	CFStringRef	matchDomain	= NULL;

	/* For now, only support EvaluateConnection, which takes a CFArray */
	if (!CFEqual(action, kSCValNetVPNOnDemandRuleActionEvaluateConnection) || !isA_CFArray(actionParameters)) {
		return NULL;
	}

	actionArray = (CFArrayRef)actionParameters;
	actionArraySize = CFArrayGetCount(actionArray);

	/* Process domain rules, with actions of ConnectIfNeeded and NeverConnect */
	for (i = 0; i < actionArraySize; i++) {
		CFStringRef	domainAction = NULL;
		CFDictionaryRef	domainRule = CFArrayGetValueAtIndex(actionArray, i);
		CFArrayRef	domains = NULL;
		CFIndex		domainsCount = 0;
		CFIndex		domainsIndex;

		if (!isA_CFDictionary(domainRule)) {
			continue;
		}

		domains = CFDictionaryGetValue(domainRule, kSCPropNetVPNOnDemandRuleActionParametersDomains);
		if (!isA_CFArray(domains)) {
			continue;
		}

		domainsCount = CFArrayGetCount(domains);
		for (domainsIndex = 0; domainsIndex < domainsCount; domainsIndex++) {
			CFStringRef	domain;
			domain = CFArrayGetValueAtIndex(domains, domainsIndex);
			if (isA_CFString(domain) && _SC_domainEndsWithDomain(hostName, domain)) {
				matchDomain = domain;
				break;
			}
		}

		if (matchDomain) {
			domainAction = CFDictionaryGetValue(domainRule, kSCPropNetVPNOnDemandRuleActionParametersDomainAction);
			if (isA_CFString(domainAction) && CFEqual(domainAction, kSCValNetVPNOnDemandRuleActionParametersDomainActionNeverConnect)) {
				return NULL;
			} else {
				/* If we found a match, save the optional probe string as well */
				if (probeString) {
					*probeString = CFDictionaryGetValue(domainRule, kSCPropNetVPNOnDemandRuleActionParametersRequiredURLStringProbe);
				}
				break;
			}
		}
	}

	return matchDomain;
}

static CFStringRef
__SCNetworkConnectionDomainGetMatch(CFDictionaryRef trigger, CFStringRef hostName, Boolean onDemandRetry)
{
	CFArrayRef	domains;
	CFIndex		domainsCount;
	int		domainsIndex;
	CFStringRef	key;
	CFStringRef	match_domain	= NULL;

	/* Old configuration: always, never, on retry lists */
	key = onDemandRetry ? kSCNetworkConnectionOnDemandMatchDomainsOnRetry : kSCNetworkConnectionOnDemandMatchDomainsAlways;

	domains = CFDictionaryGetValue(trigger, key);
	domainsCount = isA_CFArray(domains) ? CFArrayGetCount(domains) : 0;
	for (domainsIndex = 0; domainsIndex < domainsCount; domainsIndex++) {
		CFStringRef	domain;

		domain = CFArrayGetValueAtIndex(domains, domainsIndex);
		if (isA_CFString(domain) && _SC_domainEndsWithDomain(hostName, domain)) {
			match_domain = domain;
			break;
		}
	}

	return match_domain;
}


static Boolean
__SCNetworkConnectionShouldAlwaysConnect(CFDictionaryRef trigger)
{
	CFStringRef action = CFDictionaryGetValue(trigger, kSCPropNetVPNOnDemandRuleAction);
	return (isA_CFString(action) && CFEqual(action, kSCValNetVPNOnDemandRuleActionConnect));
}


static Boolean
__SCNetworkConnectionShouldIgnoreTrigger(CFDictionaryRef trigger)
{
	CFStringRef	action		= CFDictionaryGetValue(trigger, kSCPropNetVPNOnDemandRuleAction);

	if (isA_CFString(action) &&
	    (CFEqual(action, kSCValNetVPNOnDemandRuleActionIgnore) ||
	     CFEqual(action, kSCValNetVPNOnDemandRuleActionDisconnect))) {
		    return TRUE;
	}

	return FALSE;
}


static CFDictionaryRef
__SCNetworkConnectionCopyMatchingTriggerWithName(CFDictionaryRef	configuration,
						 CFStringRef		hostName,
						 pid_t			client_pid,
						 Boolean		onDemandRetry,
						 CFDictionaryRef	*match_info,
						 Boolean		*triggerNow,
						 CFStringRef		*probe_string)
{
	CFDictionaryRef	result		= NULL;
	int		sc_status	= kSCStatusOK;
	CFArrayRef	triggers;
	uint64_t	triggersCount	= 0;
	int		triggersIndex;
	Boolean		usedOnDemandRetry = FALSE;

	if (triggerNow != NULL) {
		*triggerNow = FALSE;
	}

	if (match_info != NULL) {
		*match_info = NULL;
	}

	triggers = CFDictionaryGetValue(configuration, kSCNetworkConnectionOnDemandTriggers);
	triggersCount = isA_CFArray(triggers) ? CFArrayGetCount(triggers) : 0;
	for (triggersIndex = 0; triggersIndex < triggersCount; triggersIndex++) {
		CFStringRef	matched_domain	= NULL;
		CFStringRef	matched_probe_string = NULL;
		CFDictionaryRef	trigger;
		Boolean		trigger_matched	= FALSE;

		usedOnDemandRetry = FALSE;

		trigger = CFArrayGetValueAtIndex(triggers, triggersIndex);
		if (!isA_CFDictionary(trigger)) {
			// if not a valid "OnDemand" configuration
			continue;
		}

		if (__SCNetworkConnectionShouldAlwaysConnect(trigger)) {
			/* If the trigger action is 'Connect', always match this trigger */
			/* First check the never match list */
			if (__SCNetworkConnectionShouldNeverMatch(trigger, hostName, client_pid)) {
				continue;
			}
			trigger_matched = TRUE;
		} else if (__SCNetworkConnectionShouldIgnoreTrigger(trigger)) {
			/* If the trigger action is 'Ignore' or 'Disconnect', skip this trigger */
			sc_status = kSCStatusConnectionIgnore;
			continue;
		} else {
			CFStringRef action = CFDictionaryGetValue(trigger, kSCPropNetVPNOnDemandRuleAction);
			CFArrayRef actionParameters = CFDictionaryGetValue(trigger, kSCPropNetVPNOnDemandRuleActionParameters);
			if (action && actionParameters) {
				matched_domain = __SCNetworkConnectionDomainGetMatchWithParameters(action, actionParameters, hostName, &matched_probe_string);
				usedOnDemandRetry = TRUE;
			} else {
				if (onDemandRetry) {
					matched_domain = __SCNetworkConnectionDomainGetMatch(trigger, hostName, TRUE);
					usedOnDemandRetry = TRUE;
				} else {
					matched_domain = __SCNetworkConnectionDomainGetMatch(trigger, hostName, FALSE);
					if (matched_domain == NULL && result == NULL) {
						/* Check the retry list if Always failed */
						matched_domain = __SCNetworkConnectionDomainGetMatch(trigger, hostName, TRUE);
						usedOnDemandRetry = TRUE;
					}
				}
			}

			if (matched_domain) {
				if (__SCNetworkConnectionShouldNeverMatch(trigger, hostName, client_pid)) {
					matched_domain = NULL;
					continue;
				} else {
					trigger_matched = TRUE;
				}
			}
		}

		if (trigger_matched) {
			// if we have a matching domain and there were no exceptions
			// then we pass back the OnDemand info
			if (match_info != NULL) {
				CFMutableDictionaryRef	minfo;
				SCNetworkConnectionType	type	= kSCNetworkConnectionTypeIPLayerVPN;
				CFNumberRef		type_num;

				if (*match_info != NULL) {
					CFRelease(*match_info);
					*match_info = NULL;
				}

				minfo = CFDictionaryCreateMutable(kCFAllocatorDefault,
								  0,
								  &kCFTypeDictionaryKeyCallBacks,
								  &kCFTypeDictionaryValueCallBacks);

				type_num = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &type);
				CFDictionarySetValue(minfo, kSCNetworkConnectionOnDemandMatchInfoVPNType, type_num);
				CFRelease(type_num);
				if (matched_domain) {
					CFDictionarySetValue(minfo, kSCNetworkConnectionOnDemandMatchInfoDomain, matched_domain);
				}
				CFDictionarySetValue(minfo,
						     kSCNetworkConnectionOnDemandMatchInfoOnRetry,
						     (usedOnDemandRetry ? kCFBooleanTrue : kCFBooleanFalse));

				*match_info = minfo;
			}

			if (probe_string != NULL) {
				if (*probe_string != NULL) {
					CFRelease(*probe_string);
					*probe_string = NULL;
				}

				if (matched_probe_string) {
					*probe_string = CFRetain(matched_probe_string);
				}
			}

			result = trigger;

			/* If retry was requested, or we found Always match, trigger now */
			if (onDemandRetry || !usedOnDemandRetry) {
				if (triggerNow != NULL) {
					*triggerNow = TRUE;
				}
				break;
			}

			/* If we matched the Retry list, but Always was requested,
			 keep going through triggers in case one matches an Always */
		}
	}

	if (result) {
		CFRetain(result);
	}

	_SCErrorSet(sc_status);
	return result;
}


static CFDictionaryRef
__SCNetworkConnectionCopyTriggerWithService(CFDictionaryRef	configuration,
					    CFStringRef		service_id)
{
	CFArrayRef	triggers;
	uint64_t	triggersCount	= 0;
	int		triggersIndex;

	triggers = CFDictionaryGetValue(configuration, kSCNetworkConnectionOnDemandTriggers);
	triggersCount = isA_CFArray(triggers) ? CFArrayGetCount(triggers) : 0;
	for (triggersIndex = 0; triggersIndex < triggersCount; triggersIndex++) {
		CFDictionaryRef	trigger;
		CFStringRef	trigger_service_id;

		trigger = CFArrayGetValueAtIndex(triggers, triggersIndex);
		if (!isA_CFDictionary(trigger)) {
			// if not a valid "OnDemand" configuration
			continue;
		}

		trigger_service_id = CFDictionaryGetValue(trigger, kSCNetworkConnectionOnDemandServiceID);
		if (isA_CFString(trigger_service_id) && CFEqual(trigger_service_id, service_id)) {
			CFRetain(trigger);
			return trigger;
		}
	}

	return NULL;
}


__private_extern__ CFDictionaryRef
__SCNetworkConnectionCopyTokenParameters(SCNetworkConnectionRef connection)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	CFDictionaryRef			parameters		= NULL;
	uint8_t     			params_buffer[PPP_MACH_MAX_INLINE_DATA];
	uint32_t    			params_buffer_len	= sizeof(params_buffer);
	int				sc_status		= kSCStatusOK;
	mach_port_t			session_port;
	kern_return_t			status;

	pthread_mutex_lock(&connectionPrivate->lock);

	parameters = connectionPrivate->flow_divert_token_params;
	if (parameters != NULL) {
	    CFRetain(parameters);
	    goto done;
	}

retry:
	if (parameters != NULL) {
		CFRelease(parameters);
		parameters = NULL;
	}

	session_port = __SCNetworkConnectionSessionPort(connectionPrivate);
	if (session_port == MACH_PORT_NULL) {
		goto done;
	}

	status = pppcontroller_flow_divert_copy_token_parameters(session_port, params_buffer, &params_buffer_len);
	if (status == KERN_SUCCESS) {
		if (params_buffer_len > 0) {
			CFDataRef params_data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault,
									    params_buffer,
									    params_buffer_len,
									    kCFAllocatorNull);
			parameters = CFPropertyListCreateWithData(kCFAllocatorDefault,
								  params_data,
								  kCFPropertyListImmutable,
								  NULL,
								  NULL);
			CFRelease(params_data);
		}
	}

	if (__SCNetworkConnectionNeedsRetry(connection, "__SCNetworkConnectionCopyTokenParameters()", status, &sc_status)) {
		goto retry;
	}

	if (sc_status != kSCStatusOK) {
		_SCErrorSet(sc_status);
	}

done:
	if (parameters != NULL && connectionPrivate->flow_divert_token_params == NULL) {
	    connectionPrivate->flow_divert_token_params = (CFDictionaryRef)CFRetain(parameters);
	}

	pthread_mutex_unlock(&connectionPrivate->lock);

	return parameters;
}

Boolean
__SCNetworkConnectionCopyOnDemandInfoWithName(SCDynamicStoreRef		*storeP,
					      CFStringRef		hostName,
					      Boolean			onDemandRetry,
					      CFStringRef		*connectionServiceID,
					      SCNetworkConnectionStatus	*connectionStatus,
					      CFStringRef		*vpnRemoteAddress)	/*  CFDictionaryRef *info */
{
	CFDictionaryRef		configuration;
	Boolean			ok		= FALSE;
	int			sc_status	= kSCStatusOK;
	CFDictionaryRef		trigger;
	Boolean			trigger_now	= FALSE;

	configuration = __SCNetworkConnectionCopyOnDemandConfiguration();
	if (configuration == NULL) {
		_SCErrorSet(sc_status);
		return ok;
	}

	trigger = __SCNetworkConnectionCopyMatchingTriggerWithName(configuration, hostName, 0, onDemandRetry, NULL, &trigger_now, NULL);
	if (trigger != NULL && trigger_now) {
		CFNumberRef			num;
		SCNetworkConnectionStatus	onDemandStatus	= kSCNetworkConnectionDisconnected;

		ok = TRUE;

		if (!CFDictionaryGetValueIfPresent(trigger, kSCNetworkConnectionOnDemandStatus, (const void **)&num) ||
		    !isA_CFNumber(num) ||
		    !CFNumberGetValue(num, kCFNumberSInt32Type, &onDemandStatus)) {
			onDemandStatus = kSCNetworkConnectionDisconnected;
		}
		if (connectionStatus != NULL) {
			*connectionStatus = onDemandStatus;
		}

		if (connectionServiceID != NULL) {
			*connectionServiceID = CFDictionaryGetValue(trigger, kSCNetworkConnectionOnDemandServiceID);
			*connectionServiceID = isA_CFString(*connectionServiceID);
			if ((*connectionServiceID != NULL) && (CFStringGetLength(*connectionServiceID) > 0)) {
				CFRetain(*connectionServiceID);
			} else {
				SCLog(TRUE, LOG_INFO, CFSTR("OnDemand%s configuration error, no serviceID"),
				      onDemandRetry ? " (on retry)" : "");
				*connectionServiceID = NULL;
				ok = FALSE;
			}
		}

		if (vpnRemoteAddress != NULL) {
			*vpnRemoteAddress = CFDictionaryGetValue(trigger, kSCNetworkConnectionOnDemandRemoteAddress);
			*vpnRemoteAddress = isA_CFString(*vpnRemoteAddress);
			if ((*vpnRemoteAddress != NULL) && (CFStringGetLength(*vpnRemoteAddress) > 0)) {
				CFRetain(*vpnRemoteAddress);
			} else {
				SCLog(TRUE, LOG_INFO, CFSTR("OnDemand%s configuration error, no server address"),
				      onDemandRetry ? " (on retry)" : "");
				*vpnRemoteAddress = NULL;
				ok = FALSE;
			}
		}

		if (!ok) {
			if ((connectionServiceID != NULL) && (*connectionServiceID != NULL)) {
				CFRelease(*connectionServiceID);
				*connectionServiceID = NULL;
			}
			if ((vpnRemoteAddress != NULL) && (*vpnRemoteAddress != NULL)) {
				CFRelease(*vpnRemoteAddress);
				*vpnRemoteAddress = NULL;
			}
			sc_status = kSCStatusFailed;
		} else {
			if (_sc_debug || (debug > 0)) {
				SCLog(TRUE, LOG_INFO, CFSTR("OnDemand%s match, connection status = %d"),
				      onDemandRetry ? " (on retry)" : "",
				      onDemandStatus);
			}
		}
	}

	if (trigger) {
		CFRelease(trigger);
	}

//	if (_sc_debug || (debug > 0)) {
//		SCLog(TRUE, LOG_INFO, CFSTR("OnDemand domain name(s) not matched"));
//	}

	if (configuration != NULL) CFRelease(configuration);
	if (!ok) {
		_SCErrorSet(sc_status);
	}
	return ok;
}

static Boolean
__SCNetworkConnectionCopyUserPreferencesInternal(CFDictionaryRef	selectionOptions,
						 CFStringRef		*serviceID,
						 CFDictionaryRef	*userOptions)
{
	int		prefsChanged	= 1;
	int		status;
	Boolean		success		= FALSE;

	if (notify_userprefs_token == -1) {
		status = notify_register_check(k_NetworkConnect_Notification, &notify_userprefs_token);
		if (status != NOTIFY_STATUS_OK) {
			SCLog(TRUE, LOG_ERR, CFSTR("notify_register_check() failed, status=%d"), status);
			(void)notify_cancel(notify_userprefs_token);
			notify_userprefs_token = -1;
		} else {
			// clear the "something has changed" state
			(void) notify_check(notify_userprefs_token, &prefsChanged);
			prefsChanged = 1;
		}
	}
	if (notify_userprefs_token != -1) {
		status = notify_check(notify_userprefs_token, &prefsChanged);
		if (status != NOTIFY_STATUS_OK) {
			SCLog(TRUE, LOG_ERR, CFSTR("notify_check() failed, status=%d"), status);
			(void)notify_cancel(notify_userprefs_token);
			notify_userprefs_token = -1;
		}
	}


	*serviceID = NULL;
	*userOptions = NULL;

	if (selectionOptions != NULL) {
		Boolean		catchAllFound	= FALSE;
		CFIndex		catchAllService	= 0;
		CFIndex		catchAllConfig	= 0;
		CFStringRef	hostName	= NULL;
		CFStringRef	priority	= NULL;
		CFArrayRef	serviceNames	= NULL;
		CFDictionaryRef	services	= NULL;
		CFIndex		serviceIndex;
		CFIndex		servicesCount;

		hostName = CFDictionaryGetValue(selectionOptions, kSCNetworkConnectionSelectionOptionOnDemandHostName);
		if (hostName == NULL) {
			hostName = CFDictionaryGetValue(selectionOptions, kSCPropNetPPPOnDemandHostName);
		}
		hostName = isA_CFString(hostName);
		if (hostName == NULL)
			goto done_selection;	// if no hostname for matching

		priority = CFDictionaryGetValue(selectionOptions, kSCPropNetPPPOnDemandPriority);
		if (!isA_CFString(priority))
			priority = kSCValNetPPPOnDemandPriorityDefault;


		if (!isA_CFArray(serviceNames))
			goto done_selection;


		if (!isA_CFDictionary(services)) {
			goto done_selection;
		}

		servicesCount = CFArrayGetCount(serviceNames);
		for (serviceIndex = 0; serviceIndex < servicesCount; serviceIndex++) {
			CFIndex		configIndex;
			CFIndex		configsCount;
			CFArrayRef	serviceConfigs;
			CFStringRef	serviceName;
			int		val;

			serviceName = CFArrayGetValueAtIndex(serviceNames, serviceIndex);
			if (!isA_CFString(serviceName)) {
				continue;
			}

			serviceConfigs = CFDictionaryGetValue(services, serviceName);
			if (!isA_CFArray(serviceConfigs)) {
				continue;
			}

			configsCount = CFArrayGetCount(serviceConfigs);
			for (configIndex = 0; configIndex < configsCount; configIndex++) {
				CFNumberRef	autodial;
				CFDictionaryRef config;
				CFDictionaryRef pppConfig;

				config = CFArrayGetValueAtIndex(serviceConfigs, configIndex);
				if (!isA_CFDictionary(config)) {
					continue;
				}

				pppConfig = CFDictionaryGetValue(config, kSCEntNetPPP);
				if (!isA_CFDictionary(pppConfig)) {
					continue;
				}

				autodial = CFDictionaryGetValue(pppConfig, kSCPropNetPPPOnDemandEnabled);
				if (!isA_CFNumber(autodial)) {
					continue;
				}

				CFNumberGetValue(autodial, kCFNumberIntType, &val);
				if (val) {
					CFArrayRef	domains;
					CFIndex		domainsCount;
					CFIndex		domainsIndex;

					/* we found an conditional connection enabled configuration */

					/* check domain */
					domains = CFDictionaryGetValue(pppConfig, kSCPropNetPPPOnDemandDomains);
					if (!isA_CFArray(domains)) {
						continue;
					}

					domainsCount = CFArrayGetCount(domains);
					for (domainsIndex = 0; domainsIndex < domainsCount; domainsIndex++) {
						CFStringRef	domain;

						domain = CFArrayGetValueAtIndex(domains, domainsIndex);
						if (!isA_CFString(domain)) {
							continue;
						}

						if (!catchAllFound &&
						    (CFStringCompare(domain, CFSTR(""), 0) == kCFCompareEqualTo
							|| CFStringCompare(domain, CFSTR("."), 0) == kCFCompareEqualTo))
						{
							// found a catch all
							catchAllFound = TRUE;
							catchAllService = serviceIndex;
							catchAllConfig = configIndex;
						}

						if (_SC_domainEndsWithDomain(hostName, domain)) {
							// found matching configuration
							*serviceID = serviceName;
							CFRetain(*serviceID);
							*userOptions = CFDictionaryCreateMutableCopy(NULL, 0, config);
							CFDictionarySetValue((CFMutableDictionaryRef)*userOptions, kSCNetworkConnectionSelectionOptionOnDemandHostName, hostName);
							CFDictionarySetValue((CFMutableDictionaryRef)*userOptions, kSCPropNetPPPOnDemandPriority, priority);
							addPasswordFromKeychain(*serviceID, userOptions);
							success = TRUE;
							goto done_selection;
						}
					}
				}
			}
		}

		// config not found, do we have a catchall ?
		if (catchAllFound) {
			CFDictionaryRef config;
			CFArrayRef	serviceConfigs;
			CFStringRef	serviceName;

			serviceName = CFArrayGetValueAtIndex(serviceNames, catchAllService);
			serviceConfigs = CFDictionaryGetValue(services, serviceName);
			config = CFArrayGetValueAtIndex(serviceConfigs, catchAllConfig);

			*serviceID = serviceName;
			CFRetain(*serviceID);
			*userOptions = CFDictionaryCreateMutableCopy(NULL, 0, config);
			CFDictionarySetValue((CFMutableDictionaryRef)*userOptions, kSCNetworkConnectionSelectionOptionOnDemandHostName, hostName);
			CFDictionarySetValue((CFMutableDictionaryRef)*userOptions, kSCPropNetPPPOnDemandPriority, priority);
			addPasswordFromKeychain(*serviceID, userOptions);
			success = TRUE;
			goto done_selection;
		}

	    done_selection:

		if (serviceNames) {
			CFRelease(serviceNames);
		}
		if (services) {
			CFRelease(services);
		}

		if (debug > 1) {
			SCLog(TRUE, LOG_DEBUG, CFSTR("SCNetworkConnectionCopyUserPreferences %@"), success ? CFSTR("succeeded") : CFSTR("failed"));
			SCLog(TRUE, LOG_DEBUG, CFSTR("Selection options: %@"), selectionOptions);
		}

		return success;
	}

	/* we don't have selection options */

	// (1) Figure out which service ID we care about, allocate it into passed "serviceID"
	success = SCNetworkConnectionPrivateCopyDefaultServiceIDForDial(serviceID);

	if (success && (*serviceID != NULL)) {
		// (2) Get the list of user data for this service ID
		CFPropertyListRef	userServices	= NULL;


		// (3) We are expecting an array if the user has defined records for this service ID or NULL if the user hasn't
		if (userServices != NULL) {
			if (isA_CFArray(userServices)) {
				// (4) Get the default set of user options for this service
				success = SCNetworkConnectionPrivateCopyDefaultUserOptionsFromArray((CFArrayRef)userServices,
												    userOptions);
				if(success && (userOptions != NULL)) {
					addPasswordFromKeychain(*serviceID, userOptions);
				}
			} else {
				SCLog(TRUE, LOG_DEBUG, CFSTR("Error, userServices are not of type CFArray!"));
			}

			CFRelease(userServices); // this is OK because SCNetworkConnectionPrivateISExpectedCFType() checks for NULL
		}
	}

	if (debug > 1) {
		SCLog(TRUE, LOG_DEBUG, CFSTR("SCNetworkConnectionCopyUserPreferences %@, no selection options"), success ? CFSTR("succeeded") : CFSTR("failed"));
	}

	return success;
}


Boolean
SCNetworkConnectionCopyUserPreferences(CFDictionaryRef	selectionOptions,
				       CFStringRef	*serviceID,
				       CFDictionaryRef	*userOptions)
{
	Boolean	success	= FALSE;


	/* initialize runtime */
	pthread_once(&initialized, __SCNetworkConnectionInitialize);

	/* first check for new VPN OnDemand style */
	if (selectionOptions != NULL) {
		CFStringRef	hostName;

		hostName = CFDictionaryGetValue(selectionOptions, kSCNetworkConnectionSelectionOptionOnDemandHostName);
		if (isA_CFString(hostName)) {
			CFStringRef			connectionServiceID	= NULL;
			SCNetworkConnectionStatus	connectionStatus	= kSCNetworkConnectionInvalid;
			Boolean				onDemandRetry;
			CFTypeRef			val;

			val = CFDictionaryGetValue(selectionOptions, kSCNetworkConnectionSelectionOptionOnDemandRetry);
			onDemandRetry = isA_CFBoolean(val) ? CFBooleanGetValue(val) : TRUE;

			success = __SCNetworkConnectionCopyOnDemandInfoWithName(NULL,
										hostName,
										onDemandRetry,
										&connectionServiceID,
										&connectionStatus,
										NULL);
			if (debug > 1) {
				SCLog(TRUE, LOG_DEBUG,
				      CFSTR("SCNetworkConnectionCopyUserPreferences __SCNetworkConnectionCopyOnDemandInfoWithName returns %d w/status %d"),
				      success,
				      connectionStatus);
			}

			if (success) {
				// if the hostname matches an OnDemand domain
				if (connectionStatus == kSCNetworkConnectionConnected) {
					// if we are already connected
					if (connectionServiceID != NULL) {
						CFRelease(connectionServiceID);
					}
					return FALSE;
				}

				*serviceID   = connectionServiceID;
				*userOptions = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
				CFDictionarySetValue((CFMutableDictionaryRef)*userOptions, kSCNetworkConnectionSelectionOptionOnDemandHostName, hostName);
				return TRUE;
			} else if (!onDemandRetry) {
				// if the hostname does not match an OnDemand domain and we have
				// not yet issued an initial DNS query (i.e. it's not a query
				// being retried after the VPN has been established) then we're
				// done
				return FALSE;
			}
		}
	}

	return __SCNetworkConnectionCopyUserPreferencesInternal(selectionOptions, serviceID, userOptions);
}


Boolean
SCNetworkConnectionOnDemandShouldRetryOnFailure (SCNetworkConnectionRef connection)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	CFDictionaryRef			match_info		= NULL;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		goto fail;
	}

	if (connectionPrivate->service == NULL) {
		_SCErrorSet(kSCStatusConnectionNoService);
		goto fail;
	}

	if (isA_CFDictionary(connectionPrivate->on_demand_user_options)) {
		match_info = CFDictionaryGetValue(connectionPrivate->on_demand_user_options, kSCNetworkConnectionSelectionOptionOnDemandMatchInfo);
		if (isA_CFDictionary(match_info)) {
			CFBooleanRef onRetry = CFDictionaryGetValue(match_info, kSCNetworkConnectionOnDemandMatchInfoOnRetry);
			if (isA_CFBoolean(onRetry)) {
				return CFBooleanGetValue(onRetry);
			}
		}
	}

    fail:
	return FALSE;
}


// Mask is optional in routes dictionary; if not present, whole addresses are matched
Boolean
__SCNetworkConnectionIPv4AddressMatchesRoutes (struct sockaddr_in *addr_in, CFDictionaryRef routes)
{
	CFIndex		count;
	CFIndex		i;
	CFDataRef	maskData	= NULL;
	struct in_addr	*maskDataArray;
	CFDataRef	routeaddrData	= NULL;
	struct in_addr	*routeaddrDataArray;

	if (!isA_CFDictionary(routes)) {
		return FALSE;
	}

	routeaddrData = CFDictionaryGetValue(routes, kSCNetworkConnectionNetworkInfoAddresses);
	maskData = CFDictionaryGetValue(routes, kSCNetworkConnectionNetworkInfoMasks);

	/* routeaddrData and maskData are packed arrays of addresses; make sure they have the same length */
	if (!isA_CFData(routeaddrData) || (maskData && (!isA_CFData(maskData) || CFDataGetLength(routeaddrData) != CFDataGetLength(maskData)))) {
		return FALSE;
	}

	routeaddrDataArray = (struct in_addr*)(void*)CFDataGetBytePtr(routeaddrData);
	if (maskData) {
		maskDataArray = (struct in_addr*)(void*)CFDataGetBytePtr(maskData);
	}

	count = CFDataGetLength(routeaddrData) / sizeof(struct in_addr);
	for (i=0; i<count; i++) {
		struct in_addr	routeAddr	= *routeaddrDataArray;

		if (maskData) {
			struct in_addr	mask	= *maskDataArray;

			if ((addr_in->sin_addr.s_addr & mask.s_addr) == (routeAddr.s_addr & mask.s_addr)) {
				return TRUE;
			}
			maskDataArray++;
		} else {
			if (addr_in->sin_addr.s_addr == routeAddr.s_addr) {
				return TRUE;
			}
		}
		routeaddrDataArray++;
	}
	return FALSE;
}


void
__SCNetworkConnectionMaskIPv6Address(struct in6_addr *addr, struct in6_addr *mask)
{
	int	i;

	for (i = 0; i < sizeof(struct in6_addr); i++)
		addr->s6_addr[i] &= mask->s6_addr[i];
}


// Mask is optional in routes dictionary; if not present, whole addresses are matched
Boolean
__SCNetworkConnectionIPv6AddressMatchesRoutes (struct sockaddr_in6 *addr_in6, CFDictionaryRef routes)
{
	CFIndex		count;
	CFIndex		i;
	CFDataRef	maskData	= NULL;
	struct in6_addr	*maskDataArray;
	CFDataRef	routeaddrData	= NULL;
	struct in6_addr	*routeaddrDataArray;

	if (!isA_CFDictionary(routes)) {
		return FALSE;
	}

	routeaddrData = CFDictionaryGetValue(routes, kSCNetworkConnectionNetworkInfoAddresses);
	maskData = CFDictionaryGetValue(routes, kSCNetworkConnectionNetworkInfoMasks);

	/* routeaddrData and maskData are packed arrays of addresses; make sure they have the same length */
	if (!isA_CFData(routeaddrData) || (maskData && (!isA_CFData(maskData) || CFDataGetLength(routeaddrData) != CFDataGetLength(maskData)))) {
		return FALSE;
	}

	routeaddrDataArray = (struct in6_addr*)(void*)CFDataGetBytePtr(routeaddrData);
	if (maskData) {
		maskDataArray = (struct in6_addr*)(void*)CFDataGetBytePtr(maskData);
	}

	count = CFDataGetLength(routeaddrData) / sizeof(struct in6_addr);
	for (i=0; i<count; i++) {
		if (maskData) {
			struct in6_addr	cmpAddr;
			struct in6_addr	*mask = maskDataArray;
			struct in6_addr	routeAddr;

			memcpy(&routeAddr, routeaddrDataArray, sizeof(routeAddr));
			memcpy(&cmpAddr, &addr_in6->sin6_addr, sizeof(cmpAddr));
			__SCNetworkConnectionMaskIPv6Address(&routeAddr, mask);
			__SCNetworkConnectionMaskIPv6Address(&cmpAddr, mask);
			maskDataArray++;
			if (!memcmp(&routeAddr, &cmpAddr, sizeof(routeAddr))) {
				return TRUE;
			}
		} else {
			if (!memcmp(routeaddrDataArray, &addr_in6->sin6_addr, sizeof(struct in6_addr))) {
				return TRUE;
			}
		}

		routeaddrDataArray++;
	}
	return FALSE;
}


static Boolean
__SCNetworkConnectionAddressMatchesRedirectedDNS(CFDictionaryRef trigger, const struct sockaddr *input_addr)
{
	CFBooleanRef redirectedRef = CFDictionaryGetValue(trigger, kSCNetworkConnectionOnDemandDNSRedirectDetected);

	if (isA_CFBoolean(redirectedRef) && CFBooleanGetValue(redirectedRef)) {
		/* DNS is redirected. Look for address list. */
		CFDictionaryRef redirectedAddressesRef = CFDictionaryGetValue(trigger, kSCNetworkConnectionOnDemandDNSRedirectedAddresses);

		if (isA_CFDictionary(redirectedAddressesRef)) {
			if (input_addr->sa_family == AF_INET) {
				return __SCNetworkConnectionIPv4AddressMatchesRoutes((struct sockaddr_in*)(void*)input_addr, CFDictionaryGetValue(redirectedAddressesRef, kSCNetworkConnectionNetworkInfoIPv4));
			} else if (input_addr->sa_family == AF_INET6) {
				return __SCNetworkConnectionIPv6AddressMatchesRoutes((struct sockaddr_in6*)(void*)input_addr, CFDictionaryGetValue(redirectedAddressesRef, kSCNetworkConnectionNetworkInfoIPv6));
			}
		}
	}

	return FALSE;
}

/* If the required probe has failed, we need to tunnel the address. Equivalent to redirected DNS. */
static Boolean
__SCNetworkConnectionRequiredProbeFailed (CFDictionaryRef trigger, CFStringRef probeString)
{
	CFDictionaryRef probeResults = NULL;

	if (!isA_CFString(probeString)) {
		return FALSE;
	}

	probeResults = CFDictionaryGetValue(trigger, kSCNetworkConnectionOnDemandProbeResults);
	if (!isA_CFDictionary(probeResults)) {
		return TRUE;
	}

	CFBooleanRef result = CFDictionaryGetValue(probeResults, probeString);

	/* Only a value of kCFBooleanFalse marks the probe as failed  */
	return (isA_CFBoolean(result) && !CFBooleanGetValue(result));
}

Boolean
SCNetworkConnectionCanTunnelAddress (SCNetworkConnectionRef connection, const struct sockaddr *address, Boolean *startImmediately)
{
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	CFStringRef			serviceID		= NULL;
	CFDictionaryRef			configuration		= NULL;
	CFDictionaryRef			trigger			= NULL;
	CFDictionaryRef			tunneledNetworks	= NULL;
	sa_family_t			address_family		= AF_UNSPEC;
	Boolean				success			= FALSE;

	if (startImmediately) {
		*startImmediately = FALSE;
	}

	if (address == NULL) {
		goto done;
	}

	address_family = address->sa_family;
	if (address_family != AF_INET && address_family != AF_INET6) {
		goto done;
	}

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		goto done;
	}

	if (connectionPrivate->service == NULL) {
		_SCErrorSet(kSCStatusConnectionNoService);
		goto done;
	}

	serviceID = SCNetworkServiceGetServiceID(connectionPrivate->service);
	if (!isA_CFString(serviceID)) {
		goto done;
	}

	configuration = __SCNetworkConnectionCopyOnDemandConfiguration();
	if (configuration == NULL) {
		goto done;
	}

	trigger = __SCNetworkConnectionCopyTriggerWithService(configuration, serviceID);
	if (trigger == NULL) {
		goto done;
	}

	if (__SCNetworkConnectionRequiredProbeFailed(trigger, connectionPrivate->on_demand_required_probe)) {
		/* If probe failed, we can't trust DNS - connect now */
		if (startImmediately) {
			*startImmediately = TRUE;
		}
		success = TRUE;
		goto done;
	}

	if (__SCNetworkConnectionAddressMatchesRedirectedDNS(trigger, address)) {
		if (startImmediately) {
			*startImmediately = TRUE;
		}
		success = TRUE;
		goto done;
	}

	tunneledNetworks = CFDictionaryGetValue(trigger, kSCNetworkConnectionOnDemandTunneledNetworks);
	if (!isA_CFDictionary(tunneledNetworks)) {
		goto done;
	}

	if (address_family == AF_INET) {
		CFDictionaryRef ip_dict;
		Boolean matches = FALSE;
		struct sockaddr_in *addr_in = (struct sockaddr_in *)(void*)address;

		ip_dict = CFDictionaryGetValue(tunneledNetworks, kSCNetworkConnectionNetworkInfoIPv4);
		if (!isA_CFDictionary(ip_dict)) {
			goto done;
		}

		matches = __SCNetworkConnectionIPv4AddressMatchesRoutes(addr_in, CFDictionaryGetValue(ip_dict, kSCNetworkConnectionNetworkInfoIncludedRoutes));

		if (matches) {
			if (!__SCNetworkConnectionIPv4AddressMatchesRoutes(addr_in, CFDictionaryGetValue(ip_dict, kSCNetworkConnectionNetworkInfoExcludedRoutes))) {
				success = TRUE;
				goto done;
			}
		}
	} else {
		CFDictionaryRef ip6_dict;
		Boolean matches = FALSE;
		struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)(void*)address;

		ip6_dict = CFDictionaryGetValue(tunneledNetworks, kSCNetworkConnectionNetworkInfoIPv6);
		if (!isA_CFDictionary(ip6_dict)) {
			goto done;
		}

		matches = __SCNetworkConnectionIPv6AddressMatchesRoutes(addr_in6, CFDictionaryGetValue(ip6_dict, kSCNetworkConnectionNetworkInfoIncludedRoutes));

		if (matches) {
			if (!__SCNetworkConnectionIPv6AddressMatchesRoutes(addr_in6, CFDictionaryGetValue(ip6_dict, kSCNetworkConnectionNetworkInfoExcludedRoutes))) {
				success = TRUE;
				goto done;
			}
		}
	}
done:
	if (configuration) {
		CFRelease(configuration);
	}
	if (trigger) {
		CFRelease(trigger);
	}
	return success;
}

Boolean
SCNetworkConnectionSelectServiceWithOptions(SCNetworkConnectionRef connection, CFDictionaryRef selectionOptions)
{
	CFStringRef			account_identifier	= NULL;
	CFDictionaryRef			configuration		= NULL;
	SCNetworkConnectionPrivateRef	connectionPrivate	= (SCNetworkConnectionPrivateRef)connection;
	CFDictionaryRef			found_trigger		= NULL;
	CFStringRef			host_name		= NULL;
	Boolean				is_retry		= TRUE;
	CFDictionaryRef			match_info		= NULL;
	CFMutableDictionaryRef		new_user_options	= NULL;
	SCNetworkConnectionStatus	on_demand_status	= kSCNetworkConnectionInvalid;
	CFStringRef			requiredProbe		= NULL;
	CFStringRef			service_id		= NULL;
	Boolean				skip_prefs		= FALSE;
	Boolean				success			= TRUE;
	CFDictionaryRef			user_options		= NULL;

	if (!isA_SCNetworkConnection(connection)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		success = FALSE;
		goto done;
	}

	/* Can't call this on a connection that is already associated with a service */
	if (connectionPrivate->service != NULL) {
		_SCErrorSet(kSCStatusInvalidArgument);
		success = FALSE;
		goto done;
	}

	if (isA_CFDictionary(selectionOptions)) {
		CFBooleanRef no_user_prefs = CFDictionaryGetValue(selectionOptions, kSCNetworkConnectionSelectionOptionNoUserPrefs);
		CFBooleanRef retry = CFDictionaryGetValue(selectionOptions, kSCNetworkConnectionSelectionOptionOnDemandRetry);

		account_identifier = CFDictionaryGetValue(selectionOptions, kSCNetworkConnectionSelectionOptionOnDemandAccountIdentifier);
		host_name = CFDictionaryGetValue(selectionOptions, kSCNetworkConnectionSelectionOptionOnDemandHostName);
		skip_prefs = (isA_CFBoolean(no_user_prefs) && CFBooleanGetValue(no_user_prefs));

		if (isA_CFBoolean(retry)) {
			is_retry = CFBooleanGetValue(retry);
		}
	}

	configuration = __SCNetworkConnectionCopyOnDemandConfiguration();

	/* First, check for a match with the App Layer rules */
	service_id = VPNAppLayerCopyMatchingService(connectionPrivate->client_audit_token,
						    connectionPrivate->client_pid,
						    connectionPrivate->client_uuid,
						    connectionPrivate->client_bundle_id,
						    host_name,
						    account_identifier,
						    &match_info);
	if (service_id != NULL) {
		Boolean	use_app_layer	= TRUE;

		if (isA_CFDictionary(configuration)) {
			found_trigger = __SCNetworkConnectionCopyTriggerWithService(configuration, service_id);
			if (found_trigger != NULL) {
				CFNumberRef status_num = CFDictionaryGetValue(found_trigger, kSCNetworkConnectionOnDemandStatus);
				if (isA_CFNumber(status_num)) {
					CFNumberGetValue(status_num, kCFNumberIntType, &on_demand_status);
				}
				/*
				 * If the trigger should be ignored, still use App Layer VPN if it is already connected or
				 * is in the process of connecting.
				 */
				if (__SCNetworkConnectionShouldIgnoreTrigger(found_trigger) &&
				    on_demand_status != kSCNetworkConnectionConnecting &&
				    on_demand_status != kSCNetworkConnectionConnected)
				{
					use_app_layer = FALSE;
				}
			}
		}

		if (use_app_layer) {
			/* If this is not the 'OnRetry' call, and the service has not yet started, the match may need to return false */
			if (!is_retry &&
			    match_info != NULL &&
			    on_demand_status != kSCNetworkConnectionConnecting &&
			    on_demand_status != kSCNetworkConnectionConnected) {
				CFBooleanRef matchedOnRetry = CFDictionaryGetValue(match_info, kSCNetworkConnectionOnDemandMatchInfoOnRetry);
				if (matchedOnRetry && CFBooleanGetValue(matchedOnRetry)) {
					/* Don't return that we matched always; wait for SCNetworkConnectionOnDemandShouldRetryOnFailure */
					success = FALSE;
				}
			}
			connectionPrivate->type = kSCNetworkConnectionTypeAppLayerVPN;
			goto search_done;
		} else {
			CFRelease(service_id);
			service_id = NULL;
			if (match_info != NULL) {
				CFRelease(match_info);
				match_info = NULL;
			}
			if (found_trigger != NULL) {
				CFRelease(found_trigger);
				found_trigger = NULL;
			}
		}
	}

	/* Next, check the IP layer rules */
	if (isA_CFDictionary(configuration) && host_name != NULL) {
		Boolean	triggerNow	= FALSE;

		found_trigger = __SCNetworkConnectionCopyMatchingTriggerWithName(configuration, host_name, connectionPrivate->client_pid, is_retry, &match_info, &triggerNow, &requiredProbe);
		if (found_trigger != NULL) {
			service_id = CFDictionaryGetValue(found_trigger, kSCNetworkConnectionOnDemandServiceID);
			if (isA_CFString(service_id)) {
				CFRetain(service_id);
				connectionPrivate->type = kSCNetworkConnectionTypeIPLayerVPN;
			} else {
				service_id = NULL;
			}
			if (!triggerNow) {
				success = FALSE;
			}
			goto search_done;
		} else if (!is_retry) {
			goto search_done;
		}

		if (match_info != NULL) {
			CFRelease(match_info);
			match_info = NULL;
		}
	}

	/* Next, check the user preferences */
	if (!skip_prefs && __SCNetworkConnectionCopyUserPreferencesInternal(selectionOptions, &service_id, &user_options)) {
		CFMutableDictionaryRef	minfo;
		CFNumberRef		type_num;

		if (isA_CFDictionary(configuration)) {
			found_trigger = __SCNetworkConnectionCopyTriggerWithService(configuration, service_id);
		}
		connectionPrivate->type = kSCNetworkConnectionTypePPP;

		minfo = CFDictionaryCreateMutable(NULL,
						  0,
						  &kCFTypeDictionaryKeyCallBacks,
						  &kCFTypeDictionaryValueCallBacks);
		type_num = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &connectionPrivate->type);
		CFDictionarySetValue(minfo, kSCNetworkConnectionOnDemandMatchInfoVPNType, type_num);
		CFRelease(type_num);
		match_info = minfo;
		goto search_done;
	}

    search_done:
	if (service_id == NULL) {
		_SCErrorSet(kSCStatusOK);
		success = FALSE;
		goto done;
	}

	connectionPrivate->service = _SCNetworkServiceCopyActive(NULL, service_id);
	if (connectionPrivate->service == NULL) {
		_SCErrorSet(kSCStatusOK);
		success = FALSE;
		goto done;
	}

	if (found_trigger != NULL) {
		if (connectionPrivate->on_demand_info) {
		      CFRelease(connectionPrivate->on_demand_info);
		}
		connectionPrivate->on_demand_info = found_trigger;
		CFRetain(connectionPrivate->on_demand_info);

		if (on_demand_status == kSCNetworkConnectionInvalid) {
			CFNumberRef status_num = CFDictionaryGetValue(found_trigger, kSCNetworkConnectionOnDemandStatus);
			if (isA_CFNumber(status_num)) {
				CFNumberGetValue(status_num, kCFNumberIntType, &on_demand_status);
			}
		}

		if (on_demand_status != kSCNetworkConnectionConnected) {
			if (connectionPrivate->type == kSCNetworkConnectionTypeAppLayerVPN) {
				/* Check App Layer OnDemand flag */
				CFBooleanRef app_on_demand_enabled =
					CFDictionaryGetValue(found_trigger, kSCNetworkConnectionOnDemandMatchAppEnabled);
				if (isA_CFBoolean(app_on_demand_enabled) && CFBooleanGetValue(app_on_demand_enabled)) {
					connectionPrivate->on_demand = TRUE;
				}
			} else {
				connectionPrivate->on_demand = TRUE;
			}
		}
	} else if (connectionPrivate->type == kSCNetworkConnectionTypePPP) {
		/* If we got the service from __SCNetworkConnectionCopyUserPreferencesInternal, then it's on demand */
		connectionPrivate->on_demand = TRUE;
	}

	if (user_options == NULL) {
		new_user_options = CFDictionaryCreateMutable(kCFAllocatorDefault,
							     0,
							     &kCFTypeDictionaryKeyCallBacks,
							     &kCFTypeDictionaryValueCallBacks);
	} else {
		new_user_options = CFDictionaryCreateMutableCopy(kCFAllocatorDefault, 0, user_options);
	}

	if (host_name != NULL) {
		CFDictionarySetValue(new_user_options, kSCNetworkConnectionSelectionOptionOnDemandHostName, host_name);
	}

	if (connectionPrivate->on_demand && match_info != NULL) {
		CFDictionarySetValue(new_user_options, kSCNetworkConnectionSelectionOptionOnDemandMatchInfo, match_info);
	}

	connectionPrivate->on_demand_user_options = new_user_options;
	CFRetain(connectionPrivate->on_demand_user_options);

	if (requiredProbe) {
		connectionPrivate->on_demand_required_probe = requiredProbe;
		CFRetain(connectionPrivate->on_demand_required_probe);
	}

done:
	if (service_id != NULL) {
		CFRelease(service_id);
	}

	if (configuration != NULL) {
		CFRelease(configuration);
	}

	if (found_trigger != NULL) {
		CFRelease(found_trigger);
	}

	if (user_options != NULL) {
		CFRelease(user_options);
	}

	if (new_user_options != NULL) {
		CFRelease(new_user_options);
	}

	if (match_info != NULL) {
		CFRelease(match_info);
	}

	if (requiredProbe != NULL) {
		CFRelease(requiredProbe);
	}

	return success;
}

//*******************************************************************************************
// SCNetworkConnectionPrivateCopyDefaultServiceIDForDial
// ----------------------------------------------------
// Try to find the service id to connect
// (1) Start by looking at the last service used in Network Pref / Network menu extra
// (2) If Network Pref / Network menu extra has not been used, find the PPP service
//     with the highest ordering
//********************************************************************************************
static Boolean
SCNetworkConnectionPrivateCopyDefaultServiceIDForDial(CFStringRef *serviceID)
{
	Boolean			foundService		= FALSE;
	CFPropertyListRef	lastServiceSelectedInIC = NULL;



	// we found the service the user last had open in IC
	if (lastServiceSelectedInIC != NULL) {
		// make sure its a PPP service
		if (SCNetworkConnectionPrivateIsPPPService(lastServiceSelectedInIC, kSCValNetInterfaceSubTypePPPSerial, kSCValNetInterfaceSubTypePPPoE)) {
			// make sure the service that we found is valid
			CFDictionaryRef	dict;
			CFStringRef	key;

			key = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
									  kSCDynamicStoreDomainSetup,
									  lastServiceSelectedInIC,
									  kSCEntNetInterface);
			dict = SCDynamicStoreCopyValue(NULL, key);
			CFRelease(key);
			if (dict != NULL) {
				CFRelease(dict);
				*serviceID = CFRetain(lastServiceSelectedInIC);
				foundService = TRUE;
			}
		}
		CFRelease(lastServiceSelectedInIC);
	}

	if (!foundService) {
		foundService = SCNetworkConnectionPrivateGetPPPServiceFromDynamicStore(serviceID);
	}

	return foundService;
}

//********************************************************************************
// SCNetworkConnectionPrivateGetPPPServiceFromDynamicStore
// -------------------------------------------------------
// Find the highest ordered PPP service in the dynamic store
//********************************************************************************
static Boolean
SCNetworkConnectionPrivateGetPPPServiceFromDynamicStore(CFStringRef *serviceID)
{
	CFDictionaryRef	dict		= NULL;
	CFStringRef	key		= NULL;
	CFArrayRef	serviceIDs	= NULL;
	Boolean		success		= FALSE;

	*serviceID = NULL;

	do {
		CFIndex count;
		CFIndex i;

		key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL, kSCDynamicStoreDomainSetup, kSCEntNetIPv4);
		if (key == NULL) {
			fprintf(stderr, "Error, Setup Key == NULL!\n");
			break;
		}

		dict = SCDynamicStoreCopyValue(NULL, key);
		if (!isA_CFDictionary(dict)) {
			fprintf(stderr, "no global IPv4 entity\n");
			break;
		}

		serviceIDs = CFDictionaryGetValue(dict, kSCPropNetServiceOrder); // array of service id's
		if (!isA_CFArray(serviceIDs)) {
			fprintf(stderr, "service order not specified\n");
			break;
		}

		count = CFArrayGetCount(serviceIDs);
		for (i = 0; i < count; i++) {
			CFStringRef service = CFArrayGetValueAtIndex(serviceIDs, i);

			if (SCNetworkConnectionPrivateIsPPPService(service, kSCValNetInterfaceSubTypePPPSerial, kSCValNetInterfaceSubTypePPPoE)) {
				*serviceID = CFRetain(service);
				success = TRUE;
				break;
			}
		}
	} while (FALSE);

	if (key != NULL)	CFRelease(key);
	if (dict != NULL)	CFRelease(dict);

	return success;
}

//********************************************************************************
// SCNetworkConnectionPrivateCopyDefaultUserOptionsFromArray
// ---------------------------------------------------------
// Copy over user preferences for a particular service if they exist
//********************************************************************************
static Boolean
SCNetworkConnectionPrivateCopyDefaultUserOptionsFromArray(CFArrayRef userOptionsArray, CFDictionaryRef *userOptions)
{
	CFIndex	count	= CFArrayGetCount(userOptionsArray);
	int	i;

	for (i = 0; i < count; i++) {
		// (1) Find the dictionary
		CFPropertyListRef propertyList = CFArrayGetValueAtIndex(userOptionsArray, i);

		if (isA_CFDictionary(propertyList) != NULL) {
			// See if there's a value for dial on demand
			CFPropertyListRef value;

			value = CFDictionaryGetValue((CFDictionaryRef)propertyList, k_Dial_Default_Key);
			if (isA_CFBoolean(value) != NULL) {
				if (CFBooleanGetValue(value)) {
					// we found the default user options
					*userOptions = CFDictionaryCreateCopy(NULL,
									      (CFDictionaryRef)propertyList);
					break;
				}
			}
		}
	}

	return TRUE;
}

//********************************************************************************
// SCNetworkConnectionPrivateIsServiceType
// --------------------------------------
// Check and see if the service is a PPP service of the given types
//********************************************************************************
static Boolean
SCNetworkConnectionPrivateIsPPPService(CFStringRef serviceID, CFStringRef subType1, CFStringRef subType2)
{
	CFStringRef	entityKey;
	Boolean		isPPPService		= FALSE;
	Boolean		isMatchingSubType	= FALSE;
	CFDictionaryRef	serviceDict;

	entityKey = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL,
								kSCDynamicStoreDomainSetup,
								serviceID,
								kSCEntNetInterface);
	if (entityKey == NULL) {
		return FALSE;
	}

	serviceDict = SCDynamicStoreCopyValue(NULL, entityKey);
	if (serviceDict != NULL) {
		if (isA_CFDictionary(serviceDict)) {
			CFStringRef	type;
			CFStringRef	subtype;

			type = CFDictionaryGetValue(serviceDict, kSCPropNetInterfaceType);
			if (isA_CFString(type)) {
				isPPPService = CFEqual(type, kSCValNetInterfaceTypePPP);
			}

			subtype = CFDictionaryGetValue(serviceDict, kSCPropNetInterfaceSubType);
			if (isA_CFString(subtype)) {
				isMatchingSubType = CFEqual(subtype, subType1);
				if (!isMatchingSubType && subType2)
					isMatchingSubType = CFEqual(subtype, subType2);
			}
		}
		CFRelease(serviceDict);
	}
	CFRelease(entityKey);

	return (isPPPService && isMatchingSubType);
}

//********************************************************************************
// addPasswordFromKeychain
// --------------------------------------
// Get the password and shared secret out of the keychain and add
// them to the PPP and IPSec dictionaries
//********************************************************************************
static void
addPasswordFromKeychain(CFStringRef serviceID, CFDictionaryRef *userOptions)
{
	CFPropertyListRef	uniqueID;
	CFStringRef		password;
	CFStringRef		sharedsecret	= NULL;

	/* user options must exist */
	if (*userOptions == NULL)
		return;

	/* first, get the unique identifier used to store passwords in the keychain */
	uniqueID = CFDictionaryGetValue(*userOptions, k_Unique_Id_Key);
	if (!isA_CFString(uniqueID))
		return;

	/* first, get the PPP password */
	password = copyPasswordFromKeychain(uniqueID);

	/* then, if necessary, get the IPSec Shared Secret */
	if (SCNetworkConnectionPrivateIsPPPService(serviceID, kSCValNetInterfaceSubTypeL2TP, 0)) {
		CFMutableStringRef	uniqueIDSS;

		uniqueIDSS = CFStringCreateMutableCopy(NULL, 0, uniqueID);
		CFStringAppend(uniqueIDSS, CFSTR(".SS"));
		sharedsecret = copyPasswordFromKeychain(uniqueIDSS);
		CFRelease(uniqueIDSS);
	}

	/* did we find our information in the key chain ? */
	if ((password != NULL) || (sharedsecret != NULL)) {
		CFMutableDictionaryRef	newOptions;

		newOptions = CFDictionaryCreateMutableCopy(NULL, 0, *userOptions);

		/* PPP password */
		if (password != NULL) {
			CFDictionaryRef		entity;
			CFMutableDictionaryRef	newEntity;

			entity = CFDictionaryGetValue(*userOptions, kSCEntNetPPP);
			if (isA_CFDictionary(entity))
				newEntity = CFDictionaryCreateMutableCopy(NULL, 0, entity);
			else
				newEntity = CFDictionaryCreateMutable(NULL,
								      0,
								      &kCFTypeDictionaryKeyCallBacks,
								      &kCFTypeDictionaryValueCallBacks);


			/* set the PPP password */
			CFDictionarySetValue(newEntity, kSCPropNetPPPAuthPassword, uniqueID);
			CFDictionarySetValue(newEntity, kSCPropNetPPPAuthPasswordEncryption, kSCValNetPPPAuthPasswordEncryptionKeychain);
			CFRelease(password);

			/* update the PPP entity */
			CFDictionarySetValue(newOptions, kSCEntNetPPP, newEntity);
			CFRelease(newEntity);
		}

		/* IPSec Shared Secret */
		if (sharedsecret != NULL) {
			CFDictionaryRef		entity;
			CFMutableDictionaryRef	newEntity;

			entity = CFDictionaryGetValue(*userOptions, kSCEntNetIPSec);
			if (isA_CFDictionary(entity))
				newEntity = CFDictionaryCreateMutableCopy(NULL, 0, entity);
			else
				newEntity = CFDictionaryCreateMutable(NULL,
								      0,
								      &kCFTypeDictionaryKeyCallBacks,
								      &kCFTypeDictionaryValueCallBacks);

			/* set the IPSec Shared Secret */
			CFDictionarySetValue(newEntity, kSCPropNetIPSecSharedSecret, sharedsecret);
			CFRelease(sharedsecret);

			/* update the IPSec entity */
			CFDictionarySetValue(newOptions, kSCEntNetIPSec, newEntity);
			CFRelease(newEntity);
		}

		/* update the userOptions dictionary */
		CFRelease(*userOptions);
		*userOptions = CFDictionaryCreateCopy(NULL, newOptions);
		CFRelease(newOptions);
	}

}

#if	!TARGET_OS_IPHONE
//********************************************************************************
// copyKeychainEnumerator
// --------------------------------------
// Gather Keychain Enumerator
//********************************************************************************
static CFArrayRef
copyKeychainEnumerator(CFStringRef uniqueIdentifier)
{
	CFArrayRef		itemArray	= NULL;
	CFMutableDictionaryRef	query;
	OSStatus		result;

	query = CFDictionaryCreateMutable(NULL,
					  0,
					  &kCFTypeDictionaryKeyCallBacks,
					  &kCFTypeDictionaryValueCallBacks);
	CFDictionarySetValue(query, kSecClass      , kSecClassGenericPassword);
	CFDictionarySetValue(query, kSecAttrService, uniqueIdentifier);
	CFDictionarySetValue(query, kSecReturnRef  , kCFBooleanTrue);
	CFDictionarySetValue(query, kSecMatchLimit , kSecMatchLimitAll);
	result = SecItemCopyMatching(query, (CFTypeRef *)&itemArray);
	CFRelease(query);
	if ((result != noErr) && (itemArray != NULL)) {
		CFRelease(itemArray);
		itemArray = NULL;
	}

	return itemArray;
}
#endif	// !TARGET_OS_IPHONE

//********************************************************************************
// copyPasswordFromKeychain
// --------------------------------------
// Given a uniqueID, retrieve the password from the keychain
//********************************************************************************
static CFStringRef
copyPasswordFromKeychain(CFStringRef uniqueID)
{
#if	!TARGET_OS_IPHONE
	CFArrayRef	enumerator;
	CFIndex		n;
	CFStringRef	password = NULL;

	enumerator = copyKeychainEnumerator(uniqueID);
	if (enumerator == NULL) {
		return NULL;		// if no keychain enumerator
	}

	n = CFArrayGetCount(enumerator);
	if (n > 0) {
		void			*data	= NULL;
		UInt32			dataLen	= 0;
		SecKeychainItemRef	itemRef;
		OSStatus		result;

		itemRef = (SecKeychainItemRef)CFArrayGetValueAtIndex(enumerator, 0);
		result = SecKeychainItemCopyContent(itemRef,		// itemRef
						    NULL,		// itemClass
						    NULL,		// attrList
						    &dataLen,		// length
						    (void *)&data);	// outData
		if ((result == noErr) && (data != NULL) && (dataLen > 0)) {
			password = CFStringCreateWithBytes(NULL, data, dataLen, kCFStringEncodingUTF8, TRUE);
			(void) SecKeychainItemFreeContent(NULL, data);
		}

	}

	CFRelease(enumerator);

	return password;
#else	// !TARGET_OS_IPHONE
	return NULL;
#endif	// !TARGET_OS_IPHONE
}


__private_extern__
char *
__SCNetworkConnectionGetControllerPortName(void)
{
#if	((__MAC_OS_X_VERSION_MIN_REQUIRED >= 1090) || (__IPHONE_OS_VERSION_MIN_REQUIRED >= 70000)) && !TARGET_IPHONE_SIMULATOR
	if (scnc_server_name == NULL){
		if (!(sandbox_check(getpid(), "mach-lookup", SANDBOX_FILTER_GLOBAL_NAME | SANDBOX_CHECK_NO_REPORT, PPPCONTROLLER_SERVER_PRIV))){
			scnc_server_name = PPPCONTROLLER_SERVER_PRIV;
		}
		else{
			scnc_server_name = PPPCONTROLLER_SERVER;
		}
		SCLog(TRUE, LOG_DEBUG, CFSTR("__SCNetworkConnectionGetControllerPortName() returns port: %s"), scnc_server_name);
	}
#else
	scnc_server_name = PPPCONTROLLER_SERVER;
#endif
	return scnc_server_name;
}

