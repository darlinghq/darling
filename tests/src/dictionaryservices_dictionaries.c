// CFLAGS: -framework CoreFoundation -framework CoreServices
#include <CoreFoundation/CoreFoundation.h>
#include <DictionaryServices/DictionaryServices.h>
#include <assert.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define PREFERENCES_DOMAIN CFSTR("com.apple.DictionaryServices")
#define ACTIVE_KEY CFSTR("DCSActiveDictionaries")

static char dir[1024], bundle[1024], contents[1100], plist[1200];
static int createdDir;
static CFPropertyListRef savedPreference;

static void cleanup(void)
{
	unlink(plist);
	rmdir(contents);
	rmdir(bundle);
	if (createdDir)
		rmdir(dir);
	CFPreferencesSetAppValue(ACTIVE_KEY, savedPreference, PREFERENCES_DOMAIN);
	CFPreferencesAppSynchronize(PREFERENCES_DOMAIN);
}

// A failed assert aborts, which skips atexit handlers.
static void cleanupOnAbort(int sig)
{
	cleanup();
	_exit(1);
}

static void writeInfoPlist(void)
{
	FILE *f = fopen(plist, "w");
	assert(f != NULL);
	fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
		"<plist version=\"1.0\"><dict>\n"
		"<key>CFBundleIdentifier</key><string>org.darlinghq.test.dictionary</string>\n"
		"<key>CFBundleName</key><string>DarlingTest</string>\n"
		"<key>CFBundleDisplayName</key><string>Darling Test Dictionary</string>\n"
		"<key>DCSDictionaryLanguages</key><array><dict>\n"
		"<key>DCSDictionaryIndexLanguage</key><string>en</string>\n"
		"<key>DCSDictionaryDescriptionLanguage</key><string>en</string>\n"
		"</dict></array>\n"
		"</dict></plist>\n", f);
	fclose(f);
}

static DCSDictionaryRef findTestDictionary(CFSetRef set)
{
	CFIndex count = CFSetGetCount(set);
	const void *values[count > 0 ? count : 1];
	CFSetGetValues(set, values);
	for (CFIndex i = 0; i < count; i++) {
		CFStringRef identifier = DCSDictionaryGetIdentifier(values[i]);
		if (identifier != NULL && CFEqual(identifier, CFSTR("org.darlinghq.test.dictionary")))
			return values[i];
	}
	return NULL;
}

int main(void)
{
	struct passwd *user = getpwuid(getuid());
	assert(user != NULL);
	snprintf(dir, sizeof(dir), "%s/Library/Dictionaries", user->pw_dir);
	snprintf(bundle, sizeof(bundle), "%s/DarlingTest.dictionary", dir);
	snprintf(contents, sizeof(contents), "%s/Contents", bundle);
	snprintf(plist, sizeof(plist), "%s/Info.plist", contents);
	savedPreference = CFPreferencesCopyAppValue(ACTIVE_KEY, PREFERENCES_DOMAIN);
	cleanup();
	atexit(cleanup);
	signal(SIGABRT, cleanupOnAbort);

	CFSetRef before = DCSCopyAvailableDictionaries();
	assert(before != NULL && CFGetTypeID(before) == CFSetGetTypeID());
	assert(findTestDictionary(before) == NULL);
	CFRelease(before);

	createdDir = mkdir(dir, 0755) == 0;
	int made = mkdir(bundle, 0755) == 0 && mkdir(contents, 0755) == 0;
	assert(made);
	writeInfoPlist();
	CFPreferencesSetAppValue(ACTIVE_KEY, NULL, PREFERENCES_DOMAIN);
	CFPreferencesAppSynchronize(PREFERENCES_DOMAIN);

	CFSetRef available = DCSCopyAvailableDictionaries();
	DCSDictionaryRef dictionary = findTestDictionary(available);
	assert(dictionary != NULL);
	assert(CFEqual(DCSDictionaryGetName(dictionary), CFSTR("Darling Test Dictionary")));
	CFArrayRef languages = DCSDictionaryGetLanguages(dictionary);
	assert(languages != NULL && CFArrayGetCount(languages) == 1);
	CFDictionaryRef language = CFArrayGetValueAtIndex(languages, 0);
	assert(CFEqual(CFDictionaryGetValue(language, kDCSDictionaryIndexLanguage), CFSTR("en")));
	CFStringRef path = CFURLCopyFileSystemPath(DCSDictionaryGetURL(dictionary), kCFURLPOSIXPathStyle);
	CFStringRef expectedPath = CFStringCreateWithCString(NULL, bundle, kCFStringEncodingUTF8);
	assert(CFEqual(path, expectedPath));

	// Without a stored preference, every installed dictionary is active.
	CFArrayRef active = DCSGetActiveDictionaries();
	assert(active != NULL && CFArrayContainsValue(active, CFRangeMake(0, CFArrayGetCount(active)), dictionary));

	CFArrayRef one = CFArrayCreate(NULL, (const void **) &dictionary, 1, &kCFTypeArrayCallBacks);
	DCSSetActiveDictionaries(one);
	active = DCSGetActiveDictionaries();
	assert(CFArrayGetCount(active) == 1 && CFEqual(CFArrayGetValueAtIndex(active, 0), dictionary));
	CFPreferencesAppSynchronize(PREFERENCES_DOMAIN);
	CFArrayRef stored = CFPreferencesCopyAppValue(ACTIVE_KEY, PREFERENCES_DOMAIN);
	assert(stored != NULL && CFArrayGetCount(stored) == 1 && CFEqual(CFArrayGetValueAtIndex(stored, 0), path));
	CFRelease(stored);

	CFArrayRef none = CFArrayCreate(NULL, NULL, 0, &kCFTypeArrayCallBacks);
	DCSSetActiveDictionaries(none);
	assert(CFArrayGetCount(DCSGetActiveDictionaries()) == 0);

	CFURLRef missing = CFURLCreateWithFileSystemPath(NULL, CFSTR("/nonexistent/None.dictionary"), kCFURLPOSIXPathStyle, true);
	assert(DCSDictionaryCreate(missing) == NULL);
	assert(DCSDictionaryGetName(NULL) == NULL);

	CFRelease(missing);
	CFRelease(none);
	CFRelease(one);
	CFRelease(expectedPath);
	CFRelease(path);
	CFRelease(available);
	puts("OK");
	return 0;
}
