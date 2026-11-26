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

// Build-machine tool: writes the ICC profile of one of the named profiles of ICCBuilder.c to a file, so that the profile files
// of the ColorSync profile folder hold the same bytes as ColorSyncProfileCreateWithName() returns.
//   genprofile genericrgb|genericgray <output file>

#include "ICCBuilder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "usage: %s genericrgb|genericgray <output file>\n", argv[0]);
		return 2;
	}

	ICCNamedProfile which;
	if (strcmp(argv[1], "genericrgb") == 0)
		which = ICCNamedGenericRGB;
	else if (strcmp(argv[1], "genericgray") == 0)
		which = ICCNamedGenericGray;
	else
	{
		fprintf(stderr, "%s: unknown profile %s\n", argv[0], argv[1]);
		return 2;
	}

	const ICCDisplayProfileSpec spec = ICCNamedProfileSpec(which);
	size_t length;
	uint8_t* bytes = ICCBuildDisplayProfile(&spec, &length);
	if (!bytes)
	{
		fprintf(stderr, "%s: cannot build the profile\n", argv[0]);
		return 1;
	}

	FILE* file = fopen(argv[2], "wb");
	const int ok = file && fwrite(bytes, 1, length, file) == length && fclose(file) == 0;
	free(bytes);
	if (!ok)
	{
		fprintf(stderr, "%s: cannot write %s\n", argv[0], argv[2]);
		return 1;
	}
	return 0;
}
