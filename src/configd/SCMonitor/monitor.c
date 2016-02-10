/*
 * Copyright (c) 2007-2014 Apple Inc. All rights reserved.
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
 * October 24, 2007		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <asl.h>
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/IOMessage.h>
#include <ApplicationServices/ApplicationServices.h>
#include "UserEventAgentInterface.h"

#define MY_BUNDLE_ID    "com.apple.SystemConfiguration.SCMonitor"
#define	MY_ICON_PATH	"/System/Library/PreferencePanes/Network.prefPane/Contents/Resources/Network.icns"

#define	NETWORK_PREF_APP	"/System/Library/PreferencePanes/Network.prefPane"
#define	NETWORK_PREF_CMD	"New Interface"

/*
 * The following keys/values control the actions taken when a new interface
 * has been detected and whether we interact with the user.
 *
 * The keys/values can be provided globally (in SCMonitor.plugin's Info.plist
 * file) or per-inteface in the IORegistry.
 *
 * For the "New Interface Detected Action" key we define the following [CFString]
 * values :
 *
 *   "None"       No action, ignore this interface.
 *   "Prompt"     Post a "new interface detected" notification to the user.
 *   "Configure"  Automatically configure this interface without any
 *                intervention.
 *
 *   Note: automatic configuration may not be possible if the logged in user
 *         is NOT "root" (eUID==0) or if the authorization right that governs
 *         SCHelper write operations (kSCPreferencesAuthorizationRight_write)
 *         is not currently available.
 *
 * An [older] "User Intervention" key is also supported.  That CFBoolean
 * key, if present and TRUE, implies "Configure" configuration of the
 * interface without intervention.
 */

typedef struct {
	UserEventAgentInterfaceStruct	*_UserEventAgentInterface;
	CFUUIDRef			_factoryID;
	UInt32				_refCount;

	Boolean				debug;

	asl_object_t			log_msg;

	CFStringRef			configuration_action;

	CFRunLoopSourceRef		monitorRls;

	IONotificationPortRef		notifyPort;
	io_iterator_t			notifyIterator;
	CFMutableArrayRef		notifyNodes;

	// interfaces that we already know about
	CFMutableSetRef			interfaces_known;

	// interfaces that should be auto-configured (no user notification)
	CFMutableArrayRef		interfaces_configure;

	// interfaces that require user notification
	CFMutableArrayRef		interfaces_prompt;

	CFUserNotificationRef		userNotification;
	CFRunLoopSourceRef		userRls;

	AuthorizationRef		authorization;
} MyType;

static CFMutableDictionaryRef	notify_to_instance	= NULL;


#pragma mark -
#pragma mark Authorization


static AuthorizationRef
getAuthorization(MyType *myInstance)
{
	if (myInstance->authorization == NULL) {
		AuthorizationFlags	flags	= kAuthorizationFlagDefaults;
		OSStatus		status;

		status = AuthorizationCreate(NULL,
					     kAuthorizationEmptyEnvironment,
					     flags,
					     &myInstance->authorization);
		if (status != errAuthorizationSuccess) {
			SCLOG(NULL, myInstance->log_msg, ASL_LEVEL_ERR,
			      CFSTR("AuthorizationCreate() failed: status = %d"),
			      (int)status);
		}
	}

	return myInstance->authorization;
}


static Boolean
hasAuthorization(MyType *myInstance)
{
	AuthorizationRef	authorization;
	Boolean			isAdmin		= FALSE;

	authorization = getAuthorization(myInstance);
	if (authorization != NULL) {
		AuthorizationFlags	flags	= kAuthorizationFlagDefaults;
		AuthorizationItem	items[1];
		AuthorizationRights	rights;
		OSStatus		status;

		items[0].name        = kSCPreferencesAuthorizationRight_write;
		items[0].value       = NULL;
		items[0].valueLength = 0;
		items[0].flags       = 0;

		rights.count = sizeof(items) / sizeof(items[0]);
		rights.items = items;

		status = AuthorizationCopyRights(authorization,
						 &rights,
						 kAuthorizationEmptyEnvironment,
						 flags,
						 NULL);
		isAdmin = (status == errAuthorizationSuccess);
	}

	return isAdmin;
}


static void
freeAuthorization(MyType *myInstance)
{
	if (myInstance->authorization != NULL) {
		AuthorizationFree(myInstance->authorization, kAuthorizationFlagDefaults);
//              AuthorizationFree(myInstance->authorization, kAuthorizationFlagDestroyRights);
		myInstance->authorization = NULL;
	}

	return;
}


#pragma mark -
#pragma mark New interface notification / configuration


