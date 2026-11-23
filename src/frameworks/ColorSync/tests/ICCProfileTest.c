// Native test: compile with the ICC layout code and run on the host.
//   cc -I src -I tests/host tests/ICCProfileTest.c src/ICCProfile.c -lcrypto -o ICCProfileTest
// Also run it with -fsanitize=address,undefined: the checks on the tag count and offsets guard reads past the end.
// The profiles are built by hand from the layout in ICC.1:2022, 7.2 and 7.3
// (https://www.color.org/specification/ICC.1-2022-05.pdf), so they contain no vendor data.
#include "ICCProfile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int g_failures;

static void expect(bool condition, const char* what)
{
	if (!condition)
	{
		printf("FAIL %s\n", what);
		g_failures++;
	}
}

static void put32(uint8_t* p, uint32_t v)
{
	p[0] = v >> 24;
	p[1] = v >> 16;
	p[2] = v >> 8;
	p[3] = v;
}

#define SIG(a, b, c, d) (((uint32_t) (a) << 24) | ((uint32_t) (b) << 16) | ((uint32_t) (c) << 8) | (uint32_t) (d))

// Two tags: 'wtpt' (4 bytes) and 'desc' (8 bytes), laid out right after the tag table.
static size_t buildProfile(uint8_t* out)
{
	const uint32_t tableEnd = ICC_HEADER_SIZE + 4 + 2 * 12;
	memset(out, 0, tableEnd + 12);
	put32(out, tableEnd + 12);
	put32(out + 36, SIG('a', 'c', 's', 'p'));
	put32(out + 128, 2);
	put32(out + 132, SIG('w', 't', 'p', 't'));
	put32(out + 136, tableEnd);
	put32(out + 140, 4);
	put32(out + 144, SIG('d', 'e', 's', 'c'));
	put32(out + 148, tableEnd + 4);
	put32(out + 152, 8);
	memcpy(out + tableEnd, "WTPT", 4);
	memcpy(out + tableEnd + 4, "DESCDESC", 8);
	return tableEnd + 12;
}

// A profile whose only tag is 'desc', holding `blob` right after the tag table.
static size_t buildWithDescription(uint8_t* out, const uint8_t* blob, size_t blobLength)
{
	const uint32_t tableEnd = ICC_HEADER_SIZE + 4 + 12;
	memset(out, 0, tableEnd);
	put32(out, (uint32_t) (tableEnd + blobLength));
	put32(out + 36, SIG('a', 'c', 's', 'p'));
	put32(out + 128, 1);
	put32(out + 132, SIG('d', 'e', 's', 'c'));
	put32(out + 136, tableEnd);
	put32(out + 140, (uint32_t) blobLength);
	memcpy(out + tableEnd, blob, blobLength);
	return tableEnd + blobLength;
}

typedef struct DescriptionRecord {
	const char* language;
	const char* country;
	const char* text; // ASCII, stored as UTF-16BE
} DescriptionRecord;

// A multiLocalizedUnicodeType (ICC.1:2022, 10.15) with the records in the order given.
static size_t buildMultiLocalized(uint8_t* out, const DescriptionRecord* records, uint32_t count)
{
	size_t stringOffset = 16 + (size_t) count * 12;
	memset(out, 0, 512);
	memcpy(out, "mluc", 4);
	put32(out + 8, count);
	put32(out + 12, 12);
	for (uint32_t i = 0; i < count; i++)
	{
		uint8_t* record = out + 16 + i * 12;
		const size_t length = strlen(records[i].text) * 2;
		memcpy(record, records[i].language, 2);
		memcpy(record + 2, records[i].country, 2);
		put32(record + 4, (uint32_t) length);
		put32(record + 8, (uint32_t) stringOffset);
		for (size_t c = 0; c < length / 2; c++)
			out[stringOffset + c * 2 + 1] = (uint8_t) records[i].text[c];
		stringOffset += length;
	}
	return stringOffset;
}

// The text found for the locale, as ASCII in a static buffer; "" and false if none.
static bool descriptionText(const uint8_t* profile, const char* language, const char* country, char* out, bool* isUTF16)
{
	ICCText text;
	out[0] = 0;
	if (!ICCProfileFindDescription(profile, language, country, &text))
		return false;
	*isUTF16 = text.isUTF16BE;
	const size_t step = text.isUTF16BE ? 2 : 1;
	size_t n = 0;
	for (size_t i = 0; i < text.length; i += step)
		out[n++] = (char) text.bytes[i + step - 1];
	out[n] = 0;
	return true;
}

