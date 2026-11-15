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

#include "prefsd-validate.h"

#include <stdio.h>
#include <string.h>

enum prefsd_file_verdict prefsd_check_file(const struct stat* st)
{
	if (!S_ISREG(st->st_mode))
		return PREFSD_FILE_NOT_REGULAR;
	if (st->st_uid != 0)
		return PREFSD_FILE_NOT_ROOT_OWNED;
	if (st->st_mode & (S_IWGRP | S_IWOTH))
		return PREFSD_FILE_WRITABLE_BY_OTHERS;
	if (st->st_size < 0 || st->st_size > PREFSD_FILE_SIZE_MAX)
		return PREFSD_FILE_TOO_LARGE;
	return PREFSD_FILE_OK;
}

const char* prefsd_file_verdict_string(enum prefsd_file_verdict verdict)
{
	switch (verdict)
	{
		case PREFSD_FILE_OK:
			return "usable";
		case PREFSD_FILE_NOT_REGULAR:
			return "not a regular file";
		case PREFSD_FILE_NOT_ROOT_OWNED:
			return "not owned by root";
		case PREFSD_FILE_WRITABLE_BY_OTHERS:
			return "writable by its group or by others";
		case PREFSD_FILE_TOO_LARGE:
			return "too large";
	}
	return "unusable";
}

bool prefsd_is_printable_ascii(const char* s, size_t len, size_t field_size)
{
	if (len == 0 || len >= field_size)
		return false;
	for (size_t i = 0; i < len; i++)
	{
		unsigned char c = (unsigned char) s[i];
		if (c < 0x20 || c > 0x7e)
			return false;
	}
	return true;
}

static int hex_value(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

bool prefsd_parse_uuid(const char* s, size_t len, uint8_t out[16])
{
	uint8_t bytes[16];
	size_t byte = 0;

	if (len != 36)
		return false;

	for (size_t i = 0; i < len; )
	{
		if (i == 8 || i == 13 || i == 18 || i == 23)
		{
			if (s[i] != '-')
				return false;
			i++;
			continue;
		}

		int high = hex_value(s[i]);
		int low = hex_value(s[i + 1]);
		if (high < 0 || low < 0)
			return false;
		bytes[byte++] = (uint8_t) ((high << 4) | low);
		i += 2;
	}

	memcpy(out, bytes, sizeof(bytes));
	return true;
}

bool prefsd_is_all_zero(const uint8_t* bytes, size_t len)
{
	for (size_t i = 0; i < len; i++)
	{
		if (bytes[i] != 0)
			return false;
	}
	return true;
}

bool prefsd_is_absolute_path(const char* s, size_t len, size_t field_size)
{
	if (len == 0 || len >= field_size || s[0] != '/')
		return false;
	return memchr(s, '\0', len) == NULL;
}

bool prefsd_parse_code_signing_policy(const char* s, size_t len, enum prefsd_code_signing_policy* out)
{
	static const struct {
		const char* name;
		enum prefsd_code_signing_policy policy;
	} names[] = {
		{ "off", PREFSD_CODE_SIGNING_OFF },
		{ "warn", PREFSD_CODE_SIGNING_WARN },
		{ "enforce", PREFSD_CODE_SIGNING_ENFORCE },
	};

	for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++)
	{
		if (strlen(names[i].name) == len && memcmp(names[i].name, s, len) == 0)
		{
			*out = names[i].policy;
			return true;
		}
	}
	return false;
}

bool prefsd_is_valid_os_product_version(const char* s, size_t len)
{
	if (s == NULL || len == 0 || len >= 32)
		return false;
	if (s[0] == '.' || s[len - 1] == '.')
		return false;

	int dot_count = 0;
	for (size_t i = 0; i < len; i++)
	{
		char c = s[i];
		if (c == '.')
		{
			dot_count++;
			if (i + 1 < len && s[i + 1] == '.')
				return false;
		}
		else if (c < '0' || c > '9')
		{
			return false;
		}
	}
	if (dot_count > 2)
		return false;

	int major = 0;
	if (sscanf(s, "%d", &major) < 1 || major < 0)
		return false;

	return true;
}