static void
open_NetworkPrefPane(MyType *myInstance)
{
	AEDesc		aeDesc	= { typeNull, NULL };
	CFArrayRef	prefArray;
	CFURLRef	prefURL;
	LSLaunchURLSpec	prefSpec;
	OSStatus	status;

	prefURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
						CFSTR(NETWORK_PREF_APP),
						kCFURLPOSIXPathStyle,
						FALSE);
	prefArray = CFArrayCreate(NULL, (const void **)&prefURL, 1, &kCFTypeArrayCallBacks);
	CFRelease(prefURL);

	status = AECreateDesc('ptru',
			      (const void *)NETWORK_PREF_CMD,
			      strlen(NETWORK_PREF_CMD),
			      &aeDesc);
	if (status != noErr) {
		SCLOG(NULL, myInstance->log_msg, ASL_LEVEL_ERR, CFSTR("SCMonitor: AECreateDesc() failed: %d"), (int)status);
	}

	prefSpec.appURL		= NULL;
	prefSpec.itemURLs	= prefArray;
	prefSpec.passThruParams	= &aeDesc;
	prefSpec.launchFlags	= kLSLaunchAsync | kLSLaunchDontAddToRecents;
	prefSpec.asyncRefCon	= NULL;

	status = LSOpenFromURLSpec(&prefSpec, NULL);
	if (status != noErr) {
		SCLOG(NULL, myInstance->log_msg, ASL_LEVEL_ERR, CFSTR("SCMonitor: LSOpenFromURLSpec() failed: %d"), (int)status);
	}

	CFRelease(prefArray);
	if (aeDesc.descriptorType != typeNull) AEDisposeDesc(&aeDesc);
	return;
}


static void
notify_remove(MyType *myInstance, Boolean cancel)
{
	if (myInstance->interfaces_configure != NULL) {
		CFRelease(myInstance->interfaces_configure);
		myInstance->interfaces_configure = NULL;
	}

	if (myInstance->interfaces_prompt != NULL) {
		CFRelease(myInstance->interfaces_prompt);
		myInstance->interfaces_prompt = NULL;
	}

	if (myInstance->userRls != NULL) {
		CFRunLoopSourceInvalidate(myInstance->userRls);
		CFRelease(myInstance->userRls);
		myInstance->userRls = NULL;
	}

	if (myInstance->userNotification != NULL) {
		if (cancel) {
			SInt32	status;

			status = CFUserNotificationCancel(myInstance->userNotification);
			if (status != 0) {
				SCLOG(NULL, myInstance->log_msg, ASL_LEVEL_ERR,
				      CFSTR("SCMonitor: CFUserNotificationCancel() failed, status=%d"),
				      (int)status);
			}
		}
		CFRelease(myInstance->userNotification);
		myInstance->userNotification = NULL;
	}

	return;
}


static void
notify_reply(CFUserNotificationRef userNotification, CFOptionFlags response_flags)
{
	MyType	*myInstance	= NULL;

	// get instance for notification
	if (notify_to_instance != NULL) {
		myInstance = (MyType *)CFDictionaryGetValue(notify_to_instance, userNotification);
		if (myInstance != NULL) {
			CFDictionaryRemoveValue(notify_to_instance, userNotification);
			if (CFDictionaryGetCount(notify_to_instance) == 0) {
				CFRelease(notify_to_instance);
				notify_to_instance = NULL;
			}
		}
	}
	if (myInstance == NULL) {
		SCLOG(NULL, NULL, ASL_LEVEL_ERR, CFSTR("SCMonitor: can't find user notification"));
		return;
	}

	// process response
	switch (response_flags & 0x3) {
		case kCFUserNotificationDefaultResponse:
			// user asked to configure interface
			open_NetworkPrefPane(myInstance);
			break;
		default:
			// user cancelled
			break;
	}

	notify_remove(myInstance, FALSE);
	return;
}


