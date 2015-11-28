/*
 * Copyright (c) 2003-2008, 2011-2014 Apple Inc. All rights reserved.
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
 * May 29, 2003			Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>

#include <SystemConfiguration/SCPreferencesSetSpecific.h>
#if	!TARGET_OS_IPHONE
#include <Security/Authorization.h>
#endif	/* !TARGET_OS_IPHONE */

#include "scutil.h"
#include "commands.h"
#include "prefs.h"


/* -------------------- */


#if	!TARGET_OS_IPHONE
static void *
__loadSecurity(void) {
	static void *image = NULL;
	if (NULL == image) {
		const char	*framework		= "/System/Library/Frameworks/Security.framework/Security";
		struct stat	statbuf;
		const char	*suffix			= getenv("DYLD_IMAGE_SUFFIX");
		char		path[MAXPATHLEN];

		strlcpy(path, framework, sizeof(path));
		if (suffix) strlcat(path, suffix, sizeof(path));
		if (0 <= stat(path, &statbuf)) {
			image = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
		} else {
			image = dlopen(framework, RTLD_LAZY | RTLD_LOCAL);
		}
	}
	return (void *)image;
}


static OSStatus
_AuthorizationCreate(const AuthorizationRights *rights, const AuthorizationEnvironment *environment, AuthorizationFlags flags, AuthorizationRef *authorization)
{
	#undef AuthorizationCreate
	static typeof (AuthorizationCreate) *dyfunc = NULL;
	if (dyfunc == NULL) {
		void *image = __loadSecurity();
		if (image) dyfunc = dlsym(image, "AuthorizationCreate");
	}
	return dyfunc ? dyfunc(rights, environment, flags, authorization) : -1;
}
#define AuthorizationCreate _AuthorizationCreate


static OSStatus
_AuthorizationFree(AuthorizationRef authorization, AuthorizationFlags flags)
{
	#undef AuthorizationFree
	static typeof (AuthorizationFree) *dyfunc = NULL;
	if (dyfunc == NULL) {
		void *image = __loadSecurity();
		if (image) dyfunc = dlsym(image, "AuthorizationFree");
	}
	return dyfunc ? dyfunc(authorization, flags) : -1;
}
#define AuthorizationFree _AuthorizationFree


/* -------------------- */

__private_extern__
AuthorizationRef
_prefs_AuthorizationCreate()
{
	AuthorizationRef	authorization	= NULL;

	if (getenv("SCPREFERENCES_USE_ENTITLEMENTS") != NULL) {
		authorization = kSCPreferencesUseEntitlementAuthorization;
	} else {
		AuthorizationFlags	flags	= kAuthorizationFlagDefaults;
		OSStatus		status;

		status = AuthorizationCreate(NULL,
					     kAuthorizationEmptyEnvironment,
					     flags,
					     &authorization);
		if (status != errAuthorizationSuccess) {
			SCPrint(TRUE,
				stdout,
				CFSTR("AuthorizationCreate() failed: status = %d\n"),
				(int)status);
			return NULL;
		}
	}

	return authorization;
}


__private_extern__
void
_prefs_AuthorizationFree(AuthorizationRef authorization)
{
	if (authorization != kSCPreferencesUseEntitlementAuthorization) {
		AuthorizationFree(authorization, kAuthorizationFlagDefaults);
//		AuthorizationFree(authorization, kAuthorizationFlagDestroyRights);
	}

	return;
}

#endif	/* !TARGET_OS_IPHONE */

/* -------------------- */


__private_extern__ Boolean	_prefs_changed	= FALSE;


