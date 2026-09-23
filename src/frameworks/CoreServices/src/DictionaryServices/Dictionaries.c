/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#include <DictionaryServices/DictionaryServices.h>
#include <CoreFoundation/CFRuntime.h>
#include <dirent.h>
#include <dispatch/dispatch.h>
#include <limits.h>
#include <pthread.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define kDictionaryServicesDomain CFSTR("com.apple.DictionaryServices")
#define kActiveDictionariesKey CFSTR("DCSActiveDictionaries")
#define kDictionaryLanguagesKey CFSTR("DCSDictionaryLanguages")
#define kAssetDictionariesPath "/System/Library/AssetsV2/com_apple_MobileAsset_DictionaryServices_dictionaryOSX"

struct __DCSDictionary {
	CFRuntimeBase base;
	CFURLRef url;
	CFBundleRef bundle;
};

static CFTypeID dictionaryTypeID;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static CFArrayRef activeDictionaries;

static void dictionaryFinalize(CFTypeRef cf)
{
	struct __DCSDictionary *dictionary = (struct __DCSDictionary *) cf;
	CFRelease(dictionary->url);
	CFRelease(dictionary->bundle);
}

static Boolean dictionaryEqual(CFTypeRef a, CFTypeRef b)
{
	return CFEqual(((DCSDictionaryRef) a)->url, ((DCSDictionaryRef) b)->url);
}

static CFHashCode dictionaryHash(CFTypeRef cf)
{
	return CFHash(((DCSDictionaryRef) cf)->url);
}

static CFStringRef dictionaryCopyDescription(CFTypeRef cf)
{
	return CFStringCreateWithFormat(NULL, NULL, CFSTR("<DCSDictionary %p %@>"), cf, ((DCSDictionaryRef) cf)->url);
}

static const CFRuntimeClass dictionaryClass = {
	0, "DCSDictionary", NULL, NULL, dictionaryFinalize, dictionaryEqual, dictionaryHash,
	NULL, dictionaryCopyDescription,
};

static CFTypeID getDictionaryTypeID(void)
{
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		dictionaryTypeID = _CFRuntimeRegisterClass(&dictionaryClass);
	});
	return dictionaryTypeID;
}

DCSDictionaryRef DCSDictionaryCreate(CFURLRef url)
{
	if (url == NULL)
		return NULL;
	CFBundleRef bundle = CFBundleCreate(NULL, url);
	if (bundle == NULL)
		return NULL;
	CFDictionaryRef info = CFBundleGetInfoDictionary(bundle);
	if (info == NULL || CFDictionaryGetCount(info) == 0) {
		CFRelease(bundle);
		return NULL;
	}

	struct __DCSDictionary *dictionary = (struct __DCSDictionary *) _CFRuntimeCreateInstance(NULL,
		getDictionaryTypeID(), sizeof(struct __DCSDictionary) - sizeof(CFRuntimeBase), NULL);
	if (dictionary == NULL) {
		CFRelease(bundle);
		return NULL;
	}
	dictionary->url = CFRetain(url);
	dictionary->bundle = bundle;
	return dictionary;
}

static Boolean isDictionary(DCSDictionaryRef dictionary)
{
	return dictionary != NULL && CFGetTypeID(dictionary) == getDictionaryTypeID();
}

CFURLRef DCSDictionaryGetURL(DCSDictionaryRef dictionary)
{
	return isDictionary(dictionary) ? dictionary->url : NULL;
}

CFStringRef DCSDictionaryGetIdentifier(DCSDictionaryRef dictionary)
{
	return isDictionary(dictionary) ? CFBundleGetIdentifier(dictionary->bundle) : NULL;
}

static CFTypeRef getInfoValue(DCSDictionaryRef dictionary, CFStringRef key, CFTypeID type)
{
	if (!isDictionary(dictionary))
		return NULL;
	CFTypeRef value = CFBundleGetValueForInfoDictionaryKey(dictionary->bundle, key);
	return value != NULL && CFGetTypeID(value) == type ? value : NULL;
}

CFStringRef DCSDictionaryGetName(DCSDictionaryRef dictionary)
{
	CFStringRef name = getInfoValue(dictionary, CFSTR("CFBundleDisplayName"), CFStringGetTypeID());
	return name != NULL ? name : getInfoValue(dictionary, kCFBundleNameKey, CFStringGetTypeID());
}

CFArrayRef DCSDictionaryGetLanguages(DCSDictionaryRef dictionary)
{
	return getInfoValue(dictionary, kDictionaryLanguagesKey, CFArrayGetTypeID());
}

static Boolean hasDictionarySuffix(const char *name)
{
	static const char suffix[] = ".dictionary";
	size_t length = strlen(name);
	return length > sizeof(suffix) - 1 && strcmp(name + length - (sizeof(suffix) - 1), suffix) == 0;
}

// The form stored in the DCSActiveDictionaries preference.
static CFStringRef copyPath(DCSDictionaryRef dictionary)
{
	return CFURLCopyFileSystemPath(dictionary->url, kCFURLPOSIXPathStyle);
}

// Adds every *.dictionary bundle directly inside dir, keyed by its path.
static void addDictionariesIn(const char *dir, CFMutableDictionaryRef found)
{
	DIR *d = opendir(dir);
	if (d == NULL)
		return;
	struct dirent *entry;
	while ((entry = readdir(d)) != NULL) {
		char path[PATH_MAX];
		if (!hasDictionarySuffix(entry->d_name) || snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name) >= (int) sizeof(path))
			continue;
		CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *) path, strlen(path), true);
		DCSDictionaryRef dictionary = url != NULL ? DCSDictionaryCreate(url) : NULL;
		if (dictionary != NULL) {
			CFStringRef key = copyPath(dictionary);
			CFDictionarySetValue(found, key, dictionary);
			CFRelease(key);
			CFRelease(dictionary);
		}
		if (url != NULL)
			CFRelease(url);
	}
	closedir(d);
}

