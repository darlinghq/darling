#include "CFPreferences.h"
#include <util/debug.h>
#import <Foundation/NSUserDefaults.h>

#ifndef CONST_STRING_DECL
#	define CONST_STRING_DECL(name, value) const CFStringRef name = CFSTR(value);
#endif

CONST_STRING_DECL(kCFPreferencesAnyApplication,
  "kCFPreferencesAnyApplication");
CONST_STRING_DECL(kCFPreferencesCurrentApplication,
  "kCFPreferencesCurrentApplication");
CONST_STRING_DECL(kCFPreferencesAnyHost,
  "kCFPreferencesAnyHost");
CONST_STRING_DECL(kCFPreferencesCurrentHost,
  "kCFPreferencesCurrentHost");
CONST_STRING_DECL(kCFPreferencesAnyUser,
  "kCFPreferencesAnyUser");
CONST_STRING_DECL(kCFPreferencesCurrentUser,
  "kCFPreferencesCurrentUser");

CFPropertyListRef CFPreferencesCopyAppValue(CFStringRef key, CFStringRef applicationID)
{
	if (applicationID == kCFPreferencesCurrentApplication)
	{
		return [[NSUserDefaults standardUserDefaults] objectForKey: (NSString*) key];
	}
	else
	{
		STUB();
		return NULL;
	}
}

Boolean CFPreferencesGetAppBooleanValue(CFStringRef key, CFStringRef applicationID, Boolean *keyExistsAndHasValidFormat)
{
	STUB();
	return 0;
}

CFIndex CFPreferencesGetAppIntegerValue(CFStringRef key, CFStringRef applicationID, Boolean *keyExistsAndHasValidFormat)
{
	STUB();
	return -1;
}

void CFPreferencesSetAppValue(CFStringRef key, CFPropertyListRef value, CFStringRef applicationID)
{
	STUB();
}

void CFPreferencesAddSuitePreferencesToApp(CFStringRef applicationID, CFStringRef suiteID)
{
	STUB();
}

void CFPreferencesRemoveSuitePreferencesFromApp(CFStringRef applicationID, CFStringRef suiteID)
{
	STUB();
}

Boolean CFPreferencesAppSynchronize(CFStringRef applicationID)
{
	STUB();
	return 0;
}

CFPropertyListRef CFPreferencesCopyValue(CFStringRef key, CFStringRef applicationID, CFStringRef userName, CFStringRef hostName)
{
	STUB();
	return NULL;
}

CFDictionaryRef CFPreferencesCopyMultiple(CFArrayRef keysToFetch, CFStringRef applicationID, CFStringRef userName, CFStringRef hostName)
{
	STUB();
	return NULL;
}

void CFPreferencesSetValue(CFStringRef key, CFPropertyListRef value, CFStringRef applicationID, CFStringRef userName, CFStringRef hostName)
{
	STUB();
}

void CFPreferencesSetMultiple(CFDictionaryRef keysToSet, CFArrayRef keysToRemove, CFStringRef applicationID, CFStringRef userName, CFStringRef hostName)
{
	STUB();
}

Boolean CFPreferencesSynchronize(CFStringRef applicationID, CFStringRef userName, CFStringRef hostName)
{
	STUB();
	return 0;
}

CFArrayRef CFPreferencesCopyApplicationList(CFStringRef userName, CFStringRef hostName)
{
	STUB();
	return NULL;
}

CFArrayRef CFPreferencesCopyKeyList(CFStringRef applicationID, CFStringRef userName, CFStringRef hostName)
{
	STUB();
	return NULL;
}

