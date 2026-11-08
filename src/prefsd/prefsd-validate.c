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