bool prefsd_is_valid_os_version(const char* s, size_t len)
{
	if (s == NULL || len == 0 || len >= 32)
		return false;

	bool has_alnum = false;
	for (size_t i = 0; i < len; i++)
	{
		char c = s[i];
		if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
			return false;
		has_alnum = true;
	}
	return has_alnum;
}

bool prefsd_is_valid_os_release(const char* s, size_t len)
{
	if (s == NULL || len == 0 || len >= 32)
		return false;
	if (s[0] == '.' || s[len - 1] == '.')
		return false;

	int dot_count = 0;
	for (size_t i = 0; i < len; i++)
	{
		char c = s[i];
		if (c == '.')
		{
			dot_count++;
			if (i + 1 < len && s[i + 1] == '.')
				return false;
		}
		else if (c < '0' || c > '9')
		{
			return false;
		}
	}
	if (dot_count < 1 || dot_count > 2)
		return false;

	int major = 0;
	if (sscanf(s, "%d", &major) < 1 || major < 0)
		return false;

	return true;
}

bool prefsd_is_valid_ios_support_version(const char* s, size_t len)
{
	if (s == NULL || len == 0 || len >= 32)
		return false;
	if (len == 4 && memcmp(s, "none", 4) == 0)
		return true;
	return prefsd_is_valid_os_product_version(s, len);
}

const char* prefsd_product_name_for_version(const char* product_version)
{
	if (product_version == NULL)
		return "macOS";
	int major = 0;
	if (sscanf(product_version, "%d", &major) == 1 && major < 11)
		return "Mac OS X";
	return "macOS";
}

bool prefsd_should_have_system_version_compat(const char* product_version)
{
	if (product_version == NULL)
		return false;
	int major = 0;
	return sscanf(product_version, "%d", &major) == 1 && major >= 11;
}

bool prefsd_compute_ios_support_version(const char* product_version, const char* override_ios_version, char* out, size_t out_len)
{
	if (out_len == 0)
		return false;
	out[0] = '\0';

	if (override_ios_version != NULL && override_ios_version[0] != '\0')
	{
		if (strcmp(override_ios_version, "none") == 0)
			return false;
		snprintf(out, out_len, "%s", override_ios_version);
		return true;
	}

	if (product_version == NULL || product_version[0] == '\0')
		return false;

	int major = 0, minor = 0, patch = 0;
	int parts = sscanf(product_version, "%d.%d.%d", &major, &minor, &patch);
	if (parts < 1)
		return false;

	if (major < 10)
		return false;

	if (major == 10)
	{
		if (minor < 14)
			return false;
		if (minor == 14)
		{
			snprintf(out, out_len, "12.2");
			return true;
		}
		if (minor == 15)
		{
			if (parts >= 3)
			{
				if (patch < 4)
					snprintf(out, out_len, "13.%d", patch);
				else
					snprintf(out, out_len, "13.4");
			}
			else
			{
				snprintf(out, out_len, "13.0");
			}
			return true;
		}
		snprintf(out, out_len, "13.4");
		return true;
	}

	if (major >= 11 && major < 26)
	{
		int ios_major = major + 3;
		if (parts >= 2)
			snprintf(out, out_len, "%d.%d", ios_major, minor);
		else
			snprintf(out, out_len, "%d.0", ios_major);
		return true;
	}

	// major >= 26
	if (parts >= 2)
		snprintf(out, out_len, "%d.%d", major, minor);
	else
		snprintf(out, out_len, "%d.0", major);
	return true;
}

void prefsd_compute_compat_product_version(const char* product_version, char* out, size_t out_len)
{
	if (out_len == 0)
		return;
	out[0] = '\0';
	if (product_version == NULL || product_version[0] == '\0')
		return;

	int major = 0;
	if (sscanf(product_version, "%d", &major) == 1 && major >= 11)
		snprintf(out, out_len, "10.16");
	else
		snprintf(out, out_len, "%s", product_version);
}
