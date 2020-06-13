/*
 * Copyright (c) 2000-2008, 2010, 2012-2019 Apple Inc. All rights reserved.
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
 * April 2, 2004		Allan Nathanson <ajn@apple.com>
 * - use SCPreference notification APIs
 *
 * June 24, 2001		Allan Nathanson <ajn@apple.com>
 * - update to public SystemConfiguration.framework APIs
 *
 * November 10, 2000		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */


#include <TargetConditionals.h>
#include <fcntl.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#define	SC_LOG_HANDLE		__log_PreferencesMonitor
#define SC_LOG_HANDLE_TYPE	static
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include <SystemConfiguration/SCValidation.h>
#include "plugin_shared.h"


#include <CommonCrypto/CommonDigest.h>


/* globals */
static SCPreferencesRef		prefs			= NULL;
static SCDynamicStoreRef	store			= NULL;

/* InterfaceNamer[.plugin] monitoring globals */
static CFMutableArrayRef	excluded_interfaces	= NULL;		// of SCNetworkInterfaceRef
static CFMutableArrayRef	excluded_names		= NULL;		// of CFStringRef (BSD name)
Boolean				haveConfiguration	= FALSE;
static CFStringRef		namerKey		= NULL;
static CFMutableArrayRef	preconfigured_interfaces= NULL;		// of SCNetworkInterfaceRef
static CFMutableArrayRef	preconfigured_names	= NULL;		// of CFStringRef (BSD name)

/* KernelEventMonitor[.plugin] monitoring globals */
static CFStringRef		interfacesKey		= NULL;

/* SCDynamicStore (Setup:) */
static CFMutableDictionaryRef	currentPrefs;		/* current prefs */
static CFMutableDictionaryRef	newPrefs;		/* new prefs */
static CFMutableArrayRef	unchangedPrefsKeys;	/* new prefs keys which match current */
static CFMutableArrayRef	removedPrefsKeys;	/* old prefs keys to be removed */

static Boolean			rofs			= FALSE;
static Boolean			restorePrefs		= FALSE;

#define MY_PLUGIN_NAME		"PreferencesMonitor"
#define	MY_PLUGIN_ID		CFSTR("com.apple.SystemConfiguration." MY_PLUGIN_NAME)


static void
updateConfiguration(SCPreferencesRef		prefs,
		    SCPreferencesNotification   notificationType,
		    void			*info);


static os_log_t
__log_PreferencesMonitor(void)
{
	static os_log_t	log	= NULL;

	if (log == NULL) {
		log = os_log_create("com.apple.SystemConfiguration", "PreferencesMonitor");
	}

	return log;
}


static Boolean
restorePreferences()
{
	Boolean			ok = FALSE;
	CFStringRef		currentModel = NULL;
	CFMutableStringRef	modelPrefixStr = NULL;
	CFArrayRef		keyList = NULL;
	CFIndex			keyListCount;
	CFIndex			idx;
	Boolean			modified = FALSE;
	int			sc_status = kSCStatusFailed;

	while (TRUE) {
		ok = SCPreferencesLock(prefs, TRUE);
		if (ok) {
			break;
		}

		sc_status = SCError();
		if (sc_status == kSCStatusStale) {
			SCPreferencesSynchronize(prefs);
		} else {
			SC_log(LOG_NOTICE, "Could not acquire network configuration lock: %s",
			       SCErrorString(sc_status));
			return FALSE;
		}
	}

	keyList = SCPreferencesCopyKeyList(prefs);
	if (keyList == NULL) {
		goto error;
	}

	currentModel = _SC_hw_model(FALSE);
	if (currentModel == NULL) {
		goto error;
	}

	/* Create "model:" string for prefix-check */
	modelPrefixStr = CFStringCreateMutableCopy(NULL, 0, currentModel);
	CFStringAppend(modelPrefixStr, CFSTR(":"));

	keyListCount = CFArrayGetCount(keyList);
	for (idx = 0; idx < keyListCount; idx++) {
		CFStringRef existingKey = CFArrayGetValueAtIndex(keyList, idx);
		CFStringRef key;
		CFArrayRef splitKey = NULL;
		CFPropertyListRef value;

		if (isA_CFString(existingKey) == NULL) {
			continue;
		}

		if (!CFStringHasPrefix(existingKey, modelPrefixStr)) {
			    continue;
		}

		splitKey = CFStringCreateArrayBySeparatingStrings(NULL, existingKey, CFSTR(":"));
		key = CFArrayGetValueAtIndex(splitKey, 1);
		value = SCPreferencesGetValue(prefs, existingKey);
		SCPreferencesSetValue(prefs, key, value);
		SCPreferencesRemoveValue(prefs, existingKey);
		modified = TRUE;
		CFRelease(splitKey);
	}

	if (modified) {
		SCPreferencesRef	ni_prefs = NULL;
		ni_prefs = SCPreferencesCreate(NULL, MY_PLUGIN_ID, CFSTR("NetworkInterfaces.plist"));
		if (ni_prefs == NULL) {
			goto error;
		}

		ok = _SCNetworkConfigurationCheckValidityWithPreferences(prefs, ni_prefs, NULL);
		CFRelease(ni_prefs);

		//Commit the changes only if prefs files valid
		if (ok) {
			if (!SCPreferencesCommitChanges(prefs)) {
				if (SCError() != EROFS) {
					SC_log(LOG_NOTICE, "SCPreferencesCommitChanges() failed: %s",
					       SCErrorString(SCError()));
				}
				goto error;

			}

			(void) SCPreferencesApplyChanges(prefs);
		}
	}

error:
	(void) SCPreferencesUnlock(prefs);

	if (keyList != NULL) {
		CFRelease(keyList);
	}
	if (modelPrefixStr != NULL) {
		CFRelease(modelPrefixStr);
	}

	return modified;
}

