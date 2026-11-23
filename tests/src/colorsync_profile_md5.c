// Copyright (C) 2026 Darling developers
//
// ColorSyncProfileGetMD5() returns the MD5 digest of a profile "calculated as defined by the ICC specification", or
// zeros on failure (https://developer.apple.com/documentation/colorsync/colorsyncprofilegetmd5(_:)). The ICC
// specification (ICC.1:2022, 7.2.18, https://www.color.org/specification/ICC.1-2022-05.pdf) defines it as the MD5 of
// RFC 1321 over the profile with the profile flags (bytes 44 to 47), the rendering intent (64 to 67) and the Profile ID
// field (84 to 99) set to zero.
// The profile is hand-built from the layout in ICC.1:2022, 7.2 and 7.3, so it holds no vendor data. The expected digest
// is Python's hashlib.md5 over the same bytes with those three fields zero.
// GUESS items pinned here: the digest is computed even when the header holds a Profile ID; it covers the number of
// bytes the size field gives (bytes after them are not hashed); and a size field below 100 or above the bytes held
// gives the zero digest, which the page defines as the failure result.
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

// 'wtpt' and 'desc' tags after the tag table; the creator field is 'ABCD' (it is part of the digest).
static size_t buildProfile(uint8_t* out)
{
	const uint32_t tableEnd = 128 + 4 + 2 * 12;
	memset(out, 0, tableEnd + 12);
	put32(out, tableEnd + 12);
	memcpy(out + 36, "acsp", 4);
	memcpy(out + 80, "ABCD", 4);
	put32(out + 128, 2);
	memcpy(out + 132, "wtpt", 4);
	put32(out + 136, tableEnd);
	put32(out + 140, 4);
	memcpy(out + 144, "desc", 4);
	put32(out + 148, tableEnd + 4);
	put32(out + 152, 8);
	memcpy(out + tableEnd, "WTPT", 4);
	memcpy(out + tableEnd + 4, "DESCDESC", 8);
	return tableEnd + 12;
}

static ColorSyncProfileRef profileOf(const uint8_t* bytes, size_t length)
{
	CFDataRef data = CFDataCreate(NULL, bytes, (CFIndex) length);
	ColorSyncProfileRef profile = ColorSyncProfileCreate(data, NULL);
	CFRelease(data);
	return profile;
}

// The same profile with its size field set to 160 and to 100 bytes (the digest is over that many bytes).
static const uint8_t kExpectedSize160[16] = { 0xc2, 0xb6, 0xf1, 0xc5, 0x73, 0x39, 0xac, 0xbd, 0x8b, 0xed, 0xf0, 0x4c, 0x29, 0x77, 0xd3, 0x17 };
static const uint8_t kExpectedSize100[16] = { 0xfd, 0xd0, 0x5f, 0x1a, 0xd8, 0x8c, 0x1e, 0x46, 0xd5, 0x3a, 0xb4, 0x72, 0x75, 0xc4, 0xd7, 0xb8 };

static const uint8_t kExpected[16] = { 0x0d, 0x3f, 0x2d, 0x84, 0x1c, 0xdf, 0x36, 0xdd, 0x5f, 0x15, 0x45, 0xd1, 0x0b, 0x73, 0xb8, 0x88 };

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(30);

	uint8_t bytes[256];
	const size_t length = buildProfile(bytes);
	CHECK(COLORSYNC_MD5_LENGTH == 16 && sizeof(ColorSyncMD5) == 16);

	ColorSyncProfileRef plain = profileOf(bytes, length);
	CHECK(plain != NULL);
	ColorSyncMD5 md5 = ColorSyncProfileGetMD5(plain);
	CHECK(memcmp(md5.digest, kExpected, 16) == 0);

	// The flags, rendering intent and an existing Profile ID do not take part in the digest (ICC.1:2022, 7.2.18).
	uint8_t filled[256];
	memcpy(filled, bytes, length);
	put32(filled + 44, 1);
	put32(filled + 64, 3);
	for (int i = 0; i < 16; i++)
		filled[84 + i] = (uint8_t) (i + 1);
	ColorSyncProfileRef withId = profileOf(filled, length);
	CHECK(withId != NULL);
	md5 = ColorSyncProfileGetMD5(withId);
	CHECK(memcmp(md5.digest, kExpected, 16) == 0);

	// GUESS 2: the size field says how many bytes are hashed; bytes after them do not count.
	uint8_t longer[256];
	memcpy(longer, bytes, length);
	longer[length] = 0x7f;
	ColorSyncProfileRef trailing = profileOf(longer, length + 1);
	CHECK(trailing != NULL);
	md5 = ColorSyncProfileGetMD5(trailing);
	CHECK(memcmp(md5.digest, kExpected, 16) == 0);

	uint8_t sized[256];
	memcpy(sized, bytes, length);
	put32(sized, 160);
	ColorSyncProfileRef size160 = profileOf(sized, length);
	CHECK(size160 != NULL);
	md5 = ColorSyncProfileGetMD5(size160);
	CHECK(memcmp(md5.digest, kExpectedSize160, 16) == 0);
	put32(sized, 100);
	ColorSyncProfileRef size100 = profileOf(sized, length);
	CHECK(size100 != NULL);
	md5 = ColorSyncProfileGetMD5(size100);
	CHECK(memcmp(md5.digest, kExpectedSize100, 16) == 0);

	// GUESS 3: a size field the digest cannot be defined for gives zeros, as failure does.
	static const uint8_t zeros[16];
	put32(sized, 99);
	ColorSyncProfileRef size99 = profileOf(sized, length);
	CHECK(size99 != NULL);
	md5 = ColorSyncProfileGetMD5(size99);
	CHECK(memcmp(md5.digest, zeros, 16) == 0);
	put32(sized, (uint32_t) length + 1);
	ColorSyncProfileRef tooBig = profileOf(sized, length);
	CHECK(tooBig != NULL);
	md5 = ColorSyncProfileGetMD5(tooBig);
	CHECK(memcmp(md5.digest, zeros, 16) == 0);

	// Another byte in the profile gives another digest.
	uint8_t changed[256];
	memcpy(changed, bytes, length);
	changed[length - 1] ^= 1;
	ColorSyncProfileRef other = profileOf(changed, length);
	CHECK(other != NULL);
	md5 = ColorSyncProfileGetMD5(other);
	CHECK(memcmp(md5.digest, kExpected, 16) != 0);

	// Failure gives zeros.
	md5 = ColorSyncProfileGetMD5(NULL);
	CHECK(memcmp(md5.digest, zeros, 16) == 0);
	md5 = ColorSyncProfileGetMD5((ColorSyncProfileRef) CFSTR("not a profile"));
	CHECK(memcmp(md5.digest, zeros, 16) == 0);

	CFRelease(plain);
	CFRelease(withId);
	CFRelease(trailing);
	CFRelease(size160);
	CFRelease(size100);
	CFRelease(size99);
	CFRelease(tooBig);
	CFRelease(other);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures != 0;
}
