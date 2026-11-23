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

#include "ICCProfile.h"
#include <CommonCrypto/CommonDigest.h>
#include <stdlib.h>
#include <string.h>

#define ICC_FILE_SIGNATURE_OFFSET 36
#define ICC_FILE_SIGNATURE 0x61637370u // 'acsp'
#define ICC_TAG_ENTRY_SIZE 12

static uint32_t readBigEndian32(const uint8_t* p)
{
	return ((uint32_t) p[0] << 24) | ((uint32_t) p[1] << 16) | ((uint32_t) p[2] << 8) | p[3];
}

static void reverseBytes(uint8_t* p, size_t count)
{
	for (size_t i = 0; i < count / 2; i++)
	{
		const uint8_t t = p[i];
		p[i] = p[count - 1 - i];
		p[count - 1 - i] = t;
	}
}

static bool hostIsLittleEndian(void)
{
	const uint16_t one = 1;
	uint8_t first;
	memcpy(&first, &one, 1);
	return first == 1;
}

void ICCHeaderSwapFields(const uint8_t in[ICC_HEADER_SIZE], uint8_t out[ICC_HEADER_SIZE])
{
	static const struct { uint8_t offset, width; } fields[] = {
		{ 0, 4 }, { 4, 4 }, { 8, 4 }, { 12, 4 }, { 16, 4 }, { 20, 4 },
		{ 24, 2 }, { 26, 2 }, { 28, 2 }, { 30, 2 }, { 32, 2 }, { 34, 2 },
		{ 36, 4 }, { 40, 4 }, { 44, 4 }, { 48, 4 }, { 52, 4 },
		{ 56, 4 }, { 60, 4 },
		{ 64, 4 }, { 68, 4 }, { 72, 4 }, { 76, 4 }, { 80, 4 },
	};
	memcpy(out, in, ICC_HEADER_SIZE);
	if (!hostIsLittleEndian())
		return;
	for (size_t i = 0; i < sizeof fields / sizeof fields[0]; i++)
		reverseBytes(out + fields[i].offset, fields[i].width);
}

uint32_t ICCProfileTagCount(const uint8_t* data)
{
	return readBigEndian32(data + ICC_HEADER_SIZE);
}

ICCTag ICCProfileTagAt(const uint8_t* data, uint32_t index)
{
	const uint8_t* entry = data + ICC_HEADER_SIZE + 4 + (size_t) index * ICC_TAG_ENTRY_SIZE;
	ICCTag tag = { readBigEndian32(entry), readBigEndian32(entry + 4), readBigEndian32(entry + 8) };
	return tag;
}

// GUESS: how strictly the original validates is not documented. This checks what is needed to read
// tags safely, plus the header signature, and stays lenient about the spec's alignment and ordering rules.
// The profile size field in the header is not compared with the length of the data: ColorSyncProfileCreate on
// macOS 26.7.3 accepts a size field that is larger or smaller than the data (black-box observation, tier 2).
bool ICCProfileValidate(const uint8_t* data, size_t length)
{
	if (!data || length < ICC_HEADER_SIZE + 4 || length > UINT32_MAX)
		return false;
	if (readBigEndian32(data + ICC_FILE_SIGNATURE_OFFSET) != ICC_FILE_SIGNATURE)
		return false;

	const uint32_t count = ICCProfileTagCount(data);
	if (count > (length - ICC_HEADER_SIZE - 4) / ICC_TAG_ENTRY_SIZE)
		return false;

	const size_t dataStart = ICC_HEADER_SIZE + 4 + (size_t) count * ICC_TAG_ENTRY_SIZE;
	for (uint32_t i = 0; i < count; i++)
	{
		const ICCTag tag = ICCProfileTagAt(data, i);
		if (tag.offset < dataStart || tag.offset > length || tag.size > length - tag.offset)
			return false;
	}
	return true;
}

static void ICCDigestUpdate(CC_MD5_CTX* context, const uint8_t* bytes, size_t length)
{
	// CC_MD5_Update takes a 32-bit length.
	while (length > 0)
	{
		CC_LONG chunk = length > 0x40000000 ? 0x40000000 : (CC_LONG) length;
		CC_MD5_Update(context, bytes, chunk);
		bytes += chunk;
		length -= chunk;
	}
}

