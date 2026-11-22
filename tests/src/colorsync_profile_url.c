// Copyright (C) 2026 Darling developers
//
// ColorSyncProfileCreateWithURL() creates a profile from the ICC profile data at a URL
// (https://developer.apple.com/documentation/colorsync/colorsyncprofilecreatewithurl(_:_:)) and
// ColorSyncProfileGetURL() returns the URL of a profile
// (https://developer.apple.com/documentation/colorsync/colorsyncprofilegeturl(_:_:)). Both return NULL
// on failure, and the error is optional.
// The profile is hand-built from the layout in ICC.1:2022, 7.2 and 7.3 (128-byte header with the 'acsp'
// signature, a tag count and 12-byte tag table entries), so it holds no vendor data.
// Cases marked GUESS pin choices the pages leave open (see ColorSyncProfile.c): only file URLs are read,
// a profile made from data has no URL, and the errors have the domains and codes the test checks.
#include <ColorSync/ColorSync.h>
#include <CoreFoundation/CoreFoundation.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	fflush(stdout);
	_exit(2);
}

static void put32(uint8_t* p, uint32_t v)
{
	p[0] = v >> 24;
	p[1] = v >> 16;
	p[2] = v >> 8;
	p[3] = v;
}

// One tag, 'wtpt', of 4 bytes right after the tag table.
static size_t buildProfile(uint8_t* out, const char* signature)
{
	const uint32_t tableEnd = 128 + 4 + 12;
	memset(out, 0, tableEnd + 4);
	put32(out, tableEnd + 4);
	memcpy(out + 36, signature, 4);
	put32(out + 128, 1);
	memcpy(out + 132, "wtpt", 4);
	put32(out + 136, tableEnd);
	put32(out + 140, 4);
	memcpy(out + tableEnd, "WTPT", 4);
	return tableEnd + 4;
}

// True if the error has the domain and code. The pages do not describe the errors; see the GUESS items in
// ColorSyncProfile.c.
static int errorIs(CFErrorRef error, CFStringRef domain, CFIndex code)
{
	return error && CFEqual(CFErrorGetDomain(error), domain) && CFErrorGetCode(error) == code;
}

static CFURLRef writeFile(const char* path, const uint8_t* bytes, size_t length)
{
	FILE* f = fopen(path, "wb");
	if (f)
	{
		fwrite(bytes, 1, length, f);
		fclose(f);
	}
	return CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) path, (CFIndex) strlen(path), false);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(30);

	char dir[] = "/tmp/colorsync-url-XXXXXX";
	CHECK(mkdtemp(dir) != NULL);
	char good[256], bad[256], missing[256];
	snprintf(good, sizeof good, "%s/good.icc", dir);
	snprintf(bad, sizeof bad, "%s/bad.icc", dir);
	snprintf(missing, sizeof missing, "%s/missing.icc", dir);

	uint8_t bytes[256], notProfile[256];
	const size_t length = buildProfile(bytes, "acsp");
	CFURLRef goodURL = writeFile(good, bytes, length);
	CFURLRef badURL = writeFile(bad, notProfile, buildProfile(notProfile, "xxxx"));
	CFURLRef missingURL = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) missing, (CFIndex) strlen(missing), false);

	// A profile from a file has the file's data, and its URL.
	CFErrorRef error = (CFErrorRef) 1;
	ColorSyncProfileRef profile = ColorSyncProfileCreateWithURL(goodURL, &error);
	CHECK(profile != NULL);
	CHECK(error == NULL);
	if (profile)
	{
		CFDataRef profileData = ColorSyncProfileCopyData(profile, NULL);
		CHECK(profileData != NULL && CFDataGetLength(profileData) == (CFIndex) length);
		CHECK(profileData && memcmp(CFDataGetBytePtr(profileData), bytes, length) == 0);
		if (profileData)
			CFRelease(profileData);

		CFStringRef sig = CFStringCreateWithCString(NULL, "wtpt", kCFStringEncodingISOLatin1);
		CHECK(ColorSyncProfileContainsTag(profile, sig));
		CFRelease(sig);

		error = (CFErrorRef) 1;
		CFURLRef url = ColorSyncProfileGetURL(profile, &error);
		CHECK(url != NULL && CFEqual(url, goodURL));
		CHECK(error == NULL);
		// The URL belongs to the profile: a Get function adds no reference.
		CFIndex retains = url ? CFGetRetainCount(url) : 0;
		CHECK(ColorSyncProfileGetURL(profile, NULL) == url);
		CHECK(!url || CFGetRetainCount(url) == retains);
		CFRelease(profile);
	}

	// A file that is not a profile, and a file that does not exist, give NULL and an error.
	// GUESS: the first is the invalid-data error (OSStatus -50), the second a POSIX error with errno ENOENT.
	error = NULL;
	CHECK(ColorSyncProfileCreateWithURL(badURL, &error) == NULL);
	CHECK(errorIs(error, kCFErrorDomainOSStatus, -50));
	if (error)
		CFRelease(error);
	error = NULL;
	CHECK(ColorSyncProfileCreateWithURL(missingURL, &error) == NULL);
	CHECK(errorIs(error, kCFErrorDomainPOSIX, ENOENT));
	if (error)
		CFRelease(error);
	CHECK(ColorSyncProfileCreateWithURL(missingURL, NULL) == NULL);

	// GUESS: a NULL URL and a URL that is not a file URL fail with the invalid-data error.
	error = NULL;
	CHECK(ColorSyncProfileCreateWithURL(NULL, &error) == NULL);
	CHECK(errorIs(error, kCFErrorDomainOSStatus, -50));
	if (error)
		CFRelease(error);
	CFURLRef web = CFURLCreateWithString(NULL, CFSTR("http://localhost:1/profile.icc"), NULL);
	error = NULL;
	CHECK(ColorSyncProfileCreateWithURL(web, &error) == NULL);
	CHECK(errorIs(error, kCFErrorDomainOSStatus, -50));
	if (error)
		CFRelease(error);
	CFRelease(web);

	// GUESS: a directory is not read: a POSIX error with EINVAL.
	CFURLRef dirURL = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) dir, (CFIndex) strlen(dir), true);
	error = NULL;
	CHECK(ColorSyncProfileCreateWithURL(dirURL, &error) == NULL);
	CHECK(errorIs(error, kCFErrorDomainPOSIX, EINVAL));
	if (error)
		CFRelease(error);
	CFRelease(dirURL);

	// GUESS: a profile made from data has no URL.
	CFDataRef data = CFDataCreate(NULL, bytes, (CFIndex) length);
	ColorSyncProfileRef fromData = ColorSyncProfileCreate(data, NULL);
	CHECK(fromData != NULL);
	error = NULL;
	CHECK(fromData && ColorSyncProfileGetURL(fromData, &error) == NULL);
	CHECK(errorIs(error, kCFErrorDomainOSStatus, -50));
	if (error)
		CFRelease(error);
	CHECK(fromData && ColorSyncProfileGetURL(fromData, NULL) == NULL);
	if (fromData)
		CFRelease(fromData);
	CFRelease(data);
	CHECK(ColorSyncProfileGetURL(NULL, NULL) == NULL);

	unlink(good);
	unlink(bad);
	rmdir(dir);
	CFRelease(goodURL);
	CFRelease(badURL);
	CFRelease(missingURL);

	printf(g_failures ? "FAILED\n" : "PASS\n");
	return g_failures ? 1 : 0;
}
