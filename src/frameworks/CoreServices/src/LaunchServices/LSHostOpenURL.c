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
 * Several mail clients attach the file named by a mailto: "attach" parameter to the draft.
 * A client may decode the URL before it splits the query, so the query is decoded once and
 * then split. A literal "%25" would let a second decoding round produce delimiters or letters
 * that this check never saw ("%2561ttach", "%2526attach"), so any mailto URL containing it
 * is refused; so is a decoded NUL.
 */
static int mailto_attaches_file(const char* url)
{
	char decoded[LS_HOST_OPEN_MAX_URL_LENGTH + 1];
	const char* query = strchr(url, '?');
	size_t n = 0;

	if (query == NULL)
		return 0;
	if (strstr(query, "%25") != NULL)
		return 1;

	for (query++; *query != '\0'; query++)
	{
		int c = (unsigned char) *query;

		if (c == '%')
		{
			c = hex_digit((unsigned char) query[1]) * 16 + hex_digit((unsigned char) query[2]);
			query += 2;
			if (c == 0)
				return 1;
		}
		decoded[n++] = (char) c;
	}
	decoded[n] = '\0';

	for (const char* name = decoded; name != NULL; )
	{
		size_t len = strcspn(name, "=&;");

		if ((len == 6 && strncasecmp(name, "attach", 6) == 0) || (len == 10 && strncasecmp(name, "attachment", 10) == 0))
			return 1;
		name = name[len] == '\0' ? NULL : name + len + 1;
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
