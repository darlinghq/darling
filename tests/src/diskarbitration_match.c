// Copyright (C) 2026 Lubos Dolezel
//
// DiskArbitration's predefined match and watch objects and its disk
// description keys have the contents macOS gives them:
// - the description keys are short strings such as "DAVolumePath";
// - the objects are NULL until the first session is created (a quirk
//   observed on macOS 27.0.1), then:
//   WatchVolumeName/Path = [VolumeNameKey] / [VolumePathKey],
//   MatchMediaWhole = { MediaWholeKey: true },
//   MatchMediaUnformatted = { MediaSizeKey: 0 },
//   MatchVolumeMountable = { VolumeMountableKey: true },
//   MatchVolumeUnrecognized = { VolumeMountableKey: false }.
// The program passes unchanged on macOS 27.0.1.
#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void finish(int status)
{
	fflush(stdout);
	_exit(status);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	finish(2);
}

static void check_key(const char* name, CFStringRef key, const char* expected)
{
	char buf[128] = "";
	if (key == NULL || !CFStringGetCString(key, buf, sizeof(buf), kCFStringEncodingUTF8) || strcmp(buf, expected) != 0)
	{
		printf("FAIL: %s = \"%s\", expected \"%s\"\n", name, buf, expected);
		g_failures++;
	}
}

static void check_watch(const char* name, CFArrayRef array, CFStringRef key)
{
	if (array == NULL || CFGetTypeID(array) != CFArrayGetTypeID()
		|| CFArrayGetCount(array) != 1 || !CFEqual(CFArrayGetValueAtIndex(array, 0), key))
	{
		printf("FAIL: %s is not an array holding exactly the expected key\n", name);
		g_failures++;
	}
}

static void check_match(const char* name, CFDictionaryRef dict, CFStringRef key, CFTypeRef value)
{
	if (dict == NULL || CFGetTypeID(dict) != CFDictionaryGetTypeID() || CFDictionaryGetCount(dict) != 1)
	{
		printf("FAIL: %s is not a dictionary with one entry\n", name);
		g_failures++;
		return;
	}

	CFTypeRef actual = CFDictionaryGetValue(dict, key);
	if (actual == NULL || CFGetTypeID(actual) != CFGetTypeID(value) || !CFEqual(actual, value))
	{
		printf("FAIL: %s does not map the expected key to the expected value\n", name);
		g_failures++;
	}
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(30);

	check_key("kDADiskDescriptionVolumeNameKey", kDADiskDescriptionVolumeNameKey, "DAVolumeName");
	check_key("kDADiskDescriptionVolumePathKey", kDADiskDescriptionVolumePathKey, "DAVolumePath");
	check_key("kDADiskDescriptionVolumeMountableKey", kDADiskDescriptionVolumeMountableKey, "DAVolumeMountable");
	check_key("kDADiskDescriptionVolumeTypeKey", kDADiskDescriptionVolumeTypeKey, "DAVolumeType");
	check_key("kDADiskDescriptionMediaWholeKey", kDADiskDescriptionMediaWholeKey, "DAMediaWhole");
	check_key("kDADiskDescriptionMediaSizeKey", kDADiskDescriptionMediaSizeKey, "DAMediaSize");
	check_key("kDADiskDescriptionMediaBSDNameKey", kDADiskDescriptionMediaBSDNameKey, "DAMediaBSDName");
	check_key("kDADiskDescriptionDeviceProtocolKey", kDADiskDescriptionDeviceProtocolKey, "DADeviceProtocol");
	check_key("kDADiskDescriptionBusPathKey", kDADiskDescriptionBusPathKey, "DABusPath");

	// Quirk: nothing is created before the first session.
	CHECK(kDADiskDescriptionWatchVolumeName == NULL);
	CHECK(kDADiskDescriptionWatchVolumePath == NULL);
	CHECK(kDADiskDescriptionMatchMediaWhole == NULL);
	CHECK(kDADiskDescriptionMatchMediaUnformatted == NULL);
	CHECK(kDADiskDescriptionMatchVolumeMountable == NULL);
	CHECK(kDADiskDescriptionMatchVolumeUnrecognized == NULL);

	DASessionRef session = DASessionCreate(kCFAllocatorDefault);
	CHECK(session != NULL);

	int zero = 0;
	CFNumberRef size = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &zero);

	check_watch("kDADiskDescriptionWatchVolumeName", kDADiskDescriptionWatchVolumeName, kDADiskDescriptionVolumeNameKey);
	check_watch("kDADiskDescriptionWatchVolumePath", kDADiskDescriptionWatchVolumePath, kDADiskDescriptionVolumePathKey);
	check_match("kDADiskDescriptionMatchMediaWhole", kDADiskDescriptionMatchMediaWhole,
		kDADiskDescriptionMediaWholeKey, kCFBooleanTrue);
	check_match("kDADiskDescriptionMatchMediaUnformatted", kDADiskDescriptionMatchMediaUnformatted,
		kDADiskDescriptionMediaSizeKey, size);
	check_match("kDADiskDescriptionMatchVolumeMountable", kDADiskDescriptionMatchVolumeMountable,
		kDADiskDescriptionVolumeMountableKey, kCFBooleanTrue);
	check_match("kDADiskDescriptionMatchVolumeUnrecognized", kDADiskDescriptionMatchVolumeUnrecognized,
		kDADiskDescriptionVolumeMountableKey, kCFBooleanFalse);

	// A second session leaves the same objects in place.
	CFDictionaryRef whole = kDADiskDescriptionMatchMediaWhole;
	DASessionRef second = DASessionCreate(kCFAllocatorDefault);
	CHECK(kDADiskDescriptionMatchMediaWhole == whole);

	CFRelease(size);
	if (second)
		CFRelease(second);
	if (session)
		CFRelease(session);

	if (g_failures)
		printf("FAIL: %d check(s) failed\n", g_failures);
	else
		printf("PASS\n");
	finish(g_failures ? 1 : 0);
}
