/*
 * Copyright (c) 2010-2014 Apple Inc. All rights reserved.
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
 * March 1, 2010			Christophe Allie <callie@apple.com>
 * - initial revision
 * February 8, 2011			Kevin Wells <kcw@apple.com>
 * - added "select" command
 * January 2012				Kevin Wells <kcw@apple.com>
 * - added arguments to "start" command to pass authentication credentials
 * - "show" now takes a service name as an alternative to a service ID
 * - fixes a bug whereby "IPv4" was being displayed as a subtype to IPsec services
 * - improved format of "list" output
 * - general cleanup of error messages and some variable names
 */


#include "scutil.h"
#include "nc.h"
#include "prefs.h"

#include <SystemConfiguration/VPNConfiguration.h>

#include <sys/time.h>

CFStringRef			username	= NULL;
CFStringRef			password	= NULL;
CFStringRef			sharedsecret	= NULL;

static	Boolean			ondemandwatch	= FALSE;
static	CFStringRef		ondemand_nodename = NULL;

static	SCNetworkConnectionRef	connection	= NULL;
static	int			n_callback	= 0;


/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
static void
my_CFRelease(void *t)
{
	void * * obj = (void * *)t;
	if (obj && *obj) {
		CFRelease(*obj);
		*obj = NULL;
	}
	return;
}

/* -----------------------------------------------------------------------------
 ----------------------------------------------------------------------------- */
static void
nc_get_service_type_and_subtype(SCNetworkServiceRef service, CFStringRef *iftype, CFStringRef *ifsubtype) {
	SCNetworkInterfaceRef interface = SCNetworkServiceGetInterface(service);
	SCNetworkInterfaceRef child = SCNetworkInterfaceGetInterface(interface);

	*iftype = SCNetworkInterfaceGetInterfaceType(interface);
	*ifsubtype = NULL;
	if (CFEqual(*iftype, kSCNetworkInterfaceTypePPP) ||
	    CFEqual(*iftype, kSCNetworkInterfaceTypeVPN)) {
	    *ifsubtype = (child != NULL) ? SCNetworkInterfaceGetInterfaceType(child) : NULL;
	}
}

/* -----------------------------------------------------------------------------
 ----------------------------------------------------------------------------- */
static SCNetworkServiceRef
nc_copy_service(SCNetworkSetRef set, CFStringRef identifier)
{
	CFIndex			i;
	CFIndex			n;
	SCNetworkServiceRef	selected	= NULL;
	CFArrayRef		services;

	services = SCNetworkConnectionCopyAvailableServices(set);
	if (services == NULL) {
		goto done;
	}

	n = CFArrayGetCount(services);

	// try to select the service by its serviceID
	for (i = 0; i < n; i++) {
		SCNetworkServiceRef	service		= NULL;
		CFStringRef		serviceID;

		service = CFArrayGetValueAtIndex(services, i);
		serviceID = SCNetworkServiceGetServiceID(service);
		if (CFEqual(identifier, serviceID)) {
			selected = service;
			goto done;
		}
	}

	// try to select the service by service name
	for (i = 0; i < n; i++) {
		SCNetworkServiceRef	service		= NULL;
		CFStringRef		serviceName;

		service = CFArrayGetValueAtIndex(services, i);
		serviceName = SCNetworkServiceGetName(service);
		if ((serviceName != NULL) && CFEqual(identifier, serviceName)) {
			if (selected == NULL) {
				selected = service;
			} else {
				// if multiple services match
				selected = NULL;
				SCPrint(TRUE, stderr, CFSTR("Multiple services match\n"));
				goto done;
			}
		}
	}

done :

	if (selected != NULL) CFRetain(selected);
	if (services != NULL) CFRelease(services);
	return selected;
}

/* -----------------------------------------------------------------------------
 ----------------------------------------------------------------------------- */
static SCNetworkServiceRef
nc_copy_service_from_arguments(int argc, char **argv, SCNetworkSetRef set) {
	CFStringRef		serviceID	= NULL;
	SCNetworkServiceRef	service		= NULL;

	if (argc == 0) {
		serviceID = _copyStringFromSTDIN(CFSTR("Service"), NULL);
	} else {
		serviceID = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
	}
	if (serviceID == NULL) {
		SCPrint(TRUE, stderr, CFSTR("No service ID specified\n"));
		return NULL;
	}
	service = nc_copy_service(set, serviceID);
	my_CFRelease(&serviceID);
	return service;
}


/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
static char *
nc_status_string(SCNetworkConnectionStatus status)
{
	switch (status) {
		case kSCNetworkConnectionInvalid:
			return "Invalid";
		case kSCNetworkConnectionDisconnected:
			return "Disconnected";
		case kSCNetworkConnectionConnecting:
			return "Connecting";
		case kSCNetworkConnectionConnected:
			return "Connected";
		case kSCNetworkConnectionDisconnecting:
			return "Disconnecting";
	}
	return "Unknown";
}

