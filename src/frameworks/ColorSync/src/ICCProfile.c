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
#include <stdlib.h>

#define ICC_FILE_SIGNATURE_OFFSET 36
#define ICC_FILE_SIGNATURE 0x61637370u // 'acsp'
#define ICC_TAG_ENTRY_SIZE 12

static uint32_t readBigEndian32(const uint8_t* p)
{
	return ((uint32_t) p[0] << 24) | ((uint32_t) p[1] << 16) | ((uint32_t) p[2] << 8) | p[3];
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
