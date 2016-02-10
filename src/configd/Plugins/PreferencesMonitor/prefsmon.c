/*
 * Copyright (c) 2000-2008, 2010, 2012-2014 Apple Inc. All rights reserved.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>
#include <SystemConfiguration/SCValidation.h>




/* globals */
static SCPreferencesRef		prefs		= NULL;
static SCDynamicStoreRef	store		= NULL;

/* preferences "initialization" globals */
static CFStringRef		initKey		= NULL;
static CFRunLoopSourceRef	initRls		= NULL;

/* SCDynamicStore (Setup:) */
static CFMutableDictionaryRef	currentPrefs;		/* current prefs */
static CFMutableDictionaryRef	newPrefs;		/* new prefs */
static CFMutableArrayRef	unchangedPrefsKeys;	/* new prefs keys which match current */
static CFMutableArrayRef	removedPrefsKeys;	/* old prefs keys to be removed */

static Boolean			rofs		= FALSE;
static Boolean			_verbose	= FALSE;


static Boolean
establishNewPreferences()
{
	CFBundleRef     bundle;
	SCNetworkSetRef	current		= NULL;
	CFStringRef	new_model;
	Boolean		ok		= FALSE;
	int		sc_status	= kSCStatusFailed;
	SCNetworkSetRef	set		= NULL;
	CFStringRef	setName		= NULL;
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
			SCLog(TRUE, LOG_ERR,
			      CFSTR("Could not acquire network configuration lock: %s"),
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
		set = SCNetworkSetCreate(prefs);
		if (set == NULL) {
			ok = FALSE;
			sc_status = SCError();
			goto done;
		}

		bundle = _SC_CFBundleGet();
		if (bundle != NULL) {
			setName = CFBundleCopyLocalizedString(bundle,
							      CFSTR("DEFAULT_SET_NAME"),
							      CFSTR("Automatic"),
							      NULL);
		}

		ok = SCNetworkSetSetName(set, (setName != NULL) ? setName : CFSTR("Automatic"));
		if (!ok) {
			sc_status = SCError();
			goto done;
		}

		ok = SCNetworkSetSetCurrent(set);
		if (!ok) {
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
			SCLog(TRUE, LOG_NOTICE, CFSTR("New network configuration saved"));
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
		SCLog(TRUE, LOG_ERR,
		      CFSTR("Could not establish network configuration: %s"),
		      SCErrorString(sc_status));
	}

	(void)SCPreferencesUnlock(prefs);
	if (setName != NULL) CFRelease(setName);
	if (set != NULL) CFRelease(set);
	return updated;
}


static Boolean
quiet(Boolean *timeout)
{
	CFDictionaryRef	dict;
	Boolean		_quiet		= FALSE;
	Boolean		_timeout	= FALSE;

	// check if quiet
	dict = SCDynamicStoreCopyValue(store, initKey);
	if (dict != NULL) {
		if (isA_CFDictionary(dict)) {
			if (CFDictionaryContainsKey(dict, CFSTR("*QUIET*"))) {
				_quiet = TRUE;
			}
			if (CFDictionaryContainsKey(dict, CFSTR("*TIMEOUT*"))) {
				_timeout = TRUE;
			}
		}
		CFRelease(dict);
	}

	if (timeout != NULL) {
		*timeout = _timeout;
	}
	return _quiet;
}


static void
watchQuietDisable()
{
	if ((initKey == NULL) || (initRls == NULL)) {
		return;
	}

	(void) SCDynamicStoreSetNotificationKeys(store, NULL, NULL);

	CFRunLoopSourceInvalidate(initRls);
	CFRelease(initRls);
	initRls = NULL;

	CFRelease(initKey);
	initKey = NULL;

	return;
}


static void
watchQuietEnable()
{
	CFArrayRef	keys;
	Boolean		ok;

	initKey = SCDynamicStoreKeyCreate(NULL,
					  CFSTR("%@" "InterfaceNamer"),
					  kSCDynamicStoreDomainPlugin);

	initRls = SCDynamicStoreCreateRunLoopSource(NULL, store, 0);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), initRls, kCFRunLoopDefaultMode);

	keys = CFArrayCreate(NULL, (const void **)&initKey, 1, &kCFTypeArrayCallBacks);
	ok = SCDynamicStoreSetNotificationKeys(store, keys, NULL);
	CFRelease(keys);
	if (!ok) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("SCDynamicStoreSetNotificationKeys() failed: %s\n"), SCErrorString(SCError()));
		watchQuietDisable();
	}

	return;
}




static void
watchQuietCallback(SCDynamicStoreRef store, CFArrayRef changedKeys, void *info)
{
	Boolean	_quiet;
	Boolean	_timeout	= FALSE;

	_quiet = quiet(&_timeout);
	if (_quiet
#if	!TARGET_OS_IPHONE
	    || _timeout
#endif	/* !TARGET_OS_IPHONE */
	   ) {
		watchQuietDisable();
	}

	if (_quiet || _timeout) {
		static int	logged	= 0;

		(void) establishNewPreferences();
		if (_timeout && (logged++ == 0)) {
			SCLog(TRUE, LOG_NOTICE,
			      CFSTR("Network configuration creation timed out waiting for IORegistry"));
		}
	}

	return;
}