static void
nc_callback(SCNetworkConnectionRef connection, SCNetworkConnectionStatus status, void *info)
{
	int		*n		= (int *)info;
	CFDictionaryRef	status_dict;

	// report status
	if (n != NULL) {
		if (*n == 0) {
			SCPrint(TRUE, stdout, CFSTR("Current status = "));
		} else {
			struct tm	tm_now;
			struct timeval	tv_now;

			(void)gettimeofday(&tv_now, NULL);
			(void)localtime_r(&tv_now.tv_sec, &tm_now);

			SCPrint(TRUE, stdout, CFSTR("\n*** %2d:%02d:%02d.%03d\n\n"),
				tm_now.tm_hour,
				tm_now.tm_min,
				tm_now.tm_sec,
				tv_now.tv_usec / 1000);
			SCPrint(TRUE, stdout, CFSTR("Callback (%d) status = "), *n);
		}
		*n = *n + 1;
	}
	SCPrint(TRUE, stdout, CFSTR("%s%s%s\n"),
		nc_status_string(status),
		(status == kSCNetworkConnectionInvalid) ? ": "                     : "",
		(status == kSCNetworkConnectionInvalid) ? SCErrorString(SCError()) : "");

	// report extended status
	status_dict = SCNetworkConnectionCopyExtendedStatus(connection);
	if (status_dict) {
		SCPrint(TRUE, stdout, CFSTR("Extended Status %@\n"), status_dict);
		CFRelease(status_dict);
	}

	return;
}

static void
nc_create_connection(int argc, char **argv, Boolean exit_on_failure)
{
	SCNetworkConnectionContext	context	= { 0, &n_callback, NULL, NULL, NULL };
	SCNetworkServiceRef		service;

	service = nc_copy_service_from_arguments(argc, argv, NULL);
	if (service == NULL) {
		SCPrint(TRUE, stderr, CFSTR("No service\n"));
		if (exit_on_failure)
			exit(1);
		return;
	}

	connection = SCNetworkConnectionCreateWithService(NULL, service, nc_callback, &context);
	CFRelease(service);
	if (connection == NULL) {
		SCPrint(TRUE, stderr, CFSTR("Could not create connection: %s\n"), SCErrorString(SCError()));
		if (exit_on_failure)
			exit(1);
		return;
	}
}

/* -----------------------------------------------------------------------------
 ----------------------------------------------------------------------------- */

static void
nc_trigger(int argc, char **argv)
{
	Boolean		background	= FALSE;
	int		i;
	CFStringRef	hostName	= NULL;
	int		port		= 80;

	for (i = 0; i < 3 && i < argc; i++) {
		/* Parse host name. Must be first arg. */
		if (i == 0) {
			hostName = CFStringCreateWithCString(NULL, argv[i], kCFStringEncodingUTF8);
			continue;
		}

		/* Check for optional background flag */
		if (strcmp(argv[i], "background") == 0) {
			background = TRUE;
			continue;
		}

		/* Parse optional port number */
		CFStringRef str = CFStringCreateWithCString(NULL, argv[i], kCFStringEncodingUTF8);
		if (str) {
			int num = CFStringGetIntValue(str);
			if (num) {
				port = num;
			}
			my_CFRelease(&str);
		}
	}

	if (hostName) {
		CFReadStreamRef		readStream	= NULL;
		CFWriteStreamRef	writeStream	= NULL;

		CFStreamCreatePairWithSocketToHost(kCFAllocatorDefault, hostName, port, &readStream, &writeStream);

		if (background) {
			CFReadStreamSetProperty(readStream, CFSTR("kCFStreamNetworkServiceType"), CFSTR("kCFStreamNetworkServiceTypeBackground"));
			CFWriteStreamSetProperty(writeStream, CFSTR("kCFStreamNetworkServiceType"), CFSTR("kCFStreamNetworkServiceTypeBackground"));
		}

		if (readStream && writeStream) {
			CFReadStreamOpen(readStream);
			CFWriteStreamOpen(writeStream);
			SCPrint(TRUE, stdout, CFSTR("Opened stream to %@, port %d%s\n"), hostName, port, background ? ", background traffic class" : "");
			sleep(1);
		}

		my_CFRelease(&readStream);
		my_CFRelease(&writeStream);
	} else {
		SCPrint(TRUE, stderr, CFSTR("Invalid or missing host name\n"));
	}

	my_CFRelease(&hostName);

	exit(0);
}

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
static void
nc_release_connection()
{
	my_CFRelease(&connection);
}

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
static void
nc_start(int argc, char **argv)
{
	CFMutableDictionaryRef		userOptions = NULL;
	CFStringRef			iftype = NULL;
	CFStringRef			ifsubtype = NULL;
	SCNetworkServiceRef		service = NULL;

	nc_create_connection(argc, argv, TRUE);

	service = SCNetworkConnectionGetService(connection);
	nc_get_service_type_and_subtype(service, &iftype, &ifsubtype);

	userOptions = CFDictionaryCreateMutable(NULL, 0,
						&kCFTypeDictionaryKeyCallBacks,
						&kCFTypeDictionaryValueCallBacks);

	Boolean isL2TP = (CFEqual(iftype, kSCEntNetPPP) &&
			  (ifsubtype != NULL) && CFEqual(ifsubtype, kSCValNetInterfaceSubTypeL2TP));

	if (CFEqual(iftype, kSCEntNetPPP)) {
		CFMutableDictionaryRef pppEntity  = CFDictionaryCreateMutable(NULL, 0,
									   &kCFTypeDictionaryKeyCallBacks,
									   &kCFTypeDictionaryValueCallBacks);

		if (username != NULL) {
			CFDictionarySetValue(pppEntity, kSCPropNetPPPAuthName, username);
		}
		if (password != NULL) {
			CFDictionarySetValue(pppEntity, kSCPropNetPPPAuthPassword, password);
		}
		CFDictionarySetValue(userOptions, kSCEntNetPPP, pppEntity);
		my_CFRelease(&pppEntity);
	}
	if (CFEqual(iftype, kSCEntNetIPSec) || isL2TP) {
		CFMutableDictionaryRef ipsecEntity  = CFDictionaryCreateMutable(NULL, 0,
									   &kCFTypeDictionaryKeyCallBacks,
									   &kCFTypeDictionaryValueCallBacks);
		if (!isL2TP) {
			if (username != NULL) {
				CFDictionarySetValue(ipsecEntity, kSCPropNetIPSecXAuthName, username);
			}
			if (password != NULL) {
				CFDictionarySetValue(ipsecEntity, kSCPropNetIPSecXAuthPassword, password);
			}
		}
		if (sharedsecret != NULL) {
			CFDictionarySetValue(ipsecEntity, kSCPropNetIPSecSharedSecret, sharedsecret);
		}
		CFDictionarySetValue(userOptions, kSCEntNetIPSec, ipsecEntity);
		my_CFRelease(&ipsecEntity);
	}
	if (CFEqual(iftype, kSCEntNetVPN)) {
		CFMutableDictionaryRef vpnEntity  = CFDictionaryCreateMutable(NULL, 0,
									   &kCFTypeDictionaryKeyCallBacks,
									   &kCFTypeDictionaryValueCallBacks);
		if (username != NULL) {
			CFDictionarySetValue(vpnEntity, kSCPropNetVPNAuthName, username);
		}
		if (password != NULL) {
			CFDictionarySetValue(vpnEntity, kSCPropNetVPNAuthPassword, password);
		}
		CFDictionarySetValue(userOptions, kSCEntNetVPN, vpnEntity);
		my_CFRelease(&vpnEntity);
	}
	// If it doesn't match any VPN type, fail silently

	if (!SCNetworkConnectionStart(connection, userOptions, TRUE)) {
		SCPrint(TRUE, stderr, CFSTR("Could not start connection: %s\n"), SCErrorString(SCError()));
		exit(1);
	};

	CFRelease(userOptions);
	nc_release_connection();
	exit(0);
}

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
static void
nc_stop(int argc, char **argv)
{
	nc_create_connection(argc, argv, TRUE);

	if (!SCNetworkConnectionStop(connection, TRUE)) {
		SCPrint(TRUE, stderr, CFSTR("Could not stop connection: %s\n"), SCErrorString(SCError()));
		exit(1);
	};

	nc_release_connection();
	exit(0);
}

