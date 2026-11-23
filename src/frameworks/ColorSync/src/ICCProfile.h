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

#ifndef _ICC_PROFILE_H_
#define _ICC_PROFILE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Read-only access to the layout of an ICC profile (ICC.1:2022, 7.2 and 7.3,
// https://www.color.org/specification/ICC.1-2022-05.pdf): a 128-byte header, a 4-byte tag count
// and 12-byte tag table entries (signature, offset, size), all big-endian.
// These functions know nothing about CoreFoundation.

#define ICC_HEADER_SIZE 128

typedef struct ICCTag {
	uint32_t signature;
	uint32_t offset;
	uint32_t size;
} ICCTag;

// Converts the 128-byte header between the layout of the file (all numbers big-endian, ICC.1:2022, 7.2) and one in
// host byte order, in either direction (the conversion is its own inverse; on a big-endian host it copies). Each
// field is swapped as the number it is: the size, the CMM, version, class, colour space and PCS words, the file
// signature, platform, flags, manufacturer and model words, the rendering intent, the three 32-bit numbers of the
// illuminant and the creator word as 32-bit numbers, so a signature such as 'RGB ' is the number 0x52474220 in
// host order; the six numbers of the date and time as 16-bit numbers; the device attributes (bytes 56 to 63) as two
// 32-bit numbers, each on its own (CM2Header declares `UInt32 deviceAttributes[2]`, see ColorSyncProfile.c). The Profile ID (84 to 99) and the reserved bytes (100 to 127) are bytes and stay as they are.
// `in` and `out` hold 128 bytes each and must not overlap.
void ICCHeaderSwapFields(const uint8_t in[ICC_HEADER_SIZE], uint8_t out[ICC_HEADER_SIZE]);

// True if the bytes have the 'acsp' file signature at byte 36 and a tag table with every tag's offset and size inside the data and past the table. Nothing else in the
// header or the tag data is checked.
bool ICCProfileValidate(const uint8_t* data, size_t length);

// Writes to outDigest the Profile ID of ICC.1:2022, 7.2.18: the MD5 digest of the `length` bytes of the data (the size
// field is not used, see the QUIRK in ICCProfile.c), with the profile flags (bytes 44 to 47), the rendering intent (64 to 67)
// and the Profile ID field (84 to 99) taken as zero. Returns false, and writes nothing, if `length` is below 100.
// Requires data that ICCProfileValidate accepted.
bool ICCProfileComputeID(const uint8_t* data, size_t length, uint8_t outDigest[16]);

// The remaining functions require data that ICCProfileValidate accepted.
uint32_t ICCProfileTagCount(const uint8_t* data);
ICCTag ICCProfileTagAt(const uint8_t* data, uint32_t index);

// Finds the last tag with the signature.
bool ICCProfileFindTag(const uint8_t* data, uint32_t signature, ICCTag* outTag);

// True for an 'RGB ' profile with the six tags of a three-component matrix-based profile (ICC.1:2022, 8.3.3 and 8.4.3):
// the red, green and blue matrix columns (rXYZ, gXYZ, bXYZ) and tone reproduction curves (rTRC, gTRC, bTRC), and for a
// 'GRAY' profile with kTRC, in both cases only if there is no A2B0, A2B1 or A2B2 table (see the QUIRK in ICCProfile.c).
// Requires data that ICCProfileValidate accepted.
bool ICCProfileHasMatrixTRC(const uint8_t* data);

// Writes to outIndices the table index of the first entry of each distinct signature, in table order, and the
// number written to outCount. outIndices has room for ICCProfileTagCount entries. Takes O(n log n) time.
// Returns false if memory runs out.
bool ICCProfileFirstTagIndices(const uint8_t* data, uint32_t* outIndices, uint32_t* outCount);

// The profile description (the 'desc' tag) as characters still in the profile's own encoding.
typedef struct ICCText {
	const uint8_t* bytes; // inside the profile data
	size_t length; // in bytes, without the terminating null (a null character, if present, is not counted)
	bool isUTF16BE; // otherwise one byte per character, in ISO Latin 1 (ICC.1:2001-04 says 7-bit ASCII)
} ICCText;

// Finds the description in a multiLocalizedUnicodeType tag (ICC.1:2022, 9.2.43 and 10.15) or in the textDescriptionType
// tag of older profiles (ICC.1:2001-04, 6.5.17). language and country are the two letters of ISO 639-1 and ISO 3166-1
// of the wanted locale, or NULL. For a multiLocalizedUnicodeType the record for English (United States) is used if there
// is one (see the QUIRK in ICCProfile.c), else the record of the same language and country, else the first of the same
// language, else the first record (10.15); a textDescriptionType has one string, its Unicode part when that has
// characters, else its ASCII part (see the QUIRK in ICCProfile.c).
// Returns false if the profile has no description, of a type that is not understood, or one that runs past its tag.
// Requires data that ICCProfileValidate accepted.
bool ICCProfileFindDescription(const uint8_t* data, const char* language, const char* country, ICCText* outText);

#endif
