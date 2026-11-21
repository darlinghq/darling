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

// Validates the format of OSProductVersion (e.g. "12.6.8", "26.4").
// 1..31 characters, digits and dots only, no leading/trailing/consecutive dots,
// at most 3 components.
bool prefsd_is_valid_os_product_version(const char* s, size_t len);

// Validates the format of OSVersion (e.g. "21G725", "25E246").
// 1..31 alphanumeric characters only.
bool prefsd_is_valid_os_version(const char* s, size_t len);

// Validates the format of OSRelease (e.g. "21.6.0", "25.4.0").
// 1..31 characters, digits and dots only, 2 or 3 components.
bool prefsd_is_valid_os_release(const char* s, size_t len);

// Validates the optional OSiOSSupportVersion override (e.g. "15.6" or "none").
bool prefsd_is_valid_ios_support_version(const char* s, size_t len);

// Returns "Mac OS X" for 10.x, or "macOS" for 11+.
const char* prefsd_product_name_for_version(const char* product_version);

// Whether SystemVersionCompat.plist should exist (major >= 11).
bool prefsd_should_have_system_version_compat(const char* product_version);

// Computes the iOSSupportVersion for a macOS version string and optional override.
// Returns true if the key should be included in SystemVersion.plist, false if omitted.
bool prefsd_compute_ios_support_version(const char* product_version, const char* override_ios_version, char* out, size_t out_len);

// Maps a macOS product version string to its corresponding compatibility version
// for SystemVersionCompat.plist: "10.16" for major >= 11; or the version unchanged for major < 11.
void prefsd_compute_compat_product_version(const char* product_version, char* out, size_t out_len);

#endif // _PREFSD_VALIDATE_H
