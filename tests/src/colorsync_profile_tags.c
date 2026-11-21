// Copyright (C) 2026 Lubos Dolezel
//
// ColorSyncProfileCreate() accepts a hand-built ICC profile (128-byte header with the 'acsp' signature, a
// tag count, 12-byte tag table entries); ColorSyncProfileCopyData() returns the same bytes, and
// ContainsTag, CopyTag and CopyTagSignatures answer from the tag table. Malformed profiles give NULL.
// Also pins behaviour observed on macOS 26.7.3 (x86_64): a header size field that differs from the length
// of the data is accepted, and for a signature that occurs twice the last entry is the one found.
#include <ColorSync/ColorSync.h>
#include <CoreFoundation/CoreFoundation.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
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

typedef struct { const char* sig; uint32_t offset, size; } Tag;

// A profile of `total` bytes whose size field says `sizeField`; each tag's data is "ABCD" then offset-derived bytes.
static CFDataRef makeProfile(const Tag* tags, uint32_t count, uint32_t sizeField, size_t total, const char* signature)
{
	uint8_t bytes[512];
	memset(bytes, 0, sizeof bytes);
	put32(bytes, sizeField);
	memcpy(bytes + 36, signature, 4);
	put32(bytes + 128, count);
	for (uint32_t i = 0; i < count; i++)
	{
		memcpy(bytes + 132 + i * 12, tags[i].sig, 4);
		put32(bytes + 136 + i * 12, tags[i].offset);
		put32(bytes + 140 + i * 12, tags[i].size);
		if ((uint64_t) tags[i].offset + tags[i].size <= sizeof bytes)
			for (uint32_t k = 0; k < tags[i].size; k++)
				bytes[tags[i].offset + k] = k < 4 ? "ABCD"[k] : (uint8_t) (tags[i].offset + k);
	}
	return CFDataCreate(NULL, bytes, (CFIndex) total);
}

// The same profile with another tag count in the table header.
static CFDataRef withCount(CFDataRef data, uint32_t count)
{
	CFMutableDataRef copy = CFDataCreateMutableCopy(NULL, 0, data);
	put32(CFDataGetMutableBytePtr(copy) + 128, count);
	CFRelease(data);
	return copy;
}

static CFStringRef str(const char* s)
{
	return CFStringCreateWithCString(NULL, s, kCFStringEncodingISOLatin1);
}

static int tagSize(ColorSyncProfileRef profile, const char* sig)
{
	CFStringRef s = str(sig);
	CFDataRef tag = ColorSyncProfileCopyTag(profile, s);
	CFRelease(s);
	int size = tag ? (int) CFDataGetLength(tag) : -1;
	if (tag)
		CFRelease(tag);
	return size;
}

