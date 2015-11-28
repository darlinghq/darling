/*
 * Copyright (c) 2000-2014 Apple Inc. All rights reserved.
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
 * October 30, 2003		Allan Nathanson <ajn@apple.com>
 * - add plugin "stop()" function support
 *
 * June 11, 2001		Allan Nathanson <ajn@apple.com>
 * - start using CFBundle code
 *
 * June 1, 2001			Allan Nathanson <ajn@apple.com>
 * - public API conversion
 *
 * May 26, 2000			Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sysexits.h>
#include <unistd.h>
#include <NSSystemDirectories.h>

#include "configd.h"
#include "configd_server.h"
#include <SystemConfiguration/SCDPlugin.h>
#include "SCNetworkReachabilityInternal.h"
void	_SCDPluginExecInit();


/*
 * path components, extensions, entry points, ...
 */
#define	BUNDLE_DIRECTORY	"/SystemConfiguration"	/* [/System/Library]/... */
#define	BUNDLE_DIR_EXTENSION	".bundle"


#define PLUGIN_ALL(p)		CFSTR(p)
#if	!TARGET_OS_IPHONE
#define PLUGIN_MACOSX(p)	CFSTR(p)
#define PLUGIN_IOS(p)		NULL
#else	// !TARGET_OS_IPHONE
#define PLUGIN_MACOSX(p)	NULL
#define PLUGIN_IOS(p)		CFSTR(p)
#endif	// !TARGET_OS_IPHONE

// white-listed (ok-to-load) bundle identifiers
static const CFStringRef	pluginWhitelist[]	= {
	PLUGIN_MACOSX("com.apple.SystemConfiguration.ApplicationFirewall"),
	PLUGIN_ALL   ("com.apple.SystemConfiguration.EAPOLController"),
	PLUGIN_ALL   ("com.apple.SystemConfiguration.IPConfiguration"),
	PLUGIN_ALL   ("com.apple.SystemConfiguration.IPMonitor"),
	PLUGIN_MACOSX("com.apple.SystemConfiguration.ISPreference"),
	PLUGIN_ALL   ("com.apple.SystemConfiguration.InterfaceNamer"),
	PLUGIN_ALL   ("com.apple.SystemConfiguration.KernelEventMonitor"),
	PLUGIN_ALL   ("com.apple.SystemConfiguration.LinkConfiguration"),
	PLUGIN_ALL   ("com.apple.SystemConfiguration.Logger"),
	PLUGIN_ALL   ("com.apple.SystemConfiguration.PPPController"),
	PLUGIN_ALL   ("com.apple.SystemConfiguration.PreferencesMonitor"),
	PLUGIN_ALL   ("com.apple.SystemConfiguration.SCNetworkReachability"),
	PLUGIN_MACOSX("com.apple.print.notification"),
};
#define	N_PLUGIN_WHITELIST	(sizeof(pluginWhitelist) / sizeof(pluginWhitelist[0]))


typedef struct {
	CFBundleRef				bundle;
	Boolean					loaded;
	Boolean					builtin;
	Boolean					enabled;
	Boolean					forced;
	Boolean					verbose;
	SCDynamicStoreBundleLoadFunction	load;
	SCDynamicStoreBundleStartFunction	start;
	SCDynamicStoreBundlePrimeFunction	prime;
	SCDynamicStoreBundleStopFunction	stop;
} *bundleInfoRef;


// all loaded bundles
static CFMutableArrayRef	allBundles		= NULL;

// exiting bundles
static CFMutableDictionaryRef	exiting			= NULL;

// plugin CFRunLoopRef
__private_extern__
CFRunLoopRef			plugin_runLoop		= NULL;


extern SCDynamicStoreBundleLoadFunction		load_IPMonitor;
extern SCDynamicStoreBundlePrimeFunction	prime_IPMonitor;
#if	!TARGET_IPHONE_SIMULATOR
extern SCDynamicStoreBundleLoadFunction		load_InterfaceNamer;
extern SCDynamicStoreBundleLoadFunction		load_KernelEventMonitor;
extern SCDynamicStoreBundlePrimeFunction	prime_KernelEventMonitor;
extern SCDynamicStoreBundleLoadFunction		load_LinkConfiguration;
extern SCDynamicStoreBundleLoadFunction		load_PreferencesMonitor;
extern SCDynamicStoreBundlePrimeFunction	prime_PreferencesMonitor;
#endif	// !TARGET_IPHONE_SIMULATOR
extern SCDynamicStoreBundleLoadFunction		load_SCNetworkReachability;


typedef struct {
	const CFStringRef	bundleID;
	const void		*load;		// SCDynamicStoreBundleLoadFunction
	const void		*start;		// SCDynamicStoreBundleStartFunction
	const void		*prime;		// SCDynamicStoreBundlePrimeFunction
	const void		*stop;		// SCDynamicStoreBundleStopFunction
} builtin, *builtinRef;