// MobileAsset keeps each downloaded dictionary in <asset>/AssetData/<name>.dictionary.
static void addAssetDictionaries(CFMutableDictionaryRef found)
{
	DIR *d = opendir(kAssetDictionariesPath);
	if (d == NULL)
		return;
	struct dirent *entry;
	while ((entry = readdir(d)) != NULL) {
		char path[PATH_MAX];
		if (entry->d_name[0] == '.')
			continue;
		if (snprintf(path, sizeof(path), "%s/%s/AssetData", kAssetDictionariesPath, entry->d_name) < (int) sizeof(path))
			addDictionariesIn(path, found);
	}
	closedir(d);
}

// Path -> DCSDictionaryRef for every installed dictionary.
static CFDictionaryRef copyInstalledDictionaries(void)
{
	CFMutableDictionaryRef found = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks,
		&kCFTypeDictionaryValueCallBacks);
	struct passwd *user = getpwuid(getuid());
	char path[PATH_MAX];
	if (user != NULL && snprintf(path, sizeof(path), "%s/Library/Dictionaries", user->pw_dir) < (int) sizeof(path))
		addDictionariesIn(path, found);
	addDictionariesIn("/Library/Dictionaries", found);
	addAssetDictionaries(found);
	return found;
}

static void addToSet(const void *key, const void *value, void *set)
{
	CFSetAddValue(set, value);
}

static void appendToArray(const void *key, const void *value, void *array)
{
	CFArrayAppendValue(array, value);
}

CFSetRef DCSCopyAvailableDictionaries(void)
{
	CFDictionaryRef installed = copyInstalledDictionaries();
	CFMutableSetRef set = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
	CFDictionaryApplyFunction(installed, addToSet, set);
	CFRelease(installed);
	return set;
}

static CFComparisonResult compareNames(const void *a, const void *b, void *context)
{
	CFStringRef nameA = DCSDictionaryGetName(a), nameB = DCSDictionaryGetName(b);
	if (nameA == NULL || nameB == NULL)
		return nameA == nameB ? kCFCompareEqualTo : (nameA == NULL ? kCFCompareGreaterThan : kCFCompareLessThan);
	return CFStringCompare(nameA, nameB, kCFCompareLocalized);
}

// The preference lists bundle paths; paths that are no longer installed are skipped. Without
// the preference, every installed dictionary is active.
static CFArrayRef copyActiveDictionaries(void)
{
	CFDictionaryRef installed = copyInstalledDictionaries();
	CFMutableArrayRef active = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	CFPropertyListRef paths = CFPreferencesCopyAppValue(kActiveDictionariesKey, kDictionaryServicesDomain);

	if (paths != NULL && CFGetTypeID(paths) == CFArrayGetTypeID()) {
		for (CFIndex i = 0; i < CFArrayGetCount(paths); i++) {
			CFTypeRef path = CFArrayGetValueAtIndex(paths, i);
			const void *dictionary = CFGetTypeID(path) == CFStringGetTypeID()
				? CFDictionaryGetValue(installed, path) : NULL;
			if (dictionary != NULL && !CFArrayContainsValue(active, CFRangeMake(0, CFArrayGetCount(active)), dictionary))
				CFArrayAppendValue(active, dictionary);
		}
	} else {
		CFDictionaryApplyFunction(installed, appendToArray, active);
		CFArraySortValues(active, CFRangeMake(0, CFArrayGetCount(active)), compareNames, NULL);
	}

	if (paths != NULL)
		CFRelease(paths);
	CFRelease(installed);
	return active;
}

CFArrayRef DCSGetActiveDictionaries(void)
{
	pthread_mutex_lock(&lock);
	if (activeDictionaries == NULL)
		activeDictionaries = copyActiveDictionaries();
	CFArrayRef result = activeDictionaries;
	pthread_mutex_unlock(&lock);
	return result;
}

void DCSSetActiveDictionaries(CFArrayRef dictionaries)
{
	if (dictionaries == NULL || CFGetTypeID(dictionaries) != CFArrayGetTypeID())
		return;

	CFMutableArrayRef active = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	CFMutableArrayRef paths = CFArrayCreateMutable(NULL, 0, &kCFTypeArrayCallBacks);
	for (CFIndex i = 0; i < CFArrayGetCount(dictionaries); i++) {
		DCSDictionaryRef dictionary = CFArrayGetValueAtIndex(dictionaries, i);
		if (!isDictionary(dictionary) || CFArrayContainsValue(active, CFRangeMake(0, CFArrayGetCount(active)), dictionary))
			continue;
		CFStringRef path = copyPath(dictionary);
		CFArrayAppendValue(active, dictionary);
		CFArrayAppendValue(paths, path);
		CFRelease(path);
	}

	pthread_mutex_lock(&lock);
	CFPreferencesSetAppValue(kActiveDictionariesKey, paths, kDictionaryServicesDomain);
	CFPreferencesAppSynchronize(kDictionaryServicesDomain);
	// The previous array is not released: callers may still hold it from DCSGetActiveDictionaries.
	activeDictionaries = active;
	pthread_mutex_unlock(&lock);
	CFRelease(paths);
}
