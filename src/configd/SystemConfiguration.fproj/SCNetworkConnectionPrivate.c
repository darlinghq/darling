/*
 * Copyright (c) 2006-2012 Apple Inc. All rights reserved.
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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCPrivate.h>		// for SCLog
#include "SCNetworkConfigurationInternal.h"
#include <notify.h>
#include <pthread.h>


#pragma mark -
#pragma mark SCUserPreferences


typedef struct {

	// base CFType information
	CFRuntimeBase		cfBase;

	// serviceID
	CFStringRef             serviceID;

	// user preferences [unique] id
	CFStringRef             prefsID;

} SCUserPreferencesPrivate, *SCUserPreferencesPrivateRef;


static CFStringRef	__SCUserPreferencesCopyDescription	(CFTypeRef cf);
static void		__SCUserPreferencesDeallocate		(CFTypeRef cf);
static Boolean		__SCUserPreferencesEqual		(CFTypeRef cf1, CFTypeRef cf2);
static CFHashCode	__SCUserPreferencesHash			(CFTypeRef cf);


static CFTypeID __kSCUserPreferencesTypeID	= _kCFRuntimeNotATypeID;


static const CFRuntimeClass __SCUserPreferencesClass = {
	0,					// version
	"SCUserPreferences",			// className
	NULL,					// init
	NULL,					// copy
	__SCUserPreferencesDeallocate,		// dealloc
	__SCUserPreferencesEqual,		// equal
	__SCUserPreferencesHash,		// hash
	NULL,					// copyFormattingDesc
	__SCUserPreferencesCopyDescription	// copyDebugDesc
};


static pthread_once_t		initialized	= PTHREAD_ONCE_INIT;


static CFStringRef
__SCUserPreferencesCopyDescription(CFTypeRef cf)
{
	CFAllocatorRef			allocator	= CFGetAllocator(cf);
	CFMutableStringRef		result;
	SCUserPreferencesPrivateRef	prefsPrivate	= (SCUserPreferencesPrivateRef)cf;

	result = CFStringCreateMutable(allocator, 0);
	CFStringAppendFormat(result, NULL, CFSTR("<SCUserPreferences %p [%p]> {"), cf, allocator);
	CFStringAppendFormat(result, NULL, CFSTR("service = %@"), prefsPrivate->serviceID);
	CFStringAppendFormat(result, NULL, CFSTR(", id = %@"), prefsPrivate->prefsID);
	CFStringAppendFormat(result, NULL, CFSTR("}"));

	return result;
}


static void
__SCUserPreferencesDeallocate(CFTypeRef cf)
{
	SCUserPreferencesPrivateRef	prefsPrivate	= (SCUserPreferencesPrivateRef)cf;

	/* release resources */

	CFRelease(prefsPrivate->prefsID);
	CFRelease(prefsPrivate->serviceID);

	return;
}


static Boolean
__SCUserPreferencesEqual(CFTypeRef cf1, CFTypeRef cf2)
{
	SCUserPreferencesPrivateRef	s1	= (SCUserPreferencesPrivateRef)cf1;
	SCUserPreferencesPrivateRef	s2	= (SCUserPreferencesPrivateRef)cf2;

	if (s1 == s2)
		return TRUE;

	if (!CFEqual(s1->prefsID, s2->prefsID))
		return FALSE;	// if not the same [unique] prefs identifier

	return TRUE;
}


static CFHashCode
__SCUserPreferencesHash(CFTypeRef cf)
{
	SCUserPreferencesPrivateRef	prefsPrivate	= (SCUserPreferencesPrivateRef)cf;

	return CFHash(prefsPrivate->prefsID);
}


static void
__SCUserPreferencesInitialize(void)
{
	__kSCUserPreferencesTypeID = _CFRuntimeRegisterClass(&__SCUserPreferencesClass);
	return;
}


static SCUserPreferencesPrivateRef
__SCUserPreferencesCreatePrivate(CFAllocatorRef		allocator,
				 CFStringRef		serviceID,
				 CFStringRef		prefsID)
{
	SCUserPreferencesPrivateRef	prefsPrivate;
	uint32_t			size;

	/* initialize runtime */
	pthread_once(&initialized, __SCUserPreferencesInitialize);

	/* allocate target */
	size         = sizeof(SCUserPreferencesPrivate) - sizeof(CFRuntimeBase);
	prefsPrivate = (SCUserPreferencesPrivateRef)_CFRuntimeCreateInstance(allocator,
									     __kSCUserPreferencesTypeID,
									     size,
									     NULL);
	if (prefsPrivate == NULL) {
		return NULL;
	}

	prefsPrivate->serviceID	= CFStringCreateCopy(NULL, serviceID);
	prefsPrivate->prefsID	= CFStringCreateCopy(NULL, prefsID);

	return prefsPrivate;
}


static __inline__ CFTypeRef
isA_SCUserPreferences(CFTypeRef obj)
{
	return (isA_CFType(obj, SCUserPreferencesGetTypeID()));
}


#pragma mark -
#pragma mark SCUserPreferences SPIs


#define	USER_PREFERENCES_NOTIFICATION	"com.apple.networkConnect"
#define	USER_PREFERENCES_APPLICATION_ID	CFSTR("com.apple.networkConnect")
#define	USER_PREFERENCES_ID		CFSTR("UniqueIdentifier")
#define	USER_PREFERENCES_DEFAULT	CFSTR("ConnectByDefault")


#define	LOG_CFPREFERENCES_CHANGES
#ifdef	LOG_CFPREFERENCES_CHANGES
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
static void
logCFPreferencesChange(CFStringRef serviceID, CFArrayRef newPreferences)
{
	CFBooleanRef	bVal;
	char		dir[256];
	CFArrayRef	oldPreferences;
	CFStringRef	str;
	CFStringRef	trace;
	struct tm	tm_now;
	struct timeval	tv_now;

	bVal = CFPreferencesCopyAppValue(CFSTR("LOG_SC_CHANGES"), USER_PREFERENCES_APPLICATION_ID);
	if (bVal != NULL) {
		Boolean	enabled;

		enabled = isA_CFBoolean(bVal) && CFBooleanGetValue(bVal);
		CFRelease(bVal);
		if (!enabled) {
			// if debugging not enabled
			return;
		}
	} else {
		// if debugging not enabled
		return;
	}

	(void)gettimeofday(&tv_now, NULL);
	(void)localtime_r(&tv_now.tv_sec, &tm_now);

	str = CFStringCreateWithFormat(NULL, NULL,
				       CFSTR("/var/tmp/com.apple.networkConnect-%@-%4d%02d%02d.%02d%02d%02d.%03d"),
				       serviceID,
				       tm_now.tm_year + 1900,
				       tm_now.tm_mon + 1,
				       tm_now.tm_mday,
				       tm_now.tm_hour,
				       tm_now.tm_min,
				       tm_now.tm_sec,
				       tv_now.tv_usec / 1000);
	_SC_cfstring_to_cstring(str, dir, sizeof(dir), kCFStringEncodingUTF8);
	CFRelease(str);

	SCLog(TRUE, LOG_ERR, CFSTR("CFPreferences being updated, old/new in \"%s\""), dir);

	if (mkdir(dir, 0755) == -1) {
		SCLog(TRUE, LOG_ERR, CFSTR("logCFPreferencesChange mkdir() failed, error = %s"), SCErrorString(errno));
		return;
	}

	trace = _SC_copyBacktrace();
	if (trace != NULL) {
		FILE	*f;
		int	fd;
		char	path[256];

		strlcpy(path, dir, sizeof(path));
		strlcat(path, "/backtrace", sizeof(path));
		fd = open(path, O_WRONLY|O_CREAT|O_TRUNC|O_EXCL, 0644);
		if (fd == -1) {
			SCLog(TRUE, LOG_ERR, CFSTR("logCFPreferencesChange fopen() failed, error = %s"), SCErrorString(errno));
			CFRelease(trace);
			return;
		}
		f = fdopen(fd, "w");
		SCPrint(TRUE, f, CFSTR("%@"), trace);
		(void) fclose(f);
		CFRelease(trace);
	}

	oldPreferences = CFPreferencesCopyAppValue(serviceID, USER_PREFERENCES_APPLICATION_ID);
	if (oldPreferences != NULL) {
		int		fd;
		CFDataRef	data;
		char		path[256];

		strlcpy(path, dir, sizeof(path));
		strlcat(path, "/old", sizeof(path));
		fd = open(path, O_WRONLY|O_CREAT|O_TRUNC|O_EXCL, 0644);
		if (fd == -1) {
			SCLog(TRUE, LOG_ERR, CFSTR("logCFPreferencesChange fopen() failed, error = %s"), SCErrorString(errno));
			CFRelease(oldPreferences);
			return;
		}
		data = CFPropertyListCreateData(NULL, oldPreferences, kCFPropertyListXMLFormat_v1_0, 0, NULL);
		if (data == NULL) {
			SCLog(TRUE, LOG_ERR, CFSTR("logCFPreferencesChange CFPropertyListCreateData() failed"));
			close(fd);
			CFRelease(oldPreferences);
			return;
		}
		(void) write(fd, CFDataGetBytePtr(data), CFDataGetLength(data));
		(void) close(fd);
		CFRelease(data);
		CFRelease(oldPreferences);
	}

	if (newPreferences != NULL) {
		int		fd;
		CFDataRef	data;
		char		path[256];

		strlcpy(path, dir, sizeof(path));
		strlcat(path, "/new", sizeof(path));
		fd = open(path, O_WRONLY|O_CREAT|O_TRUNC|O_EXCL, 0644);
		if (fd == -1) {
			SCLog(TRUE, LOG_ERR, CFSTR("logCFPreferencesChange fopen() failed, error = %s"), SCErrorString(errno));
			return;
		}
		data = CFPropertyListCreateData(NULL, newPreferences, kCFPropertyListXMLFormat_v1_0, 0, NULL);
		if (data == NULL) {
			SCLog(TRUE, LOG_ERR, CFSTR("logCFPreferencesChange CFPropertyListCreateData() failed"));
			close(fd);
			return;
		}
		(void) write(fd, CFDataGetBytePtr(data), CFDataGetLength(data));
		(void) close(fd);
		CFRelease(data);
	}

	return;
}
#endif	// LOG_CFPREFERENCES_CHANGES