static const builtin builtin_plugins[] = {
	{
		CFSTR("com.apple.SystemConfiguration.IPMonitor"),
		&load_IPMonitor,
		NULL,
		&prime_IPMonitor,
		NULL
	},
#if	!TARGET_IPHONE_SIMULATOR
	{
		CFSTR("com.apple.SystemConfiguration.InterfaceNamer"),
		&load_InterfaceNamer,
		NULL,
		NULL,
		NULL
	},
	{
		CFSTR("com.apple.SystemConfiguration.KernelEventMonitor"),
		&load_KernelEventMonitor,
		NULL,
		&prime_KernelEventMonitor,
		NULL
	},
	{
		CFSTR("com.apple.SystemConfiguration.LinkConfiguration"),
		&load_LinkConfiguration,
		NULL,
		NULL,
		NULL
	},
	{
		CFSTR("com.apple.SystemConfiguration.PreferencesMonitor"),
		&load_PreferencesMonitor,
		NULL,
		&prime_PreferencesMonitor,
		NULL
	},
#endif	// !TARGET_IPHONE_SIMULATOR
	{
		CFSTR("com.apple.SystemConfiguration.SCNetworkReachability"),
		&load_SCNetworkReachability,
		NULL,
		NULL,
		NULL
	},
};


#ifdef	DEBUG
static void
traceBundle(const char *op, CFBundleRef bundle)
{
	if (_configd_trace != NULL) {
		if (bundle != NULL) {
			CFStringRef	bundleID	= CFBundleGetIdentifier(bundle);

			SCTrace(TRUE, _configd_trace,
				CFSTR("bundle  : %s %@\n"),
				op,
				bundleID);
		} else {
			SCTrace(TRUE, _configd_trace,
				CFSTR("bundle  : %s\n"),
				op);
		}
	}

	return;
}
#endif	/* DEBUG */


static void
addBundle(CFBundleRef bundle, Boolean forceEnabled)
{
	CFDictionaryRef		bundleDict;
	bundleInfoRef		bundleInfo;

	bundleInfo = CFAllocatorAllocate(NULL, sizeof(*bundleInfo), 0);
	bundleInfo->bundle	= (CFBundleRef)CFRetain(bundle);
	bundleInfo->loaded	= FALSE;
	bundleInfo->builtin	= FALSE;
	bundleInfo->enabled	= TRUE;
	bundleInfo->forced	= forceEnabled;
	bundleInfo->verbose	= FALSE;
	bundleInfo->load	= NULL;
	bundleInfo->start	= NULL;
	bundleInfo->prime	= NULL;
	bundleInfo->stop	= NULL;

	bundleDict = CFBundleGetInfoDictionary(bundle);
	if (isA_CFDictionary(bundleDict)) {
		CFBooleanRef	bVal;

		bVal = CFDictionaryGetValue(bundleDict, kSCBundleIsBuiltinKey);
		if (isA_CFBoolean(bVal)) {
			bundleInfo->builtin = CFBooleanGetValue(bVal);
		}

		bVal = CFDictionaryGetValue(bundleDict, kSCBundleEnabledKey);
		if (isA_CFBoolean(bVal)) {
			bundleInfo->enabled = CFBooleanGetValue(bVal);
		}

		bVal = CFDictionaryGetValue(bundleDict, kSCBundleVerboseKey);
		if (isA_CFBoolean(bVal)) {
			bundleInfo->verbose = CFBooleanGetValue(bVal);
		}
	}

	CFArrayAppendValue(allBundles, bundleInfo);
	return;
}


static CF_RETURNS_RETAINED CFStringRef
shortBundleIdentifier(CFStringRef bundleID)
{
	CFIndex		len	= CFStringGetLength(bundleID);
	CFRange		range;
	CFStringRef	shortID	= NULL;

	if (CFStringFindWithOptions(bundleID,
				    CFSTR("."),
				    CFRangeMake(0, len),
				    kCFCompareBackwards,
				    &range)) {
		range.location = range.location + range.length;
		range.length   = len - range.location;
		shortID = CFStringCreateWithSubstring(NULL, bundleID, range);
	}

	return shortID;
}


static void *
getBundleSymbol(CFBundleRef bundle, CFStringRef functionName, CFStringRef shortID)
{
	void	*func;

	// search for load(), start(), prime(), stop(), ...
	func = CFBundleGetFunctionPointerForName(bundle, functionName);
	if (func != NULL) {
		return func;
	}

	if (shortID != NULL) {
		CFStringRef	altFunctionName;

		// search for load_XXX(), ...
		altFunctionName = CFStringCreateWithFormat(NULL,
						    NULL,
						    CFSTR("%@_%@"),
						    functionName,
						    shortID);
		func = CFBundleGetFunctionPointerForName(bundle, altFunctionName);
		CFRelease(altFunctionName);
	}

	return func;
}