static Boolean
establishNewPreferences()
{
	SCNetworkSetRef	current		= NULL;
	CFStringRef	new_model;
	Boolean		ok		= FALSE;
	int		sc_status	= kSCStatusFailed;
	SCNetworkSetRef	set		= NULL;
	Boolean		updated		= FALSE;

	while (TRUE) {
		ok = SCPreferencesLock(prefs, TRUE);
		if (ok) {
			break;
		}

		sc_status = SCError();
		if (sc_status == kSCStatusStale) {
			SCPreferencesSynchronize(prefs);
		} else {
			SC_log(LOG_NOTICE, "Could not acquire network configuration lock: %s",
			       SCErrorString(sc_status));
			return FALSE;
		}
	}

	/* Ensure that the preferences has the new model */
	new_model = _SC_hw_model(FALSE);

	/* Need to regenerate the new configuration for new model */
	if (new_model != NULL) {
		CFStringRef	old_model;

		old_model = SCPreferencesGetValue(prefs, MODEL);
		if ((old_model != NULL) && !_SC_CFEqual(old_model, new_model)) {
			CFIndex		count;
			CFIndex		index;
			CFArrayRef	keys;

			keys = SCPreferencesCopyKeyList(prefs);
			count = (keys != NULL) ? CFArrayGetCount(keys) : 0;
			// if new hardware
			for (index = 0; index < count; index++) {
				CFStringRef		existing_key;

				existing_key = CFArrayGetValueAtIndex(keys, index);
				if (isA_CFString(existing_key) != NULL) {
					CFStringRef		new_key;
					CFPropertyListRef	value;

					/* If it already contains a Model
					   or if it already contains a MODEL:KEY key skip it*/
					if (CFEqual(existing_key, MODEL)
					    || CFStringFind(existing_key, CFSTR(":"), 0).location
					    != kCFNotFound) {
						continue;
					}

					value = SCPreferencesGetValue(prefs, existing_key);

					/* Create a new key as OLD_MODEL:OLD_KEY */
					new_key = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@:%@"),
									   old_model, existing_key);
					SCPreferencesSetValue(prefs, new_key, value);
					if (!CFEqual(existing_key, kSCPrefSystem)) {
						/* preserve existing host names */
						SCPreferencesRemoveValue(prefs, existing_key);
					}
					CFRelease(new_key);
				}
			}

			if (keys != NULL) {
				CFRelease(keys);
			}
		}
		/* Set the new model */
		SCPreferencesSetValue(prefs, MODEL, new_model);
	}

	current = SCNetworkSetCopyCurrent(prefs);
	if (current != NULL) {
		set = current;
	}

	if (set == NULL) {
		set = _SCNetworkSetCreateDefault(prefs);
		if (set == NULL) {
			ok = FALSE;
			sc_status = SCError();
			goto done;
		}
	}

	ok = SCNetworkSetEstablishDefaultConfiguration(set);
	if (!ok) {
		sc_status = SCError();
		goto done;
	}

    done :

	if (ok) {
		ok = SCPreferencesCommitChanges(prefs);
		if (ok) {
			SC_log(LOG_NOTICE, "New network configuration saved");
			updated = TRUE;
		} else {
			sc_status = SCError();
			if (sc_status == EROFS) {
				/* a read-only fileysstem is OK */
				ok = TRUE;

				/* ... but we don't want to synchronize */
				rofs = TRUE;
			}
		}

		/* apply (committed or temporary/read-only) changes */
		(void) SCPreferencesApplyChanges(prefs);
	} else if ((current == NULL) && (set != NULL)) {
		(void) SCNetworkSetRemove(set);
	}

	if (!ok) {
		if (sc_status == kSCStatusOK) {
			SC_log(LOG_NOTICE, "Network configuration not updated");
		} else {
			SC_log(LOG_NOTICE, "Could not establish network configuration: %s",
			       SCErrorString(sc_status));
		}
	}

	(void)SCPreferencesUnlock(prefs);
	if (set != NULL) CFRelease(set);
	return updated;
}