__private_extern__
Boolean
_prefs_open(CFStringRef name, CFStringRef prefsID)
{
	CFMutableDictionaryRef	options		= NULL;
	Boolean			useHelper	= FALSE;
	Boolean			useOptions	= FALSE;

	authorization = NULL;

	if (geteuid() != 0) {
		// if we need to use a helper
		useHelper = TRUE;

#if	!TARGET_OS_IPHONE
		authorization = _prefs_AuthorizationCreate();
#else
		authorization = kSCPreferencesUseEntitlementAuthorization;
#endif	/* !TARGET_OS_IPHONE */
	}

	if (getenv("SCPREFERENCES_REMOVE_WHEN_EMPTY") != NULL) {
		// if we have options
		useOptions = TRUE;

		if (options == NULL) {
			options = CFDictionaryCreateMutable(NULL,
							    0,
							    &kCFTypeDictionaryKeyCallBacks,
							    &kCFTypeDictionaryValueCallBacks);
		}
		CFDictionarySetValue(options, kSCPreferencesOptionRemoveWhenEmpty, kCFBooleanTrue);
	}

	if (!useHelper && !useOptions) {
		// if no helper/options needed
		prefs = SCPreferencesCreate(NULL, name, prefsID);
	} else if (!useOptions) {
		// if no options needed
		prefs = SCPreferencesCreateWithAuthorization(NULL, name, prefsID, authorization);
	} else {
		prefs = SCPreferencesCreateWithOptions(NULL, name, prefsID, authorization, options);
		CFRelease(options);
	}

	if (prefs == NULL) {
		return FALSE;
	}

	_prefs_changed = FALSE;
	return TRUE;
}


__private_extern__
void
_prefs_save()
{
	if (!SCPreferencesCommitChanges(prefs)) {
		switch (SCError()) {
			case kSCStatusAccessError :
				SCPrint(TRUE, stderr, CFSTR("Permission denied.\n"));
				break;
			default :
				SCPrint(TRUE,
					stdout,
					CFSTR("SCPreferencesCommitChanges() failed: %s\n"),
					SCErrorString(SCError()));
				break;
		}
		exit (1);
	}

	_prefs_changed = FALSE;

	if (!SCPreferencesApplyChanges(prefs)) {
		SCPrint(TRUE,
			stdout,
			CFSTR("SCPreferencesApplyChanges() failed: %s\n"),
			SCErrorString(SCError()));
		exit (1);
	}

	return;
}


__private_extern__
void
_prefs_close()
{
	if (prefs != NULL) {
		CFRelease(prefs);
		prefs = NULL;
		_prefs_changed = FALSE;
	}

	if (authorization != NULL) {
#if	!TARGET_OS_IPHONE
		_prefs_AuthorizationFree(authorization);
#else	/* !TARGET_OS_IPHONE */
		// Uh...if authorization isn't NULL, something went horribly wrong.
#endif	/* !TARGET_OS_IPHONE */
		authorization = NULL;
	}

	return;
}


__private_extern__
Boolean
_prefs_commitRequired(int argc, char **argv, const char *command)
{
	if (_prefs_changed) {
		if ((currentInput != NULL)			&&
		    isatty(fileno(currentInput->fp))		&&
		    ((argc < 1) || (strcmp(argv[0], "!") != 0))
		   ) {
			SCPrint(TRUE, stdout,
				CFSTR("preference changes have not been committed\n"
				      "use \"commit\" to save changes"));
			if (command != NULL) {
				SCPrint(TRUE, stdout,
					CFSTR(" or \"%s !\" to abandon changes"),
					command);
			}
			SCPrint(TRUE, stdout, CFSTR("\n"));
			return TRUE;
		}

		SCPrint(TRUE, stdout, CFSTR("preference changes abandoned\n"));
	}

	return FALSE;
}


/* -------------------- */


static void
get_ComputerName(int argc, char **argv)
{
	CFStringEncoding	encoding;
	CFStringRef		hostname;

	hostname = SCDynamicStoreCopyComputerName(NULL, &encoding);
	if (hostname == NULL) {
		int	sc_status	= SCError();

		switch (sc_status) {
			case kSCStatusNoKey :
				SCPrint(TRUE,
					stderr,
					CFSTR("ComputerName: not set\n"));
				break;
			default :
				SCPrint(TRUE,
					stderr,
					CFSTR("SCDynamicStoreCopyComputerName() failed: %s\n"),
					SCErrorString(SCError()));
				break;
		}
		exit (1);
	}

	SCPrint(TRUE, stdout, CFSTR("%@\n"), hostname);
	CFRelease(hostname);

	exit(0);
}