static const char *
getBundleDirNameAndPath(CFBundleRef bundle, char *buf, size_t buf_len)
{
	char		*cp;
	size_t		len;
	Boolean		ok;
	CFURLRef	url;

	url = CFBundleCopyBundleURL(bundle);
	if (url == NULL) {
		return NULL;
	}

	ok = CFURLGetFileSystemRepresentation(url, TRUE, (UInt8 *)buf, buf_len);
	CFRelease(url);
	if (!ok) {
		return NULL;
	}

	cp = strrchr(buf, '/');
	if (cp != NULL) {
		cp++;
	} else {
		cp = buf;
	}

	/* check if this directory entry is a valid bundle name */
	len = strlen(cp);
	if (len <= sizeof(BUNDLE_DIR_EXTENSION)) {
		/* if entry name isn't long enough */
		return NULL;
	}

	len -= sizeof(BUNDLE_DIR_EXTENSION) - 1;
	if (strcmp(&cp[len], BUNDLE_DIR_EXTENSION) != 0) {
		/* if entry name doesn't end with ".bundle" */
		return NULL;
	}

	return cp;
}


#pragma mark -
#pragma mark load


static void
loadBundle(const void *value, void *context) {
	CFStringRef	bundleID;
	Boolean		bundleAllowed;
	bundleInfoRef	bundleInfo	= (bundleInfoRef)value;
	Boolean		bundleExclude;
	CFIndex		*nLoaded	= (CFIndex *)context;
	CFStringRef	shortID;

	bundleID = CFBundleGetIdentifier(bundleInfo->bundle);
	if (bundleID == NULL) {
		// sorry, no bundles without a bundle identifier
		SCLog(TRUE, LOG_NOTICE, CFSTR("skipped %@ (no bundle ID)"), bundleInfo->bundle);
		return;
	}

	shortID = shortBundleIdentifier(bundleID);

	bundleAllowed = ((CFSetGetCount(_plugins_allowed) == 0)		||	// if no white-listing
			 CFSetContainsValue(_plugins_allowed, bundleID)	||	// if [bundleID] white-listed
			 ((shortID != NULL) &&
			  CFSetContainsValue(_plugins_allowed, shortID))||	// if [short bundleID] white-listed
			 bundleInfo->forced					// if "testing" plugin
			);
	if (!bundleAllowed) {
		SCLog(TRUE, LOG_WARNING, CFSTR("skipped %@ (not allowed)"), bundleID);
		goto done;
	}

	bundleExclude = (CFSetContainsValue(_plugins_exclude, bundleID)	||	// if [bundleID] excluded
			 ((shortID != NULL) &&
			  CFSetContainsValue(_plugins_exclude, shortID))	// if [short bundleID] excluded
			);
	if (bundleExclude) {
		// sorry, this bundle has been excluded
		SCLog(TRUE, LOG_NOTICE, CFSTR("skipped %@ (excluded)"), bundleID);
		goto done;
	}

	if (!bundleInfo->enabled && !bundleInfo->forced) {
		// sorry, this bundle has not been enabled
		SCLog(TRUE, LOG_INFO, CFSTR("skipped %@ (disabled)"), bundleID);
		goto done;
	}

	if (!bundleInfo->verbose) {
		bundleInfo->verbose = CFSetContainsValue(_plugins_verbose, bundleID);
		if (!bundleInfo->verbose) {
			if (shortID != NULL) {
				bundleInfo->verbose = CFSetContainsValue(_plugins_verbose, shortID);
			}
		}
	}

	if (bundleInfo->builtin) {
		int		i;

		SCLog(TRUE, LOG_DEBUG, CFSTR("adding  %@"), bundleID);

		for (i = 0; i < sizeof(builtin_plugins)/sizeof(builtin_plugins[0]); i++) {
			if (CFEqual(bundleID, builtin_plugins[i].bundleID)) {
				bundleInfo->load  = builtin_plugins[i].load;
				bundleInfo->start = builtin_plugins[i].start;
				bundleInfo->prime = builtin_plugins[i].prime;
				bundleInfo->stop  = builtin_plugins[i].stop;
				break;
			}
		}

		if ((bundleInfo->load  == NULL) &&
		    (bundleInfo->start == NULL) &&
		    (bundleInfo->prime == NULL) &&
		    (bundleInfo->stop  == NULL)) {
			SCLog(TRUE, LOG_NOTICE, CFSTR("%@ add failed"), bundleID);
			goto done;
		}
	} else {
		CFErrorRef	error	= NULL;

		SCLog(TRUE, LOG_DEBUG, CFSTR("loading %@"), bundleID);

#ifdef	DEBUG
		traceBundle("loading", bundleInfo->bundle);
#endif	/* DEBUG */

		if (!CFBundleLoadExecutableAndReturnError(bundleInfo->bundle, &error)) {
			CFDictionaryRef	user_info;
			
			SCLog(TRUE, LOG_NOTICE, CFSTR("%@ load failed"), bundleID);
			user_info = CFErrorCopyUserInfo(error);
			if (user_info != NULL) {
				CFStringRef	link_error_string;

				link_error_string = CFDictionaryGetValue(user_info,
									 CFSTR("NSDebugDescription"));
				if (link_error_string != NULL) {
					SCLog(TRUE, LOG_NOTICE, CFSTR("%@"), link_error_string);
				}
				CFRelease(user_info);
			}
			CFRelease(error);
			goto done;
		}

		// get bundle entry points
		bundleInfo->load  = getBundleSymbol(bundleInfo->bundle, CFSTR("load" ), shortID);
		bundleInfo->start = getBundleSymbol(bundleInfo->bundle, CFSTR("start"), shortID);
		bundleInfo->prime = getBundleSymbol(bundleInfo->bundle, CFSTR("prime"), shortID);
		bundleInfo->stop  = getBundleSymbol(bundleInfo->bundle, CFSTR("stop" ), shortID);
	}

	/* mark this bundle as having been loaded */
	bundleInfo->loaded = TRUE;

	/* bump the count of loaded bundles */
	*nLoaded = *nLoaded + 1;

    done :

	if (shortID != NULL)	CFRelease(shortID);
	return;
}