/* -----------------------------------------------------------------------------
 ----------------------------------------------------------------------------- */
static void
nc_suspend(int argc, char **argv)
{
	nc_create_connection(argc, argv, TRUE);

	SCNetworkConnectionSuspend(connection);

	nc_release_connection();
	exit(0);
}

/* -----------------------------------------------------------------------------
 ----------------------------------------------------------------------------- */
static void
nc_resume(int argc, char **argv)
{
	nc_create_connection(argc, argv, TRUE);

	SCNetworkConnectionResume(connection);

	nc_release_connection();
	exit(0);
}

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
static void
nc_status(int argc, char **argv)
{
	SCNetworkConnectionStatus	status;

	nc_create_connection(argc, argv, TRUE);

	status = SCNetworkConnectionGetStatus(connection);
	nc_callback(connection, status, NULL);

	nc_release_connection();
	exit(0);
}

static void
nc_watch(int argc, char **argv)
{
	SCNetworkConnectionStatus	status;

	nc_create_connection(argc, argv, TRUE);

	status = SCNetworkConnectionGetStatus(connection);

	// report initial status
	n_callback = 0;
	nc_callback(connection, status, &n_callback);

	// setup watcher
	if (doDispatch) {
		if (!SCNetworkConnectionSetDispatchQueue(connection, dispatch_get_main_queue())) {
			SCPrint(TRUE, stderr, CFSTR("Unable to schedule watch process: %s\n"), SCErrorString(SCError()));
			exit(1);
		}
	} else {
		if (!SCNetworkConnectionScheduleWithRunLoop(connection, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode)) {
			SCPrint(TRUE, stderr, CFSTR("Unable to schedule watch process: %s\n"), SCErrorString(SCError()));
			exit(1);
		}
	}

	// wait for changes
	CFRunLoopRun();

	nc_release_connection();
	exit(0);
}

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
static void
nc_statistics(int argc, char **argv)
{
	CFDictionaryRef stats_dict;

	nc_create_connection(argc, argv, TRUE);

	stats_dict = SCNetworkConnectionCopyStatistics(connection);

	if (stats_dict) {
		SCPrint(TRUE, stdout, CFSTR("%@\n"), stats_dict);
	} else {
		SCPrint(TRUE, stdout, CFSTR("No statistics available\n"));
	}

	my_CFRelease(&stats_dict);

	nc_release_connection();
	exit(0);
}

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
static void
checkOnDemandHost(SCDynamicStoreRef store, CFStringRef nodeName, Boolean retry)
{
	Boolean				ok;
	CFStringRef			connectionServiceID	= NULL;
	SCNetworkConnectionStatus	connectionStatus	= 0;
	CFStringRef			vpnRemoteAddress	= NULL;

	SCPrint(TRUE, stdout, CFSTR("OnDemand host/domain check (%sretry)\n"), retry ? "" : "no ");

	ok = __SCNetworkConnectionCopyOnDemandInfoWithName(&store,
							   nodeName,
							   retry,
							   &connectionServiceID,
							   &connectionStatus,
							   &vpnRemoteAddress);

	if (ok) {
		SCPrint(TRUE, stdout, CFSTR("  serviceID      = %@\n"), connectionServiceID);
		SCPrint(TRUE, stdout, CFSTR("  remote address = %@\n"), vpnRemoteAddress);
	} else if (SCError() != kSCStatusOK) {
		SCPrint(TRUE, stdout, CFSTR("%sretry\n"), retry ? "" : "no ");
		SCPrint(TRUE, stdout,
			CFSTR("  Unable to copy OnDemand information for connection: %s\n"),
			SCErrorString(SCError()));
	} else {
		SCPrint(TRUE, stdout, CFSTR("  no match\n"));
	}

	if (connectionServiceID != NULL) {
		CFRelease(connectionServiceID);
		connectionServiceID = NULL;
	}
	if (vpnRemoteAddress != NULL) {
		CFRelease(vpnRemoteAddress);
		vpnRemoteAddress = NULL;
	}

	return;
}

