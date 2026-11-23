// Copyright (C) 2026 Darling developers
//
// ColorSyncProfileCopyHeader() copies the 128-byte header of a profile "in host endianness", and
// ColorSyncProfileSetHeader() sets the header of a mutable profile from data in the same form
// (https://developer.apple.com/documentation/colorsync/colorsyncprofilecopyheader(_:),
// https://developer.apple.com/documentation/colorsync/colorsyncprofilesetheader(_:_:),
// https://developer.apple.com/documentation/colorsync/colorsyncprofilecreatemutablecopy(_:)).
// The profile is hand-built from the layout in ICC.1:2022, 7.2 and 7.3 (https://www.color.org/specification/ICC.1-2022-05.pdf),
// whose numbers are big-endian, so it holds no vendor data.
// Observed on macOS 26.7.3 and 10.14.4 (x86_64), where the pages do not say which fields "host endianness" covers: the
// 32-bit words (signatures too), the six 16-bit date and time numbers and the two 32-bit device attributes words are
// numbers in host order, the Profile ID and reserved bytes are bytes; and SetHeader ignores a profile that is not mutable,
// a NULL profile and a header that is not 128 bytes. (SetHeader with a NULL header crashes on macOS: not called here.)
#include <ColorSync/ColorSync.h>
#include <CoreFoundation/CoreFoundation.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

#define SIG(a, b, c, d) (((uint32_t) (a) << 24) | ((uint32_t) (b) << 16) | ((uint32_t) (c) << 8) | (uint32_t) (d))

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

static const uint16_t kDate[6] = { 2026, 11, 23, 14, 30, 59 };

// One tag, 'wtpt' of 4 bytes, after the tag table. The header fields are all different and nonzero.
static size_t buildProfile(uint8_t* out)
{
	const uint32_t tableEnd = 128 + 4 + 12;
	memset(out, 0, tableEnd + 4);
	put32(out + 0, tableEnd + 4);
	put32(out + 4, SIG('A', 'B', 'C', 'D'));
	put32(out + 8, 0x04300000u);
	put32(out + 12, SIG('m', 'n', 't', 'r'));
	put32(out + 16, SIG('R', 'G', 'B', ' '));
	put32(out + 20, SIG('X', 'Y', 'Z', ' '));
	for (int i = 0; i < 6; i++)
	{
		out[24 + i * 2] = kDate[i] >> 8;
		out[25 + i * 2] = (uint8_t) kDate[i];
	}
	put32(out + 36, SIG('a', 'c', 's', 'p'));
	put32(out + 40, SIG('A', 'P', 'P', 'L'));
	put32(out + 44, 3);
	put32(out + 48, 0x01020304u);
	put32(out + 52, 0x05060708u);
	put32(out + 56, 0x0a0b0c0du);
	put32(out + 60, 0x0e0f1011u);
	put32(out + 64, 1);
	put32(out + 68, 0x0000f6d6u);
	put32(out + 72, 0x00010000u);
	put32(out + 76, 0x0000d32du);
	put32(out + 80, SIG('Z', 'Z', 'Z', 'Z'));
	for (int i = 0; i < 16; i++)
		out[84 + i] = (uint8_t) (0xa0 + i);
	put32(out + 128, 1);
	memcpy(out + 132, "wtpt", 4);
	put32(out + 136, tableEnd);
	put32(out + 140, 4);
	memcpy(out + tableEnd, "WTPT", 4);
	return tableEnd + 4;
}

static uint32_t host32(CFDataRef header, int offset)
{
	uint32_t v;
	memcpy(&v, CFDataGetBytePtr(header) + offset, 4);
	return v;
}