// QUIRK: the digest covers every byte held, not only the number of bytes the size field gives, and the size field never
// makes the digest fail. ICC.1:2022, 7.2.18 defines it over "the entire profile, whose length is given by the size field";
// ColorSyncProfileCreate accepts a profile whose size field is wrong or whose data has bytes after the profile. Evidence:
// black-box observation of ColorSyncProfileGetMD5, tier 2: macOS 10.14.4 (x86_64 and i386) hashes all the bytes held for
// a size field of 160 or 100 or 99 or 0 or above the data, and with a trailing byte; 26.7.3 (x86_64) does the same when the
// size field is above the data but hashes only the size field's bytes when it is within the data (and gives other digests
// for a size field of 99, 100 or 0). Where they differ the widest behaviour, Mojave's, is followed. Neither gives the zero
// digest for these inputs.
// Test: ICCProfileTest.c, "profile ID covers all bytes held"; tests/src/colorsync_profile_md5.c.
bool ICCProfileComputeID(const uint8_t* data, size_t length, uint8_t outDigest[16])
{
	static const struct { size_t start, end; } zeroed[] = { { 44, 48 }, { 64, 68 }, { 84, 100 } };
	static const uint8_t zeros[16];

	// The zeroed fields end at byte 100.
	if (length < 100)
		return false;

	CC_MD5_CTX context;
	CC_MD5_Init(&context);
	size_t position = 0;
	for (size_t i = 0; i < sizeof zeroed / sizeof zeroed[0]; i++)
	{
		ICCDigestUpdate(&context, data + position, zeroed[i].start - position);
		ICCDigestUpdate(&context, zeros, zeroed[i].end - zeroed[i].start);
		position = zeroed[i].end;
	}
	ICCDigestUpdate(&context, data + position, length - position);
	CC_MD5_Final(outDigest, &context);
	return true;
}

// QUIRK: duplicate signatures are invalid under ICC.1:2022, 7.3.1, but are accepted, and the last entry of a
// signature is the one that ColorSyncProfileContainsTag and ColorSyncProfileCopyTag see. Not documented.
// Evidence: black-box observation of ColorSyncProfileCopyTag on macOS 26.7.3 (x86_64), tier 2, for two and
// three entries with one signature: the data of the last entry came back.
// Test: ICCProfileTest.c, "last duplicate wins".
bool ICCProfileFindTag(const uint8_t* data, uint32_t signature, ICCTag* outTag)
{
	const uint32_t count = ICCProfileTagCount(data);
	for (uint32_t i = count; i-- > 0;)
	{
		const ICCTag tag = ICCProfileTagAt(data, i);
		if (tag.signature == signature)
		{
			*outTag = tag;
			return true;
		}
	}
	return false;
}

#define SIG(a, b, c, d) (((uint32_t) (a) << 24) | ((uint32_t) (b) << 16) | ((uint32_t) (c) << 8) | (uint32_t) (d))