static void
watchSCDynamicStore()
{
	CFMutableArrayRef	keys;
	Boolean			ok;
	CFRunLoopSourceRef	rls;

	/*
	 * watch for KernelEventMonitor[.bundle] changes (the list of
	 * active network interfaces)
	 */
	interfacesKey = SCDynamicStoreKeyCreateNetworkInterface(NULL,
								kSCDynamicStoreDomainState);

	/*
	 * watch for InterfaceNamer[.bundle] changes (quiet, timeout,
	 * and the list of pre-configured interfaces)
	 */
	namerKey = SCDynamicStoreKeyCreate(NULL,
					   CFSTR("%@" "InterfaceNamer"),
					   kSCDynamicStoreDomainPlugin);

	rls = SCDynamicStoreCreateRunLoopSource(NULL, store, 0);
	if (rls == NULL) {
		SC_log(LOG_NOTICE, "SCDynamicStoreCreateRunLoopSource() failed: %s", SCErrorString(SCError()));
		haveConfiguration = TRUE;
		return;
	}
	CFRunLoopAddSource(CFRunLoopGetCurrent(), rls, kCFRunLoopDefaultMode);
	CFRelease(rls);

	keys = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	CFArrayAppendValue(keys, interfacesKey);
	CFArrayAppendValue(keys, namerKey);
	ok = SCDynamicStoreSetNotificationKeys(store, keys, NULL);
	CFRelease(keys);
	if (!ok) {
		SC_log(LOG_NOTICE, "SCDynamicStoreSetNotificationKeys() failed: %s", SCErrorString(SCError()));
		haveConfiguration = TRUE;
	}

	return;
}


static Boolean
previousConfigurationAvailable()
{
	CFStringRef		backupKey = NULL;
	CFStringRef		currentModel = NULL;
	CFPropertyListRef	properties = NULL;

	currentModel = _SC_hw_model(FALSE);
	if (currentModel == NULL) {
		goto done;
	}

	/* Currently relying only if a backup of "Sets" is present */
	backupKey = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@:Sets"), currentModel);
	properties = SCPreferencesGetValue(prefs, backupKey);
	CFRelease(backupKey);
done:
	return (properties != NULL);
}


static Boolean
findInterfaces(CFArrayRef interfaces, CFMutableArrayRef *matched_interfaces, CFMutableArrayRef *matched_names)
{
	CFIndex		n;
	CFIndex		nx	= 0;
	Boolean		updated	= FALSE;

	// start clean
	if (*matched_interfaces != NULL) {
		CFRelease(*matched_interfaces);
		*matched_interfaces = NULL;
	}
	if (*matched_names != NULL) {
		nx = CFArrayGetCount(*matched_names);
		CFRelease(*matched_names);
		*matched_names = NULL;
	}

	n = (interfaces != NULL) ? CFArrayGetCount(interfaces) : 0;
	for (CFIndex i = 0; i < n; i++) {
		CFStringRef		bsdName	 = CFArrayGetValueAtIndex(interfaces, i);
		SCNetworkInterfaceRef	interface;

		for (int retry = 0; retry < 10; retry++) {
			if (retry != 0) {
				// add short delay (before retry)
				usleep(20 * 1000);	// 20ms
			}

			interface = _SCNetworkInterfaceCreateWithBSDName(NULL, bsdName, kIncludeNoVirtualInterfaces);
			if (interface == NULL) {
				SC_log(LOG_ERR, "could not create network interface for %@", bsdName);
			} else if (_SCNetworkInterfaceGetIOPath(interface) == NULL) {
				SC_log(LOG_ERR, "could not get IOPath for %@", bsdName);
				CFRelease(interface);
				interface = NULL;
			}

			if (interface == NULL) {
				// if SCNetworkInterface not [currently] available
				continue;
			}

			// keep track of the interface name (quicker than having to iterate the list
			// of SCNetworkInterfaces, extract the name, and compare).
			if (*matched_names == NULL) {
				*matched_names = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
			}
			CFArrayAppendValue(*matched_names, bsdName);

			if (*matched_interfaces == NULL) {
				*matched_interfaces = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
			}
			CFArrayAppendValue(*matched_interfaces, interface);
			CFRelease(interface);

			updated = TRUE;
			break;
		}
	}

	// check if all interfaces were detached
	n = (*matched_names != NULL) ? CFArrayGetCount(*matched_names) : 0;
	if ((nx > 0) && (n == 0)) {
		updated = TRUE;
	}

	return updated;
}