static int contains(ColorSyncProfileRef profile, const char* sig)
{
	CFStringRef s = str(sig);
	int result = ColorSyncProfileContainsTag(profile, s);
	CFRelease(s);
	return result;
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(30);

	// Three tags after the table (table ends at 168).
	const Tag three[] = { { "desc", 168, 10 }, { "cprt", 180, 5 }, { "wtpt", 188, 20 } };
	CFDataRef data = makeProfile(three, 3, 208, 208, "acsp");
	CFErrorRef error = NULL;
	ColorSyncProfileRef profile = ColorSyncProfileCreate(data, &error);
	CHECK(profile != NULL);
	CHECK(CFGetTypeID(profile) == ColorSyncProfileGetTypeID());
	if (profile == NULL)
		return 1;

	CFDataRef copy = ColorSyncProfileCopyData(profile, NULL);
	CHECK(copy != NULL && CFEqual(copy, data));
	if (copy)
		CFRelease(copy);

	CHECK(contains(profile, "desc") && contains(profile, "cprt") && contains(profile, "wtpt"));
	CHECK(!contains(profile, "dmnd") && !contains(profile, "DESC") && !contains(profile, "de") && !contains(profile, ""));
	CHECK(!ColorSyncProfileContainsTag(profile, NULL));
	CHECK(tagSize(profile, "desc") == 10 && tagSize(profile, "cprt") == 5 && tagSize(profile, "wtpt") == 20);
	CHECK(tagSize(profile, "dmnd") == -1);
	{
		CFStringRef s = str("desc");
		CFDataRef tag = ColorSyncProfileCopyTag(profile, s);
		CFRelease(s);
		CHECK(tag != NULL && memcmp(CFDataGetBytePtr(tag), "ABCD", 4) == 0 && CFDataGetBytePtr(tag)[4] == 172);
		if (tag)
			CFRelease(tag);
	}

	CFArrayRef sigs = ColorSyncProfileCopyTagSignatures(profile);
	CHECK(sigs != NULL && CFArrayGetCount(sigs) == 3);
	if (sigs)
	{
		CFStringRef d = str("desc"), c = str("cprt"), w = str("wtpt");
		CHECK(CFArrayContainsValue(sigs, CFRangeMake(0, 3), d) && CFArrayContainsValue(sigs, CFRangeMake(0, 3), c) && CFArrayContainsValue(sigs, CFRangeMake(0, 3), w));
		CFRelease(d); CFRelease(c); CFRelease(w);
		CFRelease(sigs);
	}
	CFRelease(profile);
	CFRelease(data);

	// A profile with no tags is valid, and so is one whose tag has size zero or ends exactly at the end of the data.
	CHECK((data = makeProfile(NULL, 0, 176, 176, "acsp")) != NULL);
	profile = ColorSyncProfileCreate(data, NULL);
	CHECK(profile != NULL);
	if (profile)
	{
		sigs = ColorSyncProfileCopyTagSignatures(profile);
		CHECK(sigs != NULL && CFArrayGetCount(sigs) == 0);
		if (sigs)
			CFRelease(sigs);
		CFRelease(profile);
	}
	CFRelease(data);
	{
		const Tag edge[] = { { "desc", 166, 10 }, { "cprt", 176, 0 } };
		data = makeProfile(edge, 2, 176, 176, "acsp");
		profile = ColorSyncProfileCreate(data, NULL);
		CHECK(profile != NULL);
		if (profile)
		{
			CHECK(tagSize(profile, "desc") == 10 && tagSize(profile, "cprt") == 0);
			CFRelease(profile);
		}
		CFRelease(data);
	}

	// QUIRK: a size field that differs from the length of the data is accepted (macOS 26.7.3, x86_64).
	{
		const Tag one[] = { { "desc", 144, 10 } };
		for (uint32_t sizeField = 100; sizeField <= 300; sizeField += 200)
		{
			data = makeProfile(one, 1, sizeField, 176, "acsp");
			profile = ColorSyncProfileCreate(data, NULL);
			CHECK(profile != NULL);
			if (profile)
				CFRelease(profile);
			CFRelease(data);
		}
	}

	// QUIRK: for a signature that occurs twice, the last entry is found (macOS 26.7.3, x86_64).
	{
		const Tag dup[] = { { "desc", 168, 4 }, { "desc", 172, 6 }, { "desc", 178, 8 } };
		data = makeProfile(dup, 3, 200, 200, "acsp");
		profile = ColorSyncProfileCreate(data, NULL);
		CHECK(profile != NULL);
		if (profile)
		{
			CHECK(tagSize(profile, "desc") == 8);
			sigs = ColorSyncProfileCopyTagSignatures(profile);
			CHECK(sigs != NULL && CFArrayGetCount(sigs) == 1);
			if (sigs)
				CFRelease(sigs);
			CFRelease(profile);
		}
		CFRelease(data);
	}

	// Malformed profiles give NULL: wrong file signature, tag past the end, tag inside the table, too short,
	// tag count larger than the data, offset plus size wrapping around 32 bits.
	{
		const Tag one[] = { { "desc", 144, 10 } };
		const Tag past[] = { { "desc", 144, 100 } };
		const Tag inTable[] = { { "desc", 140, 10 } };
		const Tag wrap[] = { { "desc", 0xfffffff0u, 0x20 } };
		CFDataRef bad[] = {
			makeProfile(one, 1, 176, 176, "xxxx"),
			makeProfile(past, 1, 176, 176, "acsp"),
			makeProfile(inTable, 1, 176, 176, "acsp"),
			makeProfile(one, 1, 176, 100, "acsp"),
			withCount(makeProfile(one, 1, 176, 176, "acsp"), 1000),
			makeProfile(wrap, 1, 176, 176, "acsp"),
			makeProfile(one, 1, 176, 128, "acsp"),
			CFDataCreate(NULL, (const uint8_t*) "x", 1),
		};
		for (size_t i = 0; i < sizeof bad / sizeof bad[0]; i++)
		{
			ColorSyncProfileRef p = ColorSyncProfileCreate(bad[i], NULL);
			if (p != NULL)
				printf("FAIL: malformed profile %zu accepted\n", i), g_failures++, CFRelease(p);
			CFRelease(bad[i]);
		}
	}
	CHECK(ColorSyncProfileCreate(NULL, NULL) == NULL);

	printf(g_failures ? "FAILED\n" : "PASS\n");
	return g_failures ? 1 : 0;
}