static void
set_ComputerName(int argc, char **argv)
{
	CFStringRef	hostname	= NULL;
	Boolean		ok;

	ok = _prefs_open(CFSTR("scutil --set ComputerName"), NULL);
	if (!ok) {
		SCPrint(TRUE,
			stdout,
			CFSTR("Could not open prefs: %s\n"),
			SCErrorString(SCError()));
		exit(1);
	}

	if (argc == 0) {
		CFStringEncoding	old_encoding;
		CFStringRef		old_hostname;

		old_hostname = SCDynamicStoreCopyComputerName(NULL, &old_encoding);
		hostname = _copyStringFromSTDIN(CFSTR("ComputerName"), old_hostname);
		if (old_hostname) CFRelease(old_hostname);
	} else if (strlen(argv[0]) > 0) {
		hostname = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
	}

	ok = SCPreferencesSetComputerName(prefs, hostname, kCFStringEncodingUTF8);
	if (hostname != NULL) CFRelease(hostname);
	if (!ok) {
		SCPrint(TRUE,
			stdout,
			CFSTR("Could not open prefs: %s\n"),
			SCErrorString(SCError()));
		_prefs_close();
		exit (1);
	}

	_prefs_save();
	_prefs_close();
	exit(0);
}


static void
get_HostName(int argc, char **argv)
{
	CFStringRef	hostname;
	Boolean		ok;

	ok = _prefs_open(CFSTR("scutil --get HostName"), NULL);
	if (!ok) {
		SCPrint(TRUE,
			stdout,
			CFSTR("SCPreferencesCreate() failed: %s\n"),
			SCErrorString(SCError()));
		exit(1);
	}

	hostname = SCPreferencesGetHostName(prefs);
	if (hostname == NULL) {
		int	sc_status	= SCError();

		switch (sc_status) {
			case kSCStatusNoKey :
				SCPrint(TRUE,
					stderr,
					CFSTR("HostName: not set\n"));
				break;
			default :
				SCPrint(TRUE,
					stderr,
					CFSTR("SCPreferencesGetHostName() failed: %s\n"),
					SCErrorString(SCError()));
				break;
		}
		_prefs_close();
		exit (1);
	}

	SCPrint(TRUE, stdout, CFSTR("%@\n"), hostname);
	_prefs_close();
	exit(0);
}


static void
set_HostName(int argc, char **argv)
{
	CFStringRef	hostname = NULL;
	Boolean		ok;

	ok = _prefs_open(CFSTR("scutil --set HostName"), NULL);
	if (!ok) {
		SCPrint(TRUE,
			stdout,
			CFSTR("Could not open prefs: %s\n"),
			SCErrorString(SCError()));
		exit(1);
	}

	if (argc == 0) {
		hostname = _copyStringFromSTDIN(CFSTR("HostName"), SCPreferencesGetHostName(prefs));
	} else if (strlen(argv[0]) > 0) {
		hostname = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
	}

	ok = SCPreferencesSetHostName(prefs, hostname);
	if (hostname != NULL) CFRelease(hostname);
	if (!ok) {
		SCPrint(TRUE,
			stderr,
			CFSTR("SCPreferencesSetHostName() failed: %s\n"),
			SCErrorString(SCError()));
		_prefs_close();
		exit (1);
	}

	_prefs_save();
	_prefs_close();
	exit(0);
}


static void
get_LocalHostName(int argc, char **argv)
{
	CFStringRef	hostname;

	hostname = SCDynamicStoreCopyLocalHostName(NULL);
	if (hostname == NULL) {
		int	sc_status	= SCError();

		switch (sc_status) {
			case kSCStatusNoKey :
				SCPrint(TRUE,
					stderr,
					CFSTR("LocalHostName: not set\n"));
				break;
			default :
				SCPrint(TRUE,
					stderr,
					CFSTR("SCDynamicStoreCopyLocalHostName() failed: %s\n"),
					SCErrorString(SCError()));
				break;
		}
		exit (1);
	}

	SCPrint(TRUE, stdout, CFSTR("%@\n"), hostname);
	CFRelease(hostname);

	exit(0);
}