void
callLoadFunction(const void *value, void *context) {
	bundleInfoRef	bundleInfo	= (bundleInfoRef)value;

	if (!bundleInfo->loaded) {
		return;
	}

	if (bundleInfo->load == NULL) {
		// if no load() function
		return;
	}

#ifdef	DEBUG
	traceBundle("calling load() for", bundleInfo->bundle);
#endif	/* DEBUG */

	(*bundleInfo->load)(bundleInfo->bundle, bundleInfo->verbose);

	return;
}


#pragma mark -
#pragma mark start


void
callStartFunction(const void *value, void *context) {
	const char	*bundleDirName;
	bundleInfoRef	bundleInfo	= (bundleInfoRef)value;
	char		bundleName[MAXNAMLEN + 1];
	char		bundlePath[MAXPATHLEN];
	size_t		len;

	if (!bundleInfo->loaded) {
		return;
	}

	if (bundleInfo->start == NULL) {
		// if no start() function
		return;
	}

	/* copy the bundle's path */
	bundleDirName = getBundleDirNameAndPath(bundleInfo->bundle, bundlePath, sizeof(bundlePath));
	if (bundleDirName == NULL) {
		// if we have a problem with the bundle's path
		return;
	}

	/* copy (just) the bundle's name */
	if (strlcpy(bundleName, bundleDirName, sizeof(bundleName)) > sizeof(bundleName)) {
		// if we have a problem with the bundle's name
		return;
	}
	len = strlen(bundleName) - (sizeof(BUNDLE_DIR_EXTENSION) - 1);
	bundleName[len] = '\0';

#ifdef	DEBUG
	traceBundle("calling start() for", bundleInfo->bundle);
#endif	/* DEBUG */

	(*bundleInfo->start)(bundleName, bundlePath);

	return;
}


#pragma mark -
#pragma mark prime


void
callPrimeFunction(const void *value, void *context) {
	bundleInfoRef	bundleInfo	= (bundleInfoRef)value;

	if (!bundleInfo->loaded) {
		return;
	}

	if (bundleInfo->prime == NULL) {
		// if no prime() function
		return;
	}

#ifdef	DEBUG
	traceBundle("calling prime() for", bundleInfo->bundle);
#endif	/* DEBUG */

	(*bundleInfo->prime)();

	return;
}


#pragma mark -
#pragma mark stop


static void
stopComplete(void *info)
{
	CFBundleRef		bundle		= (CFBundleRef)info;
	CFStringRef		bundleID	= CFBundleGetIdentifier(bundle);
	CFRunLoopSourceRef	stopRls;

	SCLog(TRUE, LOG_DEBUG, CFSTR("** %@ complete (%f)"), bundleID, CFAbsoluteTimeGetCurrent());

	stopRls = (CFRunLoopSourceRef)CFDictionaryGetValue(exiting, bundle);
	if (stopRls == NULL) {
		return;
	}

	CFRunLoopSourceInvalidate(stopRls);

	CFDictionaryRemoveValue(exiting, bundle);

	if (CFDictionaryGetCount(exiting) == 0) {
		int	status;

		// if all of the plugins are happy
		status = server_shutdown();
		SCLog(TRUE, LOG_DEBUG, CFSTR("server shutdown complete (%f)"), CFAbsoluteTimeGetCurrent());
		exit (status);
	}

	return;
}


