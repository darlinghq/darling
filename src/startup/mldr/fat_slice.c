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

#include "fat_slice.h"

static uint32_t rd32(const uint8_t* p, int big)
{
	if (big)
		return ((uint32_t) p[0] << 24) | ((uint32_t) p[1] << 16) | ((uint32_t) p[2] << 8) | p[3];
	return ((uint32_t) p[3] << 24) | ((uint32_t) p[2] << 16) | ((uint32_t) p[1] << 8) | p[0];
}

static uint64_t rd64(const uint8_t* p, int big)
{
	if (big)
		return ((uint64_t) rd32(p, 1) << 32) | rd32(p + 4, 1);
	return ((uint64_t) rd32(p + 4, 0) << 32) | rd32(p, 0);
}

// Byte order and record width from the magic; 0 if it is not a universal magic.
static int classify(const uint8_t* p, int* big, int* wide)
{
	switch (rd32(p, 1))
	{
		case 0xcafebabeu: *big = 1; *wide = 0; return 1;
		case 0xbebafecau: *big = 0; *wide = 0; return 1;
		case 0xcafebabfu: *big = 1; *wide = 1; return 1;
		case 0xbfbafecau: *big = 0; *wide = 1; return 1;
		default: return 0;
	}
}

int mldr_fat_is_magic(const uint8_t* bytes4)
{
	int big, wide;
	return classify(bytes4, &big, &wide);
}

enum mldr_fat_result mldr_fat_select(const uint8_t* data, size_t len, uint64_t file_size,
		const uint32_t prefs[4], uint32_t forced, uint32_t native, uint32_t fallback,
		struct mldr_fat_slice* out)
{
	int big, wide;

	if (data == NULL || out == NULL || len < 4 || !classify(data, &big, &wide))
		return MLDR_FAT_NOT_FAT;
	if (len < 8 || file_size < 8)
		return MLDR_FAT_MALFORMED;

	const uint32_t count = rd32(data + 4, big);
	const unsigned width = wide ? 32 : 20;

	// the table has to be inside the file and inside what the caller gave us
	if (count > MLDR_FAT_MAX_ARCHES || (uint64_t) 8 + (uint64_t) count * width > file_size ||
			(uint64_t) 8 + (uint64_t) count * width > len)
		return MLDR_FAT_MALFORMED;

	const uint64_t table_end = 8 + (uint64_t) count * width;
	struct mldr_fat_slice pref_best = {0}, native_best = {0}, fallback_best = {0}, forced_best = {0};
	int have_pref = 0, have_native = 0, have_fallback = 0, have_forced = 0;
	unsigned pref_index = 4;

	for (uint32_t i = 0; i < count; i++)
	{
		const uint8_t* row = data + 8 + (size_t) i * width;
		struct mldr_fat_slice s;

		s.cputype = rd32(row, big);
		s.cpusubtype = rd32(row + 4, big);
		s.offset = wide ? rd64(row + 8, big) : rd32(row + 8, big);
		s.size = wide ? rd64(row + 16, big) : rd32(row + 12, big);

		// the slice must be nonempty, past the table, and inside the file
		if (s.size == 0 || s.offset < table_end || s.offset > file_size || s.size > file_size - s.offset)
			continue;

		if (forced)
		{
			if (s.cputype == forced)
			{
				forced_best = s;
				have_forced = 1;
			}
			continue;
		}

		for (unsigned j = 0; j < 4; j++)
		{
			if (prefs != NULL && prefs[j] != 0 && s.cputype == prefs[j])
			{
				if (j < pref_index)
				{
					pref_best = s;
					pref_index = j;
					have_pref = 1;
				}
				break;
			}
		}

		if (native != 0 && s.cputype == native)
		{
			native_best = s;
			have_native = 1;
		}
		else if (fallback != 0 && s.cputype == fallback && !have_fallback)
		{
			fallback_best = s;
			have_fallback = 1;
		}
	}

	if (forced)
	{
		if (!have_forced)
			return MLDR_FAT_NO_MATCH;
		*out = forced_best;
	}
	else if (have_pref)
		*out = pref_best;
	else if (have_native)
		*out = native_best;
	else if (have_fallback)
		*out = fallback_best;
	else
		return MLDR_FAT_NO_MATCH;

	return MLDR_FAT_OK;
}
