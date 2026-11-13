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

/*
 * iconset2icns: packs the PNG images of an .iconset folder into an .icns file,
 * as `iconutil -c icns` does on macOS.
 *
 * Usage: iconset2icns <folder.iconset> <output.icns>
 *
 * An .icns file is a big-endian 'icns' header (type, total length) followed by
 * elements of the same shape (type, length including the 8-byte header, data).
 * Each PNG found in the folder under its iconset name becomes the element type
 * for its pixel size; sizes that are missing are skipped.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static const struct
{
	const char* name;
	const char type[4];
} elements[] = {
	{ "icon_16x16.png", "icp4" },
	{ "icon_16x16@2x.png", "ic11" },
	{ "icon_32x32.png", "icp5" },
	{ "icon_32x32@2x.png", "ic12" },
	{ "icon_128x128.png", "ic07" },
	{ "icon_128x128@2x.png", "ic13" },
	{ "icon_256x256.png", "ic08" },
	{ "icon_256x256@2x.png", "ic14" },
	{ "icon_512x512.png", "ic09" },
	{ "icon_512x512@2x.png", "ic10" },
};

static void put_be32(unsigned char* p, uint32_t v)
{
	p[0] = v >> 24;
	p[1] = v >> 16;
	p[2] = v >> 8;
	p[3] = v;
}

static unsigned char* read_file(const char* path, size_t* length)
{
	FILE* f = fopen(path, "rb");
	if (!f)
		return NULL;

	unsigned char* data = NULL;
	if (fseek(f, 0, SEEK_END) == 0)
	{
		long size = ftell(f);
		if (size > 0 && fseek(f, 0, SEEK_SET) == 0)
		{
			data = malloc(size);
			if (data && fread(data, 1, size, f) != (size_t) size)
			{
				free(data);
				data = NULL;
			}
			*length = size;
		}
	}
	fclose(f);
	return data;
}

int main(int argc, const char** argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s <folder.iconset> <output.icns>\n", argv[0]);
		return 1;
	}

	unsigned char* out = malloc(8);
	size_t total = 8;
	int found = 0;

	for (size_t i = 0; i < sizeof(elements) / sizeof(elements[0]); i++)
	{
		char path[4096];
		size_t length;

		snprintf(path, sizeof(path), "%s/%s", argv[1], elements[i].name);
		unsigned char* png = read_file(path, &length);
		if (!png)
			continue;

		out = realloc(out, total + 8 + length);
		memcpy(out + total, elements[i].type, 4);
		put_be32(out + total + 4, 8 + length);
		memcpy(out + total + 8, png, length);
		total += 8 + length;
		found++;
		free(png);
	}

	if (!found)
	{
		fprintf(stderr, "%s: no icon images in %s\n", argv[0], argv[1]);
		return 1;
	}

	memcpy(out, "icns", 4);
	put_be32(out + 4, total);

	FILE* f = fopen(argv[2], "wb");
	if (!f || fwrite(out, 1, total, f) != total || fclose(f) != 0)
	{
		perror(argv[2]);
		return 1;
	}

	free(out);
	return 0;
}