static void
notify_add(MyType *myInstance)
{
	CFBundleRef		bundle;
	CFMutableDictionaryRef	dict	= NULL;
	SInt32			error	= 0;
	CFIndex			i;
	CFIndex			n	= CFArrayGetCount(myInstance->interfaces_prompt);
	CFURLRef		url	= NULL;

	if (myInstance->userNotification != NULL) {
		CFMutableArrayRef	save	= NULL;

		if (n > 0) {
			CFRetain(myInstance->interfaces_prompt);
			save = myInstance->interfaces_prompt;
		}
		notify_remove(myInstance, TRUE);
		myInstance->interfaces_prompt = save;
		if (n == 0) {
			return;
		}
	}

	dict = CFDictionaryCreateMutable(NULL,
					 0,
					 &kCFTypeDictionaryKeyCallBacks,
					 &kCFTypeDictionaryValueCallBacks);

	// set localization URL
	bundle = CFBundleGetBundleWithIdentifier(CFSTR(MY_BUNDLE_ID));
	if (bundle != NULL) {
		url = CFBundleCopyBundleURL(bundle);
	}
#ifdef	MAIN
	if (url == NULL) {
		url = CFURLCreateFromFileSystemRepresentation(NULL,
							      (const UInt8 *)"/System/Library/UserEventPlugins/SCMonitor.plugin",
							      strlen("/System/Library/UserEventPlugins/SCMonitor.plugin"),
							      FALSE);
		if (bundle == NULL) {
			bundle = CFBundleCreate(NULL, url);
		}
	}
#endif	// MAIN
	if (url != NULL) {
		// set URL
		CFDictionarySetValue(dict, kCFUserNotificationLocalizationURLKey, url);
		CFRelease(url);
	} else {
		SCLOG(NULL, myInstance->log_msg, ASL_LEVEL_ERR, CFSTR("SCMonitor: can't find bundle"));
		goto done;
	}

	// set icon URL
	url = CFURLCreateFromFileSystemRepresentation(NULL,
						      (const UInt8 *)MY_ICON_PATH,
						      strlen(MY_ICON_PATH),
						      FALSE);
	if (url != NULL) {
		CFDictionarySetValue(dict, kCFUserNotificationIconURLKey, url);
		CFRelease(url);
	}

	// header
	CFDictionarySetValue(dict,
			     kCFUserNotificationAlertHeaderKey,
			     (n == 1) ? CFSTR("HEADER_1") : CFSTR("HEADER_N"));

	// message
	if (n == 1) {
		CFStringRef		format;
		SCNetworkInterfaceRef	interface;
		CFStringRef		message;
		CFStringRef		name;

#define MESSAGE_1 "The \"%@\" network interface has not been set up. To set up this interface, use Network Preferences."

		format = CFBundleCopyLocalizedString(bundle,
						     CFSTR("MESSAGE_1"),
						     CFSTR(MESSAGE_1),
						     NULL);
		interface = CFArrayGetValueAtIndex(myInstance->interfaces_prompt, 0);
		name = SCNetworkInterfaceGetLocalizedDisplayName(interface);
		message = CFStringCreateWithFormat(NULL, NULL, format, name);
		CFDictionarySetValue(dict, kCFUserNotificationAlertMessageKey, message);
		CFRelease(message);
		CFRelease(format);
	} else {
		CFMutableArrayRef	message;

		message = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		CFArrayAppendValue(message, CFSTR("MESSAGE_SN"));
		for (i = 0; i < n; i++) {
			SCNetworkInterfaceRef	interface;
			CFStringRef		name;
			CFStringRef		str;

			interface = CFArrayGetValueAtIndex(myInstance->interfaces_prompt, i);
			name = SCNetworkInterfaceGetLocalizedDisplayName(interface);
			str = CFStringCreateWithFormat(NULL, NULL, CFSTR("\r\t%@"), name);
			CFArrayAppendValue(message, str);
			CFRelease(str);
		}
		CFArrayAppendValue(message, CFSTR("MESSAGE_EN"));
		CFDictionarySetValue(dict, kCFUserNotificationAlertMessageKey, message);
		CFRelease(message);
	}

	// button titles
	CFDictionaryAddValue(dict, kCFUserNotificationDefaultButtonTitleKey,   CFSTR("OPEN_NP"));
	CFDictionaryAddValue(dict, kCFUserNotificationAlternateButtonTitleKey, CFSTR("CANCEL"));

	// create and post notification
	myInstance->userNotification = CFUserNotificationCreate(NULL,
								0,
								kCFUserNotificationNoteAlertLevel,
								&error,
								dict);
	if (myInstance->userNotification == NULL) {
		SCLOG(NULL, myInstance->log_msg, ASL_LEVEL_ERR, CFSTR("SCMonitor: CFUserNotificationCreate() failed, %d"), (int)error);
		goto done;
	}

	// establish callback
	myInstance->userRls = CFUserNotificationCreateRunLoopSource(NULL,
								    myInstance->userNotification,
								    notify_reply,
								    0);
	if (myInstance->userRls == NULL) {
		SCLOG(NULL, myInstance->log_msg, ASL_LEVEL_ERR, CFSTR("SCMonitor: CFUserNotificationCreateRunLoopSource() failed"));
		CFRelease(myInstance->userNotification);
		myInstance->userNotification = NULL;
		goto done;
	}
	CFRunLoopAddSource(CFRunLoopGetCurrent(), myInstance->userRls,  kCFRunLoopDefaultMode);

	// add instance for notification
	if (notify_to_instance == NULL) {
		notify_to_instance = CFDictionaryCreateMutable(NULL,
							       0,
							       &kCFTypeDictionaryKeyCallBacks,
							       NULL);	// no retain/release/... for values
	}
	CFDictionarySetValue(notify_to_instance, myInstance->userNotification, myInstance);

    done :

	if (dict != NULL) CFRelease(dict);
	return;
}


static void
notify_configure(MyType *myInstance)
{
	CFIndex			i;
	CFIndex			n		= CFArrayGetCount(myInstance->interfaces_configure);
	Boolean			ok;
	SCPreferencesRef	prefs		= NULL;
	SCNetworkSetRef		set		= NULL;

	if (geteuid() == 0) {
		prefs = SCPreferencesCreate(NULL, CFSTR("SCMonitor"), NULL);
	} else {
		AuthorizationRef	authorization;

		authorization = getAuthorization(myInstance);
		if (authorization == NULL) {
			return;
		}

		prefs = SCPreferencesCreateWithAuthorization(NULL, CFSTR("SCMonitor"), NULL, authorization);
	}

	set = SCNetworkSetCopyCurrent(prefs);
	if (set == NULL) {
		set = SCNetworkSetCreate(prefs);
		if (set == NULL) {
			goto done;
		}
	}

	for (i = 0; i < n; i++) {
		SCNetworkInterfaceRef	interface;

		interface = CFArrayGetValueAtIndex(myInstance->interfaces_configure, i);
		ok = SCNetworkSetEstablishDefaultInterfaceConfiguration(set, interface);
		if (ok) {
			CFStringRef	name;

			name = SCNetworkInterfaceGetLocalizedDisplayName(interface);
			SCLOG(NULL, myInstance->log_msg, ASL_LEVEL_NOTICE, CFSTR("add/update service for %@"), name);
		}
	}

	ok = SCPreferencesCommitChanges(prefs);
	if (!ok) {
		SCLOG(NULL, myInstance->log_msg, ASL_LEVEL_ERR,
		      CFSTR("SCPreferencesCommitChanges() failed: %s"),
		      SCErrorString(SCError()));
		goto done;
	}

	ok = SCPreferencesApplyChanges(prefs);
	if (!ok) {
		SCLOG(NULL, myInstance->log_msg, ASL_LEVEL_ERR,
		      CFSTR("SCPreferencesApplyChanges() failed: %s"),
		      SCErrorString(SCError()));
		goto done;
	}

    done :

	if (set != NULL) {
		CFRelease(set);
		set = NULL;
	}

	if (prefs != NULL) {
		CFRelease(prefs);
		prefs = NULL;
	}

	CFRelease(myInstance->interfaces_configure);
	myInstance->interfaces_configure = NULL;

	return;
}


