/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// Print settings: the copies, the pages, the collation and the duplex mode of a job, and where the
// job goes

#include "PMInternal.h"
#include <limits.h>
#include <string.h>

OSStatus PMCreatePrintSettings(PMPrintSettings* printSettings)
{
	if (!printSettings)
		return kPMInvalidParameter;
	*printSettings = (PMPrintSettings) PMNewObject(PMPrintSettingsTypeID(), sizeof(struct __PMPrintSettings));
	return *printSettings ? noErr : kPMGeneralError;
}

// The keys that the defaults of a session set. There is no duplex mode in them: the mode is a key of
// the printer's.
static void resetSettings(struct __PMPrintSettings* settings)
{
	settings->first = 1;
	settings->last = INT_MAX;
	settings->minPage = 1;
	settings->maxPage = INT_MAX;
	settings->copies = 1;
	settings->collate = true;
	settings->destination = kPMDestinationPrinter;
	settings->keys |= kPMKeyCopies | kPMKeyFirstPage | kPMKeyLastPage | kPMKeyPageRange | kPMKeyCollate;
}

OSStatus PMSessionDefaultPrintSettings(PMPrintSession printSession, PMPrintSettings printSettings)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);

	if (!PMAsSession(printSession))
		return kPMInvalidPrintSession;
	if (!settings)
		return kPMInvalidPrintSettings;
	resetSettings(settings);
	return noErr;
}

OSStatus PMSessionValidatePrintSettings(PMPrintSession printSession, PMPrintSettings printSettings, Boolean* changed)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);
	UInt32 missing;

	if (!PMAsSession(printSession))
		return kPMInvalidPrintSession;
	if (!settings)
		return kPMInvalidPrintSettings;
	if (changed)
		*changed = false;
	// QUIRK: no copies, or a first page after the last one, is a job that cannot be printed, and the
	// validation puts the values that a job has by default in their place (and reports a change); the calls
	// that set them accept them. Not documented. Evidence: black-box observation on macOS 10.14.4 and 26.7.3
	// (tier 2). Test: tests/src/carbon_print_objects.c (test_quirk_validate_print_settings)
	if ((settings->keys & kPMKeyCopies) && settings->copies == 0)
	{
		settings->copies = 1;
		if (changed)
			*changed = true;
	}
	if (((settings->keys & (kPMKeyFirstPage | kPMKeyLastPage)) == (kPMKeyFirstPage | kPMKeyLastPage)) && settings->first > settings->last)
	{
		settings->first = 1;
		settings->last = INT_MAX;
		if (changed)
			*changed = true;
	}
	missing = (kPMKeyCopies | kPMKeyFirstPage | kPMKeyLastPage | kPMKeyPageRange | kPMKeyCollate) & ~settings->keys;
	if (missing)
	{
		struct __PMPrintSettings defaults;

		memset(&defaults, 0, sizeof(defaults));
		resetSettings(&defaults);
		if (missing & kPMKeyCopies) settings->copies = defaults.copies;
		if (missing & kPMKeyFirstPage) settings->first = defaults.first;
		if (missing & kPMKeyLastPage) settings->last = defaults.last;
		if (missing & kPMKeyPageRange) { settings->minPage = defaults.minPage; settings->maxPage = defaults.maxPage; }
		if (missing & kPMKeyCollate) settings->collate = defaults.collate;
		settings->keys |= missing;
		if (changed)
			*changed = true;
	}
	return noErr;
}

static void replaceDictionary(CFMutableDictionaryRef* destination, CFDictionaryRef source)
{
	if (*destination)
		CFRelease(*destination);
	*destination = source ? CFDictionaryCreateMutableCopy(NULL, 0, source) : NULL;
}

static void replaceObject(CFTypeRef* destination, CFTypeRef source)
{
	if (source)
		CFRetain(source);
	if (*destination)
		CFRelease(*destination);
	*destination = source;
}

OSStatus PMCopyPrintSettings(PMPrintSettings settingsSrc, PMPrintSettings settingsDest)
{
	struct __PMPrintSettings* source = PMAsSettings(settingsSrc);
	struct __PMPrintSettings* destination = PMAsSettings(settingsDest);

	if (!source || !destination)
		return kPMInvalidPrintSettings;
	if (source == destination)
		return noErr;
	destination->keys = source->keys;
	destination->first = source->first;
	destination->last = source->last;
	destination->minPage = source->minPage;
	destination->maxPage = source->maxPage;
	destination->copies = source->copies;
	destination->collate = source->collate;
	destination->duplex = source->duplex;
	destination->destination = source->destination;
	replaceObject((CFTypeRef*) &destination->jobName, source->jobName);
	replaceObject((CFTypeRef*) &destination->destinationFormat, source->destinationFormat);
	replaceObject((CFTypeRef*) &destination->destinationURL, source->destinationURL);
	replaceDictionary(&destination->values, source->values);
	replaceDictionary(&destination->extended, source->extended);
	return noErr;
}