static void
nc_ondemand_callback(SCDynamicStoreRef store, CFArrayRef changedKeys, void *info)
{
	CFStringRef		key		= NULL;
	CFDictionaryRef		ondemand_dict	= NULL;
	struct tm		tm_now;
	struct timeval		tv_now;

	if (CFArrayGetCount(changedKeys) < 1) {
		return;
	}

	(void)gettimeofday(&tv_now, NULL);
	(void)localtime_r(&tv_now.tv_sec, &tm_now);

	SCPrint(TRUE, stdout, CFSTR("\n*** %2d:%02d:%02d.%03d\n\n"),
		tm_now.tm_hour,
		tm_now.tm_min,
		tm_now.tm_sec,
		tv_now.tv_usec / 1000);

	if (ondemand_nodename) {
		checkOnDemandHost(store, ondemand_nodename, FALSE);
		checkOnDemandHost(store, ondemand_nodename, TRUE);
	} else {
		key = CFArrayGetValueAtIndex(changedKeys, 0);

		ondemand_dict = SCDynamicStoreCopyValue(store, key);
		if (ondemand_dict) {
			SCPrint(TRUE, stdout, CFSTR("%@ %@\n"), kSCEntNetOnDemand, ondemand_dict);
		} else {
			SCPrint(TRUE, stdout, CFSTR("%@ not configured\n"), kSCEntNetOnDemand);
		}

		my_CFRelease(&ondemand_dict);
	}
}

static void
nc_ondemand(int argc, char **argv)
{
	int			exit_code	= 1;
	CFStringRef		key		= NULL;
	CFDictionaryRef		ondemand_dict	= NULL;
	SCDynamicStoreRef	store;

	store = SCDynamicStoreCreate(NULL, CFSTR("scutil --nc"), nc_ondemand_callback, NULL);
	if (store == NULL) {
		SCPrint(TRUE, stderr, CFSTR("Unable to create dynamic store: %s\n"), SCErrorString(SCError()));
		goto done;
	}

	if (argc == 1) {
#if	!TARGET_IPHONE_SIMULATOR
		if (strcmp("--refresh", argv[0]) == 0) {
			SCNetworkConnectionRef	connection	= NULL;

			connection = SCNetworkConnectionCreate(kCFAllocatorDefault, NULL, NULL);
			if (connection && SCNetworkConnectionRefreshOnDemandState(connection)) {
				exit_code = 0;
			}

			if (exit_code) {
				SCPrint(TRUE, stderr, CFSTR("Unable to refresh OnDemand state: %s\n"), SCErrorString(SCError()));
			}

			my_CFRelease(&connection);
			goto done;
		}
#endif	// !TARGET_IPHONE_SIMULATOR

		ondemand_nodename = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
	} else if (argc != 0) {
		SCPrint(TRUE, stderr, CFSTR("Usage: scutil --nc ondemand [-W] [hostname]\n"
					    "       scutil --nc ondemand -- --refresh\n"));
		goto done;
	}

	key = SCDynamicStoreKeyCreateNetworkGlobalEntity(NULL, kSCDynamicStoreDomainState, kSCEntNetOnDemand);

	if (ondemand_nodename) {
		checkOnDemandHost(store, ondemand_nodename, FALSE);
		checkOnDemandHost(store, ondemand_nodename, TRUE);
	} else {
		ondemand_dict = SCDynamicStoreCopyValue(store, key);
		if (ondemand_dict) {
			SCPrint(TRUE, stdout, CFSTR("%@ %@\n"), kSCEntNetOnDemand, ondemand_dict);
		} else {
			SCPrint(TRUE, stdout, CFSTR("%@ not configured\n"), kSCEntNetOnDemand);
		}
	}

	if (ondemandwatch) {
		CFMutableArrayRef	keys	= NULL;

		keys = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		CFArrayAppendValue(keys, key);
		SCDynamicStoreSetNotificationKeys(store, keys, NULL);

		my_CFRelease(&keys);

		SCDynamicStoreSetDispatchQueue(store, dispatch_get_main_queue());

		CFRunLoopRun();
	}

	exit_code = 0;
done:
	my_CFRelease(&ondemand_dict);
	my_CFRelease(&key);
	my_CFRelease(&store);
	my_CFRelease(&ondemand_nodename);
	exit(exit_code);
}