#pragma mark -


// configure ONLY IF authorized
#define kSCNetworkInterfaceConfigurationActionValueConfigureAuthorized	CFSTR("Configure-Authorized")


static void
updateInterfaceList(MyType *myInstance)
{
	Boolean			changed		= FALSE;
	CFIndex			i;
	CFArrayRef		interfaces;
	CFMutableSetRef		interfaces_old	= NULL;
	CFIndex			n;
	SCPreferencesRef	prefs;
	SCNetworkSetRef		set		= NULL;

	prefs = SCPreferencesCreate(NULL, CFSTR("SCMonitor"), NULL);
	if (prefs == NULL) {
		return;
	}

	set = SCNetworkSetCopyCurrent(prefs);
	if (set == NULL) {
		set = SCNetworkSetCreate(prefs);
		if (set == NULL) {
			goto done;
		}
	}

	interfaces_old = CFSetCreateMutableCopy(NULL, 0, myInstance->interfaces_known);

	interfaces = _SCNetworkInterfaceCopyAllWithPreferences(prefs);
	if (interfaces != NULL) {
		n = CFArrayGetCount(interfaces);
		for (i = 0; i < n; i++) {
			SCNetworkInterfaceRef	interface;
			Boolean			ok;

			interface = CFArrayGetValueAtIndex(interfaces, i);

			// track new vs. old (removed) interfaces
			CFSetRemoveValue(interfaces_old, interface);
			if (CFSetContainsValue(myInstance->interfaces_known, interface)) {
				// if we already know about this interface
				continue;
			}
			CFSetAddValue(myInstance->interfaces_known, interface);
			changed = TRUE;

			ok = SCNetworkSetEstablishDefaultInterfaceConfiguration(set, interface);
			if (ok) {
				CFStringRef		action;

				// this is a *new* interface

				action = _SCNetworkInterfaceGetConfigurationAction(interface);
				if (action == NULL) {
					// if no per-interface action, use [global] default
					action = myInstance->configuration_action;
				}
				if ((action == NULL) ||
				    (!CFEqual(action, kSCNetworkInterfaceConfigurationActionValueNone) &&
				     !CFEqual(action, kSCNetworkInterfaceConfigurationActionValueConfigure))) {
					    if (_SCNetworkInterfaceIsBuiltin(interface)) {
						    // if built-in interface
						    action = kSCNetworkInterfaceConfigurationActionValueConfigureAuthorized;
					    } else {
						    action = kSCNetworkInterfaceConfigurationActionValuePrompt;
					    }
				}

				if (CFEqual(action, kSCNetworkInterfaceConfigurationActionValueNone)) {
					continue;
				} else if (CFEqual(action, kSCNetworkInterfaceConfigurationActionValueConfigure)) {
					// configure automatically (without user intervention)
					if (myInstance->interfaces_configure == NULL) {
						myInstance->interfaces_configure = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
					}
					CFArrayAppendValue(myInstance->interfaces_configure, interface);
				} else if (hasAuthorization(myInstance)) {
					// if we already have the "admin" (kSCPreferencesAuthorizationRight_write)
					// right, configure automatically (without user intervention)
					if (myInstance->interfaces_configure == NULL) {
						myInstance->interfaces_configure = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
					}
					CFArrayAppendValue(myInstance->interfaces_configure, interface);
				} else if (!CFEqual(action, kSCNetworkInterfaceConfigurationActionValueConfigureAuthorized)) {
					// notify user
					if (myInstance->interfaces_prompt == NULL) {
						myInstance->interfaces_prompt = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
					}
					CFArrayAppendValue(myInstance->interfaces_prompt, interface);
				}
			}
		}

		CFRelease(interfaces);
	}

	// remove any posted notifications for network interfaces that have been removed
	n = CFSetGetCount(interfaces_old);
	if (n > 0) {
		const void *    paths_q[32];
		const void **   paths                 = paths_q;

		if (n > (CFIndex)(sizeof(paths_q) / sizeof(CFTypeRef)))
			paths = CFAllocatorAllocate(NULL, n * sizeof(CFTypeRef), 0);
		CFSetGetValues(interfaces_old, paths);
		for (i = 0; i < n; i++) {
			if (myInstance->interfaces_prompt != NULL) {
				CFIndex	j;

				j = CFArrayGetCount(myInstance->interfaces_prompt);
				while (j > 0) {
					SCNetworkInterfaceRef	interface;

					j--;
					interface = CFArrayGetValueAtIndex(myInstance->interfaces_prompt, j);
					if (CFEqual(interface, paths[i])) {
						// if we have previously posted a notification
						// for this no-longer-present interface
						CFArrayRemoveValueAtIndex(myInstance->interfaces_prompt, j);
						changed = TRUE;
					}
				}
			}

			CFSetRemoveValue(myInstance->interfaces_known, paths[i]);
		}
		if (paths != paths_q)       CFAllocatorDeallocate(NULL, paths);
	}

    done :

	if (changed) {
		if (myInstance->interfaces_configure != NULL) {
			// if we have network services to configure automatically
			notify_configure(myInstance);
		}

		if (myInstance->interfaces_prompt != NULL) {
			// if we have network services that require user intervention
			// post notification for new interfaces
			notify_add(myInstance);
		}
	}

	if (interfaces_old != NULL) CFRelease(interfaces_old);
	if (set != NULL) CFRelease(set);
	CFRelease(prefs);
	return;
}