static void
set_LocalHostName(int argc, char **argv)
{
	CFStringRef	hostname	= NULL;
	Boolean		ok;

	ok = _prefs_open(CFSTR("scutil --set LocalHostName"), NULL);
	if (!ok) {
		SCPrint(TRUE,
			stdout,
			CFSTR("Could not open prefs: %s\n"),
			SCErrorString(SCError()));
		exit(1);
	}

	if (argc == 0) {
		CFStringRef	old_hostname;

		old_hostname = SCDynamicStoreCopyLocalHostName(NULL);
		hostname = _copyStringFromSTDIN(CFSTR("LocalHostName"), old_hostname);
		if (old_hostname) CFRelease(old_hostname);
	} else if (strlen(argv[0]) > 0) {
		hostname = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
	}

	ok = SCPreferencesSetLocalHostName(prefs, hostname);
	if (hostname != NULL) CFRelease(hostname);
	if (!ok) {
		SCPrint(TRUE,
			stderr,
			CFSTR("SCPreferencesSetLocalHostName() failed: %s\n"),
			SCErrorString(SCError()));
		_prefs_close();
		exit (1);
	}

	_prefs_save();
	_prefs_close();
	exit(0);
}


/* -------------------- */


typedef void (*pref_func) (int argc, char **argv);

static const struct {
	char		*pref;
	pref_func	get;
	pref_func	set;
} pref_keys[] = {
	{ "ComputerName",	get_ComputerName,	set_ComputerName	},
	{ "HostName",		get_HostName,		set_HostName		},
	{ "LocalHostName",	get_LocalHostName,	set_LocalHostName	}
};
#define	N_PREF_KEYS	(sizeof(pref_keys) / sizeof(pref_keys[0]))


__private_extern__
int
findPref(char *pref)
{
	int	i;

	for (i = 0; i < (int)N_PREF_KEYS; i++) {
		if (strcmp(pref, pref_keys[i].pref) == 0) {
			return i;
		}
	}

	return -1;
}


__private_extern__
void
do_getPref(char *pref, int argc, char **argv)
{
	int	i;

	if (argc == 0) {
		i = findPref(pref);
		if (i >= 0) {
			(*pref_keys[i].get)(argc, argv);
		}
		return;
	}

	// Add support to parse out extended get
	// ie. scutil --get <filename> <prefs path> <key>
	do_prefs_init();
	do_prefs_open(argc, argv);
	do_prefs_get(--argc, ++argv);

	if (value != NULL) {
		CFStringRef key;
		CFStringRef prefs_val;

		key = CFStringCreateWithCString(NULL, *(++argv), kCFStringEncodingUTF8);
		prefs_val = CFDictionaryGetValue(value, key);
		CFRelease(key);

		if (prefs_val != NULL) {
			SCPrint(TRUE, stdout, CFSTR("%@\n"), prefs_val);
			_prefs_close();
			exit(0);
		}
	}

	// if path or key not found
	_prefs_close();
	exit(1);
}


__private_extern__
void
do_setPref(char *pref, int argc, char **argv)
{
	int	i;

	i = findPref(pref);
	if (i >= 0) {
		(*pref_keys[i].set)(argc, argv);
	}
	return;
}


/* -------------------- */


__private_extern__
void
do_prefs_init()
{
	return;
}


__private_extern__
void
do_prefs_open(int argc, char **argv)
{
	Boolean		ok;
	CFStringRef	prefsID	= NULL;

	if (prefs != NULL) {
		if (_prefs_commitRequired(argc, argv, "close")) {
			return;
		}

		do_prefs_close(0, NULL);
	}

	if (argc > 0) {
		prefsID = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
	}

	ok = _prefs_open(CFSTR("scutil --prefs"), prefsID);
	if (prefsID != NULL) CFRelease(prefsID);
	if (!ok) {
		SCPrint(TRUE,
			stdout,
			CFSTR("Could not open prefs: %s\n"),
			SCErrorString(SCError()));
		return;
	}

	return;
}


__private_extern__
void
do_prefs_lock(int argc, char **argv)
{
	Boolean	wait	= (argc > 0) ? TRUE : FALSE;

	if (!SCPreferencesLock(prefs, wait)) {
		SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
		return;
	}

	return;
}


__private_extern__
void
do_prefs_unlock(int argc, char **argv)
{
	if (!SCPreferencesUnlock(prefs)) {
		SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
		return;
	}

	return;
}


__private_extern__
void
do_prefs_commit(int argc, char **argv)
{
	if (!SCPreferencesCommitChanges(prefs)) {
		SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
		return;
	}

	_prefs_changed = FALSE;
	return;
}