// QUIRK: whether a profile is matrix-based depends on its colour space and on its AToB tables. ColorSyncProfileIsMatrixBased
// is true for an 'RGB ' profile with all six of rXYZ, gXYZ, bXYZ, rTRC, gTRC, bTRC and for a 'GRAY' profile with kTRC, and
// false when the profile also has an A2B0, A2B1 or A2B2 tag (other tags, such as B2A0, D2B0 or gamt, do not matter) and for any
// other colour space, whatever the profile class, version or PCS. Evidence: black-box observation on macOS 26.7.3 (x86_64),
// tier 2, of hand-built profiles; the function is not in macOS 10.14.4.
// Test: ICCProfileTest.c, "matrix profile"; tests/src/colorsync_profile_matrix_based.c.
bool ICCProfileHasMatrixTRC(const uint8_t* data)
{
	static const uint32_t rgbRequired[] = {
		SIG('r', 'X', 'Y', 'Z'), SIG('g', 'X', 'Y', 'Z'), SIG('b', 'X', 'Y', 'Z'),
		SIG('r', 'T', 'R', 'C'), SIG('g', 'T', 'R', 'C'), SIG('b', 'T', 'R', 'C'),
	};
	static const uint32_t grayRequired[] = { SIG('k', 'T', 'R', 'C') };
	static const uint32_t forbidden[] = { SIG('A', '2', 'B', '0'), SIG('A', '2', 'B', '1'), SIG('A', '2', 'B', '2') };

	const uint32_t colorSpace = readBigEndian32(data + 16);
	const uint32_t* required;
	size_t requiredCount;
	if (colorSpace == SIG('R', 'G', 'B', ' '))
	{
		required = rgbRequired;
		requiredCount = sizeof rgbRequired / sizeof rgbRequired[0];
	}
	else if (colorSpace == SIG('G', 'R', 'A', 'Y'))
	{
		required = grayRequired;
		requiredCount = sizeof grayRequired / sizeof grayRequired[0];
	}
	else
		return false;

	ICCTag tag;
	for (size_t i = 0; i < requiredCount; i++)
		if (!ICCProfileFindTag(data, required[i], &tag))
			return false;
	for (size_t i = 0; i < sizeof forbidden / sizeof forbidden[0]; i++)
		if (ICCProfileFindTag(data, forbidden[i], &tag))
			return false;
	return true;
}

typedef struct SignatureEntry {
	uint32_t signature;
	uint32_t index;
} SignatureEntry;

static int compareSignatureEntries(const void* a, const void* b)
{
	const SignatureEntry* x = a;
	const SignatureEntry* y = b;
	if (x->signature != y->signature)
		return x->signature < y->signature ? -1 : 1;
	return x->index < y->index ? -1 : (x->index > y->index ? 1 : 0);
}

bool ICCProfileFirstTagIndices(const uint8_t* data, uint32_t* outIndices, uint32_t* outCount)
{
	const uint32_t count = ICCProfileTagCount(data);
	*outCount = 0;
	if (count == 0)
		return true;

	// Sorting by (signature, index) puts the first entry of each signature first in its run.
	SignatureEntry* sorted = malloc((size_t) count * sizeof *sorted);
	unsigned char* isFirst = calloc(count, 1);
	if (!sorted || !isFirst)
	{
		free(sorted);
		free(isFirst);
		return false;
	}
	for (uint32_t i = 0; i < count; i++)
	{
		sorted[i].signature = ICCProfileTagAt(data, i).signature;
		sorted[i].index = i;
	}
	qsort(sorted, count, sizeof *sorted, compareSignatureEntries);

	for (uint32_t i = 0; i < count; i++)
		if (i == 0 || sorted[i].signature != sorted[i - 1].signature)
			isFirst[sorted[i].index] = 1;

	for (uint32_t i = 0; i < count; i++)
		if (isFirst[i])
			outIndices[(*outCount)++] = i;

	free(sorted);
	free(isFirst);
	return true;
}