static bool dataEquals(CFDataRef data, const uint8_t* bytes, size_t length)
{
	return data && (size_t) CFDataGetLength(data) == length && memcmp(CFDataGetBytePtr(data), bytes, length) == 0;
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(30);

	uint8_t bytes[256];
	const size_t length = buildProfile(bytes);
	CFDataRef data = CFDataCreate(NULL, bytes, (CFIndex) length);
	ColorSyncProfileRef profile = ColorSyncProfileCreate(data, NULL);
	CHECK(profile != NULL);

	// CopyHeader: 128 bytes, each field a host number.
	CFDataRef header = ColorSyncProfileCopyHeader(profile);
	CHECK(header != NULL && CFDataGetLength(header) == 128);
	CHECK(host32(header, 0) == length);
	CHECK(host32(header, 4) == SIG('A', 'B', 'C', 'D'));
	CHECK(host32(header, 8) == 0x04300000u);
	CHECK(host32(header, 16) == SIG('R', 'G', 'B', ' '));
	for (int i = 0; i < 6; i++)
	{
		uint16_t field;
		memcpy(&field, CFDataGetBytePtr(header) + 24 + i * 2, 2);
		CHECK(field == kDate[i]);
	}
	CHECK(host32(header, 36) == SIG('a', 'c', 's', 'p'));
	uint32_t attributes[2];
	memcpy(attributes, CFDataGetBytePtr(header) + 56, 8);
	CHECK(attributes[0] == 0x0a0b0c0du && attributes[1] == 0x0e0f1011u);
	CHECK(host32(header, 64) == 1);
	CHECK(host32(header, 72) == 0x00010000u);
	CHECK(memcmp(CFDataGetBytePtr(header) + 84, bytes + 84, 16) == 0);

	// Failure.
	CHECK(ColorSyncProfileCopyHeader(NULL) == NULL);
	CHECK(ColorSyncProfileCopyHeader((ColorSyncProfileRef) CFSTR("not a profile")) == NULL);

	// A mutable copy has the same data and header.
	ColorSyncMutableProfileRef mutableCopy = ColorSyncProfileCreateMutableCopy(profile);
	CHECK(mutableCopy != NULL && mutableCopy != profile);
	CFDataRef copiedData = ColorSyncProfileCopyData(mutableCopy, NULL);
	CHECK(dataEquals(copiedData, bytes, length));
	CHECK(ColorSyncProfileCreateMutableCopy(NULL) == NULL);

	// Round trip: a changed header is read back changed, and the profile keeps its tags.
	uint8_t edited[128];
	memcpy(edited, CFDataGetBytePtr(header), 128);
	const uint32_t intent = 3, creator = SIG('N', 'E', 'W', '!');
	const uint16_t newYear = 1999;
	memcpy(edited + 64, &intent, 4);
	memcpy(edited + 80, &creator, 4);
	memcpy(edited + 24, &newYear, 2);
	CFDataRef editedHeader = CFDataCreate(NULL, edited, 128);
	ColorSyncProfileSetHeader(mutableCopy, editedHeader);
	CFDataRef after = ColorSyncProfileCopyHeader(mutableCopy);
	CHECK(dataEquals(after, edited, 128));

	uint8_t expected[256];
	memcpy(expected, bytes, length);
	put32(expected + 64, 3);
	put32(expected + 80, SIG('N', 'E', 'W', '!'));
	expected[24] = 1999 >> 8;
	expected[25] = 1999 & 0xff;
	CFDataRef afterData = ColorSyncProfileCopyData(mutableCopy, NULL);
	CHECK(dataEquals(afterData, expected, length));
	CHECK(ColorSyncProfileContainsTag(mutableCopy, CFSTR("wtpt")));
	CHECK(dataEquals(copiedData, bytes, length)); // data copied before the change did not change
	CFDataRef original = ColorSyncProfileCopyData(profile, NULL);
	CHECK(dataEquals(original, bytes, length)); // nor did the profile it was copied from

	// Ignored without a result.
	ColorSyncProfileSetHeader((ColorSyncMutableProfileRef) profile, editedHeader); // not mutable
	CFDataRef unchanged = ColorSyncProfileCopyData(profile, NULL);
	CHECK(dataEquals(unchanged, bytes, length));
	CFDataRef shortHeader = CFDataCreate(NULL, edited, 127);
	ColorSyncProfileSetHeader(mutableCopy, shortHeader);
	ColorSyncProfileSetHeader(NULL, editedHeader);
	CFDataRef stillAfter = ColorSyncProfileCopyHeader(mutableCopy);
	CHECK(dataEquals(stillAfter, edited, 128));

	CFRelease(data);
	CFRelease(profile);
	CFRelease(header);
	CFRelease(mutableCopy);
	CFRelease(copiedData);
	CFRelease(editedHeader);
	CFRelease(after);
	CFRelease(afterData);
	CFRelease(original);
	CFRelease(unchanged);
	CFRelease(shortHeader);
	CFRelease(stillAfter);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures != 0;
}
