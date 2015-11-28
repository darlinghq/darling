/*
 * Copyright (c) 2000-2009, 2011, 2012, 2014 Apple Inc. All rights reserved.
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
 * January 15, 2004		Allan Nathanson <ajn@apple.com>
 * - limit location changes to "root" (uid==0), users who are
 *   a member of group "admin", and processses which have access
 *   to a local graphics console.
 *
 * June 1, 2001			Allan Nathanson <ajn@apple.com>
 * - public API conversion
 *
 * January 1, 2001		Allan Nathanson <ajn@apple.com>
 * - initial revision
 */

#include <getopt.h>
#include <unistd.h>
#include <sysexits.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <grp.h>

#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>

#if	!TARGET_OS_IPHONE
#include <Security/Authorization.h>
#endif	/* !TARGET_OS_IPHONE */


static Boolean	apply	= TRUE;


static const struct option longopts[] = {
//	{ "debug",		no_argument,		0,	'd' },
//	{ "verbose",		no_argument,		0,	'v' },
//	{ "do-not-apply",	no_argument,		0,	'n' },
	{ "help",		no_argument,		0,	'?' },
	{ 0,			0,                      0,	0 }
};


static void
usage(const char *command)
{
	SCPrint(TRUE, stderr, CFSTR("usage: %s [-n] new-set-name\n"), command);
	exit (EX_USAGE);
}