// QUIRK: the call fails when the settings do not have the key, and sets the value to zero: the settings made
// by PMCreatePrintSettings have none until the defaults of a session or a call of the application sets them.
// Not documented. Evidence: black-box observation on macOS 10.14.4 and 26.7.3 (tier 2).
// Test: tests/src/carbon_print_objects.c (test_quirk_settings_keys)
static OSStatus keyedValue(PMPrintSettings printSettings, void* value, UInt32 key, size_t size, struct __PMPrintSettings** settings)
{
	*settings = PMAsSettings(printSettings);
	if (!value)
		return kPMInvalidParameter;
	if (!*settings)
		return kPMInvalidPrintSettings;
	if (!((*settings)->keys & key))
	{
		memset(value, 0, size);
		return kPMKeyNotFound;
	}
	return noErr;
}

OSStatus PMGetCopies(PMPrintSettings printSettings, UInt32* copies)
{
	struct __PMPrintSettings* settings;
	OSStatus status = keyedValue(printSettings, copies, kPMKeyCopies, sizeof(*copies), &settings);

	if (status == noErr)
		*copies = settings->copies;
	return status;
}

// QUIRK: the number of copies is not checked: none (and then the validation of the settings makes it
// one) and a lot are accepted. Not documented. Evidence: black-box observation on macOS 10.14.4 and 26.7.3
// (tier 2). Test: tests/src/carbon_print_objects.c (test_quirk_validate_print_settings)
OSStatus PMSetCopies(PMPrintSettings printSettings, UInt32 copies, Boolean lock)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);

	if (!settings)
		return kPMInvalidPrintSettings;
	settings->copies = copies;
	settings->keys |= kPMKeyCopies;
	return noErr;
}

OSStatus PMGetFirstPage(PMPrintSettings printSettings, UInt32* first)
{
	struct __PMPrintSettings* settings;
	OSStatus status = keyedValue(printSettings, first, kPMKeyFirstPage, sizeof(*first), &settings);

	if (status == noErr)
		*first = settings->first;
	return status;
}

OSStatus PMGetLastPage(PMPrintSettings printSettings, UInt32* last)
{
	struct __PMPrintSettings* settings;
	OSStatus status = keyedValue(printSettings, last, kPMKeyLastPage, sizeof(*last), &settings);

	if (status == noErr)
		*last = settings->last;
	return status;
}

// The page range that the first and the last page must lie in: the range of the settings, or every page
// when the settings have none
static void effectiveRange(const struct __PMPrintSettings* settings, UInt32* minPage, UInt32* maxPage)
{
	*minPage = settings->keys & kPMKeyPageRange ? settings->minPage : 1;
	*maxPage = settings->keys & kPMKeyPageRange ? settings->maxPage : INT_MAX;
}

// QUIRK: the first and last page must lie inside the page range of the settings, else the call fails
// with kPMValueOutOfRange; the page range stays as it was when it is set (PMSetPageRange does not move
// the first and last page), and a first page after the last one is accepted (the validation of the
// settings resets them). Not documented. Evidence: black-box observation on macOS 10.14.4 and 26.7.3
// (tier 2). Test: tests/src/printcore_session.c (test_quirk_first_last_in_range, in tests/src/carbon_print_objects.c)
OSStatus PMSetFirstPage(PMPrintSettings printSettings, UInt32 first, Boolean lock)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);
	UInt32 minPage, maxPage;

	if (!settings)
		return kPMInvalidPrintSettings;
	effectiveRange(settings, &minPage, &maxPage);
	if (first < minPage || first > maxPage)
		return kPMValueOutOfRange;
	settings->first = first;
	settings->keys |= kPMKeyFirstPage;
	return noErr;
}

// The last page that means all of them is kPMPrintAllPages
OSStatus PMSetLastPage(PMPrintSettings printSettings, UInt32 last, Boolean lock)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);
	UInt32 minPage, maxPage;

	if (!settings)
		return kPMInvalidPrintSettings;
	if (last == (UInt32) kPMPrintAllPages)
		last = INT_MAX;
	effectiveRange(settings, &minPage, &maxPage);
	if (last < minPage || last > maxPage)
		return kPMValueOutOfRange;
	settings->last = last;
	settings->keys |= kPMKeyLastPage;
	return noErr;
}