#pragma mark -
#pragma mark Watch for new [network] interfaces


static void
update_lan(SCDynamicStoreRef store, CFArrayRef changes, void * arg)
{
	MyType	*myInstance	= (MyType *)arg;

	updateInterfaceList(myInstance);
	return;
}


static void
watcher_add_lan(MyType *myInstance)
{
	SCDynamicStoreContext	context	= { 0, (void *)myInstance, NULL, NULL, NULL };
	CFStringRef		key;
	CFArrayRef		keys;
	SCDynamicStoreRef	store;

	store = SCDynamicStoreCreate(NULL, CFSTR("SCMonitor"), update_lan, &context);
	if (store == NULL) {
		SCLOG(NULL, myInstance->log_msg, ASL_LEVEL_ERR,
		      CFSTR("SCMonitor: SCDynamicStoreCreate() failed: %s"),
		      SCErrorString(SCError()));
		return;
	}

	key = SCDynamicStoreKeyCreateNetworkInterface(NULL, kSCDynamicStoreDomainState);

	// watch for changes to the list of network interfaces
	keys = CFArrayCreate(NULL, (const void **)&key, 1, &kCFTypeArrayCallBacks);
	SCDynamicStoreSetNotificationKeys(store, NULL, keys);
	CFRelease(keys);
	myInstance->monitorRls = SCDynamicStoreCreateRunLoopSource(NULL, store, 0);
	CFRunLoopAddSource(CFRunLoopGetCurrent(),
			   myInstance->monitorRls,
			   kCFRunLoopDefaultMode);

	// check if we already have the "admin" (kSCPreferencesAuthorizationRight_write)
	// right.  If so, we can automatically configure (without user intervention) any
	// "new" network interfaces that are present at login (e.g. a USB ethernet
	// dongle that was plugged in before login).
	if (!hasAuthorization(myInstance)) {
		CFDictionaryRef		dict;

		// ... and if we don't have the right then we populate the list of
		// known interfaces with those already named so that we avoid any
		// login prompts (that the user might have no choice but to dismiss)
		dict = SCDynamicStoreCopyValue(store, key);
		if (dict != NULL) {
			if (isA_CFDictionary(dict)) {
				CFIndex		i;
				CFArrayRef	interfaces;
				CFIndex		n;

				interfaces = CFDictionaryGetValue(dict, kSCPropNetInterfaces);
				n = isA_CFArray(interfaces) ? CFArrayGetCount(interfaces) : 0;
				for (i = 0; i < n; i++) {
					CFStringRef	bsdName;

					bsdName = CFArrayGetValueAtIndex(interfaces, i);
					if (isA_CFString(bsdName)) {
						SCNetworkInterfaceRef	interface;

						interface = _SCNetworkInterfaceCreateWithBSDName(NULL, bsdName, kIncludeNoVirtualInterfaces);
						if (interface != NULL) {
							CFSetAddValue(myInstance->interfaces_known, interface);
							CFRelease(interface);
						}
					}
				}
			}

			CFRelease(dict);
		}
	}

	CFRelease(key);
	CFRelease(store);
	return;
}


static void
watcher_remove_lan(MyType *myInstance)
{
	if (myInstance->monitorRls != NULL) {
		CFRunLoopSourceInvalidate(myInstance->monitorRls);
		CFRelease(myInstance->monitorRls);
		myInstance->monitorRls = NULL;
	}

	return;
}


#pragma mark -


typedef struct {
	io_registry_entry_t	interface;
	io_registry_entry_t	interface_node;
	MyType			*myInstance;
	io_object_t		notification;
} MyNode;


static void
add_node_watcher(MyType *myInstance, io_registry_entry_t node, io_registry_entry_t interface);