static void
storeCallback(SCDynamicStoreRef store, CFArrayRef changedKeys, void *info)
{
#pragma unused(info)
	CFDictionaryRef	dict;
	Boolean		quiet		= FALSE;
	Boolean		timeout		= FALSE;
	Boolean		updated		= FALSE;

	/*
	 * Capture/process InterfaceNamer[.bundle] info
	 * 1. check if IORegistry "quiet", "timeout"
	 * 2. update list of excluded interfaces (e.g. those requiring that
	 *    the attached host be trusted)
	 * 3. update list of named pre-configured interfaces
	 */
	dict = SCDynamicStoreCopyValue(store, namerKey);
	if (dict != NULL) {
		if (isA_CFDictionary(dict)) {
			CFArrayRef	excluded;
			CFArrayRef	preconfigured;

			if (CFDictionaryContainsKey(dict, kInterfaceNamerKey_Quiet)) {
				quiet = TRUE;
			}
			if (CFDictionaryContainsKey(dict, kInterfaceNamerKey_Timeout)) {
				timeout = TRUE;
			}

			excluded = CFDictionaryGetValue(dict, kInterfaceNamerKey_ExcludedInterfaces);
			excluded = isA_CFArray(excluded);
			if (!_SC_CFEqual(excluded, excluded_names)) {
				Boolean		excluded_updated;

				excluded_updated = findInterfaces(excluded, &excluded_interfaces, &excluded_names);
				if (excluded_updated) {
					CFStringRef	interfaces	= CFSTR("<empty>");

					// report [updated] pre-configured interfaces
					if (excluded_names != NULL) {
						interfaces = CFStringCreateByCombiningStrings(NULL, excluded_names, CFSTR(","));
					} else {
						CFRetain(interfaces);
					}
					SC_log(LOG_INFO, "excluded interface list changed: %@", interfaces);
					CFRelease(interfaces);

					updated = TRUE;
				}
			}

			preconfigured = CFDictionaryGetValue(dict, kInterfaceNamerKey_PreConfiguredInterfaces);
			preconfigured = isA_CFArray(preconfigured);
			if (!_SC_CFEqual(preconfigured, preconfigured_names)) {
				Boolean		preconfigured_updated;

				preconfigured_updated = findInterfaces(preconfigured, &preconfigured_interfaces, &preconfigured_names);
				if (preconfigured_updated) {
					CFStringRef	interfaces	= CFSTR("<empty>");

					// report [updated] pre-configured interfaces
					if (preconfigured_names != NULL) {
						interfaces = CFStringCreateByCombiningStrings(NULL, preconfigured_names, CFSTR(","));
					} else {
						CFRetain(interfaces);
					}
					SC_log(LOG_INFO, "pre-configured interface list changed: %@", interfaces);
					CFRelease(interfaces);

					updated = TRUE;
				}
			}
		}

		CFRelease(dict);
	}

	if (!haveConfiguration && (quiet || timeout)) {
		static int	logged	= 0;

		if (quiet
#if	!TARGET_OS_IPHONE
		    || timeout
#endif	/* !TARGET_OS_IPHONE */
		    ) {
			haveConfiguration = TRUE;
		}

		(void) establishNewPreferences();

		if (restorePrefs) {
			(void) restorePreferences();
			restorePrefs = FALSE;
		}

		if (timeout && (logged++ == 0)) {
			SC_log(LOG_ERR, "Network configuration creation timed out waiting for IORegistry");
		}
	}

	if (updated && (changedKeys != NULL)) {
		// if pre-configured interface list changed
		updateConfiguration(prefs, kSCPreferencesNotificationApply, (void *)store);
	}

	return;
}


static void
updateCache(const void *key, const void *value, void *context)
{
#pragma unused(context)
	CFStringRef		configKey	= (CFStringRef)key;
	CFPropertyListRef	configData	= (CFPropertyListRef)value;
	CFPropertyListRef	cacheData;
	CFIndex			i;

	cacheData = CFDictionaryGetValue(currentPrefs, configKey);
	if (cacheData) {
		/* key exists */
		if (CFEqual(cacheData, configData)) {
			/*
			 * if the old & new property list values have
			 * not changed then we don't need to update
			 * the preference.
			 */
			CFArrayAppendValue(unchangedPrefsKeys, configKey);
		}
	}

	/* in any case, this key should not be removed */
	i = CFArrayGetFirstIndexOfValue(removedPrefsKeys,
					CFRangeMake(0, CFArrayGetCount(removedPrefsKeys)),
					configKey);
	if (i != kCFNotFound) {
		CFArrayRemoveValueAtIndex(removedPrefsKeys, i);
	}

	return;
}