__private_extern__
void
do_prefs_apply(int argc, char **argv)
{
	if (!SCPreferencesApplyChanges(prefs)) {
		SCPrint(TRUE, stdout, CFSTR("%s\n"), SCErrorString(SCError()));
	}

	return;
}


__private_extern__
void
do_prefs_close(int argc, char **argv)
{
	if (_prefs_commitRequired(argc, argv, "close")) {
		return;
	}

	_prefs_close();
	return;
}


__private_extern__
void
do_prefs_quit(int argc, char **argv)
{
	if (_prefs_commitRequired(argc, argv, "quit")) {
		return;
	}

	_prefs_close();

	termRequested = TRUE;
	return;
}


__private_extern__
void
do_prefs_synchronize(int argc, char **argv)
{
	SCPreferencesSynchronize(prefs);
	return;
}


static CFComparisonResult
sort_paths(const void *p1, const void *p2, void *context) {
	CFStringRef path1 = (CFStringRef)p1;
	CFStringRef path2 = (CFStringRef)p2;
	return CFStringCompare(path1, path2, 0);
}


__private_extern__
void
do_prefs_list(int argc, char **argv)
{
	int			i;
	CFIndex			n;
	CFMutableArrayRef	paths		= NULL;
	CFStringRef		prefix;
	CFDictionaryRef		entity;

	prefix = CFStringCreateWithCString(NULL,
					   (argc >= 1) ? argv[0] : "/",
					   kCFStringEncodingUTF8);

	entity = SCPreferencesPathGetValue(prefs, prefix);
	if (entity == NULL) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
		goto done;
	}

	paths = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);

	n = isA_CFDictionary(entity) ? CFDictionaryGetCount(entity) : 0;
	if (n > 0) {
		CFIndex		i;
		const void **	keys;
		const void **	vals;

		keys = CFAllocatorAllocate(NULL, n * sizeof(CFStringRef), 0);
		vals = CFAllocatorAllocate(NULL, n * sizeof(CFPropertyListRef), 0);
		CFDictionaryGetKeysAndValues(entity, keys, vals);
		for (i = 0; i < n; i++) {
			if (isA_CFDictionary(vals[i])) {
				CFArrayAppendValue(paths, keys[i]);
			}
		}
		CFAllocatorDeallocate(NULL, keys);
		CFAllocatorDeallocate(NULL, vals);
	}

	n = CFArrayGetCount(paths);
	CFArraySortValues(paths,
			  CFRangeMake(0, n),
			  sort_paths,
			  NULL);

	if (n > 0) {
		for (i = 0; i < n; i++) {
			SCPrint(TRUE,
				stdout,
				CFSTR("  path [%d] = %@/%@\n"),
				i,
				CFEqual(prefix, CFSTR("/")) ? CFSTR("") : prefix,
				CFArrayGetValueAtIndex(paths, i));
		}
	} else {
		SCPrint(TRUE, stdout, CFSTR("  no paths.\n"));
	}

	CFRelease(paths);

    done :

	CFRelease(prefix);
	return;
}


__private_extern__
void
do_prefs_get(int argc, char **argv)
{
	CFDictionaryRef		dict;
	CFStringRef		link;
	CFIndex			n;
	CFMutableDictionaryRef	newDict;
	CFStringRef		path;

	path = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);

	link = SCPreferencesPathGetLink(prefs, path);
	if (link != NULL) {
		SCPrint(TRUE, stdout, CFSTR("  --> %@\n"), link);
	}

	dict = SCPreferencesPathGetValue(prefs, path);
	CFRelease(path);
	if (dict == NULL) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
		return;
	}

	newDict = CFDictionaryCreateMutable(NULL,
					     0,
					     &kCFTypeDictionaryKeyCallBacks,
					     &kCFTypeDictionaryValueCallBacks);

	// remove [path] children
	n = isA_CFDictionary(dict) ? CFDictionaryGetCount(dict) : 0;
	if (n > 0) {
		CFIndex		i;
		const void **	keys;
		const void **	vals;

		keys = CFAllocatorAllocate(NULL, n * sizeof(CFStringRef), 0);
		vals = CFAllocatorAllocate(NULL, n * sizeof(CFPropertyListRef), 0);
		CFDictionaryGetKeysAndValues(dict, keys, vals);
		for (i = 0; i < n; i++) {
			if (!isA_CFDictionary(vals[i])) {
				CFDictionaryAddValue(newDict, keys[i], vals[i]);
			}
		}
		CFAllocatorDeallocate(NULL, keys);
		CFAllocatorDeallocate(NULL, vals);
	}

	if (value != NULL) {
		CFRelease(value);		/* we have new information, release the old */
	}

	value = newDict;

	return;
}


