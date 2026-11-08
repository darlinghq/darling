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
 * adhoc-sign-check, the host test of src/buildtools/adhoc-sign: checks with darlingserver's
 * code signature parser that every image in the given files (a thin Mach-O, or each slice of
 * a universal file) carries a valid ad-hoc signature that darlingserver accepts under the `enforce` policy: the signature parses, it is
 * ad-hoc (no CMS blob), every code page matches its hash, and the image is within the sizes
 * darlingserver verifies.
 *
 * Usage: adhoc-sign-check [--images-only] [--expect-identifier=<id>] <file>...
 *
 * With --images-only, files that are not Mach-O images (scripts, data, object files, archives)
 * are skipped instead of reported.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mach-o/fat.h>
#include <mach-o/loader.h>

#include <darlingserver/dc_codesign.h>
#include <kern/cs_blobs.h>

/* the largest image whose code pages darlingserver verifies (see its code-signing.hpp) */
#define MAX_VERIFIED_SIZE (256ull * 1024 * 1024)

static uint32_t get_be32(const uint8_t* p) {
	return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}

static uint64_t get_be64(const uint8_t* p) {
	return ((uint64_t)get_be32(p) << 32) | get_be32(p + 4);
}

static uint8_t* read_file(const char* path, size_t* length) {
	FILE* file = fopen(path, "rb");
	uint8_t* data = NULL;
	long size;

	if (!file) {
		return NULL;
	}
	if (fseek(file, 0, SEEK_END) == 0 && (size = ftell(file)) >= 0 && fseek(file, 0, SEEK_SET) == 0) {
		data = malloc(size ? (size_t)size : 1);
		if (data && fread(data, 1, (size_t)size, file) != (size_t)size) {
			free(data);
			data = NULL;
		}
		*length = (size_t)size;
	}
	fclose(file);
	return data;
}

static bool check_image(const char* path, uint64_t offset, const uint8_t* file, size_t file_length, const char* expected_identifier) {
	struct dc_cs_image image;
	char err[256] = "";
	uint64_t size = 0;
	uint32_t bad_page = 0;

	if (dc_cs_image_at(file, file_length, file_length, offset, &size, err, sizeof(err)) != 0) {
		fprintf(stderr, "%s @%llu: %s\n", path, (unsigned long long)offset, err);
		return false;
	}

	const uint8_t* data = file + offset;
	int parsed = dc_cs_parse(data, (size_t)size, &image, err, sizeof(err));
	if (parsed == DC_CS_UNSIGNED) {
		fprintf(stderr, "%s @%llu: not signed\n", path, (unsigned long long)offset);
		return false;
	} else if (parsed != 0) {
		fprintf(stderr, "%s @%llu: %s\n", path, (unsigned long long)offset, err);
		return false;
	}

	if (image.cms_length != 0 || !(image.cd_flags & CS_ADHOC)) {
		fprintf(stderr, "%s @%llu: the signature is not ad-hoc\n", path, (unsigned long long)offset);
		return false;
	}
	if (image.hash_type != CS_HASHTYPE_SHA256) {
		fprintf(stderr, "%s @%llu: hash type %u instead of SHA-256\n", path, (unsigned long long)offset, image.hash_type);
		return false;
	}
	if (image.code_limit > MAX_VERIFIED_SIZE) {
		fprintf(stderr, "%s @%llu: too large for darlingserver to verify\n", path, (unsigned long long)offset);
		return false;
	}
	if (dc_cs_verify_pages(&image, data, (size_t)size, &bad_page, err, sizeof(err)) != 0) {
		fprintf(stderr, "%s @%llu: code page %u: %s\n", path, (unsigned long long)offset, bad_page, err);
		return false;
	}
	if (expected_identifier && strcmp(image.identifier, expected_identifier) != 0) {
		fprintf(stderr, "%s @%llu: identifier \"%s\" instead of \"%s\"\n", path, (unsigned long long)offset, image.identifier, expected_identifier);
		return false;
	}

	printf("%s @%llu: ad-hoc, %u code pages of %u bytes, identifier %s\n", path, (unsigned long long)offset,
		image.code_slot_count, 1u << image.page_size_log2, image.identifier);
	return true;
}

/* whether a thin file is a Mach-O image that adhoc-sign signs */
static bool is_image(const uint8_t* data, size_t length) {
	uint32_t magic, filetype;

	if (length < 16) {
		return false;
	}
	memcpy(&magic, data, sizeof(magic));
	memcpy(&filetype, data + 12, sizeof(filetype));
	return (magic == MH_MAGIC || magic == MH_MAGIC_64) &&
		(filetype == MH_EXECUTE || filetype == MH_DYLIB || filetype == MH_DYLINKER || filetype == MH_BUNDLE);
}

static bool check_file(const char* path, bool images_only, const char* expected_identifier) {
	size_t length = 0;
	uint8_t* data = read_file(path, &length);
	bool ok = true;

	if (!data) {
		fprintf(stderr, "%s: cannot read the file\n", path);
		return false;
	}
	uint32_t magic = length >= 8 ? get_be32(data) : 0;
	uint32_t count = length >= 8 ? get_be32(data + 4) : 0;
	if ((magic == FAT_MAGIC || magic == FAT_MAGIC_64) && count > 0 && count <= DC_CS_FAT_MAX_ARCHS) {
		bool is_64 = (magic == FAT_MAGIC_64);
		size_t entry_size = is_64 ? 32 : 20;
		unsigned images = 0;

		if (8 + count * entry_size > length) {
			fprintf(stderr, "%s: bad universal header\n", path);
			ok = false;
		}
		for (uint32_t i = 0; ok && i < count; ++i) {
			const uint8_t* entry = data + 8 + i * entry_size;
			uint64_t offset = is_64 ? get_be64(entry + 8) : get_be32(entry + 8);
			uint64_t size = is_64 ? get_be64(entry + 16) : get_be32(entry + 12);
			if (offset > length || size > length - offset || !is_image(data + offset, (size_t)size)) {
				continue;
			}
			++images;
			ok = check_image(path, offset, data, length, expected_identifier);
		}
		if (ok && images == 0 && !images_only) {
			fprintf(stderr, "%s: no slice is a Mach-O image\n", path);
			ok = false;
		}
	} else if (is_image(data, length)) {
		ok = check_image(path, 0, data, length, expected_identifier);
	} else if (!images_only) {
		fprintf(stderr, "%s: not a Mach-O image\n", path);
		ok = false;
	}

	free(data);
	return ok;
}

int main(int argc, char** argv) {
	const char* expected_identifier = NULL;
	bool images_only = false;
	int first = 1;
	int failures = 0;

	for (; first < argc && strncmp(argv[first], "--", 2) == 0; ++first) {
		if (strncmp(argv[first], "--expect-identifier=", 20) == 0) {
			expected_identifier = argv[first] + 20;
		} else if (strcmp(argv[first], "--images-only") == 0) {
			images_only = true;
		} else {
			first = argc;
		}
	}
	if (first >= argc) {
		fprintf(stderr, "usage: %s [--images-only] [--expect-identifier=<id>] <file>...\n", argv[0]);
		return 2;
	}

	for (int i = first; i < argc; ++i) {
		if (!check_file(argv[i], images_only, expected_identifier)) {
			++failures;
		}
	}

	return failures ? 1 : 0;
}