static void
flatten(SCPreferencesRef	prefs,
	CFStringRef		key,
	CFDictionaryRef		base)
{
	CFDictionaryRef		subset;
	CFStringRef		link;
	CFMutableDictionaryRef	myDict;
	CFStringRef		myKey;
	CFIndex			i;
	CFIndex			nKeys;
	const void		**keys;
	const void		**vals;

	if (!CFDictionaryGetValueIfPresent(base, kSCResvLink, (const void **)&link)) {
		/* if this dictionary is not linked */
		subset = base;
	} else {
		/* if __LINK__ key is present */
		subset = SCPreferencesPathGetValue(prefs, link);
		if (!subset) {
			/* if error with link */
			SC_log(LOG_NOTICE, "SCPreferencesPathGetValue(,%@,) failed: %s",
			       link,
			       SCErrorString(SCError()));
			return;
		}
	}

	if (CFDictionaryContainsKey(subset, kSCResvInactive)) {
		/* if __INACTIVE__ key is present */
		return;
	}

	myKey = CFStringCreateWithFormat(NULL,
					 NULL,
					 CFSTR("%@%@"),
					 kSCDynamicStoreDomainSetup,
					 key);

	myDict = (CFMutableDictionaryRef)CFDictionaryGetValue(newPrefs, myKey);
	if (myDict) {
		myDict = CFDictionaryCreateMutableCopy(NULL,
						       0,
						       (CFDictionaryRef)myDict);
	} else {
		myDict = CFDictionaryCreateMutable(NULL,
						   0,
						   &kCFTypeDictionaryKeyCallBacks,
						   &kCFTypeDictionaryValueCallBacks);
	}

	nKeys = CFDictionaryGetCount(subset);
	if (nKeys > 0) {
		keys  = CFAllocatorAllocate(NULL, nKeys * sizeof(CFStringRef)      , 0);
		vals  = CFAllocatorAllocate(NULL, nKeys * sizeof(CFPropertyListRef), 0);
		CFDictionaryGetKeysAndValues(subset, keys, vals);
		for (i = 0; i < nKeys; i++) {
			if (CFGetTypeID((CFTypeRef)vals[i]) != CFDictionaryGetTypeID()) {
				/* add this key/value to the current dictionary */
				CFDictionarySetValue(myDict, keys[i], vals[i]);
			} else {
				CFStringRef	subKey;

				/* flatten [sub]dictionaries */
				subKey = CFStringCreateWithFormat(NULL,
								  NULL,
								  CFSTR("%@%s%@"),
								  key,
								  CFEqual(key, CFSTR("/")) ? "" : "/",
								  keys[i]);
				flatten(prefs, subKey, vals[i]);
				CFRelease(subKey);
			}
		}
		CFAllocatorDeallocate(NULL, keys);
		CFAllocatorDeallocate(NULL, vals);
	}

	if (CFDictionaryGetCount(myDict) > 0) {
		/* add this dictionary to the new preferences */
		CFDictionarySetValue(newPrefs, myKey, myDict);
	}

	CFRelease(myDict);
	CFRelease(myKey);

	return;
}


static CF_RETURNS_RETAINED CFStringRef
copyInterfaceUUID(CFStringRef bsdName)
{
	union {
		unsigned char	sha256_bytes[CC_SHA256_DIGEST_LENGTH];
		CFUUIDBytes	uuid_bytes;
	} bytes;
	CC_SHA256_CTX	ctx;
	char		if_name[IF_NAMESIZE];
	CFUUIDRef	uuid;
	CFStringRef	uuid_str;

	// start with interface name
	memset(&if_name, 0, sizeof(if_name));
	(void) _SC_cfstring_to_cstring(bsdName,
				       if_name,
				       sizeof(if_name),
				       kCFStringEncodingASCII);

	// create SHA256 hash
	memset(&bytes, 0, sizeof(bytes));
	CC_SHA256_Init(&ctx);
	CC_SHA256_Update(&ctx,
		       if_name,
		       sizeof(if_name));
	CC_SHA256_Final(bytes.sha256_bytes, &ctx);

	// create UUID string
	uuid = CFUUIDCreateFromUUIDBytes(NULL, bytes.uuid_bytes);
	uuid_str = CFUUIDCreateString(NULL, uuid);
	CFRelease(uuid);

	return uuid_str;
}


static void
excludeConfigurations(SCPreferencesRef prefs)
{
	Boolean		ok;
	CFRange		range;
	CFArrayRef	services;
	SCNetworkSetRef	set;

	range = CFRangeMake(0,
			    (excluded_names != NULL) ? CFArrayGetCount(excluded_names) : 0);
	if (range.length == 0) {
		// if no [excluded] interfaces
		return;
	}

	set = SCNetworkSetCopyCurrent(prefs);
	if (set == NULL) {
		// if no current set
		return;
	}

	/*
	 * Check for (and remove) any network services associated with
	 * an excluded interface from the prefs.
	 */
	services = SCNetworkSetCopyServices(set);
	if (services != NULL) {
		CFIndex		n;

		n = CFArrayGetCount(services);
		for (CFIndex i = 0; i < n; i++) {
			CFStringRef		bsdName;
			SCNetworkInterfaceRef	interface;
			SCNetworkServiceRef	service;

			service = CFArrayGetValueAtIndex(services, i);

			interface = SCNetworkServiceGetInterface(service);
			if (interface == NULL) {
				// if no interface
				continue;
			}

			bsdName = SCNetworkInterfaceGetBSDName(interface);
			if (bsdName == NULL) {
				// if no interface name
				continue;
			}

			if (!CFArrayContainsValue(excluded_names, range, bsdName)) {
				// if not excluded
				continue;
			}

			// remove [excluded] network service from the prefs
			SC_log(LOG_NOTICE, "excluding network service for %@", bsdName);
			ok = SCNetworkSetRemoveService(set, service);
			if (!ok) {
				SC_log(LOG_ERR, "SCNetworkSetRemoveService() failed: %s",
				       SCErrorString(SCError()));
			}
		}

		CFRelease(services);
	}

	CFRelease(set);
	return;
}