static void
update_node(void *refCon, io_service_t service, natural_t messageType, void *messageArgument)
{
	CFIndex		i;
	CFDataRef	myData		= (CFDataRef)refCon;
	MyType		*myInstance;
	MyNode		*myNode;

	/* ALIGN: CF aligns to at least >8 bytes */
	myNode     = (MyNode *)(void *)CFDataGetBytePtr(myData);
	myInstance = myNode->myInstance;

	switch (messageType) {
		case kIOMessageServicePropertyChange : {
			Boolean			initializing	= FALSE;
			SCNetworkInterfaceRef	interface;
			CFTypeRef		val;

			if (myNode->interface == MACH_PORT_NULL) {
				// if we are not watching the "Initializing" property
				return;
			}

			val = IORegistryEntryCreateCFProperty(service, CFSTR("Initializing"), NULL, 0);
			if (val != NULL) {
				initializing = (isA_CFBoolean(val) && CFBooleanGetValue(val));
				CFRelease(val);
				if (initializing) {
					// if initialization not complete, keep watching
					return;
				}
			}

			// node is ready
			interface = _SCNetworkInterfaceCreateWithIONetworkInterfaceObject(myNode->interface);
			if (interface != NULL) {
				CFRelease(interface);

				// watch interface (to see when/if it's removed)
				add_node_watcher(myInstance, myNode->interface, MACH_PORT_NULL);
			}
			break;
		}

		case kIOMessageServiceIsTerminated :
			break;

		default :
			return;
	}

	// remove no-longer-needed notification
	if (myNode->interface != myNode->interface_node) {
		IOObjectRelease(myNode->interface_node);
	}
	if (myNode->interface != MACH_PORT_NULL) {
		IOObjectRelease(myNode->interface);
	}
	IOObjectRelease(myNode->notification);
	i = CFArrayGetFirstIndexOfValue(myInstance->notifyNodes,
					CFRangeMake(0, CFArrayGetCount(myInstance->notifyNodes)),
					myData);
	if (i != kCFNotFound) {
		CFArrayRemoveValueAtIndex(myInstance->notifyNodes, i);
		if (CFArrayGetCount(myInstance->notifyNodes) == 0) {
			CFRelease(myInstance->notifyNodes);
			myInstance->notifyNodes = NULL;
		}
	}

	updateInterfaceList(myInstance);
	return;
}


static void
add_node_watcher(MyType *myInstance, io_registry_entry_t node, io_registry_entry_t interface)
{
	kern_return_t		kr;
	CFMutableDataRef	myData;
	MyNode			*myNode;

	// wait for initialization to complete
	myData = CFDataCreateMutable(NULL, sizeof(MyNode));
	CFDataSetLength(myData, sizeof(MyNode));

	/* ALIGN: CF aligns to at least >8 bytes */
	myNode = (MyNode *)(void *)CFDataGetBytePtr(myData);

	bzero(myNode, sizeof(MyNode));
	myNode->interface      = interface;
	if (myNode->interface != MACH_PORT_NULL) {
		IOObjectRetain(myNode->interface);
	}
	myNode->interface_node = (interface == MACH_PORT_NULL) ? node : interface;
	if (myNode->interface != myNode->interface_node) {
		IOObjectRetain(myNode->interface_node);
	}
	myNode->myInstance     = myInstance;
	myNode->notification   = MACH_PORT_NULL;

	kr = IOServiceAddInterestNotification(myInstance->notifyPort,	// IONotificationPortRef
					      node,			// io_service_t
					      kIOGeneralInterest,	// interestType
					      update_node,		// IOServiceInterestCallback
					      (void *)myData,		// refCon
					      &myNode->notification);	// notification
	if (kr == KERN_SUCCESS) {
		if (myInstance->notifyNodes == NULL) {
			myInstance->notifyNodes = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		}
		CFArrayAppendValue(myInstance->notifyNodes, myData);
	} else {
		SCLOG(NULL, myInstance->log_msg, ASL_LEVEL_ERR,
		      CFSTR("add_init_watcher IOServiceAddInterestNotification() failed, kr =  0x%x"), kr);
	}
	CFRelease(myData);
}


static void
add_init_watcher(MyType *myInstance, io_registry_entry_t interface)
{
	kern_return_t		kr;
	io_registry_entry_t	node		= interface;
	CFTypeRef		val		= NULL;

	while (node != MACH_PORT_NULL) {
		io_registry_entry_t	parent;

		val = IORegistryEntryCreateCFProperty(node, kSCNetworkInterfaceHiddenPortKey, NULL, 0);
		if (val != NULL) {
			CFRelease(val);
			val = NULL;
			break;
		}

		val = IORegistryEntryCreateCFProperty(node, kSCNetworkInterfaceInitializingKey, NULL, 0);
		if (val != NULL) {
			break;
		}

		parent = MACH_PORT_NULL;
		kr = IORegistryEntryGetParentEntry(node, kIOServicePlane, &parent);
		switch (kr) {
			case kIOReturnSuccess  :	// if we have a parent node
			case kIOReturnNoDevice :	// if we have hit the root node
				break;
			default :
				SCLOG(NULL, myInstance->log_msg, ASL_LEVEL_ERR, CFSTR("add_init_watcher IORegistryEntryGetParentEntry() failed, kr = 0x%x"), kr);
				break;
		}
		if (node != interface) {
			IOObjectRelease(node);
		}
		node = parent;
	}

	if (val != NULL) {
		if (isA_CFBoolean(val) && CFBooleanGetValue(val)) {
			// watch the "Initializing" node
			add_node_watcher(myInstance, node, interface);
		}

		CFRelease(val);
	}

	if ((node != MACH_PORT_NULL) && (node != interface)) {
		IOObjectRelease(node);
	}

	return;
}