/* -----------------------------------------------------------------------------
 ----------------------------------------------------------------------------- */
CFStringRef
copy_padded_string(CFStringRef original, int width, CFStringRef prefix, CFStringRef suffix)
{
	CFMutableStringRef	padded;

	padded = CFStringCreateMutable(NULL, 0);
	if (prefix != NULL) {
		CFStringAppend(padded, prefix);
	}
	if (original != NULL) {
		CFStringAppend(padded, original);
	}
	if (suffix != NULL) {
		CFStringAppend(padded, suffix);
	}
	CFStringPad(padded, CFSTR(" "), MAX(CFStringGetLength(original), width), 0);
	return padded;
}

CFStringRef
copy_VPN_status(SCNetworkServiceRef service)
{
	CFStringRef output = NULL;
	SCNetworkConnectionStatus status = kSCNetworkConnectionInvalid;
	SCNetworkConnectionRef service_connection = NULL;

	/* Only calculate status is the service is enabled. Default is invalid. */
	if (SCNetworkServiceGetEnabled(service)) {
		service_connection = SCNetworkConnectionCreateWithService(NULL, service, NULL, NULL);
		if (service_connection == NULL) goto done;
		status = SCNetworkConnectionGetStatus(service_connection);
	}

	output = CFStringCreateWithCString(NULL, nc_status_string(status), kCFStringEncodingUTF8);

done:
	my_CFRelease(&service_connection);
	return output;
}

static void
nc_print_VPN_service(SCNetworkServiceRef service)
{
	SCNetworkInterfaceRef interface = NULL;
	CFStringRef display_name = NULL;
	CFStringRef display_name_padded = NULL;
	CFStringRef service_id = NULL;
	CFStringRef service_name = NULL;
	CFStringRef service_name_padded = NULL;
	CFStringRef service_status = NULL;
	CFStringRef service_status_padded = NULL;
	CFStringRef sub_type = NULL;
	CFStringRef type = NULL;

	nc_get_service_type_and_subtype(service, &type, &sub_type);

	service_name = SCNetworkServiceGetName(service);
	service_name_padded = copy_padded_string(service_name, 32, CFSTR("\""), CFSTR("\""));

	service_id = SCNetworkServiceGetServiceID(service);

	interface = SCNetworkServiceGetInterface(service);
	display_name = SCNetworkInterfaceGetLocalizedDisplayName(interface);
	display_name_padded = copy_padded_string(display_name, 18, NULL, NULL);

	service_status = copy_VPN_status(service);
	service_status_padded = copy_padded_string(service_status, 16, CFSTR("("), CFSTR(")"));

	SCPrint(TRUE,
		stdout,
		CFSTR("%@ %@ %@ %@ %@ [%@%@%@]\n"),
		SCNetworkServiceGetEnabled(service) ? CFSTR("*") : CFSTR(" "),
		service_status_padded,
		service_id,
		display_name_padded,
		service_name_padded,
		type,
		(sub_type == NULL) ? CFSTR("") : CFSTR(":"),
		(sub_type == NULL) ? CFSTR("") : sub_type);

	CFRelease(display_name_padded);
	CFRelease(service_name_padded);
	CFRelease(service_status_padded);
	my_CFRelease(&service_status);
}


/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
static void
nc_list(int argc, char **argv)
{
	CFIndex			count;
	CFIndex			i;
	CFArrayRef		services	= NULL;

	SCPrint(TRUE, stdout, CFSTR("Available network connection services in the current set (*=enabled):\n"));
	services = SCNetworkConnectionCopyAvailableServices(NULL);
	if (services != NULL) {
		count = CFArrayGetCount(services);
		for (i = 0; i < count; i++) {
			SCNetworkServiceRef	service;

			service = CFArrayGetValueAtIndex(services, i);
			nc_print_VPN_service(service);
		}

	}
	my_CFRelease(&services);
	exit(0);
}

/* -----------------------------------------------------------------------------
 ----------------------------------------------------------------------------- */