OSStatus PMGetPageRange(PMPrintSettings printSettings, UInt32* minPage, UInt32* maxPage)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);

	if (!settings)
		return kPMInvalidPrintSettings;
	if (!(settings->keys & kPMKeyPageRange))
	{
		if (minPage)
			*minPage = 0;
		if (maxPage)
			*maxPage = 0;
		return kPMKeyNotFound;
	}
	if (minPage)
		*minPage = settings->minPage;
	if (maxPage)
		*maxPage = settings->maxPage;
	return noErr;
}

OSStatus PMSetPageRange(PMPrintSettings printSettings, UInt32 minPage, UInt32 maxPage)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);

	if (!settings)
		return kPMInvalidPrintSettings;
	if (maxPage == (UInt32) kPMPrintAllPages)
		maxPage = INT_MAX;
	if (minPage > maxPage)
		return kPMValueOutOfRange;
	settings->minPage = minPage;
	settings->maxPage = maxPage;
	settings->keys |= kPMKeyPageRange;
	return noErr;
}

OSStatus PMGetCollate(PMPrintSettings printSettings, Boolean* collate)
{
	struct __PMPrintSettings* settings;
	OSStatus status = keyedValue(printSettings, collate, kPMKeyCollate, sizeof(*collate), &settings);

	if (status == noErr)
		*collate = settings->collate;
	return status;
}

OSStatus PMSetCollate(PMPrintSettings printSettings, Boolean collate)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);

	if (!settings)
		return kPMInvalidPrintSettings;
	settings->collate = collate ? true : false;
	settings->keys |= kPMKeyCollate;
	return noErr;
}

OSStatus PMGetDuplex(PMPrintSettings printSettings, PMDuplexMode* duplexSetting)
{
	struct __PMPrintSettings* settings;
	OSStatus status = keyedValue(printSettings, duplexSetting, kPMKeyDuplex, sizeof(*duplexSetting), &settings);

	if (status == noErr)
		*duplexSetting = settings->duplex;
	return status;
}

// The mode is not checked
OSStatus PMSetDuplex(PMPrintSettings printSettings, PMDuplexMode duplexSetting)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);

	if (!settings)
		return kPMInvalidPrintSettings;
	settings->duplex = duplexSetting;
	settings->keys |= kPMKeyDuplex;
	return noErr;
}

OSStatus PMPrintSettingsGetJobName(PMPrintSettings printSettings, CFStringRef* name)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);

	if (!name)
		return kPMInvalidParameter;
	if (!settings)
		return kPMInvalidPrintSettings;
	*name = settings->jobName;
	return noErr;
}

OSStatus PMPrintSettingsSetJobName(PMPrintSettings printSettings, CFStringRef name)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);

	if (!settings)
		return kPMInvalidPrintSettings;
	if (!name)
		return kPMInvalidParameter;
	replaceObject((CFTypeRef*) &settings->jobName, name);
	return noErr;
}

OSStatus PMSetJobNameCFString(PMPrintSettings printSettings, CFStringRef name)
{
	return PMPrintSettingsSetJobName(printSettings, name);
}

// The values of the application, by the keys it chooses
OSStatus PMPrintSettingsSetValue(PMPrintSettings printSettings, CFStringRef key, CFTypeRef value, Boolean locked)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);

	if (!settings)
		return kPMInvalidPrintSettings;
	if (!key)
		return kPMInvalidParameter;
	if (!settings->values)
		settings->values = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	if (value)
		CFDictionarySetValue(settings->values, key, value);
	else
		CFDictionaryRemoveValue(settings->values, key);
	return noErr;
}

OSStatus PMPrintSettingsGetValue(PMPrintSettings printSettings, CFStringRef key, CFTypeRef* value)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);
	CFTypeRef found;

	if (!settings)
		return kPMInvalidPrintSettings;
	if (!key || !value)
		return kPMInvalidParameter;
	found = settings->values ? CFDictionaryGetValue(settings->values, key) : NULL;
	*value = found;
	return found ? noErr : kPMKeyNotFound;
}

OSStatus PMSetPrintSettingsExtendedData(PMPrintSettings printSettings, OSType dataID, UInt32 size, void* extendedData)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);
	CFDataRef data;
	CFStringRef key;

	if (!settings)
		return kPMInvalidPrintSettings;
	if (!extendedData && size)
		return kPMInvalidParameter;
	if (!settings->extended)
		settings->extended = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
	data = CFDataCreate(NULL, extendedData, size);
	key = CFStringCreateWithFormat(NULL, NULL, CFSTR("%08x"), (unsigned) dataID);
	CFDictionarySetValue(settings->extended, key, data);
	CFRelease(key);
	CFRelease(data);
	return noErr;
}