static void
stopDelayed(CFRunLoopTimerRef timer, void *info)
{
	const void	**keys;
	CFIndex		i;
	CFIndex		n;
	int		status;

	SCLog(TRUE, LOG_ERR, CFSTR("server shutdown was delayed, unresponsive plugins:"));

	/*
	 * we've asked our plugins to shutdown but someone
	 * isn't listening.
	 */
	n = CFDictionaryGetCount(exiting);
	keys = CFAllocatorAllocate(NULL, n * sizeof(CFTypeRef), 0);
	CFDictionaryGetKeysAndValues(exiting, keys, NULL);
	for (i = 0; i < n; i++) {
		CFBundleRef	bundle;
		CFStringRef	bundleID;

		bundle   = (CFBundleRef)keys[i];
		bundleID = CFBundleGetIdentifier(bundle);
		SCLog(TRUE, LOG_ERR, CFSTR("** %@"), bundleID);
	}
	CFAllocatorDeallocate(NULL, keys);

	status = server_shutdown();
	exit (status);
}

static CFStringRef
stopRLSCopyDescription(const void *info)
{
	CFBundleRef	bundle	= (CFBundleRef)info;

	return CFStringCreateWithFormat(NULL,
					NULL,
					CFSTR("<stopRLS %p> {bundleID = %@}"),
					info,
					CFBundleGetIdentifier(bundle));
}


static void
stopBundle(const void *value, void *context) {
	bundleInfoRef			bundleInfo	= (bundleInfoRef)value;
	CFRunLoopSourceRef		stopRls;
	CFRunLoopSourceContext		stopContext	= { 0				// version
							  , bundleInfo->bundle		// info
							  , CFRetain			// retain
							  , CFRelease			// release
							  , stopRLSCopyDescription	// copyDescription
							  , CFEqual			// equal
							  , CFHash			// hash
							  , NULL			// schedule
							  , NULL			// cancel
							  , stopComplete		// perform
							  };

	if (!bundleInfo->loaded) {
		return;
	}

	if (bundleInfo->stop == NULL) {
		// if no stop() function
		return;
	}

	stopRls = CFRunLoopSourceCreate(NULL, 0, &stopContext);
	CFRunLoopAddSource(CFRunLoopGetCurrent(), stopRls, kCFRunLoopDefaultMode);
	CFDictionaryAddValue(exiting, bundleInfo->bundle, stopRls);
	CFRelease(stopRls);

	(*bundleInfo->stop)(stopRls);

	return;
}


static void
stopBundles()
{
	/*
	 * If defined, call each bundles stop() function.  This function is
	 * called when configd has been asked to shut down (via a SIGTERM).  The
	 * function should signal the provided run loop source when it is "ready"
	 * for the shut down to proceeed.
	 */
	SCLog(_configd_verbose, LOG_DEBUG, CFSTR("calling bundle stop() functions"));
	CFArrayApplyFunction(allBundles,
			     CFRangeMake(0, CFArrayGetCount(allBundles)),
			     stopBundle,
			     NULL);

	if (CFDictionaryGetCount(exiting) == 0) {
		int	status;

		// if all of the plugins are happy
		status = server_shutdown();
		SCLog(TRUE, LOG_DEBUG, CFSTR("server shutdown complete (%f)"), CFAbsoluteTimeGetCurrent());
		exit (status);
	} else {
		CFRunLoopTimerRef	timer;

		/*
		 * launchd will only wait 20 seconds before sending us a
		 * SIGKILL and because we want to know what's stuck before
		 * that time so set our own "we're not waiting any longer"
		 * timeout for 15 seconds.
		 */
		timer = CFRunLoopTimerCreate(NULL,				/* allocator */
					     CFAbsoluteTimeGetCurrent() + 15.0,	/* fireDate (in 15 seconds) */
					     0.0,				/* interval (== one-shot) */
					     0,					/* flags */
					     0,					/* order */
					     stopDelayed,			/* callout */
					     NULL);				/* context */
		CFRunLoopAddTimer(CFRunLoopGetCurrent(), timer, kCFRunLoopDefaultMode);
		CFRelease(timer);
	}

	return;
}


#pragma mark -
#pragma mark term


static CFStringRef
termRLSCopyDescription(const void *info)
{
	return CFStringCreateWithFormat(NULL, NULL, CFSTR("<SIGTERM RLS>"));
}