static void
update_serial(void *refcon, io_iterator_t iter)
{
	MyType			*myInstance	= (MyType *)refcon;
	io_registry_entry_t	obj;

	while ((obj = IOIteratorNext(iter)) != MACH_PORT_NULL) {
		SCNetworkInterfaceRef	interface;

		interface = _SCNetworkInterfaceCreateWithIONetworkInterfaceObject(obj);
		if (interface != NULL) {
			CFRelease(interface);

			// watch interface (to see when/if it's removed)
			add_node_watcher(myInstance, obj, MACH_PORT_NULL);
		} else {
			// check interface, watch if initializing
			add_init_watcher(myInstance, obj);
		}

		IOObjectRelease(obj);
	}

	return;
}


static void
update_serial_nodes(void *refcon, io_iterator_t iter)
{
	MyType	*myInstance	= (MyType *)refcon;

	update_serial(refcon, iter);
	updateInterfaceList(myInstance);
}


static void
watcher_add_serial(MyType *myInstance)
{
	kern_return_t	kr;

	myInstance->notifyPort = IONotificationPortCreate(kIOMasterPortDefault);
	if (myInstance->notifyPort == NULL) {
		SCLOG(NULL, myInstance->log_msg, ASL_LEVEL_ERR,
		      CFSTR("SCMonitor: IONotificationPortCreate failed"));
		return;
	}

	// watch for the introduction of new network serial devices
	kr = IOServiceAddMatchingNotification(myInstance->notifyPort,
					      kIOFirstMatchNotification,
					      IOServiceMatching("IOSerialBSDClient"),
					      &update_serial_nodes,
					      (void *)myInstance,		// refCon
					      &myInstance->notifyIterator);	// notification
	if (kr != KERN_SUCCESS) {
		SCLOG(NULL, myInstance->log_msg, ASL_LEVEL_ERR,
		      CFSTR("SCMonitor : IOServiceAddMatchingNotification returned 0x%x"),
		      kr);
		return;
	}

	myInstance->notifyNodes = NULL;

	// Get the current list of matches and arm the notification for
	// future interface arrivals.
	update_serial((void *)myInstance, myInstance->notifyIterator);

	if (myInstance->notifyNodes != NULL) {
		// if we have any serial nodes, check if we already have the
		// "admin" (kSCPreferencesAuthorizationRight_write) right.  If
		// so, we can automatically configure (without user intervention)
		// any "new" network interfaces that are present at login (e.g. a
		// USB modem that was plugged in before login).

		if (!hasAuthorization(myInstance)) {
			CFIndex	i;
			CFIndex	n	= CFArrayGetCount(myInstance->notifyNodes);

			// ... and if we don't have the right then we populate the list of
			// known interfaces with those already named so that we avoid any
			// login prompts (that the user might have no choice but to dismiss)

			for (i = 0; i < n; i++) {
				SCNetworkInterfaceRef	interface;
				CFDataRef		myData;
				MyNode			*myNode;

				myData = CFArrayGetValueAtIndex(myInstance->notifyNodes, i);

				/* ALIGN: CF aligns to at least >8 bytes */
				myNode = (MyNode *)(void *)CFDataGetBytePtr(myData);

				interface = _SCNetworkInterfaceCreateWithIONetworkInterfaceObject(myNode->interface_node);
				if (interface != NULL) {
					CFSetAddValue(myInstance->interfaces_known, interface);
					CFRelease(interface);
				}
			}
		}
	}

	// and keep watching
	CFRunLoopAddSource(CFRunLoopGetCurrent(),
			   IONotificationPortGetRunLoopSource(myInstance->notifyPort),
			   kCFRunLoopDefaultMode);
	return;
}


static void
watcher_remove_serial(MyType *myInstance)
{
	if (myInstance->notifyNodes != NULL) {
		CFIndex	i;
		CFIndex	n	= CFArrayGetCount(myInstance->notifyNodes);

		for (i = 0; i < n; i++) {
			CFDataRef	myData;
			MyNode		*myNode;

			myData = CFArrayGetValueAtIndex(myInstance->notifyNodes, i);

			/* ALIGN: CF aligns to at least >8 bytes */
			myNode = (MyNode *)(void *)CFDataGetBytePtr(myData);

			if (myNode->interface != myNode->interface_node) {
				IOObjectRelease(myNode->interface_node);
			}
			if (myNode->interface != MACH_PORT_NULL) {
				IOObjectRelease(myNode->interface);
			}
			IOObjectRelease(myNode->notification);
		}

		CFRelease(myInstance->notifyNodes);
		myInstance->notifyNodes = NULL;
	}

	if (myInstance->notifyIterator != MACH_PORT_NULL) {
		IOObjectRelease(myInstance->notifyIterator);
		myInstance->notifyIterator = MACH_PORT_NULL;
	}

	if (myInstance->notifyPort != MACH_PORT_NULL) {
		IONotificationPortDestroy(myInstance->notifyPort);
		myInstance->notifyPort = NULL;
	}

	return;
}


#pragma mark -