static CFArrayRef
copyCFPreferencesForServiceID(CFStringRef serviceID)
{
	CFArrayRef	prefs;

	// fetch "Managed" or "ByHost" user preferences
	(void) CFPreferencesAppSynchronize(USER_PREFERENCES_APPLICATION_ID);
	prefs = CFPreferencesCopyAppValue(serviceID,
					  USER_PREFERENCES_APPLICATION_ID);

	if ((prefs != NULL) && !isA_CFArray(prefs)) {
		CFRelease(prefs);
		return NULL;
	}

	return prefs;
}


static Boolean
setCFPreferencesForServiceID(CFStringRef serviceID, CFArrayRef newPreferences)
{
	Boolean	ok;

	if (CFPreferencesAppValueIsForced(serviceID, USER_PREFERENCES_APPLICATION_ID)) {
		return FALSE;
	}

#ifdef	LOG_CFPREFERENCES_CHANGES
	logCFPreferencesChange(serviceID, newPreferences);
#endif	// LOG_CFPREFERENCES_CHANGES

	CFPreferencesSetValue(serviceID,
			      newPreferences,
			      USER_PREFERENCES_APPLICATION_ID,
			      kCFPreferencesCurrentUser,
			      kCFPreferencesCurrentHost);
	ok = CFPreferencesSynchronize(USER_PREFERENCES_APPLICATION_ID,
				      kCFPreferencesCurrentUser,
				      kCFPreferencesCurrentHost);

	(void) notify_post(USER_PREFERENCES_NOTIFICATION);

	return ok;
}


static void
addPreference(CFMutableArrayRef *newPrefs, CFDictionaryRef newDict)
{
	if (*newPrefs == NULL) {
		*newPrefs = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	}
	CFArrayAppendValue(*newPrefs, newDict);

	return;
}


typedef CFDictionaryRef (*processPreferencesCallout)	(CFStringRef		serviceID,
							 CFDictionaryRef	current,
							 void			*context1,
							 void			*context2,
							 void			*context3);


static Boolean
processPreferences(CFStringRef			serviceID,
		   processPreferencesCallout	callout,
		   void				*context1,
		   void				*context2,
		   void				*context3)
{
	Boolean				changed		= FALSE;
	CFIndex				i;
	CFIndex				n;
	CFDictionaryRef			newDict		= NULL;
	CFMutableArrayRef		newPrefs	= NULL;
	Boolean				ok		= TRUE;
	CFArrayRef			prefs;

	prefs = copyCFPreferencesForServiceID(serviceID);
	n = (prefs != NULL) ? CFArrayGetCount(prefs) : 0;
	for (i = 0; i < n; i++) {
		CFDictionaryRef	dict;

		dict = CFArrayGetValueAtIndex(prefs, i);
		assert(dict != NULL);
		if (isA_CFDictionary(dict)) {
			newDict = (*callout)(serviceID, dict, context1, context2, context3);
			if (newDict == NULL) {
				// if entry to be removed
				changed = TRUE;
				continue;
			}
		} else {
			// if not a CFDictionary, leave as-is
			newDict = CFRetain(dict);
		}

		if (!CFEqual(dict, newDict)) {
			changed = TRUE;
		}

		addPreference(&newPrefs, newDict);
		CFRelease(newDict);
	}
	if (prefs != NULL) CFRelease(prefs);

	newDict = (*callout)(serviceID, NULL, context1, context2, context3);
	if (newDict != NULL) {
		// if new entry
		changed = TRUE;
		addPreference(&newPrefs, newDict);
		CFRelease(newDict);
	}

	if (changed) {
		ok = setCFPreferencesForServiceID(serviceID, newPrefs);
	}
	if (newPrefs != NULL) CFRelease(newPrefs);

	return ok;
}


static __inline__ Boolean
isMatchingPrefsID(CFDictionaryRef dict, CFStringRef matchID)
{
	CFStringRef	prefsID;

	prefsID = CFDictionaryGetValue(dict, USER_PREFERENCES_ID);
	if (isA_CFString(prefsID)) {
		if (CFEqual(prefsID, matchID)) {
			return TRUE;
		}
	}

	return FALSE;
}


CFTypeID
SCUserPreferencesGetTypeID(void)
{
	pthread_once(&initialized, __SCUserPreferencesInitialize);	/* initialize runtime */
	return __kSCUserPreferencesTypeID;
}