static void
updatePreConfiguredConfiguration(SCPreferencesRef prefs)
{
	Boolean		ok;
	CFRange		range;
	CFArrayRef	services;
	SCNetworkSetRef	set;
	Boolean		updated	= FALSE;

	range = CFRangeMake(0,
			    (preconfigured_names != NULL) ? CFArrayGetCount(preconfigured_names) : 0);
	if (range.length == 0) {
		// if no [pre-configured] interfaces
		return;
	}

	set = SCNetworkSetCopyCurrent(prefs);
	if (set == NULL) {
		// if no current set
		return;
	}

	/*
	 * Check for (and remove) any network services associated with
	 * a pre-configured interface from the prefs.
	 */
	services = SCNetworkServiceCopyAll(prefs);
	if (services != NULL) {
		CFIndex		n;

		n = CFArrayGetCount(services);
		for (CFIndex i = 0; i < n; i++) {
			CFStringRef		bsdName;
			SCNetworkInterfaceRef	interface;
			SCNetworkServiceRef	service;

			service = CFArrayGetValueAtIndex(services, i);

			interface = SCNetworkServiceGetInterface(service);
			if (interface == NULL) {
				// if no interface
				continue;
			}

			bsdName = SCNetworkInterfaceGetBSDName(interface);
			if (bsdName == NULL) {
				// if no interface name
				continue;
			}

			if (!CFArrayContainsValue(preconfigured_names, range, bsdName)) {
				// if not preconfigured
				continue;
			}

			// remove [preconfigured] network service from the prefs
			SC_log(LOG_NOTICE, "removing network service for %@", bsdName);
			ok = SCNetworkServiceRemove(service);
			if (!ok) {
				SC_log(LOG_ERR, "SCNetworkServiceRemove() failed: %s",
				       SCErrorString(SCError()));
			}
			updated = TRUE;
		}

		CFRelease(services);
	}

	if (updated) {
		// commit the updated prefs ... but don't apply
		ok = SCPreferencesCommitChanges(prefs);
		if (!ok) {
			if (SCError() != EROFS) {
				SC_log(LOG_ERR, "SCPreferencesCommitChanges() failed: %s",
				       SCErrorString(SCError()));
			}
		}
	}

	/*
	 * Now, add a new network service for each pre-configured interface
	 */
	for (CFIndex i = 0; i < range.length; i++) {
		CFStringRef		bsdName;
		SCNetworkInterfaceRef	interface	= CFArrayGetValueAtIndex(preconfigured_interfaces, i);
		SCNetworkServiceRef	service;
		CFStringRef		serviceID;

		bsdName = SCNetworkInterfaceGetBSDName(interface);

		// create network service
		service = SCNetworkServiceCreate(prefs, interface);
		if (service == NULL) {
			SC_log(LOG_ERR, "could not create network service for \"%@\": %s",
			       bsdName,
			       SCErrorString(SCError()));
			continue;
		}

		// update network service to use a consistent serviceID
		serviceID = copyInterfaceUUID(bsdName);
		if (serviceID != NULL) {
			ok = _SCNetworkServiceSetServiceID(service, serviceID);
			CFRelease(serviceID);
			if (!ok) {
				SC_log(LOG_ERR, "_SCNetworkServiceSetServiceID() failed: %s",
				       SCErrorString(SCError()));
				// ... and keep whatever random UUID was created for the service
			}
		} else {
			SC_log(LOG_ERR, "could not create serviceID for \"%@\"", bsdName);
			// ... and we'll use whatever random UUID was created for the service
		}

		// establish [template] configuration
		ok = SCNetworkServiceEstablishDefaultConfiguration(service);
		if (!ok) {
			SC_log(LOG_ERR, "could not establish network service for \"%@\": %s",
			       bsdName,
			       SCErrorString(SCError()));
			SCNetworkServiceRemove(service);
			CFRelease(service);
			continue;
		}

		// add network service to the current set
		ok = SCNetworkSetAddService(set, service);
		if (!ok) {
			SC_log(LOG_ERR, "could not add service for \"%@\": %s",
			       bsdName,
			       SCErrorString(SCError()));
			SCNetworkServiceRemove(service);
			CFRelease(service);
			continue;
		}

		SC_log(LOG_INFO, "network service %@ added for \"%@\"",
		       SCNetworkServiceGetServiceID(service),
		       bsdName);

		CFRelease(service);
	}

	CFRelease(set);
	return;
}