int main(void)
{
	uint8_t profile[256];
	const size_t length = buildProfile(profile);

	expect(ICCProfileValidate(profile, length), "well-formed profile is valid");
	expect(ICCProfileTagCount(profile) == 2, "tag count");

	ICCTag tag;
	expect(ICCProfileTagAt(profile, 0).signature == SIG('w', 't', 'p', 't'), "first tag signature");
	expect(ICCProfileFindTag(profile, SIG('d', 'e', 's', 'c'), &tag), "find desc");
	expect(tag.offset == 160 && tag.size == 8, "desc offset and size");
	expect(memcmp(profile + tag.offset, "DESCDESC", 8) == 0, "desc bytes");
	expect(!ICCProfileFindTag(profile, SIG('c', 'p', 'r', 't'), &tag), "missing tag");

	// A profile with no tags is valid.
	uint8_t empty[132] = { 0 };
	put32(empty, 132);
	put32(empty + 36, SIG('a', 'c', 's', 'p'));
	expect(ICCProfileValidate(empty, sizeof empty), "empty tag table");
	expect(ICCProfileTagCount(empty) == 0, "empty count");

	// Duplicate signatures are accepted; the last entry wins (QUIRK, see ICCProfileFindTag) and each signature is
	// listed once, in table order.
	uint8_t dup[256];
	memcpy(dup, profile, length);
	put32(dup + 144, SIG('w', 't', 'p', 't'));
	expect(ICCProfileValidate(dup, length), "duplicate signatures validate");
	expect(ICCProfileFindTag(dup, SIG('w', 't', 'p', 't'), &tag) && tag.offset == 160, "last duplicate wins");
	uint32_t indices[2], indexCount;
	expect(ICCProfileFirstTagIndices(dup, indices, &indexCount) && indexCount == 1 && indices[0] == 0, "duplicate listed once");
	expect(ICCProfileFirstTagIndices(profile, indices, &indexCount) && indexCount == 2 && indices[0] == 0 && indices[1] == 1, "distinct signatures all listed");

	// A hostile profile: the most zero-size tags that fit, all with one signature, at offset == length.
	// Validation and listing must stay close to linear (a quadratic scan would take tens of seconds here).
	const size_t hostileLength = 4u << 20;
	const uint32_t hostileCount = (uint32_t) ((hostileLength - ICC_HEADER_SIZE - 4) / 12);
	uint8_t* hostile = calloc(hostileLength, 1);
	put32(hostile, (uint32_t) hostileLength);
	put32(hostile + 36, SIG('a', 'c', 's', 'p'));
	put32(hostile + 128, hostileCount);
	for (uint32_t i = 0; i < hostileCount; i++)
	{
		uint8_t* entry = hostile + 132 + (size_t) i * 12;
		put32(entry, SIG('d', 'u', 'p', ' '));
		put32(entry + 4, (uint32_t) hostileLength);
	}
	uint32_t* hostileIndices = malloc((size_t) hostileCount * sizeof(uint32_t));
	const clock_t start = clock();
	expect(ICCProfileValidate(hostile, hostileLength), "hostile profile validates");
	expect(ICCProfileFirstTagIndices(hostile, hostileIndices, &indexCount) && indexCount == 1 && hostileIndices[0] == 0, "hostile profile lists one signature");
	expect(clock() - start < 2 * CLOCKS_PER_SEC, "hostile profile is handled in linear time");
	free(hostileIndices);
	free(hostile);

	// The Profile ID (ICC.1:2022, 7.2.18) is the MD5 digest of the profile with the flags (44 to 47), the rendering intent
	// (64 to 67) and the ID field (84 to 99) zeroed. The expected digests are Python's hashlib.md5 over the same bytes
	// (those fields are zero already).
	uint8_t digest[16];
	char hex[33];
	expect(ICCProfileComputeID(profile, length, digest), "profile ID of a valid profile");
	for (int i = 0; i < 16; i++)
		snprintf(hex + i * 2, 3, "%02x", digest[i]);
	expect(strcmp(hex, "537ca42b81eb6c480db1bacbfb538365") == 0, "profile ID of a profile with those fields zero");

	uint8_t filled[256];
	uint8_t other[16];
	memcpy(filled, profile, length);
	memset(filled + 44, 0xff, 4);
	memset(filled + 64, 0x55, 4);
	memset(filled + 84, 0xaa, 16);
	expect(ICCProfileComputeID(filled, length, other) && memcmp(digest, other, 16) == 0, "flags, rendering intent and ID field do not change the profile ID");
	for (size_t byte = 4; byte < length; byte++) // the size field itself, bytes 0 to 3, is tested below
	{
		const bool excluded = (byte >= 44 && byte < 48) || (byte >= 64 && byte < 68) || (byte >= 84 && byte < 100);
		memcpy(filled, profile, length);
		filled[byte] ^= 1;
		if (!ICCProfileComputeID(filled, length, other) || (memcmp(digest, other, 16) != 0) == excluded)
		{
			printf("FAIL profile ID: flipping byte %zu %s the digest\n", byte, excluded ? "changed" : "did not change");
			g_failures++;
		}
	}

	// QUIRK pinned here, "profile ID covers all bytes held": every byte held is hashed, whatever the size field says, and
	// the size field never makes the digest fail (macOS 10.14.4 behaviour).
	memcpy(filled, profile, length);
	memset(filled + length, 0x7f, 8);
	expect(ICCProfileComputeID(filled, length + 8, other), "bytes after the profile");
	for (int i = 0; i < 16; i++)
		snprintf(hex + i * 2, 3, "%02x", other[i]);
	expect(strcmp(hex, "2878d838d326269f78d2c40a3673e0cd") == 0, "profile ID covers all bytes held");
	static const struct { uint32_t sizeField; const char* digest; } sizeFields[] = {
		{ 160, "848ff48cb05e181f8217568eb2c5df1e" }, { 100, "549bc6b5ca2bd260b6e70d181ecaf9c0" },
		{ 99, "1f27c22277ad2a4574d2677860e9bd6c" }, { 169, "46176971c29ec1fe743ed5ff72c621c1" }, { 0, "725b712e67e6859c50e3ecade59ce6e2" },
	};
	for (size_t i = 0; i < sizeof sizeFields / sizeof sizeFields[0]; i++)
	{
		memcpy(filled, profile, length);
		put32(filled, sizeFields[i].sizeField);
		expect(ICCProfileComputeID(filled, length, other), "profile ID with a wrong size field");
		for (int k = 0; k < 16; k++)
			snprintf(hex + k * 2, 3, "%02x", other[k]);
		expect(strcmp(hex, sizeFields[i].digest) == 0, "profile ID covers all bytes held, whatever the size field");
	}
	memset(other, 0x33, 16);
	expect(!ICCProfileComputeID(filled, 99, other) && other[0] == 0x33, "fewer than 100 bytes held");

	// Invalid inputs.
	expect(!ICCProfileValidate(NULL, 0), "null data");
	expect(!ICCProfileValidate(profile, 131), "shorter than header and tag count");

	// QUIRK pinned here, "matrix profile": an 'RGB ' profile with rXYZ, gXYZ, bXYZ, rTRC, gTRC and bTRC (ICC.1:2022, 8.3.3 and
	// 8.4.3) or a 'GRAY' profile with kTRC is matrix-based, unless it has an A2B0, A2B1 or A2B2 tag. Tags of 4 bytes follow the table.
	static const char* const rgbSignatures[6] = { "rXYZ", "gXYZ", "bXYZ", "rTRC", "gTRC", "bTRC" };
	const struct { const char* name; uint32_t colorSpace; const char* const* signatures; uint32_t count; const char* extra; bool expected; } cases[] = {
		{ "RGB six tags", SIG('R', 'G', 'B', ' '), rgbSignatures, 6, NULL, true },
		{ "RGB six tags and A2B0", SIG('R', 'G', 'B', ' '), rgbSignatures, 6, "A2B0", false },
		{ "RGB six tags and A2B1", SIG('R', 'G', 'B', ' '), rgbSignatures, 6, "A2B1", false },
		{ "RGB six tags and A2B2", SIG('R', 'G', 'B', ' '), rgbSignatures, 6, "A2B2", false },
		{ "RGB six tags and B2A0", SIG('R', 'G', 'B', ' '), rgbSignatures, 6, "B2A0", true },
		{ "RGB six tags and D2B0", SIG('R', 'G', 'B', ' '), rgbSignatures, 6, "D2B0", true },
		{ "RGB six tags and gamt", SIG('R', 'G', 'B', ' '), rgbSignatures, 6, "gamt", true },
		{ "CMYK six tags", SIG('C', 'M', 'Y', 'K'), rgbSignatures, 6, NULL, false },
		{ "GRAY six tags", SIG('G', 'R', 'A', 'Y'), rgbSignatures, 6, NULL, false },
		{ "GRAY kTRC", SIG('G', 'R', 'A', 'Y'), (const char* const[]) { "kTRC" }, 1, NULL, true },
		{ "GRAY kTRC and A2B0", SIG('G', 'R', 'A', 'Y'), (const char* const[]) { "kTRC" }, 1, "A2B0", false },
		{ "GRAY kTRC and B2A0", SIG('G', 'R', 'A', 'Y'), (const char* const[]) { "kTRC" }, 1, "B2A0", true },
		{ "RGB kTRC only", SIG('R', 'G', 'B', ' '), (const char* const[]) { "kTRC" }, 1, NULL, false },
		{ "GRAY without kTRC", SIG('G', 'R', 'A', 'Y'), (const char* const[]) { "rTRC" }, 1, NULL, false },
	};
	uint8_t matrix[256];
	for (size_t c = 0; c < sizeof cases / sizeof cases[0]; c++)
	{
		const uint32_t count = cases[c].count + (cases[c].extra ? 1 : 0);
		const uint32_t end = ICC_HEADER_SIZE + 4 + count * 12 + count * 4;
		memset(matrix, 0, sizeof matrix);
		put32(matrix, end);
		put32(matrix + 16, cases[c].colorSpace);
		put32(matrix + 36, SIG('a', 'c', 's', 'p'));
		put32(matrix + 128, count);
		for (uint32_t i = 0; i < count; i++)
		{
			memcpy(matrix + 132 + i * 12, i < cases[c].count ? cases[c].signatures[i] : cases[c].extra, 4);
			put32(matrix + 136 + i * 12, ICC_HEADER_SIZE + 4 + count * 12 + i * 4);
			put32(matrix + 140 + i * 12, 4);
		}
		expect(ICCProfileValidate(matrix, end), "matrix profile validates");
		if (ICCProfileHasMatrixTRC(matrix) != cases[c].expected)
		{
			printf("FAIL matrix profile: %s\n", cases[c].name);
			g_failures++;
		}
		if (cases[c].expected && !cases[c].extra && cases[c].count == 6)
			for (uint32_t missing = 0; missing < 6; missing++)
			{
				uint8_t partial[256];
				memcpy(partial, matrix, sizeof partial);
				partial[132 + missing * 12] = 'Q'; // another signature in its place
				expect(!ICCProfileHasMatrixTRC(partial), "matrix profile without one of its six tags");
			}
	}
	expect(!ICCProfileHasMatrixTRC(profile), "profile with other tags only");
	expect(!ICCProfileHasMatrixTRC(empty), "profile without tags");

	// Cut-off tag tables, in buffers of exactly the stated size so a read past the end is a sanitizer error.
	uint8_t* cut = malloc(132);
	memcpy(cut, profile, 132);
	put32(cut, 132);
	put32(cut + 128, 1);
	expect(!ICCProfileValidate(cut, 132), "tag count 1 with no room for the entry");
	free(cut);

	uint8_t bad[256];
	memcpy(bad, profile, length);
	bad[36] = 'x';
	expect(!ICCProfileValidate(bad, length), "wrong file signature");

	// The header in host byte order (ICCHeaderSwapFields). Each field is read back with memcpy as the host number it is.
	uint8_t fileHeader[ICC_HEADER_SIZE], hostHeader[ICC_HEADER_SIZE], again[ICC_HEADER_SIZE];
	memset(fileHeader, 0, sizeof fileHeader);
	put32(fileHeader + 0, 0x11223344u);
	put32(fileHeader + 4, SIG('A', 'B', 'C', 'D'));
	put32(fileHeader + 8, 0x04300000u);
	put32(fileHeader + 12, SIG('m', 'n', 't', 'r'));
	put32(fileHeader + 16, SIG('R', 'G', 'B', ' '));
	put32(fileHeader + 20, SIG('X', 'Y', 'Z', ' '));
	const uint16_t dateFields[6] = { 2026, 11, 23, 14, 30, 59 };
	for (int i = 0; i < 6; i++)
	{
		fileHeader[24 + i * 2] = dateFields[i] >> 8;
		fileHeader[25 + i * 2] = (uint8_t) dateFields[i];
	}
	put32(fileHeader + 36, SIG('a', 'c', 's', 'p'));
	put32(fileHeader + 40, SIG('A', 'P', 'P', 'L'));
	put32(fileHeader + 44, 3);
	put32(fileHeader + 48, 0x01020304u);
	put32(fileHeader + 52, 0x05060708u);
	put32(fileHeader + 56, 0x0a0b0c0du);
	put32(fileHeader + 60, 0x0e0f1011u);
	put32(fileHeader + 64, 1);
	put32(fileHeader + 68, 0x0000f6d6u);
	put32(fileHeader + 72, 0x00010000u);
	put32(fileHeader + 76, 0x0000d32du);
	put32(fileHeader + 80, SIG('Z', 'Z', 'Z', 'Z'));
	for (int i = 0; i < 16; i++)
		fileHeader[84 + i] = (uint8_t) (0xa0 + i);
	for (int i = 0; i < 28; i++)
		fileHeader[100 + i] = (uint8_t) (0xc0 + i);

	ICCHeaderSwapFields(fileHeader, hostHeader);
	static const struct { int offset; uint32_t value; } words[] = {
		{ 0, 0x11223344u }, { 4, SIG('A', 'B', 'C', 'D') }, { 8, 0x04300000u }, { 12, SIG('m', 'n', 't', 'r') },
		{ 16, SIG('R', 'G', 'B', ' ') }, { 20, SIG('X', 'Y', 'Z', ' ') }, { 36, SIG('a', 'c', 's', 'p') },
		{ 40, SIG('A', 'P', 'P', 'L') }, { 44, 3 }, { 48, 0x01020304u }, { 52, 0x05060708u }, { 64, 1 },
		{ 68, 0x0000f6d6u }, { 72, 0x00010000u }, { 76, 0x0000d32du }, { 80, SIG('Z', 'Z', 'Z', 'Z') },
	};
	for (size_t i = 0; i < sizeof words / sizeof words[0]; i++)
	{
		uint32_t word;
		memcpy(&word, hostHeader + words[i].offset, 4);
		expect(word == words[i].value, "32-bit header field in host order");
	}
	for (int i = 0; i < 6; i++)
	{
		uint16_t field;
		memcpy(&field, hostHeader + 24 + i * 2, 2);
		expect(field == dateFields[i], "date and time field in host order");
	}
	uint32_t attributes[2];
	memcpy(attributes, hostHeader + 56, 8);
	expect(attributes[0] == 0x0a0b0c0du && attributes[1] == 0x0e0f1011u, "device attributes in host order as two 32-bit numbers");
	expect(memcmp(hostHeader + 84, fileHeader + 84, 44) == 0, "Profile ID and reserved bytes are not swapped");
	ICCHeaderSwapFields(hostHeader, again);
	expect(memcmp(again, fileHeader, ICC_HEADER_SIZE) == 0, "swapping twice gives back the file header");

	// A buffer of exactly the profile's size, so a read past the tag table is an address sanitizer error.
	uint8_t* exact = malloc(length);
	memcpy(exact, profile, length);
	put32(exact + 128, 0x20000000u);
	expect(!ICCProfileValidate(exact, length), "huge tag count");
	put32(exact + 128, 3);
	expect(!ICCProfileValidate(exact, length), "tag count one larger than the data holds");
	free(exact);

	memcpy(bad, profile, length);
	put32(bad + 152, 9);
	expect(!ICCProfileValidate(bad, length), "tag runs past the end");

	// One tag at the first legal offset (144) with a size whose sum with the offset wraps to 128 in 32 bits.
	uint8_t* wrap = malloc(148);
	memset(wrap, 0, 148);
	put32(wrap, 148);
	put32(wrap + 36, SIG('a', 'c', 's', 'p'));
	put32(wrap + 128, 1);
	put32(wrap + 132, SIG('w', 't', 'p', 't'));
	put32(wrap + 136, 144);
	put32(wrap + 140, 0xFFFFFFF0u);
	expect(!ICCProfileValidate(wrap, 148), "offset plus size wraps around 32 bits");
	free(wrap);

	memcpy(bad, profile, length);
	put32(bad + 148, 0xFFFFFFFCu);
	put32(bad + 152, 8);
	expect(!ICCProfileValidate(bad, length), "offset beyond the data");

	memcpy(bad, profile, length);
	put32(bad + 136, 130);
	expect(!ICCProfileValidate(bad, length), "tag data inside the tag table");

	memcpy(bad, profile, length);
	put32(bad, (uint32_t) length + 1);
	expect(ICCProfileValidate(bad, length), "header size larger than the data is accepted");
	put32(bad, 10);
	expect(ICCProfileValidate(bad, length), "header size smaller than the data is accepted");

	// The profile description (ICC.1:2022, 10.15, 9.2.43; ICC.1:2001-04, 6.5.17).
	uint8_t blob[512], withDesc[1024];
	char text[64];
	bool isUTF16 = false;
	const DescriptionRecord several[] = { { "fr", "FR", "Bonjour" }, { "en", "GB", "Hello GB" }, { "en", "US", "Hello US" }, { "de", "DE", "Hallo" } };
	size_t blobLength = buildMultiLocalized(blob, several, 4);
	const size_t withDescLength = buildWithDescription(withDesc, blob, blobLength);
	expect(ICCProfileValidate(withDesc, withDescLength), "profile with a multi-localized description validates");
	// QUIRK pinned here, "English (United States) record wins": a record for en-US is chosen in every locale.
	static const char* const locales[][2] = { { "en", "US" }, { "en", "GB" }, { "de", "DE" }, { "fr", "FR" }, { "ja", "JP" }, { "en", NULL }, { NULL, NULL }, { NULL, "US" } };
	for (size_t i = 0; i < sizeof locales / sizeof locales[0]; i++)
		expect(descriptionText(withDesc, locales[i][0], locales[i][1], text, &isUTF16) && isUTF16 && strcmp(text, "Hello US") == 0, "English (United States) record wins");

	// Without an en-US record: the locale's language and country, else the first of the language, else the first.
	const DescriptionRecord others[] = { { "fr", "FR", "Bonjour" }, { "en", "GB", "Hello GB" }, { "en", "AU", "Hello AU" }, { "de", "DE", "Hallo" } };
	blobLength = buildMultiLocalized(blob, others, 4);
	buildWithDescription(withDesc, blob, blobLength);
	expect(descriptionText(withDesc, "en", "AU", text, &isUTF16) && strcmp(text, "Hello AU") == 0, "same language and country");
	expect(descriptionText(withDesc, "en", "GB", text, &isUTF16) && strcmp(text, "Hello GB") == 0, "same language and country, earlier record");
	expect(descriptionText(withDesc, "de", "AT", text, &isUTF16) && strcmp(text, "Hallo") == 0, "same language, other country");
	expect(descriptionText(withDesc, "en", "NZ", text, &isUTF16) && strcmp(text, "Hello GB") == 0, "same language: the first of them");
	expect(descriptionText(withDesc, "en", NULL, text, &isUTF16) && strcmp(text, "Hello GB") == 0, "language only");
	expect(descriptionText(withDesc, "ja", "JP", text, &isUTF16) && strcmp(text, "Bonjour") == 0, "no matching record: the first");
	expect(descriptionText(withDesc, NULL, NULL, text, &isUTF16) && strcmp(text, "Bonjour") == 0, "no preference: the first");
	expect(descriptionText(withDesc, NULL, "AU", text, &isUTF16) && strcmp(text, "Bonjour") == 0, "a country without a language is no preference");
	expect(descriptionText(withDesc, "ru", "AU", text, &isUTF16) && strcmp(text, "Bonjour") == 0, "a country of another language does not match");

	// A null character at the end of a string is not part of it; an empty string is empty.
	const DescriptionRecord terminated[] = { { "en", "US", "Hi" } };
	blobLength = buildMultiLocalized(blob, terminated, 1);
	put32(blob + 16 + 4, 6); // three characters: "Hi" and a null
	buildWithDescription(withDesc, blob, blobLength + 2);
	expect(descriptionText(withDesc, "en", "US", text, &isUTF16) && strcmp(text, "Hi") == 0, "trailing null character dropped");
	put32(blob + 16 + 4, 5); // "Hi" and half a character
	blob[blobLength] = 'x';
	buildWithDescription(withDesc, blob, blobLength + 1);
	ICCText halfCharacter;
	expect(ICCProfileFindDescription(withDesc, "en", "US", &halfCharacter) && halfCharacter.length == 4, "half a character at the end dropped");
	put32(blob + 16 + 4, 0);
	buildWithDescription(withDesc, blob, blobLength);
	expect(descriptionText(withDesc, "en", "US", text, &isUTF16) && text[0] == 0, "empty string");

	// Damaged multi-localized tags.
	blobLength = buildMultiLocalized(blob, others, 4);
	put32(blob + 16 + 4 + 12 * 3, 4000); // last record: string longer than the tag
	buildWithDescription(withDesc, blob, blobLength);
	expect(descriptionText(withDesc, "de", "DE", text, &isUTF16) == false, "chosen string runs past the tag");
	expect(descriptionText(withDesc, "fr", "FR", text, &isUTF16) && strcmp(text, "Bonjour") == 0, "another record of that tag still reads");
	blobLength = buildMultiLocalized(blob, others, 4);
	put32(blob + 8, 0);
	buildWithDescription(withDesc, blob, blobLength);
	expect(!descriptionText(withDesc, "en", "US", text, &isUTF16), "no records");
	put32(blob + 8, 1000);
	buildWithDescription(withDesc, blob, blobLength);
	expect(!descriptionText(withDesc, "en", "US", text, &isUTF16), "more records than the tag holds");
	put32(blob + 8, 4);
	put32(blob + 12, 11);
	buildWithDescription(withDesc, blob, blobLength);
	expect(!descriptionText(withDesc, "en", "US", text, &isUTF16), "record size below 12");
	expect(!ICCProfileFindDescription(profile, "en", "US", &(ICCText) { 0 }), "profile without a description tag");
	buildWithDescription(withDesc, blob, 6);
	expect(!descriptionText(withDesc, "en", "US", text, &isUTF16), "tag shorter than its header");

	// textDescriptionType: the Unicode string if it has characters (QUIRK, "Unicode part of a textDescriptionType wins"), else
	// the ASCII string; the ScriptCode part after them is ignored.
	uint8_t old[128] = { 0 };
	memcpy(old, "desc", 4);
	put32(old + 8, 6);
	memcpy(old + 12, "Plain", 6);
	put32(old + 18, 0x656e5553); // Unicode language code
	put32(old + 22, 3); // Unicode count: "Hi" and a null
	memcpy(old + 26, "\0H\0i\0\0", 6);
	buildWithDescription(withDesc, old, 40);
	expect(descriptionText(withDesc, "en", "US", text, &isUTF16) && isUTF16 && strcmp(text, "Hi") == 0, "Unicode part of a textDescriptionType wins");
	put32(old + 22, 0); // no Unicode characters: the ASCII string
	buildWithDescription(withDesc, old, 40);
	expect(descriptionText(withDesc, "en", "US", text, &isUTF16) && !isUTF16 && strcmp(text, "Plain") == 0, "ASCII part of a textDescriptionType without a Unicode part");
	put32(old + 22, 1); // only the null
	buildWithDescription(withDesc, old, 40);
	expect(descriptionText(withDesc, "en", "US", text, &isUTF16) && !isUTF16 && strcmp(text, "Plain") == 0, "a Unicode part of only a null is empty");
	put32(old + 22, 3);
	put32(old + 8, 1); // empty ASCII string (just its null): the Unicode part starts at byte 13
	memset(old + 12, 0, 20);
	put32(old + 13, 0x656e5553);
	put32(old + 17, 3);
	memcpy(old + 21, "\0H\0i\0\0", 6);
	buildWithDescription(withDesc, old, 40);
	expect(descriptionText(withDesc, "en", "US", text, &isUTF16) && isUTF16 && strcmp(text, "Hi") == 0, "Unicode part after an empty ASCII string");
	put32(old + 17, 1000); // Unicode count past the tag: the ASCII string
	buildWithDescription(withDesc, old, 40);
	expect(descriptionText(withDesc, "en", "US", text, &isUTF16) && !isUTF16 && text[0] == 0, "Unicode count past the tag is ignored");
	memset(old, 0, sizeof old);
	memcpy(old, "desc", 4);
	put32(old + 8, 1000);
	buildWithDescription(withDesc, old, 40);
	expect(!descriptionText(withDesc, "en", "US", text, &isUTF16), "ASCII count past the tag");
	memcpy(old, "text", 4);
	put32(old + 8, 6);
	buildWithDescription(withDesc, old, 40);
	expect(!descriptionText(withDesc, "en", "US", text, &isUTF16), "description of a type that is not understood");

	if (g_failures)
	{
		printf("ICCProfileTest: %d failures\n", g_failures);
		return 1;
	}
	printf("ICCProfileTest: OK\n");
	return 0;
}
