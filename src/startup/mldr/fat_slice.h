/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

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

#ifndef _MLDR_FAT_SLICE_H_
#define _MLDR_FAT_SLICE_H_

#include <stdint.h>
#include <stddef.h>

// A universal ("fat") file with 32-bit (FAT_MAGIC) or 64-bit (FAT_MAGIC_64)
// architecture records, in either byte order.
#define MLDR_FAT_MAX_ARCHES 64
// The most bytes of the file mldr_fat_select() ever looks at: the header and
// the largest architecture table.
#define MLDR_FAT_MAX_HEADER (8 + MLDR_FAT_MAX_ARCHES * 32)

struct mldr_fat_slice
{
	uint64_t offset;
	uint64_t size;
	uint32_t cputype;
	uint32_t cpusubtype;
};

enum mldr_fat_result
{
	MLDR_FAT_OK = 0,
	MLDR_FAT_NOT_FAT,     // the magic is not a universal file's
	MLDR_FAT_MALFORMED,   // header or architecture table does not fit the file, or too many architectures
	MLDR_FAT_NO_MATCH,    // no usable slice for the requested/native architectures
};

/**
 * Whether the first four bytes of a file are the magic of a universal file
 * (any of the four spellings).
 */
int mldr_fat_is_magic(const uint8_t* bytes4);

/**
 * Picks the slice to run from a universal file. `data` holds the first `len`
 * bytes of the file (at most MLDR_FAT_MAX_HEADER are used), `file_size` is its
 * real size. Every architecture record is bounds-checked against the file; a
 * record whose slice is empty or lies (even partly) outside the file is never
 * selected. Nothing is allocated and `out` is only written on success.
 *
 * Selection: with `forced` nonzero the last slice of that cputype; otherwise
 * the slice matching the earliest nonzero entry of `prefs` (the first one among
 * equals); otherwise the last slice of `native`, else the first slice of
 * `fallback` (nonzero only where the host can run a second ABI).
 */
enum mldr_fat_result mldr_fat_select(const uint8_t* data, size_t len, uint64_t file_size,
		const uint32_t prefs[4], uint32_t forced, uint32_t native, uint32_t fallback,
		struct mldr_fat_slice* out);

#endif