__private_extern__
Boolean
plugin_term(int *status)
{
	CFRunLoopSourceContext	termContext = { 0			// version
					      , (void *)1		// info
					      , NULL			// retain
					      , NULL			// release
					      , termRLSCopyDescription	// copyDescription
					      , NULL			// equal
					      , NULL			// hash
					      , NULL			// schedule
					      , NULL			// cancel
					      , stopBundles		// perform
					      };
	CFRunLoopSourceRef	termRls;

	if (plugin_runLoop == NULL) {
		// if no plugins
		*status = EX_OK;
		return FALSE;	// don't delay shutdown
	}

	if (exiting != NULL) {
		// if shutdown already active
		return TRUE;
	}

	SCLog(TRUE, LOG_DEBUG, CFSTR("starting server shutdown (%f)"), CFAbsoluteTimeGetCurrent());

	exiting = CFDictionaryCreateMutable(NULL,
					    0,
					    &kCFTypeDictionaryKeyCallBacks,
					    &kCFTypeDictionaryValueCallBacks);

	termRls = CFRunLoopSourceCreate(NULL, 0, &termContext);
	CFRunLoopAddSource(plugin_runLoop, termRls, kCFRunLoopDefaultMode);
	CFRunLoopSourceSignal(termRls);
	CFRelease(termRls);
	CFRunLoopWakeUp(plugin_runLoop);

	return TRUE;
}


#pragma mark -
#pragma mark initialization


#ifdef	DEBUG
static void
timerCallback(CFRunLoopTimerRef timer, void *info)
{
	static int	pass	= 0;

	pass++;
	if ((pass > 120) && ((pass % 60) != 0)) {
		return;
	}

	traceBundle("the [plugin] CFRunLoop is waiting...", NULL);
	return;
}
#endif	/* DEBUG */


static void
sortBundles(CFMutableArrayRef orig)
{
	CFIndex			i;
	CFIndex			n;
	CFMutableArrayRef	new;
	CFMutableSetRef		orig_bundleIDs;

	orig_bundleIDs = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);

	n = CFArrayGetCount(orig);
	for (i = 0; i < n; i++) {
		bundleInfoRef	bundleInfo	= (bundleInfoRef)CFArrayGetValueAtIndex(orig, i);
		CFStringRef	bundleID	= CFBundleGetIdentifier(bundleInfo->bundle);

		if (bundleID != NULL) {
			CFSetAddValue(orig_bundleIDs, bundleID);
		}
	}

	new = CFArrayCreateMutable(NULL, 0, NULL);
	while (n > 0) {
		Boolean	inserted	= FALSE;

		for (i = 0; i < n; i++) {
			bundleInfoRef	bundleInfo1	= (bundleInfoRef)CFArrayGetValueAtIndex(orig, i);
			CFStringRef	bundleID1	= CFBundleGetIdentifier(bundleInfo1->bundle);
			CFIndex		count;
			CFDictionaryRef	dict;
			CFIndex		j;
			CFIndex		nRequires;
			CFArrayRef	requires  = NULL;

			dict = isA_CFDictionary(CFBundleGetInfoDictionary(bundleInfo1->bundle));
			if (dict) {
				requires = CFDictionaryGetValue(dict, kSCBundleRequiresKey);
				requires = isA_CFArray(requires);
			}
			if (bundleID1 == NULL || requires == NULL) {
				CFArrayInsertValueAtIndex(new, 0, bundleInfo1);
				CFArrayRemoveValueAtIndex(orig, i);
				inserted = TRUE;
				break;
			}
			count = nRequires = CFArrayGetCount(requires);
			for (j = 0; j < nRequires; j++) {
				CFIndex		k;
				CFIndex		nNew;
				CFStringRef	r	= CFArrayGetValueAtIndex(requires, j);

				if (!CFSetContainsValue(orig_bundleIDs, r)) {
					// if dependency not present
					count--;
					continue;
				}

				nNew = CFArrayGetCount(new);
				for (k = 0; k < nNew; k++) {
					bundleInfoRef	bundleInfo2	= (bundleInfoRef)CFArrayGetValueAtIndex(new, k);
					CFStringRef	bundleID2	= CFBundleGetIdentifier(bundleInfo2->bundle);

					if (bundleID2 && CFEqual(bundleID2, r)) {
						count--;
					}
				}
			}
			if (count == 0) {
				/* all dependencies are met, append */
				CFArrayAppendValue(new, bundleInfo1);
				CFArrayRemoveValueAtIndex(orig, i);
				inserted = TRUE;
				break;
			}
		}

		if (inserted == FALSE) {
			SCLog(TRUE, LOG_NOTICE, CFSTR("Bundles have circular dependency!!!"));
			break;
		}

		n = CFArrayGetCount(orig);
	}
	if (CFArrayGetCount(orig) > 0) {
		/* we have a circular dependency, append remaining items on new array */
		CFArrayAppendArray(new, orig, CFRangeMake(0, CFArrayGetCount(orig)));
	}
	else {
		/* new one is a sorted version of original */
	}

	CFArrayRemoveAllValues(orig);
	CFArrayAppendArray(orig, new, CFRangeMake(0, CFArrayGetCount(new)));
	CFRelease(new);
	CFRelease(orig_bundleIDs);
	return;
}


/*
 * ALT_CFRelease()
 *
 * An alternate CFRelease() that we can use to fake out the
 * static analyzer.
 */