// multiLocalizedUnicodeType, ICC.1:2022, 10.15: 16 bytes of type, count and record size, then records of language (2),
// country (2), string length (4) and string offset from the start of the tag (4).
static bool findMultiLocalizedText(const uint8_t* tag, size_t size, const char* language, const char* country, ICCText* outText)
{
	if (size < 16)
		return false;
	const uint32_t count = readBigEndian32(tag + 8);
	const uint32_t recordSize = readBigEndian32(tag + 12);
	if (count == 0 || recordSize < 12 || (size - 16) / recordSize < count)
		return false;

	// QUIRK: a record for English (United States) is chosen whenever there is one, before the record for the locale's
	// language and country. ICC.1:2022, 10.15 asks for the record of the desired language and country. Evidence:
	// black-box observation of ColorSyncProfileCopyDescriptionString, tier 2, on macOS 26.7.3 (x86_64, locales en_US, en_GB,
	// en_AU, fr_FR, fr_CA, fr_CH, de_DE, de_CH, ja_JP, es_ES, es_US, pt_BR, zh_TW) and 10.14.4 (x86_64 and i386, the same
	// locales): with enUS among the records the result is enUS in every locale. Without it, the locale's language and
	// country, else the first record of the language, else the first record, as before (macOS chooses arbitrarily among
	// several records of a language that is not the locale's, and 10.14.4 i386 differs from x86_64 there).
	// Test: ICCProfileTest.c, "English (United States) record wins".
	const uint8_t* chosen = tag + 16;
	int bestRank = 0;
	for (uint32_t i = 0; i < count; i++)
	{
		const uint8_t* record = tag + 16 + (size_t) i * recordSize;
		const bool sameLanguage = language && record[0] == (uint8_t) language[0] && record[1] == (uint8_t) language[1];
		const bool sameCountry = country && record[2] == (uint8_t) country[0] && record[3] == (uint8_t) country[1];
		const bool englishUS = memcmp(record, "enUS", 4) == 0;
		const int rank = englishUS ? 3 : (sameLanguage ? (sameCountry ? 2 : 1) : 0);
		if (rank > bestRank)
		{
			bestRank = rank;
			chosen = record;
		}
	}

	size_t length = readBigEndian32(chosen + 4);
	const size_t offset = readBigEndian32(chosen + 8);
	if (offset > size || length > size - offset)
		return false;
	length &= ~(size_t) 1;
	while (length >= 2 && tag[offset + length - 2] == 0 && tag[offset + length - 1] == 0)
		length -= 2;
	outText->bytes = tag + offset;
	outText->length = length;
	outText->isUTF16BE = true;
	return true;
}

// textDescriptionType, ICC.1:2001-04, 6.5.17: 8 bytes of type and zero, the count of the ASCII string with its null, then the
// string; then the Unicode language code (4 bytes), the count of UCS-2 characters with their null (4 bytes) and the
// characters (big-endian); the ScriptCode part that follows is not read.
//
// QUIRK: the Unicode string is the description when it has characters, and the ASCII string only when it has none. ICC.1:2022,
// 10.20.3 takes the 7-bit ASCII part. Evidence: black-box observation of ColorSyncProfileCopyDescriptionString, tier 2, on
// macOS 26.7.3 (x86_64) and 10.14.4 (x86_64 and i386): a tag with the ASCII string "Plain" and the Unicode string "Hi" reads
// "Hi", a tag with an empty ASCII string and the Unicode string "Hi" reads "Hi", and one with an empty Unicode part reads the
// ASCII string; the profile version does not matter.
// Test: ICCProfileTest.c, "Unicode part of a textDescriptionType wins"; tests/src/colorsync_profile_description.c.
static bool findPlainText(const uint8_t* tag, size_t size, ICCText* outText)
{
	if (size < 12)
		return false;
	const size_t count = readBigEndian32(tag + 8);
	if (count > size - 12)
		return false;

	const size_t unicodeStart = 12 + count; // the language code, then the count
	if (size - unicodeStart >= 8)
	{
		const size_t characters = readBigEndian32(tag + unicodeStart + 4);
		if (characters > 1 && characters <= (size - unicodeStart - 8) / 2)
		{
			size_t length = characters * 2;
			while (length >= 2 && tag[unicodeStart + 8 + length - 2] == 0 && tag[unicodeStart + 8 + length - 1] == 0)
				length -= 2;
			if (length > 0)
			{
				outText->bytes = tag + unicodeStart + 8;
				outText->length = length;
				outText->isUTF16BE = true;
				return true;
			}
		}
	}

	size_t length = 0;
	while (length < count && tag[12 + length] != 0)
		length++;
	outText->bytes = tag + 12;
	outText->length = length;
	outText->isUTF16BE = false;
	return true;
}

bool ICCProfileFindDescription(const uint8_t* data, const char* language, const char* country, ICCText* outText)
{
	ICCTag descTag;
	if (!ICCProfileFindTag(data, 0x64657363u /* 'desc' */, &descTag) || descTag.size < 4)
		return false;

	const uint8_t* tag = data + descTag.offset;
	const uint32_t type = readBigEndian32(tag);
	if (type == 0x6d6c7563u /* 'mluc' */)
		return findMultiLocalizedText(tag, descTag.size, language, country, outText);
	if (type == 0x64657363u /* 'desc' */)
		return findPlainText(tag, descTag.size, outText);
	return false;
}