static Boolean
nc_enable_vpntype(CFStringRef vpnType)
{
	Boolean			is_enabled = FALSE;
	Boolean			success = FALSE;

	if (vpnType == NULL) {
		SCPrint(TRUE, stderr, CFSTR("No VPN type provided\n"));
		goto done;
	}

	is_enabled = VPNConfigurationIsVPNTypeEnabled(vpnType);

	if (is_enabled) {
		SCPrint(TRUE, stdout, CFSTR("VPN is already enabled\n"));
	} else {
#if	!TARGET_OS_IPHONE
		AuthorizationRef	authorization;

		authorization = _prefs_AuthorizationCreate();
		if ((authorization == NULL) ||
		    !VPNConfigurationSetAuthorization(authorization)) {
			SCPrint(TRUE, stderr, CFSTR("VPNConfigurationSetAuthorization failed: %s\n"), SCErrorString(SCError()));
			goto done;
		}
#endif	// !TARGET_OS_IPHONE

		if (!VPNConfigurationEnableVPNType(vpnType)) {
			SCPrint(TRUE, stderr, CFSTR("VPN could not be enabled: %s\n"), SCErrorString(SCError()));
			goto done;
		}

#if	!TARGET_OS_IPHONE
		_prefs_AuthorizationFree(authorization);
#endif	// !TARGET_OS_IPHONE

		SCPrint(TRUE, stdout, CFSTR("VPN enabled\n"));
	}
	success = TRUE;

done:
	return success;
}

/* Turns a service ID or name into a vendor type, or preserves type */
static CFStringRef
nc_copy_vendor_type (CFStringRef input)
{
	SCNetworkInterfaceRef	child;
	SCNetworkInterfaceRef	interface;
	CFStringRef		output_name	= input;
	SCNetworkServiceRef	service		= NULL;
	CFStringRef		type;

	if (input == NULL) {
		goto done;
	}

	service = nc_copy_service(NULL, input);
	if (service != NULL) {
		interface = SCNetworkServiceGetInterface(service);
		child = SCNetworkInterfaceGetInterface(interface);
		type = SCNetworkInterfaceGetInterfaceType(interface);

		/* Must be of type VPN */
		if (!CFEqual(type, kSCNetworkInterfaceTypeVPN)) {
			output_name = NULL;
			goto done;
		}
		output_name = SCNetworkInterfaceGetInterfaceType(child);
		goto done;
	}

done :
	if (output_name != NULL) CFRetain(output_name);
	my_CFRelease(&service);
	return output_name;
}

/* -----------------------------------------------------------------------------
 ----------------------------------------------------------------------------- */
#if !TARGET_OS_IPHONE
static const CFStringRef PREF_PREFIX                       = CFSTR("VPN-");
static const CFStringRef PREF_SUFFIX                       = CFSTR(".plist");
static void
nc_set_application_url(CFStringRef subtype, CFStringRef directory)
{
	CFURLRef	directory_url		= NULL;
	CFDataRef	directory_url_data	= NULL;
	CFStringRef	vpnprefpath		= NULL;
	char	       *path			= NULL;
	CFIndex		path_len		= 0;

	if (subtype == NULL || directory == NULL) {
		goto done;
	}

	directory_url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
						      directory,
						      kCFURLPOSIXPathStyle,
						      FALSE);
	if (directory_url == NULL) {
		SCPrint(TRUE, stderr, CFSTR("CFURLCreateWithFileSystemPath failed\n"));
		goto done;
	}

	directory_url_data = CFURLCreateBookmarkData(NULL, directory_url, 0, 0, 0, 0);
	if (directory_url_data == NULL) {
		SCPrint(TRUE, stderr, CFSTR("CFURLCreateBookmarkData failed\n"));
		goto done;
	}

	vpnprefpath = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@%@%@"), PREF_PREFIX, subtype, PREF_SUFFIX );
	if (vpnprefpath == NULL) {
		SCPrint(TRUE, stderr, CFSTR("CFStringCreateWithFormat failed\n"));
		goto done;
	}

	path_len = CFStringGetLength(vpnprefpath) + 1;
	path = malloc(path_len);
	if (path == NULL) {
		goto done;
	}

	if (!CFStringGetCString(vpnprefpath, path, path_len, kCFStringEncodingASCII)) {
		SCPrint(TRUE, stderr, CFSTR("CFStringGetCString failed\n"));
		goto done;
	}

	do_prefs_init();		/* initialization */
	do_prefs_open(1, &path);	/* open prefs */

	if (!SCPreferencesSetValue(prefs, CFSTR("ApplicationURL"), directory_url_data)) {
		SCPrint(TRUE, stderr,
			CFSTR("SCPreferencesSetValue ApplicationURL failed, %s\n"),
			SCErrorString(SCError()));
		goto done;
	}

	_prefs_save();

done:
	my_CFRelease(&directory_url);
	my_CFRelease(&directory_url_data);
	my_CFRelease(&vpnprefpath);
	if (path) {
		free(path);
	}
	_prefs_close();

	exit(0);
}
#endif

/* -----------------------------------------------------------------------------
 ----------------------------------------------------------------------------- */
