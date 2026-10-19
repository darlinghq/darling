/*
This file is part of Darling.

Copyright (C) 2026 Darling developers

Based on work by lucannez64 (public repository lucannez64/darling-lol,
commit 8f62017), GPL-3.0.

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

#include "LSHostOpenURL.h"
#include <string.h>
#include <strings.h>

static const char* const supported_schemes[] = { "http:", "https:", "mailto:" };


int _LSHostOpenSchemeSupported(const char* url)
{
	if (url == NULL)
		return 0;
	for (size_t i = 0; i < sizeof(supported_schemes) / sizeof(supported_schemes[0]); i++)
	{
		size_t len = strlen(supported_schemes[i]);
		if (strncasecmp(url, supported_schemes[i], len) == 0)
			return 1;
	}
	return 0;
}

/* RFC 3986 reserved and unreserved characters except '$' and the single quote, which a shell
   script such as xdg-open may interpret; '%' is checked separately. */
static int url_char_allowed(unsigned char c)
{
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
		return 1;
	return c != '\0' && strchr("-._~:/?#[]@!&()*+,;=", c) != NULL;
}

static int hex_digit(unsigned char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

/*
 * Compares the percent-decoded query parameter name starting at "name" (ending at
 * '=', '&', ';' or the end of the string) with a lowercase ASCII word.
 */
static int query_name_is(const char* name, const char* word)
{
	size_t w = 0;

	while (*name != '\0' && *name != '=' && *name != '&' && *name != ';')
	{
		int c = (unsigned char) *name;

		if (c == '%' && hex_digit((unsigned char) name[1]) >= 0 && hex_digit((unsigned char) name[2]) >= 0)
		{
			c = hex_digit((unsigned char) name[1]) * 16 + hex_digit((unsigned char) name[2]);
			name += 2;
		}
		if (c >= 'A' && c <= 'Z')
			c += 'a' - 'A';
		if (word[w] == '\0' || c != (unsigned char) word[w])
			return 0;
		w++;
		name++;
	}
	return word[w] == '\0';
}

/* Several mail clients attach the file named by a mailto: "attach" parameter to the draft. */
static int mailto_attaches_file(const char* url)
{
	const char* query = strchr(url, '?');

	while (query != NULL)
	{
		const char* name = query + 1;

		if (query_name_is(name, "attach") || query_name_is(name, "attachment"))
			return 1;
		query = strpbrk(name, "&;");
	}
	return 0;
}

int _LSHostOpenValidateURL(const char* url)
{
	size_t len;
	int is_mailto;

	if (!_LSHostOpenSchemeSupported(url))
		return -LS_HOST_LINUX_EINVAL;

	len = strlen(url);
	if (len > LS_HOST_OPEN_MAX_URL_LENGTH)
		return -LS_HOST_LINUX_EINVAL;

	/* Only characters that can appear in an RFC 3986 URL; this rejects whitespace, control and
	   non-ASCII bytes, quotes, angle brackets, backslash, caret, backtick, braces and bar. */
	for (size_t i = 0; i < len; i++)
	{
		unsigned char c = (unsigned char) url[i];

		if (c == '%')
		{
			if (hex_digit((unsigned char) url[i + 1]) < 0 || hex_digit((unsigned char) url[i + 2]) < 0)
				return -LS_HOST_LINUX_EINVAL;
			i += 2;
		}
		else if (!url_char_allowed(c))
			return -LS_HOST_LINUX_EINVAL;
	}

	is_mailto = strncasecmp(url, "mailto:", 7) == 0;
	if (!is_mailto)
	{
		/* http: and https: need an authority; "http:foo" is not a web link */
		const char* after = url + (strncasecmp(url, "https:", 6) == 0 ? 6 : 5);

		if (strncmp(after, "//", 2) != 0 || after[2] == '\0' || after[2] == '/')
			return -LS_HOST_LINUX_EINVAL;
	}
	else if (mailto_attaches_file(url))
		return -LS_HOST_LINUX_EINVAL;
	return 0;
}