static __inline__ void
ALT_CFRelease(CFTypeRef cf)
{
	CFRelease(cf);
}


__private_extern__
void *
plugin_exec(void *arg)
{
	int		i;
	CFIndex		nLoaded		= 0;

	/* keep track of bundles */
	allBundles = CFArrayCreateMutable(NULL, 0, NULL);

	/* add white-listed plugins to those we'll allow to be loaded */
	for (i = 0; i < N_PLUGIN_WHITELIST; i++) {
		if (pluginWhitelist[i] != NULL) {
			CFSetSetValue(_plugins_allowed, pluginWhitelist[i]);
		}
	}

	/* allow plug-ins to exec child/helper processes */
	_SCDPluginExecInit();

	if (arg == NULL) {
		char				path[MAXPATHLEN];
		NSSearchPathEnumerationState	state;

		/*
		 * identify and load all bundles
		 */
		state = NSStartSearchPathEnumeration(NSLibraryDirectory,
						     NSSystemDomainMask);
		while ((state = NSGetNextSearchPathEnumeration(state, path))) {
			CFArrayRef	bundles;
			CFURLRef	url;

#if	TARGET_IPHONE_SIMULATOR
			const char	*path_sim_prefix;

			path_sim_prefix = getenv("IPHONE_SIMULATOR_ROOT");
			if ((path_sim_prefix != NULL) && (strcmp(path_sim_prefix, ".") != 0)) {
				char	path_sim[MAXPATHLEN];

				strlcpy(path_sim, path_sim_prefix, sizeof(path_sim));
				strlcat(path_sim, path, sizeof(path_sim));
				strlcpy(path, path_sim, sizeof(path));
			} else {
				path[0] = '\0';
			}
#endif	// TARGET_IPHONE_SIMULATOR

			/* load any available bundle */
			strlcat(path, BUNDLE_DIRECTORY, sizeof(path));
			SCLog(_configd_verbose, LOG_DEBUG, CFSTR("searching for bundles in \"%s\""), path);
			url = CFURLCreateFromFileSystemRepresentation(NULL,
								      (UInt8 *)path,
								      strlen(path),
								      TRUE);
			bundles = CFBundleCreateBundlesFromDirectory(NULL, url, CFSTR(".bundle"));
			CFRelease(url);

			if (bundles != NULL) {
				CFIndex	i;
				CFIndex	n;

				n = CFArrayGetCount(bundles);
				for (i = 0; i < n; i++) {
					CFBundleRef	bundle;

					bundle = (CFBundleRef)CFArrayGetValueAtIndex(bundles, i);
					addBundle(bundle, FALSE);

					// The CFBundleCreateBundlesFromDirectory() API has
					// a known/outstanding bug in that it over-retains the
					// returned bundles.  Since we do not expect this to
					// be fixed we release the extra references.
					//
					//   See <rdar://problems/4912137&6078752> for more info.
					//
					// Also, we use the hack below to keep the static
					// analyzer happy.
					ALT_CFRelease(bundle);
				}
				CFRelease(bundles);
			}
		}

		sortBundles(allBundles);
	} else {
		CFBundleRef	bundle;
		CFURLRef	url;

		/*
		 * load (only) the specified bundle
		 */
		url = CFURLCreateFromFileSystemRepresentation(NULL,
							      (UInt8 *)arg,
							      strlen((char *)arg),
							      TRUE);
		bundle = CFBundleCreate(NULL, url);
		if (bundle != NULL) {
			addBundle(bundle, TRUE);
			CFRelease(bundle);
		}
		CFRelease(url);
	}

	/*
	 * Look for the InterfaceNamer plugin, and move it to the start
	 * of the list.
	 *
	 * Load the InterfaceNamer plugin (and thereby start its thread)
	 * first in an attempt to minimize the amount of time that
	 * opendirectoryd has to wait for the platform UUID to appear in
	 * nvram.
	 *
	 * InterfaceNamer is responsible for creating the platform UUID on
	 * platforms without a UUID in ROM. Until the platform UUID is created
	 * and stashed in nvram, all calls to opendirectoryd to do things like
	 * getpwuid() will block, because opendirectoryd will block while trying
	 * to read the platform UUID from the kernel.
	 *
	 * As an example, dlopen() causes XPC to do some intialization, and
	 * part of that initialization involves communicating with xpcd.
	 * Since xpcd calls getpwuid_r() during its initialization, it will
	 * block until the platform UUID is available.
	 */
	for (i = 0; i < CFArrayGetCount(allBundles); i++) {
		bundleInfoRef	bi		= (bundleInfoRef)CFArrayGetValueAtIndex(allBundles, i);
		CFStringRef	bundleID	= CFBundleGetIdentifier(bi->bundle);

		if (_SC_CFEqual(bundleID,
				CFSTR("com.apple.SystemConfiguration.InterfaceNamer")))
		{
			CFArrayRemoveValueAtIndex(allBundles, i);
			CFArrayInsertValueAtIndex(allBundles, 0, bi);
			break;
		}
	}

#ifdef	DEBUG
	traceBundle("before loading any plugins", NULL);
#endif	/* DEBUG */

	/*
	 * load each bundle.
	 */
	SCLog(_configd_verbose, LOG_DEBUG, CFSTR("loading bundles"));
	CFArrayApplyFunction(allBundles,
			     CFRangeMake(0, CFArrayGetCount(allBundles)),
			     loadBundle,
			     &nLoaded);

	/*
	 * If defined, call each bundles load() function.  This function (or
	 * the start() function) should initialize any variables, open any
	 * sessions with "configd", and register any needed notifications.
	 *
	 * Note: Establishing initial information in the store should be
	 *       deferred until the prime() initialization function so that
	 *       any bundles which want to receive a notification that the
	 *       data has changed will have an opportunity to install a
	 *       notification handler.
	 */
	SCLog(_configd_verbose, LOG_DEBUG, CFSTR("calling bundle load() functions"));
	CFArrayApplyFunction(allBundles,
			     CFRangeMake(0, CFArrayGetCount(allBundles)),
			     callLoadFunction,
			     NULL);

	if (nLoaded == 0) {
		// if no bundles loaded
		goto done;
	}

	/*
	 * If defined, call each bundles start() function.  This function is
	 * called after the bundle has been loaded and its load() function has
	 * been called.  It should initialize any variables, open any sessions
	 * with "configd", and register any needed notifications.
	 *
	 * Note: Establishing initial information in the store should be
	 *       deferred until the prime() initialization function so that
	 *       any bundles which want to receive a notification that the
	 *       data has changed will have an opportunity to install a
	 *       notification handler.
	 */
	SCLog(_configd_verbose, LOG_DEBUG, CFSTR("calling bundle start() functions"));
	CFArrayApplyFunction(allBundles,
			     CFRangeMake(0, CFArrayGetCount(allBundles)),
			     callStartFunction,
			     NULL);

	/*
	 * If defined, call each bundles prime() function.  This function is
	 * called after the bundle has been loaded and its load() and start()
	 * functions have been called.  It should initialize any configuration
	 * information and/or state in the store.
	 */
	SCLog(_configd_verbose, LOG_DEBUG, CFSTR("calling bundle prime() functions"));
	CFArrayApplyFunction(allBundles,
			     CFRangeMake(0, CFArrayGetCount(allBundles)),
			     callPrimeFunction,
			     NULL);

#ifdef	DEBUG
	if (arg == NULL && (nLoaded > 0)) {
		CFRunLoopTimerRef	timer;

		/* allocate a periodic event (to help show we're not blocking) */
		timer = CFRunLoopTimerCreate(NULL,				/* allocator */
					     CFAbsoluteTimeGetCurrent() + 1.0,	/* fireDate */
					     1.0,				/* interval */
					     0,					/* flags */
					     0,					/* order */
					     timerCallback,			/* callout */
					     NULL);				/* context */
		CFRunLoopAddTimer(CFRunLoopGetCurrent(), timer, kCFRunLoopDefaultMode);
		CFRelease(timer);
	}
#endif	/* DEBUG */

#ifdef	DEBUG
	traceBundle("about to start plugin CFRunLoop", NULL);
#endif	/* DEBUG */

	/*
	 * The assumption is that each loaded plugin will establish CFMachPortRef,
	 * CFSocketRef, and CFRunLoopTimerRef input sources to handle any events
	 * and register these sources with this threads run loop. If the plugin
	 * needs to wait and/or block at any time it should do so only in its a
	 * private thread.
	 */
	SCLog(_configd_verbose, LOG_DEBUG, CFSTR("starting plugin CFRunLoop"));
	plugin_runLoop = CFRunLoopGetCurrent();
	pthread_setname_np("Main plugin thread");
	CFRunLoopRun();

    done :

	SCLog(_configd_verbose, LOG_INFO, CFSTR("No more work for the \"configd\" plugins"));
	plugin_runLoop = NULL;
	return NULL;
}


__private_extern__
void
plugin_init()
{
	pthread_attr_t	tattr;
	pthread_t	tid;

	SCLog(_configd_verbose, LOG_DEBUG, CFSTR("Starting thread for plug-ins..."));
	pthread_attr_init(&tattr);
	pthread_attr_setscope(&tattr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
//      pthread_attr_setstacksize(&tattr, 96 * 1024); // each thread gets a 96K stack
	pthread_create(&tid, &tattr, plugin_exec, NULL);
	pthread_attr_destroy(&tattr);
	SCLog(_configd_verbose, LOG_DEBUG, CFSTR("  thread id=%p"), tid);

	return;
}