static void
nc_enablevpn(int argc, char **argv)
{
	CFStringRef		argument = NULL;
	CFStringRef		vendorType = NULL;
	int			exit_code = 1;

	if (argc == 0) {
		SCPrint(TRUE, stderr, CFSTR("No service type or ID\n"));
	} else {
		argument = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
		vendorType = nc_copy_vendor_type(argument);
		my_CFRelease(&argument);

		if (!nc_enable_vpntype(vendorType)) {
			goto done;
		}
#if !TARGET_OS_IPHONE
		if (argc >= 2) {
			argument = CFStringCreateWithCString(NULL, argv[1], kCFStringEncodingUTF8);
			nc_set_application_url(vendorType, argument);
			my_CFRelease(&argument);
		}
#endif
	}

	exit_code = 0;

done:
	my_CFRelease(&vendorType);
	exit(exit_code);
}

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
static void
nc_show(int argc, char **argv)
{
	SCNetworkServiceRef	service			= NULL;
	SCDynamicStoreRef	store			= NULL;
	int			exit_code		= 1;
	CFStringRef		serviceID		= NULL;
	CFStringRef		iftype			= NULL;
	CFStringRef		ifsubtype		= NULL;
	CFStringRef		type_entity_key		= NULL;
	CFStringRef		subtype_entity_key	= NULL;
	CFDictionaryRef		type_entity_dict	= NULL;
	CFDictionaryRef		subtype_entity_dict	= NULL;
	CFStringRef		vpnprefpath		= NULL;
#if !TARGET_OS_IPHONE
	CFDataRef		bookmarkData		= NULL;
	CFURLRef		directory		= NULL;
	Boolean			isStale			= FALSE;
	char			*path			= NULL;
	CFIndex			path_len		= 0;
#endif

	service = nc_copy_service_from_arguments(argc, argv, NULL);
	if (service == NULL) {
		SCPrint(TRUE, stderr, CFSTR("No service\n"));
		exit(exit_code);
	}

	serviceID = SCNetworkServiceGetServiceID(service);

	nc_get_service_type_and_subtype(service, &iftype, &ifsubtype);

	if (!CFEqual(iftype, kSCEntNetPPP) &&
	    !CFEqual(iftype, kSCEntNetIPSec) &&
	    !CFEqual(iftype, kSCEntNetVPN)) {
		SCPrint(TRUE, stderr, CFSTR("Not a connection oriented service: %@\n"), serviceID);
		goto done;
	}

	type_entity_key = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL, kSCDynamicStoreDomainSetup, serviceID, iftype);

	nc_print_VPN_service(service);

#if !TARGET_OS_IPHONE
	vpnprefpath = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@%@%@"), PREF_PREFIX, ifsubtype, PREF_SUFFIX);
	if (vpnprefpath == NULL) {
		goto skipURL;
	}

	path_len = CFStringGetLength(vpnprefpath) + 1;
	path = malloc(path_len);
	if (path == NULL) {
		goto skipURL;
	}

	if (!CFStringGetCString(vpnprefpath, path, path_len, kCFStringEncodingASCII)) {
		SCPrint(TRUE, stderr, CFSTR("CFStringGetCString failed\n"));
		goto done;
	}

	do_prefs_init();		/* initialization */
	do_prefs_open(1, &path);	/* open prefs */

	bookmarkData = SCPreferencesGetValue(prefs, CFSTR("ApplicationURL"));
	if (bookmarkData == NULL) {
		goto skipURL;
	}

	directory = CFURLCreateByResolvingBookmarkData(kCFAllocatorDefault, bookmarkData, 0, NULL, NULL, &isStale, NULL);
	if (directory == NULL) {
		goto skipURL;
	}

	SCPrint(TRUE, stdout, CFSTR("ApplicationURL: %@\n"), directory);
skipURL:
#endif

	store = SCDynamicStoreCreate(NULL, CFSTR("scutil --nc"), NULL, NULL);
	if (store == NULL) {
		SCPrint(TRUE, stderr, CFSTR("Unable to create dynamic store: %s\n"), SCErrorString(SCError()));
		goto done;
	}
	type_entity_dict = SCDynamicStoreCopyValue(store, type_entity_key);

	if (!type_entity_dict) {
		SCPrint(TRUE, stderr, CFSTR("No \"%@\" configuration available\n"), iftype);
	} else {
		SCPrint(TRUE, stdout, CFSTR("%@ %@\n"), iftype, type_entity_dict);
	}

	if (ifsubtype) {
		subtype_entity_key = SCDynamicStoreKeyCreateNetworkServiceEntity(NULL, kSCDynamicStoreDomainSetup, serviceID, ifsubtype);
		subtype_entity_dict = SCDynamicStoreCopyValue(store, subtype_entity_key);
		if (!subtype_entity_dict) {
			//
		}
		else {
			SCPrint(TRUE, stdout, CFSTR("%@ %@\n"), ifsubtype, subtype_entity_dict);
		}
	}

	exit_code = 0;

done:
	my_CFRelease(&type_entity_key);
	my_CFRelease(&type_entity_dict);
	my_CFRelease(&subtype_entity_key);
	my_CFRelease(&subtype_entity_dict);
	my_CFRelease(&store);
	my_CFRelease(&service);
	my_CFRelease(&vpnprefpath);
	_prefs_close();
	exit(exit_code);
}

/* -----------------------------------------------------------------------------
 ----------------------------------------------------------------------------- */