OSStatus PMGetPrintSettingsExtendedData(PMPrintSettings printSettings, OSType dataID, UInt32* size, void* extendedData)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);
	CFStringRef key;
	CFDataRef data;

	if (!settings)
		return kPMInvalidPrintSettings;
	key = CFStringCreateWithFormat(NULL, NULL, CFSTR("%08x"), (unsigned) dataID);
	data = settings->extended ? CFDictionaryGetValue(settings->extended, key) : NULL;
	CFRelease(key);
	if (!data)
		return kPMKeyNotFound;
	if (extendedData && size)
		memcpy(extendedData, CFDataGetBytePtr(data), *size < CFDataGetLength(data) ? *size : CFDataGetLength(data));
	if (size)
		*size = CFDataGetLength(data);
	return noErr;
}

// ---- destination

OSStatus PMSessionGetDestinationType(PMPrintSession printSession, PMPrintSettings printSettings, PMDestinationType* destTypeP)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);

	if (!PMAsSession(printSession))
		return kPMInvalidPrintSession;
	if (!settings)
		return kPMInvalidPrintSettings;
	if (!destTypeP)
		return kPMInvalidParameter;
	*destTypeP = settings->destination ? settings->destination : kPMDestinationPrinter;
	return noErr;
}

OSStatus PMSessionSetDestination(PMPrintSession printSession, PMPrintSettings printSettings, PMDestinationType destType, CFStringRef destFormat, CFURLRef destLocation)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);

	if (!PMAsSession(printSession))
		return kPMInvalidPrintSession;
	if (!settings)
		return kPMInvalidPrintSettings;
	if (destType < kPMDestinationPrinter || destType > kPMDestinationProcessPDF)
		return kPMValueOutOfRange;
	settings->destination = destType;
	replaceObject((CFTypeRef*) &settings->destinationFormat, destFormat);
	replaceObject((CFTypeRef*) &settings->destinationURL, destLocation);
	return noErr;
}

OSStatus PMSessionCopyDestinationFormat(PMPrintSession printSession, PMPrintSettings printSettings, CFStringRef* destFormatP)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);

	if (!PMAsSession(printSession))
		return kPMInvalidPrintSession;
	if (!settings)
		return kPMInvalidPrintSettings;
	if (!destFormatP)
		return kPMInvalidParameter;
	*destFormatP = settings->destinationFormat ? CFRetain(settings->destinationFormat) : NULL;
	return noErr;
}

OSStatus PMSessionCopyDestinationLocation(PMPrintSession printSession, PMPrintSettings printSettings, CFURLRef* destLocationP)
{
	struct __PMPrintSettings* settings = PMAsSettings(printSettings);

	if (!PMAsSession(printSession))
		return kPMInvalidPrintSession;
	if (!settings)
		return kPMInvalidPrintSettings;
	if (!destLocationP)
		return kPMInvalidParameter;
	*destLocationP = settings->destinationURL ? CFRetain(settings->destinationURL) : NULL;
	return noErr;
}

// ---- the data of settings, for documents that keep them

OSStatus PMPrintSettingsCreateDataRepresentation(PMPrintSettings printSettings, CFDataRef* data, PMDataFormat format)
{
	struct __PMPrintSettings* s = PMAsSettings(printSettings);
	CFMutableDictionaryRef dictionary;
	SInt64 number;
	CFNumberRef n;

	if (!s)
		return kPMInvalidPrintSettings;
	if (!data)
		return kPMInvalidParameter;
	dictionary = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
#define NUMBER(name, value) number = (value); n = CFNumberCreate(NULL, kCFNumberSInt64Type, &number); CFDictionarySetValue(dictionary, CFSTR(name), n); CFRelease(n)
	NUMBER("keys", s->keys);
	NUMBER("first", s->first);
	NUMBER("last", s->last);
	NUMBER("minPage", s->minPage);
	NUMBER("maxPage", s->maxPage);
	NUMBER("copies", s->copies);
	NUMBER("duplex", s->duplex);
	NUMBER("destination", s->destination);
#undef NUMBER
	CFDictionarySetValue(dictionary, CFSTR("collate"), s->collate ? kCFBooleanTrue : kCFBooleanFalse);
	if (s->jobName)
		CFDictionarySetValue(dictionary, CFSTR("jobName"), s->jobName);
	if (s->destinationFormat)
		CFDictionarySetValue(dictionary, CFSTR("destinationFormat"), s->destinationFormat);
	if (s->destinationURL)
	{
		CFStringRef string = CFURLGetString(s->destinationURL);

		if (string)
			CFDictionarySetValue(dictionary, CFSTR("destinationURL"), string);
	}
	if (s->extended)
		CFDictionarySetValue(dictionary, CFSTR("extended"), s->extended);
	*data = CFPropertyListCreateData(NULL, dictionary, kCFPropertyListXMLFormat_v1_0, 0, NULL);
	CFRelease(dictionary);
	return *data ? noErr : kPMGeneralError;
}