static void
updateSCDynamicStore(SCPreferencesRef prefs)
{
	CFStringRef		current		= NULL;
	CFDateRef		date		= NULL;
	CFMutableDictionaryRef	dict		= NULL;
	CFDictionaryRef		global		= NULL;
	CFIndex			i;
	CFArrayRef		keys;
	CFIndex			n;
	CFStringRef		pattern;
	CFMutableArrayRef	patterns;
	CFDictionaryRef		set		= NULL;

	/*
	 * initialize old preferences, new preferences, an array
	 * of keys which have not changed, and an array of keys
	 * to be removed (cleaned up).
	 */

	patterns = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	pattern  = CFStringCreateWithFormat(NULL,
					    NULL,
					    CFSTR("^%@.*"),
					    kSCDynamicStoreDomainSetup);
	CFArrayAppendValue(patterns, pattern);
	dict = (CFMutableDictionaryRef)SCDynamicStoreCopyMultiple(store, NULL, patterns);
	CFRelease(patterns);
	CFRelease(pattern);
	if (dict) {
		currentPrefs = CFDictionaryCreateMutableCopy(NULL, 0, dict);
		CFRelease(dict);
	} else {
		currentPrefs = CFDictionaryCreateMutable(NULL,
							 0,
							 &kCFTypeDictionaryKeyCallBacks,
							 &kCFTypeDictionaryValueCallBacks);
	}

	unchangedPrefsKeys = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	i = CFDictionaryGetCount(currentPrefs);
	if (i > 0) {
		const void	**currentKeys;
		CFArrayRef	array;

		currentKeys = CFAllocatorAllocate(NULL, i * sizeof(CFStringRef), 0);
		CFDictionaryGetKeysAndValues(currentPrefs, currentKeys, NULL);
		array = CFArrayCreate(NULL, currentKeys, i, &kCFTypeArrayCallBacks);
		removedPrefsKeys = CFArrayCreateMutableCopy(NULL, 0, array);
		CFRelease(array);
		CFAllocatorDeallocate(NULL, currentKeys);
	} else {
		removedPrefsKeys = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	}

	/*
	 * The "newPrefs" dictionary will contain the new / updated
	 * configuration which will be written to the configuration cache.
	 */
	newPrefs = CFDictionaryCreateMutable(NULL,
						 0,
						 &kCFTypeDictionaryKeyCallBacks,
						 &kCFTypeDictionaryValueCallBacks);

	/*
	 * create status dictionary associated with current configuration
	 * information including:
	 *   - current set "name" to cache
	 *   - time stamp indicating when the cache preferences were
	 *     last updated.
	 */
	dict = CFDictionaryCreateMutable(NULL,
					 0,
					 &kCFTypeDictionaryKeyCallBacks,
					 &kCFTypeDictionaryValueCallBacks);
	date = CFDateCreate(NULL, CFAbsoluteTimeGetCurrent());

	/*
	 * load preferences
	 */
	keys = SCPreferencesCopyKeyList(prefs);
	if ((keys == NULL) || (CFArrayGetCount(keys) == 0)) {
		SC_log(LOG_NOTICE, "updateConfiguration(): no preferences");
		goto done;
	}

	/*
	 * get "global" system preferences
	 */
	global = SCPreferencesGetValue(prefs, kSCPrefSystem);
	if (!global) {
		/* if no global preferences are defined */
		goto getSet;
	}

	if (!isA_CFDictionary(global)) {
		SC_log(LOG_NOTICE, "updateConfiguration(): %@ is not a dictionary",
		       kSCPrefSystem);
		goto done;
	}

	/* flatten property list */
	flatten(prefs, CFSTR("/"), global);

    getSet :

	/*
	 * get current set name
	 */
	current = SCPreferencesGetValue(prefs, kSCPrefCurrentSet);
	if (!current) {
		/* if current set not defined */
		goto done;
	}

	if (!isA_CFString(current)) {
		SC_log(LOG_NOTICE, "updateConfiguration(): %@ is not a string",
		       kSCPrefCurrentSet);
		goto done;
	}

	/*
	 * get current set
	 */
	set = SCPreferencesPathGetValue(prefs, current);
	if (!set) {
		/* if error with path */
		SC_log(LOG_NOTICE, "%@ value (%@) not valid",
		       kSCPrefCurrentSet,
		       current);
		goto done;
	}

	if (!isA_CFDictionary(set)) {
		SC_log(LOG_NOTICE, "updateConfiguration(): %@ is not a dictionary",
		       current);
		goto done;
	}

	/* flatten property list */
	flatten(prefs, CFSTR("/"), set);

	CFDictionarySetValue(dict, kSCDynamicStorePropSetupCurrentSet, current);

    done :

	/* add last updated time stamp */
	CFDictionarySetValue(dict, kSCDynamicStorePropSetupLastUpdated, date);

	/* add Setup: key */
	CFDictionarySetValue(newPrefs, kSCDynamicStoreDomainSetup, dict);

	/* compare current and new preferences */
	CFDictionaryApplyFunction(newPrefs, updateCache, NULL);

	/* remove those keys which have not changed from the update */
	n = CFArrayGetCount(unchangedPrefsKeys);
	for (i = 0; i < n; i++) {
		CFStringRef	key;

		key = CFArrayGetValueAtIndex(unchangedPrefsKeys, i);
		CFDictionaryRemoveValue(newPrefs, key);
	}

	/* Update the dynamic store */
#ifndef MAIN
	if (!SCDynamicStoreSetMultiple(store, newPrefs, removedPrefsKeys, NULL)) {
		SC_log(LOG_NOTICE, "SCDynamicStoreSetMultiple() failed: %s", SCErrorString(SCError()));
	}
#else	// !MAIN
	SC_log(LOG_DEBUG, "SCDynamicStore\nset: %@\nremove: %@",
	       newPrefs,
	       removedPrefsKeys);
#endif	// !MAIN

	CFRelease(currentPrefs);
	CFRelease(newPrefs);
	CFRelease(unchangedPrefsKeys);
	CFRelease(removedPrefsKeys);
	if (dict)	CFRelease(dict);
	if (date)	CFRelease(date);
	if (keys)	CFRelease(keys);
	return;
}


