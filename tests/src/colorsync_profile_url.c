// Copyright (C) 2026 Darling developers
//
// ColorSyncProfileCreateWithURL() creates a profile from the ICC profile data at a URL
// (https://developer.apple.com/documentation/colorsync/colorsyncprofilecreatewithurl(_:_:)) and
// ColorSyncProfileGetURL() returns the URL of a profile
// (https://developer.apple.com/documentation/colorsync/colorsyncprofilegeturl(_:_:)). Both return NULL
// on failure, and the error is optional.
// The profile is hand-built from the layout in ICC.1:2022, 7.2 and 7.3 (128-byte header with the 'acsp'
// signature, a tag count and 12-byte tag table entries), so it holds no vendor data.
// The checks pin observations of macOS 26.7.3 and 10.14.4 (identical on both) that the pages leave open: a
// failure of any kind returns NULL and leaves the error NULL, only file URLs are read, the URL is kept as given,
// and a profile made from data has no URL.
#include <ColorSync/ColorSync.h>
#include <CoreFoundation/CoreFoundation.h>
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
	char good[256], bad[256], missing[256], shortPath[256], emptyPath[256];
	snprintf(good, sizeof good, "%s/good.icc", dir);
	snprintf(bad, sizeof bad, "%s/bad.icc", dir);
	snprintf(missing, sizeof missing, "%s/missing.icc", dir);
	snprintf(shortPath, sizeof shortPath, "%s/short.icc", dir);
	snprintf(emptyPath, sizeof emptyPath, "%s/empty.icc", dir);

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

	// Every failure returns NULL and leaves the error NULL: a file that is not a profile, a missing file, an empty
	// or short file, a NULL URL, a URL that is not a file URL and a directory.
	CFURLRef shortURL = writeFile(shortPath, bytes, 10);
	CFURLRef emptyURL = writeFile(emptyPath, bytes, 0);
	CFURLRef web = CFURLCreateWithString(NULL, CFSTR("http://localhost:1/profile.icc"), NULL);
	CFURLRef dirURL = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) dir, (CFIndex) strlen(dir), true);
	CFURLRef failing[] = { badURL, missingURL, shortURL, emptyURL, NULL, web, dirURL };
	for (size_t i = 0; i < sizeof failing / sizeof failing[0]; i++)
	{
		error = (CFErrorRef) 1;
		CHECK(ColorSyncProfileCreateWithURL(failing[i], &error) == NULL);
		CHECK(error == NULL);
		CHECK(ColorSyncProfileCreateWithURL(failing[i], NULL) == NULL);
	}
	CFRelease(shortURL);
	CFRelease(emptyURL);
	CFRelease(web);
	CFRelease(dirURL);

	// A URL with a base URL is resolved to read the file, and the profile keeps the URL as given.
	CFURLRef base = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8*) dir, (CFIndex) strlen(dir), true);
	CFURLRef relative = CFURLCreateWithString(NULL, CFSTR("good.icc"), base);
	profile = ColorSyncProfileCreateWithURL(relative, NULL);
	CHECK(profile != NULL);
	if (profile)
	{
		CFURLRef url = ColorSyncProfileGetURL(profile, NULL);
		CHECK(url != NULL && CFEqual(url, relative));
		CFRelease(profile);
	}
	CFRelease(relative);
	CFRelease(base);

	// A profile made from data has no URL; a NULL argument and an argument that is not a profile give NULL as well.
	CFDataRef data = CFDataCreate(NULL, bytes, (CFIndex) length);
	ColorSyncProfileRef fromData = ColorSyncProfileCreate(data, NULL);
	CHECK(fromData != NULL);
	error = (CFErrorRef) 1;
	CHECK(fromData && ColorSyncProfileGetURL(fromData, &error) == NULL);
	CHECK(error == NULL);
	CHECK(fromData && ColorSyncProfileGetURL(fromData, NULL) == NULL);
	if (fromData)
		CFRelease(fromData);
	CFRelease(data);
	error = (CFErrorRef) 1;
	CHECK(ColorSyncProfileGetURL(NULL, &error) == NULL);
	CHECK(error == NULL);
	CHECK(ColorSyncProfileGetURL((ColorSyncProfileRef) CFSTR("not a profile"), NULL) == NULL);

	unlink(good);
	unlink(shortPath);
	unlink(emptyPath);
	unlink(bad);
	rmdir(dir);
	CFRelease(goodURL);
	CFRelease(badURL);
	CFRelease(missingURL);

	printf(g_failures ? "FAILED\n" : "PASS\n");
	return g_failures ? 1 : 0;
}