static UInt32 number32(CFDictionaryRef dictionary, CFStringRef key, UInt32 fallback)
{
	CFTypeRef n = CFDictionaryGetValue(dictionary, key);
	SInt64 value;

	if (n && CFGetTypeID(n) == CFNumberGetTypeID() && CFNumberGetValue(n, kCFNumberSInt64Type, &value))
		return (UInt32) value;
	return fallback;
}

OSStatus PMPrintSettingsCreateWithDataRepresentation(CFDataRef data, PMPrintSettings* printSettings)
{
	CFTypeRef list;
	OSStatus status = kPMInvalidObject;

	if (!data || !printSettings)
		return kPMInvalidParameter;
	list = CFPropertyListCreateWithData(NULL, data, kCFPropertyListImmutable, NULL, NULL);
	if (list && CFGetTypeID(list) == CFDictionaryGetTypeID())
	{
		CFDictionaryRef dictionary = list;
		struct __PMPrintSettings* s;

		status = PMCreatePrintSettings((PMPrintSettings*) &s);
		if (status == noErr)
		{
			CFTypeRef value;

			s->keys = number32(dictionary, CFSTR("keys"), 0);
			s->first = number32(dictionary, CFSTR("first"), 1);
			s->last = number32(dictionary, CFSTR("last"), INT_MAX);
			s->minPage = number32(dictionary, CFSTR("minPage"), 1);
			s->maxPage = number32(dictionary, CFSTR("maxPage"), INT_MAX);
			s->copies = number32(dictionary, CFSTR("copies"), 1);
			s->duplex = number32(dictionary, CFSTR("duplex"), 0);
			s->destination = number32(dictionary, CFSTR("destination"), kPMDestinationPrinter);
			s->collate = CFDictionaryGetValue(dictionary, CFSTR("collate")) == kCFBooleanTrue;
			if ((value = CFDictionaryGetValue(dictionary, CFSTR("jobName"))) && CFGetTypeID(value) == CFStringGetTypeID())
				s->jobName = CFRetain(value);
			if ((value = CFDictionaryGetValue(dictionary, CFSTR("destinationFormat"))) && CFGetTypeID(value) == CFStringGetTypeID())
				s->destinationFormat = CFRetain(value);
			if ((value = CFDictionaryGetValue(dictionary, CFSTR("destinationURL"))) && CFGetTypeID(value) == CFStringGetTypeID())
				s->destinationURL = CFURLCreateWithString(NULL, value, NULL);
			if ((value = CFDictionaryGetValue(dictionary, CFSTR("extended"))) && CFGetTypeID(value) == CFDictionaryGetTypeID())
				s->extended = CFDictionaryCreateMutableCopy(NULL, 0, value);
			*printSettings = (PMPrintSettings) s;
		}
	}
	if (list)
		CFRelease(list);
	return status;
}

#ifndef __LP64__
OSStatus PMFlattenPrintSettings(PMPrintSettings printSettings, Handle* flatSettings)
{
	CFDataRef data;
	OSStatus status;

	if (!flatSettings)
		return kPMInvalidParameter;
	status = PMPrintSettingsCreateDataRepresentation(printSettings, &data, kPMDataFormatXMLDefault);
	if (status == noErr)
	{
		Handle handle = NewHandle(CFDataGetLength(data));

		if (handle)
		{
			memcpy(*handle, CFDataGetBytePtr(data), CFDataGetLength(data));
			*flatSettings = handle;
		}
		else
			status = kPMGeneralError;
		CFRelease(data);
	}
	return status;
}

OSStatus PMUnflattenPrintSettings(Handle flatSettings, PMPrintSettings* printSettings)
{
	CFDataRef data;
	OSStatus status;

	if (!printSettings || !flatSettings)
		return kPMInvalidParameter;
	data = CFDataCreate(NULL, (const UInt8*) *flatSettings, GetHandleSize(flatSettings));
	status = PMPrintSettingsCreateWithDataRepresentation(data, printSettings);
	CFRelease(data);
	return status;
}
#endif