static void
updateConfiguration(SCPreferencesRef		prefs,
		    SCPreferencesNotification   notificationType,
		    void			*info)
{
#pragma unused(info)
#if	!TARGET_OS_IPHONE
	if ((notificationType & kSCPreferencesNotificationCommit) == kSCPreferencesNotificationCommit) {
		SCNetworkSetRef	current;

		current = SCNetworkSetCopyCurrent(prefs);
		if (current != NULL) {
			/* network configuration available, disable template creation */
			haveConfiguration = TRUE;
			CFRelease(current);
		}
	}
#endif	/* !TARGET_OS_IPHONE */

	if ((notificationType & kSCPreferencesNotificationApply) != kSCPreferencesNotificationApply) {
		goto done;
	}

	SC_log(LOG_INFO, "updating configuration");

	/* add any [Apple] pre-configured network services */
	updatePreConfiguredConfiguration(prefs);

	/* remove any excluded network services */
	excludeConfigurations(prefs);

	/* update SCDynamicStore (Setup:) */
	updateSCDynamicStore(prefs);

	/* finished with current prefs, wait for changes */
	if (!rofs) {
		SCPreferencesSynchronize(prefs);
	}

    done :

	return;
}


__private_extern__
void
prime_PreferencesMonitor()
{
	SC_log(LOG_DEBUG, "prime() called");

	/* load the initial configuration from the database */
	updateConfiguration(prefs, kSCPreferencesNotificationApply, (void *)store);

	return;
}


__private_extern__
void
load_PreferencesMonitor(CFBundleRef bundle, Boolean bundleVerbose)
{
#pragma unused(bundle)
#pragma unused(bundleVerbose)
	SC_log(LOG_DEBUG, "load() called");
	SC_log(LOG_DEBUG, "  bundle ID = %@", CFBundleGetIdentifier(bundle));

	/* open a SCDynamicStore session to allow cache updates */
	store = SCDynamicStoreCreate(NULL,
				     CFSTR("PreferencesMonitor.bundle"),
				     storeCallback,
				     NULL);
	if (store == NULL) {
		SC_log(LOG_NOTICE, "SCDynamicStoreCreate() failed: %s", SCErrorString(SCError()));
		goto error;
	}

	/* open a SCPreferences session */
#ifndef	MAIN
	prefs = SCPreferencesCreate(NULL, CFSTR("PreferencesMonitor.bundle"), NULL);
#else	// !MAIN
	prefs = SCPreferencesCreate(NULL, CFSTR("PreferencesMonitor.bundle"), CFSTR("/tmp/preferences.plist"));
#endif	// !MAIN
	if (prefs != NULL) {
		Boolean		need_update = FALSE;
		CFStringRef	new_model;

		new_model = _SC_hw_model(FALSE);

		/* Need to regenerate the new configuration for new model */
		if (new_model != NULL) {
			CFStringRef	old_model;

			old_model = SCPreferencesGetValue(prefs, MODEL);
			if (old_model != NULL && !_SC_CFEqual(old_model, new_model)) {
				// if new hardware
				need_update = TRUE;
				restorePrefs = previousConfigurationAvailable();
			}
		}

		if (!need_update) {
			SCNetworkSetRef current;

			current = SCNetworkSetCopyCurrent(prefs);
			if (current != NULL) {
				/* network configuration available, disable template creation */
				haveConfiguration = TRUE;
				CFRelease(current);
			}
		}
	} else {
		SC_log(LOG_NOTICE, "SCPreferencesCreate() failed: %s", SCErrorString(SCError()));
		goto error;
	}

	/*
	 * register for change notifications.
	 */
	if (!SCPreferencesSetCallback(prefs, updateConfiguration, NULL)) {
		SC_log(LOG_NOTICE, "SCPreferencesSetCallBack() failed: %s", SCErrorString(SCError()));
		goto error;
	}

	if (!SCPreferencesScheduleWithRunLoop(prefs, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode)) {
		SC_log(LOG_NOTICE, "SCPreferencesScheduleWithRunLoop() failed: %s", SCErrorString(SCError()));
		goto error;
	}

	/*
	 * watch InterfaceNamer and KernelEventMonitor changes to know when
	 * the IORegistry has quiesced (to create the initial configuration
	 * template), to track any pre-configured interfaces, and to ensure
	 * that we create a network service for any active interfaces.
	 */
	watchSCDynamicStore();
	storeCallback(store, NULL, NULL);

	return;

    error :

	if (store != NULL)	CFRelease(store);
	if (prefs != NULL)	CFRelease(prefs);
	haveConfiguration = TRUE;

	return;
}


#ifdef  MAIN
int
main(int argc, char **argv)
{
	_sc_log     = FALSE;
	_sc_verbose = (argc > 1) ? TRUE : FALSE;

	load_PreferencesMonitor(CFBundleGetMainBundle(), (argc > 1) ? TRUE : FALSE);
	prime_PreferencesMonitor();
	CFRunLoopRun();
	/* not reached */
	exit(0);
	return 0;
}
#endif