CFStringRef
SCUserPreferencesGetUniqueID(SCUserPreferencesRef userPreferences)
{
	SCUserPreferencesPrivateRef	userPrivate	= (SCUserPreferencesPrivateRef)userPreferences;

	if (!isA_SCUserPreferences(userPreferences)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	return userPrivate->prefsID;
}


Boolean
SCUserPreferencesIsForced(SCUserPreferencesRef userPreferences)
{
	SCUserPreferencesPrivateRef	userPrivate	= (SCUserPreferencesPrivateRef)userPreferences;

	if (!isA_SCUserPreferences(userPreferences)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	return CFPreferencesAppValueIsForced(userPrivate->serviceID, USER_PREFERENCES_APPLICATION_ID);
}


static CFDictionaryRef
removeCallout(CFStringRef	serviceID,
	      CFDictionaryRef	current,
	      void		*context1,
	      void		*context2,
	      void		*context3)
{
	CFStringRef	matchID	= (CFStringRef)context1;

	if (current == NULL) {
		// we have nothing to "add"
		return NULL;
	}

	if (isMatchingPrefsID(current, matchID)) {
		// if we match, don't add (i.e. remove)
		return NULL;
	}

	return CFRetain(current);
}


Boolean
SCUserPreferencesRemove(SCUserPreferencesRef userPreferences)
{
	SCUserPreferencesPrivateRef	userPrivate	= (SCUserPreferencesPrivateRef)userPreferences;

	if (!isA_SCUserPreferences(userPreferences)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	return processPreferences(userPrivate->serviceID,
				  removeCallout,
				  (void *)userPrivate->prefsID,
				  NULL,
				  NULL);
}


static CF_RETURNS_RETAINED CFDictionaryRef
setCurrentCallout(CFStringRef		serviceID,
		  CFDictionaryRef	current,
		  void			*context1,
		  void			*context2,
		  void			*context3)
{
	CFStringRef			matchID		= (CFStringRef)context1;
	CFMutableDictionaryRef		newDict;

	if (current == NULL) {
		// we have nothing to "add"
		return NULL;
	}

	newDict = CFDictionaryCreateMutableCopy(NULL, 0, current);

	// remove "default" flag
	CFDictionaryRemoveValue(newDict, USER_PREFERENCES_DEFAULT);

	if (isMatchingPrefsID(current, matchID)) {
		// if we match, set "default" flag
		CFDictionarySetValue(newDict, USER_PREFERENCES_DEFAULT, kCFBooleanTrue);
	}

	return newDict;
}


Boolean
SCUserPreferencesSetCurrent(SCUserPreferencesRef userPreferences)
{
	SCUserPreferencesPrivateRef	userPrivate	= (SCUserPreferencesPrivateRef)userPreferences;

	if (!isA_SCUserPreferences(userPreferences)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	return processPreferences(userPrivate->serviceID,
				  setCurrentCallout,
				  (void *)userPrivate->prefsID,
				  NULL,
				  NULL);
}


static CFDictionaryRef
copyNameCallout(CFStringRef	serviceID,
		CFDictionaryRef	current,
		void		*context1,
		void		*context2,
		void		*context3)
{
	CFStringRef	matchID	= (CFStringRef)context1;
	CFStringRef	*name	= (CFStringRef *)context3;

	if (current == NULL) {
		// we have nothing to "add"
		return NULL;
	}

	if (isMatchingPrefsID(current, matchID)) {
		*name = CFDictionaryGetValue(current, kSCPropUserDefinedName);

		// for backwards compatibility, we also check for the name in the PPP entity
		if (*name == NULL) {
			CFDictionaryRef	ppp;

			ppp = CFDictionaryGetValue(current, kSCEntNetPPP);
			if (isA_CFDictionary(ppp)) {
				*name = CFDictionaryGetValue(ppp, kSCPropUserDefinedName);
			}
		}

		*name = isA_CFString(*name);
		if (*name != NULL) {
			CFRetain(*name);
		}
	}

	return CFRetain(current);
}


CFStringRef
SCUserPreferencesCopyName(SCUserPreferencesRef userPreferences)
{
	CFStringRef			name		= NULL;
	Boolean				ok;
	SCUserPreferencesPrivateRef	userPrivate	= (SCUserPreferencesPrivateRef)userPreferences;

	if (!isA_SCUserPreferences(userPreferences)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	// find SCUserPreferences and copy name
	ok = processPreferences(userPrivate->serviceID,
				copyNameCallout,
				(void *)userPrivate->prefsID,
				NULL,
				(void *)&name);
	if (!ok) {
		if (name != NULL) {
			CFRelease(name);
			name = NULL;
		}
	}

	return name;
}


static CF_RETURNS_RETAINED CFDictionaryRef
setNameCallout(CFStringRef	serviceID,
	       CFDictionaryRef	current,
	       void		*context1,
	       void		*context2,
	       void		*context3)
{
	CFStringRef		matchID	= (CFStringRef)context1;
	CFMutableDictionaryRef	newDict;
	CFStringRef		newName	= (CFStringRef)context2;

	if (current == NULL) {
		// we have nothing to "add"
		return NULL;
	}

	newDict = CFDictionaryCreateMutableCopy(NULL, 0, current);

	if (isMatchingPrefsID(current, matchID)) {
		CFDictionaryRef	pppEntity;

		// set the name
		if (newName != NULL) {
			CFDictionarySetValue(newDict, kSCPropUserDefinedName, newName);
		} else {
			CFDictionaryRemoveValue(newDict, kSCPropUserDefinedName);
		}

		// for backwards compatibility, we also set the name in the PPP entity
		pppEntity = CFDictionaryGetValue(newDict, kSCEntNetPPP);
		if (isA_CFDictionary(pppEntity)) {
			CFMutableDictionaryRef	newPPPEntity;

			newPPPEntity = CFDictionaryCreateMutableCopy(NULL, 0, pppEntity);
			if (newName != NULL) {
				CFDictionarySetValue(newPPPEntity, kSCPropUserDefinedName, newName);
			} else {
				CFDictionaryRemoveValue(newPPPEntity, kSCPropUserDefinedName);
			}
			CFDictionarySetValue(newDict, kSCEntNetPPP, newPPPEntity);
			CFRelease(newPPPEntity);
		}
	}

	return newDict;
}


Boolean
SCUserPreferencesSetName(SCUserPreferencesRef userPreferences, CFStringRef newName)
{
	Boolean				ok;
	SCUserPreferencesPrivateRef	userPrivate	= (SCUserPreferencesPrivateRef)userPreferences;

	if (!isA_SCUserPreferences(userPreferences)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if ((newName != NULL) && !isA_CFString(newName)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	// find SCUserPreferences and set name
	ok = processPreferences(userPrivate->serviceID,
				setNameCallout,
				(void *)userPrivate->prefsID,
				(void *)newName,
				NULL);

	return ok;
}


static CFDictionaryRef
copyInterfaceConfigurationCallout(CFStringRef		serviceID,
				  CFDictionaryRef	current,
				  void			*context1,
				  void			*context2,
				  void			*context3)
{
	CFDictionaryRef	*dict		= (CFDictionaryRef *)context3;
	CFStringRef	interfaceType	= (CFStringRef)context2;
	CFStringRef	matchID		= (CFStringRef)context1;

	if (current == NULL) {
		// we have nothing to "add"
		return NULL;
	}

	if (isMatchingPrefsID(current, matchID)) {
		*dict = CFDictionaryGetValue(current, interfaceType);
		*dict = isA_CFDictionary(*dict);
		if (*dict != NULL) {
			CFRetain(*dict);
		}
	}

	return CFRetain(current);
}


CFDictionaryRef
SCUserPreferencesCopyInterfaceConfiguration(SCUserPreferencesRef	userPreferences,
					    SCNetworkInterfaceRef	interface)
{
	CFStringRef			defaultType;
	CFDictionaryRef			entity		= NULL;
	Boolean				ok;
	SCUserPreferencesPrivateRef	userPrivate	= (SCUserPreferencesPrivateRef)userPreferences;

	if (!isA_SCUserPreferences(userPreferences)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	if (!isA_SCNetworkInterface(interface)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	// get InterfaceType
	defaultType = __SCNetworkInterfaceGetDefaultConfigurationType(interface);
	if (defaultType == NULL) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	// find SCUserPreferences and copy interface entity
	ok = processPreferences(userPrivate->serviceID,
				copyInterfaceConfigurationCallout,
				(void *)userPrivate->prefsID,
				(void *)defaultType,
				(void *)&entity);
	if (!ok) {
		if (entity != NULL) {
			CFRelease(entity);
			entity = NULL;
		}
	}

	return entity;
}


static CF_RETURNS_RETAINED CFDictionaryRef
setInterfaceConfigurationCallout(CFStringRef		serviceID,
				  CFDictionaryRef	current,
				  void			*context1,
				  void			*context2,
				  void			*context3)
{
	CFStringRef		interfaceType	= (CFStringRef)context2;
	CFStringRef		matchID		= (CFStringRef)context1;
	CFMutableDictionaryRef	newDict;
	CFDictionaryRef		newOptions	= (CFDictionaryRef)context3;

	if (current == NULL) {
		// we have nothing to "add"
		return NULL;
	}

	newDict = CFDictionaryCreateMutableCopy(NULL, 0, current);

	if (isMatchingPrefsID(current, matchID)) {
		if (newOptions != NULL) {
			CFDictionarySetValue(newDict, interfaceType, newOptions);

			// for backwards compatibility, we want to ensure that
			// the name is set in both the top level and in the PPP
			// entity.
			if (CFEqual(interfaceType, kSCEntNetPPP)) {
				CFStringRef	name;

				name = CFDictionaryGetValue(newOptions, kSCPropUserDefinedName);
				if (name != NULL) {
					// if name was passed in newOptions, push up
					CFDictionarySetValue(newDict, kSCPropUserDefinedName, name);
				} else {
					name = CFDictionaryGetValue(newDict, kSCPropUserDefinedName);
					if (name != NULL) {
						CFMutableDictionaryRef	newPPPEntity;

						// if name in parent, push into entity
						newPPPEntity = CFDictionaryCreateMutableCopy(NULL, 0, newOptions);
						CFDictionarySetValue(newPPPEntity, kSCPropUserDefinedName, name);
						CFDictionarySetValue(newDict, interfaceType, newPPPEntity);
						CFRelease(newPPPEntity);
					}
				}
			}
		} else {
			CFDictionaryRemoveValue(newDict, interfaceType);
		}
	}

	return newDict;
}


Boolean
SCUserPreferencesSetInterfaceConfiguration(SCUserPreferencesRef		userPreferences,
					   SCNetworkInterfaceRef	interface,
					   CFDictionaryRef		newOptions)
{
	CFStringRef			defaultType;
	Boolean				ok;
	SCUserPreferencesPrivateRef	userPrivate	= (SCUserPreferencesPrivateRef)userPreferences;

	if (!isA_SCUserPreferences(userPreferences)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (!isA_SCNetworkInterface(interface)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	// get InterfaceType
	defaultType = __SCNetworkInterfaceGetDefaultConfigurationType(interface);
	if (defaultType == NULL) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	// set new interface entity for SCUserPreferences
	ok = processPreferences(userPrivate->serviceID,
				setInterfaceConfigurationCallout,
				(void *)userPrivate->prefsID,
				(void *)defaultType,
				(void *)newOptions);

	return ok;
}


CFDictionaryRef
SCUserPreferencesCopyExtendedInterfaceConfiguration(SCUserPreferencesRef	userPreferences,
						    SCNetworkInterfaceRef	interface,
						    CFStringRef			extendedType)
{
	CFDictionaryRef			entity		= NULL;
	Boolean				ok;
	SCUserPreferencesPrivateRef	userPrivate	= (SCUserPreferencesPrivateRef)userPreferences;

	if (!isA_SCUserPreferences(userPreferences)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	if (!isA_SCNetworkInterface(interface)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	if (!__SCNetworkInterfaceIsValidExtendedConfigurationType(interface, extendedType, FALSE)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return NULL;
	}

	// find SCUserPreferences and copy interface entity
	ok = processPreferences(userPrivate->serviceID,
				copyInterfaceConfigurationCallout,
				(void *)userPrivate->prefsID,
				(void *)extendedType,
				(void *)&entity);
	if (!ok) {
		if (entity != NULL) {
			CFRelease(entity);
			entity = NULL;
		}
	}

	return entity;
}


Boolean
SCUserPreferencesSetExtendedInterfaceConfiguration(SCUserPreferencesRef		userPreferences,
						   SCNetworkInterfaceRef	interface,
						   CFStringRef			extendedType,
						   CFDictionaryRef		newOptions)
{
	Boolean				ok;
	SCUserPreferencesPrivateRef	userPrivate	= (SCUserPreferencesPrivateRef)userPreferences;

	if (!isA_SCUserPreferences(userPreferences)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (!isA_SCNetworkInterface(interface)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (!__SCNetworkInterfaceIsValidExtendedConfigurationType(interface, extendedType, FALSE)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	// set new interface entity for SCUserPreferences
	ok = processPreferences(userPrivate->serviceID,
				setInterfaceConfigurationCallout,
				(void *)userPrivate->prefsID,
				(void *)extendedType,
				(void *)newOptions);

	return ok;
}


#pragma mark -
#pragma mark SCNetworkConnection + SCUserPreferences SPIs


static CFDictionaryRef
copyAllCallout(CFStringRef	serviceID,
	       CFDictionaryRef	current,
	       void		*context1,
	       void		*context2,
	       void		*context3)
{
	CFMutableArrayRef		*prefs		= (CFMutableArrayRef *)context3;
	CFStringRef			prefsID;
	SCUserPreferencesPrivateRef	userPrivate;

	if (current == NULL) {
		// we have nothing to "add"
		return NULL;
	}

	prefsID = CFDictionaryGetValue(current, USER_PREFERENCES_ID);
	if (!isA_CFString(prefsID)) {
		// if no unique ID
		goto done;
	}

	userPrivate = __SCUserPreferencesCreatePrivate(NULL, serviceID, prefsID);
	if (userPrivate != NULL) {
		if (*prefs == NULL) {
			*prefs = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
		}
		CFArrayAppendValue(*prefs, (SCUserPreferencesRef)userPrivate);
		CFRelease(userPrivate);
	}

    done :

	return CFRetain(current);
}


CFArrayRef /* of SCUserPreferencesRef's */
SCNetworkConnectionCopyAllUserPreferences(SCNetworkConnectionRef connection)
{
	Boolean			ok;
	CFMutableArrayRef	prefs		= NULL;
	CFStringRef		serviceID;

	// get serviceID
	serviceID = SCNetworkConnectionCopyServiceID(connection);

	// collect SCUserPreferences
	ok = processPreferences(serviceID,
				copyAllCallout,
				NULL,
				NULL,
				(void *)&prefs);
	if (!ok) {
		if (prefs != NULL) {
			CFRelease(prefs);
			prefs = NULL;
		}
	}

	CFRelease(serviceID);
	return prefs;
}


static CFDictionaryRef
copyCurrentCallout(CFStringRef		serviceID,
		   CFDictionaryRef	current,
		   void			*context1,
		   void			*context2,
		   void			*context3)
{
	CFBooleanRef			isDefault;
	CFStringRef			prefsID;
	SCUserPreferencesPrivateRef	*userPrivate	= (SCUserPreferencesPrivateRef *)context3;

	if (current == NULL) {
		// we have nothing to "add"
		return NULL;
	}

	prefsID = CFDictionaryGetValue(current, USER_PREFERENCES_ID);
	if (!isA_CFString(prefsID)) {
		// if no unique ID
		goto done;
	}

	isDefault = CFDictionaryGetValue(current, USER_PREFERENCES_DEFAULT);
	if (!isA_CFBoolean(isDefault) || !CFBooleanGetValue(isDefault)) {
		// if not the default configuration
		goto done;
	}

	*userPrivate = __SCUserPreferencesCreatePrivate(NULL, serviceID, prefsID);

    done :

	return CFRetain(current);
}


SCUserPreferencesRef
SCNetworkConnectionCopyCurrentUserPreferences(SCNetworkConnectionRef connection)
{
	SCUserPreferencesRef	current		= NULL;
	Boolean			ok;
	CFStringRef		serviceID;

	// get serviceID
	serviceID = SCNetworkConnectionCopyServiceID(connection);

	// collect SCUserPreferences
	ok = processPreferences(serviceID,
				copyCurrentCallout,
				NULL,
				NULL,
				(void *)&current);
	if (!ok) {
		if (current != NULL) {
			CFRelease(current);
			current = NULL;
		}
	}

	CFRelease(serviceID);
	return current;
}


static CFDictionaryRef
createCallout(CFStringRef	serviceID,
	      CFDictionaryRef	current,
	      void		*context1,
	      void		*context2,
	      void		*context3)
{
	CFMutableDictionaryRef		newDict;
	CFStringRef			newPrefsID	= (CFStringRef)context1;

	if (current != NULL) {
		// don't change existing entries
		return CFRetain(current);
	}

	newDict = CFDictionaryCreateMutable(NULL,
					    0,
					    &kCFTypeDictionaryKeyCallBacks,
					    &kCFTypeDictionaryValueCallBacks);
	CFDictionarySetValue(newDict, USER_PREFERENCES_ID, newPrefsID);
	return newDict;
}


SCUserPreferencesRef
SCNetworkConnectionCreateUserPreferences(SCNetworkConnectionRef connection)
{
	CFStringRef			newPrefsID;
	CFStringRef			serviceID;
	SCUserPreferencesPrivateRef	userPrivate;
	CFUUIDRef			uuid;

	// get serviceID
	serviceID = SCNetworkConnectionCopyServiceID(connection);

	// allocate a new user preferences ID
	uuid       = CFUUIDCreate(NULL);
	newPrefsID = CFUUIDCreateString(NULL, uuid);
	CFRelease(uuid);

	userPrivate = __SCUserPreferencesCreatePrivate(NULL, serviceID, newPrefsID);
	if (userPrivate != NULL) {
		(void) processPreferences(serviceID,
					  createCallout,
					  (void *)newPrefsID,
					  NULL,
					  NULL);
	}

	CFRelease(newPrefsID);
	CFRelease(serviceID);
	return (SCUserPreferencesRef)userPrivate;
}


static void
update_PPP_entity(SCUserPreferencesRef userPreferences, CFDictionaryRef *userOptions)
{
	CFStringRef	encryption;
	CFDictionaryRef	entity;
	CFStringRef	keychainID;

	entity = CFDictionaryGetValue(*userOptions, kSCEntNetPPP);
	if (!isA_CFDictionary(entity)) {
		return;
	}

	encryption = CFDictionaryGetValue(entity, kSCPropNetPPPAuthPasswordEncryption);
	if (encryption == NULL) {
		// provide default encryption method
		encryption = kSCValNetPPPAuthPasswordEncryptionKeychain;
	}

	if (!isA_CFString(encryption) ||
	    !CFEqual(encryption, kSCValNetPPPAuthPasswordEncryptionKeychain)) {
		return;
	}

	keychainID = CFDictionaryGetValue(entity, kSCPropNetPPPAuthPassword);
	if (isA_CFString(keychainID)) {
		// if password is keychain ID
	} else if (isA_CFData(keychainID) &&
		   ((CFDataGetLength((CFDataRef)keychainID) % sizeof(UniChar)) == 0)) {
		// if inline password
		return;
	} else {
		keychainID = SCUserPreferencesGetUniqueID(userPreferences);
	}

	if (_SCSecKeychainPasswordItemExists(NULL, keychainID)) {
		CFMutableDictionaryRef	new_entity;
		CFMutableDictionaryRef	new_options;

		// access PPP password from system keychain
		new_entity = CFDictionaryCreateMutableCopy(NULL, 0, entity);

		CFDictionarySetValue(new_entity,
				     kSCPropNetPPPAuthPassword,
				     keychainID);
		CFDictionarySetValue(new_entity,
				     kSCPropNetPPPAuthPasswordEncryption,
				     kSCValNetPPPAuthPasswordEncryptionKeychain);

		new_options = CFDictionaryCreateMutableCopy(NULL, 0, *userOptions);
		CFDictionarySetValue(new_options, kSCEntNetPPP, new_entity);
		CFRelease(new_entity);

		CFRelease(*userOptions);
		*userOptions = new_options;
	}

	return;
}


static void
update_IPSec_entity(SCUserPreferencesRef userPreferences, CFDictionaryRef *userOptions)
{
	CFStringRef	encryption;
	CFDictionaryRef	entity;
	SecKeychainRef	keychain	= NULL;
	CFStringRef	keychainID;
	CFStringRef	method;
	CFDataRef	sharedSecret;

	entity = CFDictionaryGetValue(*userOptions, kSCEntNetIPSec);
	if (!isA_CFDictionary(entity)) {
		return;
	}

	method = CFDictionaryGetValue(entity, kSCPropNetIPSecAuthenticationMethod);
	if (!isA_CFString(method) ||
	    !CFEqual(method, kSCValNetIPSecAuthenticationMethodSharedSecret)) {
		return;
	}

	encryption = CFDictionaryGetValue(entity, kSCPropNetIPSecSharedSecretEncryption);
	if (encryption == NULL) {
		// provide default encryption method
		encryption = kSCValNetIPSecSharedSecretEncryptionKeychain;
	}

	if (!isA_CFString(encryption) ||
	    !CFEqual(encryption, kSCValNetIPSecSharedSecretEncryptionKeychain)) {
		return;
	}

	keychainID = CFDictionaryGetValue(entity, kSCPropNetIPSecSharedSecret);
	if (isA_CFString(keychainID)) {
		// if shared secret is keychain ID
		CFRetain(keychainID);
	} else if (isA_CFData(keychainID) &&
		   ((CFDataGetLength((CFDataRef)keychainID) % sizeof(UniChar)) == 0)) {
		// if inline shared secret
		return;
	} else {
		CFStringRef	unique_id;

		unique_id = SCUserPreferencesGetUniqueID(userPreferences);
		keychainID = (CFStringRef)CFStringCreateMutableCopy(NULL, 0, unique_id);
		CFStringAppend((CFMutableStringRef)keychainID, CFSTR(".SS"));
	}

	sharedSecret = _SCSecKeychainPasswordItemCopy(NULL, keychainID);
	if (sharedSecret != NULL) {
		CFMutableDictionaryRef	new_entity;
		CFMutableDictionaryRef	new_options;
		CFStringRef		password;

		// pass SharedSecret from user keychain
		new_entity = CFDictionaryCreateMutableCopy(NULL, 0, entity);

		password = CFStringCreateWithBytes(NULL,
						   CFDataGetBytePtr(sharedSecret),
						   CFDataGetLength(sharedSecret),
						   kCFStringEncodingUTF8,
						   FALSE);
		CFRelease(sharedSecret);
		CFDictionarySetValue(new_entity,
				     kSCPropNetIPSecSharedSecret,
				     password);
		CFRelease(password);
		CFDictionaryRemoveValue(new_entity,
					kSCPropNetIPSecSharedSecretEncryption);

		new_options = CFDictionaryCreateMutableCopy(NULL, 0, *userOptions);
		CFDictionarySetValue(new_options, kSCEntNetIPSec, new_entity);
		CFRelease(new_entity);

		CFRelease(*userOptions);
		*userOptions = new_options;
		goto done;
	}

	keychain = _SCSecKeychainCopySystemKeychain();
	if (keychain == NULL) {
		goto done;
	}

	if (_SCSecKeychainPasswordItemExists(keychain, keychainID)) {
		CFMutableDictionaryRef	new_entity;
		CFMutableDictionaryRef	new_options;

		// access SharedSecret from system keychain
		new_entity = CFDictionaryCreateMutableCopy(NULL, 0, entity);

		CFDictionarySetValue(new_entity,
				     kSCPropNetIPSecSharedSecret,
				     keychainID);
		CFDictionarySetValue(new_entity,
				     kSCPropNetIPSecSharedSecretEncryption,
				     kSCValNetIPSecSharedSecretEncryptionKeychain);

		new_options = CFDictionaryCreateMutableCopy(NULL, 0, *userOptions);
		CFDictionarySetValue(new_options, kSCEntNetIPSec, new_entity);
		CFRelease(new_entity);

		CFRelease(*userOptions);
		*userOptions = new_options;
	}

    done :

	if (keychain != NULL) CFRelease(keychain);
	CFRelease(keychainID);
	return;
}


static CFDictionaryRef
copyOptionsCallout(CFStringRef		serviceID,
		   CFDictionaryRef	current,
		   void			*context1,
		   void			*context2,
		   void			*context3)
{
	CFStringRef		matchID		= (CFStringRef)context1;
	CFMutableDictionaryRef	*userOptions	= (CFMutableDictionaryRef *)context3;

	if (current == NULL) {
		// we have nothing to "add"
		return NULL;
	}

	if (isMatchingPrefsID(current, matchID)) {
		// if we match, return options dictionary
		if (*userOptions != NULL) CFRelease(*userOptions);
		*userOptions = CFDictionaryCreateMutableCopy(NULL, 0, current);
		CFDictionaryRemoveValue(*userOptions, USER_PREFERENCES_ID);
		CFDictionaryRemoveValue(*userOptions, USER_PREFERENCES_DEFAULT);
	}

	return CFRetain(current);
}


Boolean
SCNetworkConnectionStartWithUserPreferences(SCNetworkConnectionRef	connection,
					    SCUserPreferencesRef	userPreferences,
					    Boolean			linger)
{
	Boolean				ok;
	CFDictionaryRef			userOptions	= NULL;
	SCUserPreferencesPrivateRef	userPrivate	= (SCUserPreferencesPrivateRef)userPreferences;

	if (!isA_SCUserPreferences(userPreferences)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	(void) processPreferences(userPrivate->serviceID,
				  copyOptionsCallout,
				  (void *)userPrivate->prefsID,
				  NULL,
				  &userOptions);

	/*
	 * For some legacy preferences, some of the user options
	 * were missing yet handled by the APIs.  Make sure that
	 * everything still works!
	 */
	if (userOptions != NULL) {
		update_PPP_entity  (userPreferences, &userOptions);
		update_IPSec_entity(userPreferences, &userOptions);
	}

	ok = SCNetworkConnectionStart(connection, userOptions, linger);

	if (userOptions != NULL) {
		CFRelease(userOptions);
	}

	return ok;
}


#pragma mark -
#pragma mark SCUserPreferences + SCNetworkInterface Password SPIs


static CFStringRef
getUserPasswordID(CFDictionaryRef config, SCUserPreferencesRef userPreferences)
{
	CFStringRef	unique_id	= NULL;

	if (config != NULL) {
		CFStringRef	encryption;

		encryption = CFDictionaryGetValue(config, kSCPropNetPPPAuthPasswordEncryption);
		if (isA_CFString(encryption) &&
		    CFEqual(encryption, kSCValNetPPPAuthPasswordEncryptionKeychain)) {
			unique_id = CFDictionaryGetValue(config, kSCPropNetPPPAuthPassword);
		}
	}
	if (unique_id == NULL) {
		unique_id = SCUserPreferencesGetUniqueID(userPreferences);
	}

	return unique_id;
}


static CFStringRef
copyUserSharedSecretID(CFDictionaryRef config, SCUserPreferencesRef userPreferences)
{
	CFMutableStringRef	sharedSecret	= NULL;

	if (config != NULL) {
		CFStringRef	encryption;

		encryption = CFDictionaryGetValue(config, kSCPropNetIPSecSharedSecretEncryption);
		if (isA_CFString(encryption) &&
		    CFEqual(encryption, kSCValNetIPSecSharedSecretEncryptionKeychain)) {
			sharedSecret = (CFMutableStringRef)CFDictionaryGetValue(config, kSCPropNetIPSecSharedSecret);
			if (sharedSecret != NULL) {
				CFRetain(sharedSecret);
			}
		}
	}

	if (sharedSecret == NULL) {
		CFStringRef	unique_id;

		unique_id = getUserPasswordID(config, userPreferences);
		sharedSecret = CFStringCreateMutableCopy(NULL, 0, unique_id);
		CFStringAppend(sharedSecret, CFSTR(".SS"));
	}

	return sharedSecret;
}


static CFStringRef
copyUserXAuthID(CFDictionaryRef config, SCUserPreferencesRef userPreferences)
{
	CFMutableStringRef	xauth_id	= NULL;

	if (config != NULL) {
		CFStringRef	encryption;

		encryption = CFDictionaryGetValue(config, kSCPropNetIPSecXAuthPasswordEncryption);
		if (isA_CFString(encryption) &&
		    CFEqual(encryption, kSCValNetIPSecXAuthPasswordEncryptionKeychain)) {
			xauth_id = (CFMutableStringRef)CFDictionaryGetValue(config, kSCPropNetIPSecXAuthPassword);
			if (xauth_id != NULL) {
				CFRetain(xauth_id);
			}
		}
	}

	if (xauth_id == NULL) {
		CFStringRef	unique_id;

		unique_id = getUserPasswordID(config, userPreferences);
		xauth_id = CFStringCreateMutableCopy(NULL, 0, unique_id);
		CFStringAppend(xauth_id, CFSTR(".XAUTH"));
	}

	return xauth_id;
}


static Boolean
checkUserPreferencesPassword(SCUserPreferencesRef		userPreferences,
			     SCNetworkInterfaceRef		interface,
			     SCNetworkInterfacePasswordType	passwordType)
{
	if (!isA_SCUserPreferences(userPreferences)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	if (!isA_SCNetworkInterface(interface)) {
		_SCErrorSet(kSCStatusInvalidArgument);
		return FALSE;
	}

	switch (passwordType) {
		case kSCNetworkInterfacePasswordTypePPP : {
			CFStringRef	interfaceType;

			interfaceType = SCNetworkInterfaceGetInterfaceType(interface);
			if (!CFEqual(interfaceType, kSCNetworkInterfaceTypePPP)) {
				_SCErrorSet(kSCStatusInvalidArgument);
				return FALSE;
			}
			break;
		}

		case kSCNetworkInterfacePasswordTypeIPSecSharedSecret : {
			CFStringRef		interfaceType;

			interfaceType = SCNetworkInterfaceGetInterfaceType(interface);
			if (!CFEqual(interfaceType, kSCNetworkInterfaceTypePPP)) {
				_SCErrorSet(kSCStatusInvalidArgument);
				return FALSE;
			}

			interface = SCNetworkInterfaceGetInterface(interface);
			if (interface == NULL) {
				_SCErrorSet(kSCStatusInvalidArgument);
				return FALSE;
			}

			interfaceType = SCNetworkInterfaceGetInterfaceType(interface);
			if (!CFEqual(interfaceType, kSCNetworkInterfaceTypeL2TP)) {
				_SCErrorSet(kSCStatusInvalidArgument);
				return FALSE;
			}
			break;
		}

		case kSCNetworkInterfacePasswordTypeEAPOL : {
			_SCErrorSet(kSCStatusInvalidArgument);
			return FALSE;
		}

		case kSCNetworkInterfacePasswordTypeIPSecXAuth : {
			CFStringRef	interfaceType;

			interfaceType = SCNetworkInterfaceGetInterfaceType(interface);
			if (!CFEqual(interfaceType, kSCNetworkInterfaceTypeIPSec)) {
				_SCErrorSet(kSCStatusInvalidArgument);
				return FALSE;
			}
			break;
		}

		case kSCNetworkInterfacePasswordTypeVPN : {
			CFStringRef	interfaceType;

			interfaceType = SCNetworkInterfaceGetInterfaceType(interface);
			if (!CFEqual(interfaceType, kSCNetworkInterfaceTypeVPN)) {
				_SCErrorSet(kSCStatusInvalidArgument);
				return FALSE;
			}
			break;
		}

		default :
			break;
	}

	return TRUE;
}


Boolean
SCUserPreferencesCheckInterfacePassword(SCUserPreferencesRef		userPreferences,
					SCNetworkInterfaceRef		interface,
					SCNetworkInterfacePasswordType	passwordType)
{
	Boolean		exists	= FALSE;

	if (!checkUserPreferencesPassword(userPreferences, interface, passwordType)) {
		return FALSE;
	}

	switch (passwordType) {
		case kSCNetworkInterfacePasswordTypePPP : {
			CFDictionaryRef	config;
			CFStringRef	unique_id;

			// get configuration
			config = SCUserPreferencesCopyInterfaceConfiguration(userPreferences, interface);

			// get userPreferences ID
			unique_id = getUserPasswordID(config, userPreferences);

			// check
			exists = __extract_password(NULL,
						    config,
						    kSCPropNetPPPAuthPassword,
						    kSCPropNetPPPAuthPasswordEncryption,
						    kSCValNetPPPAuthPasswordEncryptionKeychain,
						    unique_id,
						    NULL);

			if (config != NULL)	CFRelease(config);
			break;
		}

		case kSCNetworkInterfacePasswordTypeIPSecSharedSecret : {
			CFDictionaryRef	config;
			CFStringRef	shared_id;

			// get configuration
			config = SCUserPreferencesCopyExtendedInterfaceConfiguration(userPreferences,
										     interface,
										     kSCEntNetIPSec);

			// get sharedSecret ID
			shared_id = copyUserSharedSecretID(config, userPreferences);

			// check
			exists = __extract_password(NULL,
						    config,
						    kSCPropNetIPSecSharedSecret,
						    kSCPropNetIPSecSharedSecretEncryption,
						    kSCValNetIPSecSharedSecretEncryptionKeychain,
						    shared_id,
						    NULL);

			if (config != NULL)	CFRelease(config);
			CFRelease(shared_id);
			break;
		}

		case kSCNetworkInterfacePasswordTypeIPSecXAuth : {
			CFDictionaryRef	config;
			CFStringRef	xauth_id;

			// get configuration
			config = SCUserPreferencesCopyInterfaceConfiguration(userPreferences, interface);

			// get XAuth ID
			xauth_id = copyUserXAuthID(config, userPreferences);

			// check
			exists = __extract_password(NULL,
						    config,
						    kSCPropNetIPSecXAuthPassword,
						    kSCPropNetIPSecXAuthPasswordEncryption,
						    kSCValNetIPSecXAuthPasswordEncryptionKeychain,
						    xauth_id,
						    NULL);

			if (config != NULL)	CFRelease(config);
			CFRelease(xauth_id);
			break;
		}

		case kSCNetworkInterfacePasswordTypeVPN : {
			CFDictionaryRef	config;
			CFStringRef	unique_id;

			// get configuration
			config = SCUserPreferencesCopyInterfaceConfiguration(userPreferences, interface);

			// get userPreferences ID
			unique_id = getUserPasswordID(config, userPreferences);

			// check
			exists = __extract_password(NULL,
						    config,
						    kSCPropNetVPNAuthPassword,
						    kSCPropNetVPNAuthPasswordEncryption,
						    kSCValNetVPNAuthPasswordEncryptionKeychain,
						    unique_id,
						    NULL);

			if (config != NULL)	CFRelease(config);
			break;
		}

		default :
			_SCErrorSet(kSCStatusInvalidArgument);
			return FALSE;
	}

	return exists;
}


CFDataRef
SCUserPreferencesCopyInterfacePassword(SCUserPreferencesRef		userPreferences,
				       SCNetworkInterfaceRef		interface,
				       SCNetworkInterfacePasswordType	passwordType)
{
	CFDataRef	password	= NULL;

	if (!checkUserPreferencesPassword(userPreferences, interface, passwordType)) {
		return FALSE;
	}

	switch (passwordType) {
		case kSCNetworkInterfacePasswordTypePPP : {
			CFDictionaryRef	config;
			CFStringRef	unique_id;

			// get configuration
			config = SCUserPreferencesCopyInterfaceConfiguration(userPreferences, interface);

			// get userPreferences ID
			unique_id = getUserPasswordID(config, userPreferences);

			// extract
			(void) __extract_password(NULL,
						  config,
						  kSCPropNetPPPAuthPassword,
						  kSCPropNetPPPAuthPasswordEncryption,
						  kSCValNetPPPAuthPasswordEncryptionKeychain,
						  unique_id,
						  &password);

			if (config != NULL)	CFRelease(config);
			break;
		}

		case kSCNetworkInterfacePasswordTypeIPSecSharedSecret : {
			CFDictionaryRef	config;
			CFStringRef	shared_id;

			// get configuration
			config = SCUserPreferencesCopyExtendedInterfaceConfiguration(userPreferences,
										     interface,
										     kSCEntNetIPSec);

			// get sharedSecret ID
			shared_id = copyUserSharedSecretID(config, userPreferences);

			// extract
			(void) __extract_password(NULL,
						  config,
						  kSCPropNetIPSecSharedSecret,
						  kSCPropNetIPSecSharedSecretEncryption,
						  kSCValNetIPSecSharedSecretEncryptionKeychain,
						  shared_id,
						  &password);

			if (config != NULL)	CFRelease(config);
			CFRelease(shared_id);
			break;
		}

		case kSCNetworkInterfacePasswordTypeIPSecXAuth : {
			CFDictionaryRef	config;
			CFStringRef	xauth_id;

			// get configuration
			config = SCUserPreferencesCopyInterfaceConfiguration(userPreferences, interface);

			// get XAuth ID
			xauth_id = copyUserXAuthID(config, userPreferences);

			// extract
			(void) __extract_password(NULL,
						  config,
						  kSCPropNetIPSecXAuthPassword,
						  kSCPropNetIPSecXAuthPasswordEncryption,
						  kSCValNetIPSecXAuthPasswordEncryptionKeychain,
						  xauth_id,
						  &password);

			if (config != NULL)	CFRelease(config);
			CFRelease(xauth_id);
			break;
		}

		case kSCNetworkInterfacePasswordTypeVPN : {
			CFDictionaryRef	config;
			CFStringRef	unique_id;

			// get configuration
			config = SCUserPreferencesCopyInterfaceConfiguration(userPreferences, interface);

			// get userPreferences ID
			unique_id = getUserPasswordID(config, userPreferences);

			// extract
			(void) __extract_password(NULL,
						  config,
						  kSCPropNetVPNAuthPassword,
						  kSCPropNetVPNAuthPasswordEncryption,
						  kSCValNetVPNAuthPasswordEncryptionKeychain,
						  unique_id,
						  &password);

			if (config != NULL)	CFRelease(config);
			break;
		}

		default :
			_SCErrorSet(kSCStatusInvalidArgument);
			return NULL;
	}

	return password;
}


Boolean
SCUserPreferencesRemoveInterfacePassword(SCUserPreferencesRef		userPreferences,
					 SCNetworkInterfaceRef		interface,
					 SCNetworkInterfacePasswordType	passwordType)
{
	Boolean		ok	= FALSE;

	if (!checkUserPreferencesPassword(userPreferences, interface, passwordType)) {
		return FALSE;
	}

	switch (passwordType) {
		case kSCNetworkInterfacePasswordTypePPP : {
			CFDictionaryRef	config;
			CFDictionaryRef	newConfig	= NULL;
			CFStringRef	unique_id;

			// get configuration
			config = SCUserPreferencesCopyInterfaceConfiguration(userPreferences, interface);

			// get userPreferences ID
			unique_id = getUserPasswordID(config, userPreferences);

			// remove password
			ok = __remove_password(NULL,
					       config,
					       kSCPropNetPPPAuthPassword,
					       kSCPropNetPPPAuthPasswordEncryption,
					       kSCValNetPPPAuthPasswordEncryptionKeychain,
					       unique_id,
					       &newConfig);
			if (ok) {
				ok = SCUserPreferencesSetInterfaceConfiguration(userPreferences, interface, newConfig);
				if (newConfig != NULL) CFRelease(newConfig);
			}

			if (config != NULL) CFRelease(config);
			break;
		}

		case kSCNetworkInterfacePasswordTypeIPSecSharedSecret : {
			CFDictionaryRef	config;
			CFDictionaryRef	newConfig	= NULL;
			CFStringRef	shared_id;

			// get configuration
			config = SCUserPreferencesCopyExtendedInterfaceConfiguration(userPreferences,
										     interface,
										     kSCEntNetIPSec);

			// get sharedSecret ID
			shared_id = copyUserSharedSecretID(config, userPreferences);

			// remove password
			ok = __remove_password(NULL,
					       config,
					       kSCPropNetIPSecSharedSecret,
					       kSCPropNetIPSecSharedSecretEncryption,
					       kSCValNetIPSecSharedSecretEncryptionKeychain,
					       shared_id,
					       &newConfig);
			if (ok) {
				ok = SCUserPreferencesSetExtendedInterfaceConfiguration(userPreferences,
											interface,
											kSCEntNetIPSec,
											newConfig);
				if (newConfig != NULL) CFRelease(newConfig);
			}

			if (config != NULL) CFRelease(config);
			CFRelease(shared_id);
			break;
		}

		case kSCNetworkInterfacePasswordTypeIPSecXAuth : {
			CFDictionaryRef	config;
			CFDictionaryRef	newConfig	= NULL;
			CFStringRef	xauth_id;

			// get configuration
			config = SCUserPreferencesCopyInterfaceConfiguration(userPreferences, interface);

			// get XAuth ID
			xauth_id = copyUserXAuthID(config, userPreferences);

			// remove password
			ok = __remove_password(NULL,
					       config,
					       kSCPropNetIPSecXAuthPassword,
					       kSCPropNetIPSecXAuthPasswordEncryption,
					       kSCValNetIPSecXAuthPasswordEncryptionKeychain,
					       xauth_id,
					       &newConfig);
			if (ok) {
				ok = SCUserPreferencesSetInterfaceConfiguration(userPreferences, interface, newConfig);
				if (newConfig != NULL) CFRelease(newConfig);
			}

			if (config != NULL) CFRelease(config);
			CFRelease(xauth_id);
			break;
		}

		case kSCNetworkInterfacePasswordTypeVPN : {
			CFDictionaryRef	config;
			CFDictionaryRef	newConfig	= NULL;
			CFStringRef	unique_id;

			// get configuration
			config = SCUserPreferencesCopyInterfaceConfiguration(userPreferences, interface);

			// get userPreferences ID
			unique_id = getUserPasswordID(config, userPreferences);

			// remove password
			ok = __remove_password(NULL,
					       config,
					       kSCPropNetVPNAuthPassword,
					       kSCPropNetVPNAuthPasswordEncryption,
					       kSCValNetVPNAuthPasswordEncryptionKeychain,
					       unique_id,
					       &newConfig);
			if (ok) {
				ok = SCUserPreferencesSetInterfaceConfiguration(userPreferences, interface, newConfig);
				if (newConfig != NULL) CFRelease(newConfig);
			}

			if (config != NULL) CFRelease(config);
			break;
		}

		default :
			_SCErrorSet(kSCStatusInvalidArgument);
			return FALSE;
	}

	return ok;
}


Boolean
SCUserPreferencesSetInterfacePassword(SCUserPreferencesRef		userPreferences,
				      SCNetworkInterfaceRef		interface,
				      SCNetworkInterfacePasswordType	passwordType,
				      CFDataRef				password,
				      CFDictionaryRef			options)
{
	CFStringRef	account		= NULL;
	CFBundleRef	bundle;
	CFDictionaryRef	config;
	CFStringRef	description	= NULL;
	CFStringRef	label		= NULL;
	Boolean		ok	= FALSE;

	if (!checkUserPreferencesPassword(userPreferences, interface, passwordType)) {
		return FALSE;
	}

	bundle = _SC_CFBundleGet();

	switch (passwordType) {
		case kSCNetworkInterfacePasswordTypePPP : {
			CFStringRef	unique_id;

			// get configuration
			config = SCUserPreferencesCopyInterfaceConfiguration(userPreferences, interface);

			// get userPreferences ID
			unique_id = getUserPasswordID(config, userPreferences);

			// User prefs auth name --> keychain "Account"
			if (config != NULL) {
				account = CFDictionaryGetValue(config, kSCPropNetPPPAuthName);
			}

			// User prefs "name" --> keychain "Name"
			label = SCUserPreferencesCopyName(userPreferences);

			// "PPP Password" --> keychain "Kind"
			if (bundle != NULL) {
				description = CFBundleCopyLocalizedString(bundle,
									  CFSTR("KEYCHAIN_KIND_PPP_PASSWORD"),
									  CFSTR("PPP Password"),
									  NULL);
			}

			// store password
			ok = _SCSecKeychainPasswordItemSet(NULL,
							   unique_id,
							   (label != NULL)       ? label       : CFSTR("Network Connection"),
							   (description != NULL) ? description : CFSTR("PPP Password"),
							   account,
							   password,
							   options);
			if (ok) {
				CFMutableDictionaryRef	newConfig;

				if (config != NULL) {
					newConfig = CFDictionaryCreateMutableCopy(NULL, 0, config);
				} else {
					newConfig = CFDictionaryCreateMutable(NULL,
									      0,
									      &kCFTypeDictionaryKeyCallBacks,
									      &kCFTypeDictionaryValueCallBacks);
				}
				CFDictionarySetValue(newConfig,
						     kSCPropNetPPPAuthPassword,
						     unique_id);
				CFDictionarySetValue(newConfig,
						     kSCPropNetPPPAuthPasswordEncryption,
						     kSCValNetPPPAuthPasswordEncryptionKeychain);
				ok = SCUserPreferencesSetInterfaceConfiguration(userPreferences, interface, newConfig);
				CFRelease(newConfig);
			}

			if (config      != NULL) CFRelease(config);
			if (description != NULL) CFRelease(description);
			if (label       != NULL) CFRelease(label);
			break;
		}

		case kSCNetworkInterfacePasswordTypeIPSecSharedSecret : {
			CFStringRef	shared_id;

			// get configuration
			config = SCUserPreferencesCopyExtendedInterfaceConfiguration(userPreferences,
										     interface,
										     kSCEntNetIPSec);

			// get sharedSecret ID
			shared_id = copyUserSharedSecretID(config, userPreferences);

			// User prefs "name" --> keychain "Name"
			label = SCUserPreferencesCopyName(userPreferences);

			// "IPSec Shared Secret" --> keychain "Kind"
			if (bundle != NULL) {
				description = CFBundleCopyLocalizedString(bundle,
									  CFSTR("KEYCHAIN_KIND_IPSEC_SHARED_SECRET"),
									  CFSTR("IPSec Shared Secret"),
									  NULL);
			}

			// set password
			ok = _SCSecKeychainPasswordItemSet(NULL,
							   shared_id,
							   (label != NULL)       ? label       : CFSTR("Network Connection"),
							   (description != NULL) ? description : CFSTR("IPSec Shared Secret"),
							   NULL,
							   password,
							   options);
			if (ok) {
				CFMutableDictionaryRef	newConfig	= NULL;

				if (config != NULL) {
					newConfig = CFDictionaryCreateMutableCopy(NULL, 0, config);
				} else {
					newConfig = CFDictionaryCreateMutable(NULL,
									      0,
									      &kCFTypeDictionaryKeyCallBacks,
									      &kCFTypeDictionaryValueCallBacks);
				}
				CFDictionarySetValue(newConfig,
						     kSCPropNetIPSecSharedSecret,
						     shared_id);
				CFDictionarySetValue(newConfig,
						     kSCPropNetIPSecSharedSecretEncryption,
						     kSCValNetIPSecSharedSecretEncryptionKeychain);
				ok = SCUserPreferencesSetExtendedInterfaceConfiguration(userPreferences,
											interface,
											kSCEntNetIPSec,
											newConfig);
				CFRelease(newConfig);
			}

			if (config      != NULL) CFRelease(config);
			if (description != NULL) CFRelease(description);
			if (label       != NULL) CFRelease(label);
			CFRelease(shared_id);
			break;
		}

		case kSCNetworkInterfacePasswordTypeIPSecXAuth : {
			CFStringRef	xauth_id;

			// get configuration
			config = SCUserPreferencesCopyInterfaceConfiguration(userPreferences, interface);

			// get XAuth ID
			xauth_id = copyUserXAuthID(config, userPreferences);

			// User prefs XAuth name --> keychain "Account"
			if (config != NULL) {
				account = CFDictionaryGetValue(config, kSCPropNetIPSecXAuthName);
			}

			// User prefs "name" --> keychain "Name"
			label = SCUserPreferencesCopyName(userPreferences);

			// "IPSec XAuth Password" --> keychain "Kind"
			if (bundle != NULL) {
				description = CFBundleCopyLocalizedString(bundle,
									  CFSTR("KEYCHAIN_KIND_IPSEC_XAUTH_PASSWORD"),
									  CFSTR("IPSec XAuth Password"),
									  NULL);
			}

			// store password
			ok = _SCSecKeychainPasswordItemSet(NULL,
							   xauth_id,
							   (label != NULL)       ? label       : CFSTR("Network Connection"),
							   (description != NULL) ? description : CFSTR("IPSec XAuth Password"),
							   account,
							   password,
							   options);
			if (ok) {
				CFMutableDictionaryRef	newConfig;

				if (config != NULL) {
					newConfig = CFDictionaryCreateMutableCopy(NULL, 0, config);
				} else {
					newConfig = CFDictionaryCreateMutable(NULL,
									      0,
									      &kCFTypeDictionaryKeyCallBacks,
									      &kCFTypeDictionaryValueCallBacks);
				}
				CFDictionarySetValue(newConfig,
						     kSCPropNetIPSecXAuthPassword,
						     xauth_id);
				CFDictionarySetValue(newConfig,
						     kSCPropNetIPSecXAuthPasswordEncryption,
						     kSCValNetIPSecXAuthPasswordEncryptionKeychain);
				ok = SCUserPreferencesSetInterfaceConfiguration(userPreferences, interface, newConfig);
				CFRelease(newConfig);
			}

			if (config      != NULL) CFRelease(config);
			if (description != NULL) CFRelease(description);
			if (label       != NULL) CFRelease(label);
			CFRelease(xauth_id);
			break;
		}

		case kSCNetworkInterfacePasswordTypeVPN : {
			CFStringRef	unique_id;

			// get configuration
			config = SCUserPreferencesCopyInterfaceConfiguration(userPreferences, interface);

			// get userPreferences ID
			unique_id = getUserPasswordID(config, userPreferences);

			// User prefs auth name --> keychain "Account"
			if (config != NULL) {
				account = CFDictionaryGetValue(config, kSCPropNetVPNAuthName);
			}

			// User prefs "name" --> keychain "Name"
			label = SCUserPreferencesCopyName(userPreferences);

			// "VPN Password" --> keychain "Kind"
			if (bundle != NULL) {
				description = CFBundleCopyLocalizedString(bundle,
									  CFSTR("KEYCHAIN_KIND_VPN_PASSWORD"),
									  CFSTR("VPN Password"),
									  NULL);
			}

			// store password
			ok = _SCSecKeychainPasswordItemSet(NULL,
							   unique_id,
							   (label != NULL)       ? label       : CFSTR("Network Connection"),
							   (description != NULL) ? description : CFSTR("VPN Password"),
							   account,
							   password,
							   options);
			if (ok) {
				CFMutableDictionaryRef	newConfig;

				if (config != NULL) {
					newConfig = CFDictionaryCreateMutableCopy(NULL, 0, config);
				} else {
					newConfig = CFDictionaryCreateMutable(NULL,
									      0,
									      &kCFTypeDictionaryKeyCallBacks,
									      &kCFTypeDictionaryValueCallBacks);
				}
				assert(newConfig != NULL);
				CFDictionarySetValue(newConfig,
						     kSCPropNetVPNAuthPassword,
						     unique_id);
				CFDictionarySetValue(newConfig,
						     kSCPropNetVPNAuthPasswordEncryption,
						     kSCValNetVPNAuthPasswordEncryptionKeychain);
				ok = SCUserPreferencesSetInterfaceConfiguration(userPreferences, interface, newConfig);
				CFRelease(newConfig);
			}

			if (config      != NULL) CFRelease(config);
			if (description != NULL) CFRelease(description);
			if (label       != NULL) CFRelease(label);
			break;
		}

		default :
			_SCErrorSet(kSCStatusInvalidArgument);
			return FALSE;
	}

	return ok;
}