static void
updateCache(const void *key, const void *value, void *context)
{
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
			SCLog(TRUE, LOG_ERR,
			      CFSTR("SCPreferencesPathGetValue(,%@,) failed: %s"),
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
		SCLog(TRUE, LOG_NOTICE, CFSTR("updateConfiguration(): no preferences."));
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
		SCLog(TRUE, LOG_ERR,
		      CFSTR("updateConfiguration(): %@ is not a dictionary."),
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
		SCLog(TRUE, LOG_ERR,
		      CFSTR("updateConfiguration(): %@ is not a string."),
		      kSCPrefCurrentSet);
		goto done;
	}

	/*
	 * get current set
	 */
	set = SCPreferencesPathGetValue(prefs, current);
	if (!set) {
		/* if error with path */
		SCLog(TRUE, LOG_ERR,
		      CFSTR("%@ value (%@) not valid"),
		      kSCPrefCurrentSet,
		      current);
		goto done;
	}

	if (!isA_CFDictionary(set)) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("updateConfiguration(): %@ is not a dictionary."),
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
		SCLog(TRUE, LOG_ERR,
		      CFSTR("SCDynamicStoreSetMultiple() failed: %s"),
		      SCErrorString(SCError()));
	}
#else	// !MAIN
	SCLog(TRUE, LOG_NOTICE,
	      CFSTR("SCDynamicStore\nset: %@\nremove: %@\n"),
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


#if	!TARGET_OS_IPHONE
	if ((notificationType & kSCPreferencesNotificationCommit) == kSCPreferencesNotificationCommit) {
		SCNetworkSetRef	current;

		current = SCNetworkSetCopyCurrent(prefs);
		if (current != NULL) {
			/* network configuration available, disable template creation */
			watchQuietDisable();
			CFRelease(current);
		}
	}
#endif	/* !TARGET_OS_IPHONE */

	if ((notificationType & kSCPreferencesNotificationApply) != kSCPreferencesNotificationApply) {
		return;
	}

	SCLog(_verbose, LOG_DEBUG, CFSTR("updating configuration"));

	/* update SCDynamicStore (Setup:) */
	updateSCDynamicStore(prefs);

	/* finished with current prefs, wait for changes */
	if (!rofs) {
		SCPreferencesSynchronize(prefs);
	}

	return;
}


__private_extern__
void
prime_PreferencesMonitor()
{
	SCLog(_verbose, LOG_DEBUG, CFSTR("prime() called"));

	/* load the initial configuration from the database */
	updateConfiguration(prefs, kSCPreferencesNotificationApply, (void *)store);

	return;
}


__private_extern__
void
load_PreferencesMonitor(CFBundleRef bundle, Boolean bundleVerbose)
{
	Boolean	initPrefs	= TRUE;

	if (bundleVerbose) {
		_verbose = TRUE;
	}

	SCLog(_verbose, LOG_DEBUG, CFSTR("load() called"));
	SCLog(_verbose, LOG_DEBUG, CFSTR("  bundle ID = %@"), CFBundleGetIdentifier(bundle));

	/* open a SCDynamicStore session to allow cache updates */
	store = SCDynamicStoreCreate(NULL,
				     CFSTR("PreferencesMonitor.bundle"),
				     watchQuietCallback,
				     NULL);
	if (store == NULL) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("SCDynamicStoreCreate() failed: %s"),
		      SCErrorString(SCError()));
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
			}
		}

		if (need_update == FALSE) {
			SCNetworkSetRef current;

			current = SCNetworkSetCopyCurrent(prefs);
			if (current != NULL) {
				/* network configuration available, disable template creation */
				initPrefs = FALSE;
				CFRelease(current);
			}
		}
	} else {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("SCPreferencesCreate() failed: %s"),
		      SCErrorString(SCError()));
		goto error;
	}

	/*
	 * register for change notifications.
	 */
	if (!SCPreferencesSetCallback(prefs, updateConfiguration, NULL)) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("SCPreferencesSetCallBack() failed: %s"),
		      SCErrorString(SCError()));
		goto error;
	}

	if (!SCPreferencesScheduleWithRunLoop(prefs, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode)) {
		SCLog(TRUE, LOG_ERR,
		      CFSTR("SCPreferencesScheduleWithRunLoop() failed: %s"),
		      SCErrorString(SCError()));
		goto error;
	}

	/*
	 * if no preferences, initialize with a template (now or
	 * when IOKit has quiesced).
	 */
	if (initPrefs) {
		watchQuietEnable();
		watchQuietCallback(store, NULL, NULL);
	}

	return;

    error :

	watchQuietDisable();
	if (store != NULL)	CFRelease(store);
	if (prefs != NULL)	CFRelease(prefs);

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
