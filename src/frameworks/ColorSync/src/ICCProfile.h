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

// True if the bytes have the 'acsp' file signature at byte 36 and a tag table with every tag's offset and size inside the data and past the table. Nothing else in the
// header or the tag data is checked.
bool ICCProfileValidate(const uint8_t* data, size_t length);

// The remaining functions require data that ICCProfileValidate accepted.
uint32_t ICCProfileTagCount(const uint8_t* data);
ICCTag ICCProfileTagAt(const uint8_t* data, uint32_t index);

// Finds the last tag with the signature.
bool ICCProfileFindTag(const uint8_t* data, uint32_t signature, ICCTag* outTag);

// Writes to outIndices the table index of the first entry of each distinct signature, in table order, and the
// number written to outCount. outIndices has room for ICCProfileTagCount entries. Takes O(n log n) time.
// Returns false if memory runs out.
bool ICCProfileFirstTagIndices(const uint8_t* data, uint32_t* outIndices, uint32_t* outCount);

#endif