static void
watcher_add(MyType *myInstance)
{
	CFBundleRef	bundle;

	if (myInstance->log_msg == NULL) {
		myInstance->log_msg = asl_new(ASL_TYPE_MSG);
		asl_set(myInstance->log_msg, ASL_KEY_FACILITY, MY_BUNDLE_ID);
	}

	bundle = CFBundleGetBundleWithIdentifier(CFSTR(MY_BUNDLE_ID));
	if (bundle != NULL) {
		CFStringRef	action;
		CFBooleanRef	bVal;
		CFDictionaryRef	info;

		info = CFBundleGetInfoDictionary(bundle);

		bVal = CFDictionaryGetValue(info, CFSTR("Debug"));
		bVal = isA_CFBoolean(bVal);
		if (bVal != NULL) {
			myInstance->debug = CFBooleanGetValue(bVal);
		}

		action = CFDictionaryGetValue(info, kSCNetworkInterfaceConfigurationActionKey);
		action = isA_CFString(action);
		if (action != NULL) {
			myInstance->configuration_action = action;
		} else {
			CFBooleanRef	user_intervention;

			user_intervention = CFDictionaryGetValue(info, CFSTR("User Intervention"));
			if (isA_CFBoolean(user_intervention) && !CFBooleanGetValue(user_intervention)) {
				myInstance->configuration_action = kSCNetworkInterfaceConfigurationActionValueConfigure;
			}
		}
	}

	// initialize the list of known interfaces
	myInstance->interfaces_known = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);

	// add LAN interfaces
	watcher_add_lan(myInstance);

	// add SERIAL interfaces
	watcher_add_serial(myInstance);

	// auto-configure (as needed)
	updateInterfaceList(myInstance);

	return;
}


static void
watcher_remove(MyType *myInstance)
{
	watcher_remove_lan(myInstance);
	watcher_remove_serial(myInstance);

	if (myInstance->interfaces_known != NULL) {
		CFRelease(myInstance->interfaces_known);
		myInstance->interfaces_known = NULL;
	}

	asl_release(myInstance->log_msg);
	myInstance->log_msg = NULL;
	return;
}


#pragma mark -
#pragma mark UserEventAgent stubs


static HRESULT
myQueryInterface(void *myInstance, REFIID iid, LPVOID *ppv)
{
	CFUUIDRef	interfaceID	= CFUUIDCreateFromUUIDBytes(NULL, iid);

	// Test the requested ID against the valid interfaces.
	if (CFEqual(interfaceID, kUserEventAgentInterfaceID)) {
		((MyType *) myInstance)->_UserEventAgentInterface->AddRef(myInstance);
		*ppv = myInstance;
		CFRelease(interfaceID);
		return S_OK;
	}

	if (CFEqual(interfaceID, IUnknownUUID)) {
		((MyType *) myInstance)->_UserEventAgentInterface->AddRef(myInstance);
		*ppv = myInstance;
		CFRelease(interfaceID);
		return S_OK;
	}

	// Requested interface unknown, bail with error.
	*ppv = NULL;
	CFRelease(interfaceID);
	return E_NOINTERFACE;
}


static ULONG
myAddRef(void *myInstance)
{
	((MyType *) myInstance)->_refCount++;
	return ((MyType *) myInstance)->_refCount;
}


static ULONG
myRelease(void *myInstance)
{
	((MyType *) myInstance)->_refCount--;
	if (((MyType *) myInstance)->_refCount == 0) {
		CFUUIDRef	factoryID	= ((MyType *) myInstance)->_factoryID;

		if (factoryID != NULL) {
			CFPlugInRemoveInstanceForFactory(factoryID);
			CFRelease(factoryID);

			watcher_remove((MyType *)myInstance);
			notify_remove((MyType *)myInstance, TRUE);
			freeAuthorization((MyType *)myInstance);
		}
		free(myInstance);
		return 0;
	}

	return ((MyType *) myInstance)->_refCount;
}


static void
myInstall(void *myInstance)
{
	watcher_add((MyType *)myInstance);
	return;
}


static UserEventAgentInterfaceStruct UserEventAgentInterfaceFtbl = {
	NULL,			// Required padding for COM
	myQueryInterface,	// These three are the required COM functions
	myAddRef,
	myRelease,
	myInstall		// Interface implementation
};


void *
UserEventAgentFactory(CFAllocatorRef allocator, CFUUIDRef typeID)
{
	MyType	*newOne	= NULL;

	if (CFEqual(typeID, kUserEventAgentTypeID)) {
		newOne	= (MyType *)malloc(sizeof(MyType));
		bzero(newOne, sizeof(*newOne));
		newOne->_UserEventAgentInterface = &UserEventAgentInterfaceFtbl;
		newOne->_factoryID = (CFUUIDRef)CFRetain(kUserEventAgentFactoryID);
		CFPlugInAddInstanceForFactory(kUserEventAgentFactoryID);
		newOne->_refCount = 1;
	}

	return newOne;
}


#ifdef	MAIN
int
main(int argc, char **argv)
{
	MyType *newOne = (MyType *)malloc(sizeof(MyType));

	_sc_log     = FALSE;
	_sc_verbose = (argc > 1) ? TRUE : FALSE;

	bzero(newOne, sizeof(*newOne));
	myInstall(newOne);
	CFRunLoopRun();
	exit(0);
	return (0);
}
#endif	// MAIN
