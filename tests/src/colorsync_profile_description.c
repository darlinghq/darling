// Copyright (C) 2026 Darling developers
//
// ColorSyncProfileCopyDescriptionString() copies the profile description, localized to the current locale
// (https://developer.apple.com/documentation/colorsync/colorsyncprofilecopydescriptionstring(_:)).
// The description is the 'desc' tag. ICC.1:2022 (https://www.color.org/specification/ICC.1-2022-05.pdf) says that a
// multiLocalizedUnicodeType holds one UTF-16BE string per language and country, and that the record for the wanted
// country is used, else the first record of the language, else the first record (10.15); an older profile holds a
// textDescriptionType, whose 7-bit ASCII part ICC.1:2022, 10.20.3 uses for the description (ICC.1:2001-04, 6.5.17).
// The profiles are hand-built from the layout in ICC.1:2022, 7.2, 7.3 and 10.15, so they hold no vendor data.
// The locale is read from the test's own CFLocaleCopyCurrent() and the records are built around it, so the test holds
// for any locale. The expectations marked QUIRK are observations of macOS 26.7.3 and 10.14.4 (see ICCProfile.c and
// ColorSyncProfile.c): a record for English (United States) wins over the locale's record, the Unicode part of a
// textDescriptionType wins over its ASCII part, and a profile without a readable 'desc' tag gives a fixed text, not NULL.
// The current locale is taken to be CFLocaleCopyCurrent(); a NULL or non-profile argument gives NULL.
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

typedef struct { const char* language; const char* country; const char* text; } Record;

// A profile whose only tag is a 'desc' multiLocalizedUnicodeType of these records (ASCII text stored as UTF-16BE).
static ColorSyncProfileRef profileWithRecords(const Record* records, uint32_t count)
{
	uint8_t tag[512];
	memset(tag, 0, sizeof tag);
	memcpy(tag, "mluc", 4);
	put32(tag + 8, count);
	put32(tag + 12, 12);
	size_t offset = 16 + count * 12;
	for (uint32_t i = 0; i < count; i++)
	{
		const size_t length = strlen(records[i].text) * 2;
		memcpy(tag + 16 + i * 12, records[i].language, 2);
		memcpy(tag + 18 + i * 12, records[i].country, 2);
		put32(tag + 20 + i * 12, (uint32_t) length);
		put32(tag + 24 + i * 12, (uint32_t) offset);
		for (size_t c = 0; c < length / 2; c++)
			tag[offset + c * 2 + 1] = (uint8_t) records[i].text[c];
		offset += length;
	}

	uint8_t bytes[1024];
	const uint32_t tableEnd = 128 + 4 + 12;
	memset(bytes, 0, tableEnd);
	put32(bytes, (uint32_t) (tableEnd + offset));
	memcpy(bytes + 36, "acsp", 4);
	put32(bytes + 128, 1);
	memcpy(bytes + 132, "desc", 4);
	put32(bytes + 136, tableEnd);
	put32(bytes + 140, (uint32_t) offset);
	memcpy(bytes + tableEnd, tag, offset);
	CFDataRef data = CFDataCreate(NULL, bytes, tableEnd + offset);
	ColorSyncProfileRef profile = ColorSyncProfileCreate(data, NULL);
	CFRelease(data);
	return profile;
}

static bool descriptionIs(ColorSyncProfileRef profile, const char* expected)
{
	CFStringRef description = ColorSyncProfileCopyDescriptionString(profile);
	CFStringRef wanted = CFStringCreateWithCString(NULL, expected, kCFStringEncodingASCII);
	const bool same = description && CFEqual(description, wanted);
	if (description)
		CFRelease(description);
	CFRelease(wanted);
	return same;
}

static bool codeOf(CFLocaleRef locale, CFStringRef key, char out[3])
{
	CFTypeRef value = CFLocaleGetValue(locale, key);
	return value && CFGetTypeID(value) == CFStringGetTypeID() && CFStringGetLength(value) == 2 &&
		CFStringGetCString(value, out, 3, kCFStringEncodingASCII);
}

