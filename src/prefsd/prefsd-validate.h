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

#ifndef _PREFSD_VALIDATE_H
#define _PREFSD_VALIDATE_H

// Checks darling-prefsd applies to the system preferences files and their
// values before anything is pushed to darlingserver. Plain C with no
// CoreFoundation, so that they can be tested on the host.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/stat.h>

// Largest preferences file darling-prefsd reads. Both domains hold a handful
// of short values.
#define PREFSD_FILE_SIZE_MAX (64 * 1024)

enum prefsd_file_verdict {
	PREFSD_FILE_OK = 0,
	PREFSD_FILE_NOT_REGULAR,
	PREFSD_FILE_NOT_ROOT_OWNED,
	PREFSD_FILE_WRITABLE_BY_OTHERS,
	PREFSD_FILE_TOO_LARGE,
};

// Whether a preferences file with these attributes may be used: a regular
// file owned by root that neither its group nor others can write, and no
// larger than PREFSD_FILE_SIZE_MAX.
enum prefsd_file_verdict prefsd_check_file(const struct stat* st);

const char* prefsd_file_verdict_string(enum prefsd_file_verdict verdict);

// Whether the `len` bytes at `s` are a value for a string field of
// `field_size` bytes (including the terminating NUL): not empty, shorter than
// the field, and printable ASCII only.
bool prefsd_is_printable_ascii(const char* s, size_t len, size_t field_size);

// Parses a UUID in its canonical text form (8-4-4-4-12 hexadecimal digits,
// either case) into 16 bytes. Rejects anything else, including surrounding
// whitespace and braces.
bool prefsd_parse_uuid(const char* s, size_t len, uint8_t out[16]);

bool prefsd_is_all_zero(const uint8_t* bytes, size_t len);

// Whether the `len` bytes at `s` are an absolute path that fits a field of
// `field_size` bytes (including the terminating NUL) and contain no NUL.
bool prefsd_is_absolute_path(const char* s, size_t len, size_t field_size);

enum prefsd_code_signing_policy {
	PREFSD_CODE_SIGNING_OFF = 0,
	PREFSD_CODE_SIGNING_WARN,
	PREFSD_CODE_SIGNING_ENFORCE,
};

// Parses the CodeSigningPolicy value: exactly "off", "warn" or "enforce".
bool prefsd_parse_code_signing_policy(const char* s, size_t len, enum prefsd_code_signing_policy* out);

#endif // _PREFSD_VALIDATE_H