__private_extern__
void
do_prefs_set(int argc, char **argv)
{
	CFDictionaryRef		dict;
	CFMutableDictionaryRef	newDict	= NULL;
	CFStringRef		path;

	path    = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);
	newDict = CFDictionaryCreateMutableCopy(NULL, 0, value);

	dict = SCPreferencesPathGetValue(prefs, path);
	if (dict != NULL) {
		CFIndex	n;

		// retain [path] children
		n = CFDictionaryGetCount(dict);
		if (n > 0) {
			CFIndex		i;
			const void **	keys;
			const void **	vals;

			keys = CFAllocatorAllocate(NULL, n * sizeof(CFStringRef), 0);
			vals = CFAllocatorAllocate(NULL, n * sizeof(CFPropertyListRef), 0);
			CFDictionaryGetKeysAndValues(dict, keys, vals);
			for (i = 0; i < n; i++) {
				if (isA_CFDictionary(vals[i])) {
					if (CFDictionaryContainsKey(newDict, keys[i])) {
						SCPrint(TRUE, stdout, CFSTR("  key %@ is already a path component and cannot be replaced\n"), keys[i]);
						goto done;
					}
					CFDictionaryAddValue(newDict, keys[i], vals[i]);
				}
			}
			CFAllocatorDeallocate(NULL, keys);
			CFAllocatorDeallocate(NULL, vals);
		}
	} else if (SCError() == kSCStatusInvalidArgument) {
		SCPrint(TRUE, stdout, CFSTR("  a path component is not a dictionary\n"));
		goto done;
	} else if (SCError() != kSCStatusNoKey) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
		goto done;
	}

	if (argc > 1) {
		CFStringRef	link;
		Boolean		ok;

		// set link
		link = CFStringCreateWithCString(NULL, argv[1], kCFStringEncodingUTF8);
		ok = SCPreferencesPathSetLink(prefs, path, link);
		CFRelease(link);
		if (!ok) {
			SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
			goto done;
		}
	} else {
		// set value
		if (!SCPreferencesPathSetValue(prefs, path, newDict)) {
			SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
			goto done;
		}
	}

	_prefs_changed = TRUE;

    done :

	CFRelease(newDict);
	CFRelease(path);
	return;
}


__private_extern__
void
do_prefs_remove(int argc, char **argv)
{
	CFStringRef	path;

	path = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingUTF8);

	if (!SCPreferencesPathRemoveValue(prefs, path)) {
		SCPrint(TRUE, stdout, CFSTR("  %s\n"), SCErrorString(SCError()));
		goto done;
	}

	_prefs_changed = TRUE;

    done :

	CFRelease(path);
	return;
}

/* -------------------- */

#include "IPMonitorControlPrefs.h"

__private_extern__
void
do_log(char * log, int argc, char **argv)
{
	if (strcmp(log, "IPMonitor")) {
	    exit(0);
	}
	if (argc == 0) {
	    printf("IPMonitor log is %s\n",
		   IPMonitorControlPrefsIsVerbose() ? "on" : "off");
	}
	else {
	    Boolean	verbose = FALSE;

	    if (strcasecmp(argv[0], "on") == 0) {
		verbose = TRUE;
	    }
	    else if (strcasecmp(argv[0], "off") == 0) {
		verbose = FALSE;
	    }
	    else {
		fprintf(stderr, "%s invalid, must be 'on' or 'off'\n",
			argv[0]);
		exit(1);
	    }
	    if (IPMonitorControlPrefsSetVerbose(verbose) == FALSE) {
		fprintf(stderr, "failed to set preferences\n");
		exit(2);
	    }
	}
	exit(0);
	return;
}