static void
nc_select(int argc, char **argv)
{
	SCNetworkSetRef		current_set;
	int			exit_code	= 1;
	SCNetworkServiceRef	service		= NULL;
	Boolean			status;

	do_prefs_init();	/* initialization */
	do_prefs_open(0, NULL);	/* open default prefs */

	current_set = SCNetworkSetCopyCurrent(prefs);
	if (current_set == NULL) {
		SCPrint(TRUE, stderr, CFSTR("No current location\n"));
		goto done;
	}

	service = nc_copy_service_from_arguments(argc, argv, current_set);
	if (service == NULL) {
		SCPrint(TRUE, stderr, CFSTR("No service\n"));
		goto done;
	}

#if !TARGET_OS_IPHONE
	status = SCNetworkServiceSetEnabled(service, TRUE);
	if (!status) {
		SCPrint(TRUE, stderr, CFSTR("Unable to enable service: %s\n"), SCErrorString(SCError()));
		goto done;
	}
#else
	status = SCNetworkSetSetSelectedVPNService(current_set, service);
	if (!status) {
		SCPrint(TRUE, stderr, CFSTR("Unable to select service: %s\n"), SCErrorString(SCError()));
		goto done;
	}
#endif

	_prefs_save();
	exit_code = 0;
done:
	my_CFRelease(&service);
	my_CFRelease(&current_set);
	_prefs_close();
	exit(exit_code);
}

/* -----------------------------------------------------------------------------
 ----------------------------------------------------------------------------- */
static void
nc_help(int argc, char **argv)
{
	SCPrint(TRUE, stderr, CFSTR("Valid commands for scutil --nc (VPN connections)\n"));
	SCPrint(TRUE, stderr, CFSTR("Usage: scutil --nc [command]\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("\tlist\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\tList available network connection services in the current set\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("\tstatus <service>\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\tIndicate whether a given service is connected, as well as extended status information for the service\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("\tshow <service>\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\tDisplay configuration information for a given service\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("\tstatistics <service>\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\tProvide statistics on bytes, packets, and errors for a given service\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("\tselect <service>\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\tMake the given service active in the current set. This allows it to be started\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("\tstart <service> [--user user] [--password password] [--secret secret]\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\tStart a given service. Can take optional arguments for user, password, and secret\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("\tstop <service>\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\tStop a given service\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("\tsuspend <service>\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\tSuspend a given service (PPP, Modem on Hold)\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("\tresume <service>\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\tResume a given service (PPP, Modem on Hold)\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("\tondemand [-W] [hostname]\n"));
	SCPrint(TRUE, stderr, CFSTR("\tondemand -- --refresh\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\tDisplay VPN on-demand information\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("\ttrigger <hostname> [background] [port]\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\tTrigger VPN on-demand with specified hostname, and optional port and background flag\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
#if !TARGET_OS_IPHONE
	SCPrint(TRUE, stderr, CFSTR("\tenablevpn <service or vpn type> [path]\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\tEnables the given VPN application type. Takes either a service or VPN type. Pass a path to set ApplicationURL\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
#else
	SCPrint(TRUE, stderr, CFSTR("\tenablevpn <service or vpn type>\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\tEnables the given VPN application type. Takes either a service or VPN type\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
#endif
	SCPrint(TRUE, stderr, CFSTR("\tdisablevpn <service or vpn type>\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\tDisables the given VPN application type. Takes either a service or VPN type\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	SCPrint(TRUE, stderr, CFSTR("\thelp\n"));
	SCPrint(TRUE, stderr, CFSTR("\t\tDisplay available commands for --nc\n"));
	SCPrint(TRUE, stderr, CFSTR("\n"));
	exit(0);
}

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
typedef void (*nc_func) (int argc, char **argv);

static const struct {
	char		*cmd;
	nc_func		func;
} nc_cmds[] = {
	{ "enablevpn",		nc_enablevpn	},
	{ "help",		nc_help		},
	{ "list",		nc_list		},
	{ "ondemand",		nc_ondemand	},
	{ "resume",		nc_resume	},
	{ "select",		nc_select	},
	{ "show",		nc_show		},
	{ "start",		nc_start	},
	{ "statistics",		nc_statistics	},
	{ "status",		nc_status	},
	{ "stop",		nc_stop		},
	{ "suspend",		nc_suspend	},
	{ "trigger",		nc_trigger	},
};
#define	N_NC_CMNDS	(sizeof(nc_cmds) / sizeof(nc_cmds[0]))


/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
int
find_nc_cmd(char *cmd)
{
	int	i;

	for (i = 0; i < (int)N_NC_CMNDS; i++) {
		if (strcmp(cmd, nc_cmds[i].cmd) == 0) {
			return i;
		}
	}

	return -1;
}


/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
void
do_nc_cmd(char *cmd, int argc, char **argv, Boolean watch)
{
	int	i;

	i = find_nc_cmd(cmd);
	if (i >= 0) {
		nc_func	func;

		func = nc_cmds[i].func;
		if (watch) {
			if (func == nc_status) {
				func = nc_watch;
			} else if (func == nc_ondemand) {
				ondemandwatch = TRUE;
			}
		}
		(*func)(argc, argv);
	}
	return;
}