int main(void)
{
	signal(SIGALRM, on_alarm);
	alarm(30);

	char language[3], country[3];
	CFLocaleRef locale = CFLocaleCopyCurrent();
	const bool hasLanguage = codeOf(locale, kCFLocaleLanguageCode, language);
	const bool hasCountry = codeOf(locale, kCFLocaleCountryCode, country);
	CFRelease(locale);

	// A profile with one record: that record, whatever the locale.
	const Record one[] = { { "zz", "ZZ", "Only" } };
	ColorSyncProfileRef single = profileWithRecords(one, 1);
	CHECK(single != NULL && descriptionIs(single, "Only"));

	// No record for the locale (the languages "zz" and "yy" do not exist): the first record.
	const Record unrelated[] = { { "zz", "ZZ", "First" }, { "yy", "YY", "Second" } };
	ColorSyncProfileRef noMatch = profileWithRecords(unrelated, 2);
	CHECK(noMatch != NULL && descriptionIs(noMatch, "First"));

	if (hasLanguage)
	{
		// The record of the locale's language, even after the first record.
		const Record sameLanguage[] = { { "zz", "ZZ", "First" }, { language, "ZZ", "Language" } };
		ColorSyncProfileRef byLanguage = profileWithRecords(sameLanguage, 2);
		CHECK(byLanguage != NULL && descriptionIs(byLanguage, "Language"));
		CFRelease(byLanguage);

		if (hasCountry)
		{
			// The record of the locale's language and country is chosen over an earlier record of the language only.
			const Record exact[] = { { "zz", "ZZ", "First" }, { language, "ZZ", "Language" }, { language, country, "Exact" } };
			ColorSyncProfileRef byCountry = profileWithRecords(exact, 3);
			CHECK(byCountry != NULL && descriptionIs(byCountry, "Exact"));
			CFRelease(byCountry);
		}
	}

	// QUIRK: a record for English (United States) wins over the locale's own record, in every locale.
	const Record english[] = { { "zz", "ZZ", "First" }, { "fr", "FR", "Locale" }, { "en", "US", "English" } };
	ColorSyncProfileRef withEnglish = profileWithRecords(english, 3);
	CHECK(withEnglish != NULL && descriptionIs(withEnglish, "English"));
	CFRelease(withEnglish);

	// An older profile: a textDescriptionType with its ASCII string, then a Unicode part.
	uint8_t old[64] = { 0 };
	memcpy(old, "desc", 4);
	put32(old + 8, 6);
	memcpy(old + 12, "Plain", 6);
	put32(old + 22, 3);
	memcpy(old + 26, "\0H\0i\0\0", 6);
	uint8_t bytes[256] = { 0 };
	const uint32_t tableEnd = 128 + 4 + 12;
	put32(bytes, tableEnd + 40);
	memcpy(bytes + 36, "acsp", 4);
	put32(bytes + 128, 1);
	memcpy(bytes + 132, "desc", 4);
	put32(bytes + 136, tableEnd);
	put32(bytes + 140, 40);
	memcpy(bytes + tableEnd, old, 40);
	CFDataRef oldData = CFDataCreate(NULL, bytes, tableEnd + 40);
	ColorSyncProfileRef oldProfile = ColorSyncProfileCreate(oldData, NULL);
	// QUIRK: the Unicode string wins over the ASCII string when it has characters.
	CHECK(oldProfile != NULL && descriptionIs(oldProfile, "Hi"));
	CFRelease(oldData);
	put32(bytes + tableEnd + 22, 0); // no Unicode characters: the ASCII string
	oldData = CFDataCreate(NULL, bytes, tableEnd + 40);
	ColorSyncProfileRef asciiOnly = ColorSyncProfileCreate(oldData, NULL);
	CHECK(asciiOnly != NULL && descriptionIs(asciiOnly, "Plain"));
	CFRelease(oldData);
	CFRelease(asciiOnly);

	// QUIRK: no description, or an unreadable one, gives a fixed text, not NULL.
	memcpy(bytes + 132, "wtpt", 4);
	CFDataRef noDesc = CFDataCreate(NULL, bytes, tableEnd + 40);
	ColorSyncProfileRef without = ColorSyncProfileCreate(noDesc, NULL);
	CHECK(without != NULL && descriptionIs(without, "This profile has no description tag"));
	CFRelease(noDesc);
	memcpy(bytes + 132, "desc", 4);
	memcpy(bytes + tableEnd, "text", 4);
	CFDataRef wrongType = CFDataCreate(NULL, bytes, tableEnd + 40);
	ColorSyncProfileRef odd = ColorSyncProfileCreate(wrongType, NULL);
	CHECK(odd != NULL && descriptionIs(odd, "This profile has a bad description tag"));
	CFRelease(wrongType);

	CHECK(ColorSyncProfileCopyDescriptionString(NULL) == NULL);
	CHECK(ColorSyncProfileCopyDescriptionString((ColorSyncProfileRef) CFSTR("not a profile")) == NULL);

	CFRelease(single);
	CFRelease(noMatch);
	CFRelease(oldProfile);
	CFRelease(without);
	CFRelease(odd);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures != 0;
}