int
main(int argc, char **argv)
{
	const char		*command	= argv[0];
	extern int		optind;
	int			opt;
	CFStringRef		current		= NULL;
	int			currentMatched	= 0;
	CFStringRef		newSet		= NULL;	/* set key */
	CFStringRef		newSetUDN	= NULL;	/* user defined name */
	CFStringRef		prefix;
	SCPreferencesRef	prefs;
	CFDictionaryRef		sets;
	CFIndex			nSets;
	const void		**setKeys	= NULL;
	const void		**setVals	= NULL;
	CFIndex			i;

#if	!TARGET_OS_IPHONE
	AuthorizationRef	authorization	= NULL;
	AuthorizationFlags	flags		= kAuthorizationFlagDefaults;
	CFMutableDictionaryRef	options;
	OSStatus		status;
#endif	// !TARGET_OS_IPHONE

	/* process any arguments */

	while ((opt = getopt_long(argc, argv, "dvn", longopts, NULL)) != -1) {
		switch(opt) {
			case 'd':
				_sc_debug = TRUE;
				_sc_log   = FALSE;	/* enable framework logging */
				break;
			case 'v':
				_sc_verbose = TRUE;
				break;
			case 'n':
				apply = FALSE;
				break;
			case '?':
			default :
				usage(command);
		}
	}
	argc -= optind;
	argv += optind;

	prefix = CFStringCreateWithFormat(NULL, NULL, CFSTR("/%@/"), kSCPrefSets);

	if (argc == 1) {
		newSet = CFStringCreateWithCString(NULL, argv[0], kCFStringEncodingMacRoman);

		/* check if a full path to the new "set" was specified */
		if ((CFStringGetLength(newSet) > 0) && CFStringHasPrefix(newSet, prefix)) {
			CFRange			range;
			CFMutableStringRef	str;

			str = CFStringCreateMutableCopy(NULL, 0, newSet);
			CFRelease(newSet);

			CFStringDelete(str, CFRangeMake(0, CFStringGetLength(prefix)));
			newSet = CFStringCreateCopy(NULL, newSet);
			CFRelease(str);

			range = CFStringFind(newSet, CFSTR("/"), 0);
			if (range.location != kCFNotFound) {
				SCPrint(TRUE, stderr, CFSTR("Set \"%@\" not available\n."), newSet);
				exit (1);
			}
		}
	} else {
		newSet = CFRetain(CFSTR(""));
	}

#if	!TARGET_OS_IPHONE
	status = AuthorizationCreate(NULL,
				     kAuthorizationEmptyEnvironment,
				     flags,
				     &authorization);
	if (status != errAuthorizationSuccess) {
		SCPrint(TRUE,
			stderr,
			CFSTR("AuthorizationCreate() failed: status = %d\n"),
			(int)status);
		exit (1);
	}

	options = CFDictionaryCreateMutable(NULL,
					    0,
					    &kCFTypeDictionaryKeyCallBacks,
					    &kCFTypeDictionaryValueCallBacks);
	CFDictionarySetValue(options, kSCPreferencesOptionChangeNetworkSet, kCFBooleanTrue);
	prefs = SCPreferencesCreateWithOptions(NULL, CFSTR("scselect"), NULL, authorization, options);
	CFRelease(options);
	if (prefs == NULL) {
		SCPrint(TRUE, stderr, CFSTR("SCPreferencesCreate() failed\n"));
		exit (1);
	}
#else	// !TARGET_OS_IPHONE
	prefs = SCPreferencesCreate(NULL, CFSTR("scselect"), NULL);
	if (prefs == NULL) {
		SCPrint(TRUE, stderr, CFSTR("SCPreferencesCreate() failed\n"));
		exit (1);
	}
#endif	// !TARGET_OS_IPHONE

	sets = SCPreferencesGetValue(prefs, kSCPrefSets);
	if (sets == NULL) {
		SCPrint(TRUE, stderr, CFSTR("No network sets defined.\n"));
		exit (1);
	}

	current = SCPreferencesGetValue(prefs, kSCPrefCurrentSet);
	if (current != NULL) {
		if (CFStringHasPrefix(current, prefix)) {
			CFMutableStringRef	tmp;

			tmp = CFStringCreateMutableCopy(NULL, 0, current);
			CFStringDelete(tmp, CFRangeMake(0, CFStringGetLength(prefix)));
			current = tmp;
		} else {
			CFRetain(current);
			currentMatched = -1;	/* not prefixed */
		}
	} else {
		current = CFRetain(CFSTR(""));
		currentMatched = -2;	/* not defined */
	}

	nSets = CFDictionaryGetCount(sets);
	if (nSets > 0) {
		setKeys = CFAllocatorAllocate(NULL, nSets * sizeof(CFStringRef), 0);
		setVals = CFAllocatorAllocate(NULL, nSets * sizeof(CFDictionaryRef), 0);
		CFDictionaryGetKeysAndValues(sets, setKeys, setVals);
	}

	/* check for set with matching name */
	for (i = 0; i < nSets; i++) {
		CFStringRef	key  = (CFStringRef)    setKeys[i];
		CFDictionaryRef	dict = (CFDictionaryRef)setVals[i];

		if ((currentMatched >= 0) && CFEqual(key, current)) {
			currentMatched++;
		}

		if (CFEqual(newSet, key)) {
			newSetUDN = CFDictionaryGetValue(dict, kSCPropUserDefinedName);
			if (newSetUDN != NULL) CFRetain(newSetUDN);
			goto found;
		}
	}

	/* check for set with matching user-defined name */
	for (i = 0; i < nSets; i++) {
		CFStringRef	key  = (CFStringRef)    setKeys[i];
		CFDictionaryRef	dict = (CFDictionaryRef)setVals[i];

		newSetUDN = CFDictionaryGetValue(dict, kSCPropUserDefinedName);
		if ((newSetUDN != NULL) && CFEqual(newSet, newSetUDN)) {
			CFRelease(newSet);
			newSet = CFRetain(key);
			CFRetain(newSetUDN);
			goto found;
		}
	}

	if (argc == 1) {
		SCPrint(TRUE, stderr, CFSTR("Set \"%@\" not available.\n"), newSet);
		exit(1);
	}

	SCPrint(TRUE, stdout,
		CFSTR("Defined sets include:%s\n"),
		(currentMatched > 0) ? " (* == current set)" : "");

	for (i = 0; i < nSets; i++) {
		CFStringRef	key  = (CFStringRef)    setKeys[i];
		CFDictionaryRef	dict = (CFDictionaryRef)setVals[i];
		CFStringRef	udn  = CFDictionaryGetValue(dict, kSCPropUserDefinedName);

		SCPrint(TRUE, stdout,
			CFSTR(" %s %@\t(%@)\n"),
			((currentMatched > 0) && CFEqual(key, current)) ? "*" : " ",
			key,
			udn ? udn : CFSTR(""));
	}

	switch (currentMatched) {
		case -2 :
			SCPrint(TRUE, stdout, CFSTR("\nCurrent set not defined.\n"));
			break;
		case -1 :
			SCPrint(TRUE, stdout, CFSTR("\nCurrent set \"%@\" may not be valid\n"), current);
			break;
		case  0 :
			SCPrint(TRUE, stdout, CFSTR("\nCurrent set \"%@\" not valid\n"), current);
			break;
		default :
			break;
	}

	CFRelease(prefix);
	exit (0);

    found :

	CFRelease(current);
	current = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@%@"), prefix, newSet);

	if (!SCPreferencesSetValue(prefs, kSCPrefCurrentSet, current)) {
		SCPrint(TRUE, stderr,
			CFSTR("SCPreferencesSetValue(...,%@,%@) failed: %s\n"),
			kSCPrefCurrentSet,
			current,
			SCErrorString(SCError()));
		exit (1);
	}

	if (!SCPreferencesCommitChanges(prefs)) {
		int	sc_status	= SCError();

		if (sc_status == kSCStatusAccessError) {
			SCPrint(TRUE, stderr,
				CFSTR("Only local console users and administrators can change locations\n"));
			exit (EX_NOPERM);
		} else {
			SCPrint(TRUE, stderr,
				CFSTR("SCPreferencesCommitChanges() failed: %s\n"),
				SCErrorString(sc_status));
			exit (1);
		}
	}

	if (apply) {
		if (!SCPreferencesApplyChanges(prefs)) {
			SCPrint(TRUE, stderr,
				CFSTR("SCPreferencesApplyChanges() failed %s\n"),
				SCErrorString(SCError()));
			exit (1);
		}
	}

	SCPrint(TRUE, stdout,
		CFSTR("%@ updated to %@ (%@)\n"),
		kSCPrefCurrentSet,
		newSet,
		newSetUDN ? newSetUDN : CFSTR(""));

	CFRelease(current);
	CFRelease(newSet);
	if (newSetUDN != NULL)	CFRelease(newSetUDN);
	CFRelease(prefix);
	CFRelease(prefs);

#if	!TARGET_OS_IPHONE
	AuthorizationFree(authorization, kAuthorizationFlagDefaults);
//	AuthorizationFree(authorization, kAuthorizationFlagDestroyRights);
#endif	/* !TARGET_OS_IPHONE */

	exit (0);
	return 0;
}
