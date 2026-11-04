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

/* A decoded text that still changes after this many rounds is refused. */
#define MAILTO_DECODE_ROUNDS 4

/*
 * Decode every %XX in place. Returns -1 for a decoded NUL, and for a malformed escape when
 * strict. After the first round a stray '%' is plain text: "%2525" decodes to "%25", whose
 * second round is "%", and a client leaves "100%" alone.
 */
static int decode_round(char* s, int strict, int* changed)
{
	char* w = s;

	for (const char* r = s; *r != '\0'; )
	{
		if (*r == '%')
		{
			int hi = hex_digit((unsigned char) r[1]);
			int lo = hi < 0 ? -1 : hex_digit((unsigned char) r[2]);

			if (hi >= 0 && lo >= 0)
			{
				if (hi == 0 && lo == 0)
					return -1;
				*w++ = (char) (hi * 16 + lo);
				r += 3;
				*changed = 1;
				continue;
			}
			if (strict)
				return -1;
		}
		*w++ = *r++;
	}
	*w = '\0';
	return 0;
}

static int is_name_padding(char c)
{
	return c == '+' || c == ' ' || (c >= '\t' && c <= '\r');
}

static int name_is_attach(const char* name, size_t len)
{
	while (len > 0 && is_name_padding(name[0]))
	{
		name++;
		len--;
	}
	while (len > 0 && is_name_padding(name[len - 1]))
		len--;
	return (len == 6 && strncasecmp(name, "attach", 6) == 0) || (len == 10 && strncasecmp(name, "attachment", 10) == 0);
}

/*
 * Several mail clients attach the file named by a mailto: "attach" parameter to the draft.
 * Read the URL the way a client does: decode what follows "mailto:" until it stops changing
 * (a text that still changes after MAILTO_DECODE_ROUNDS rounds is refused), take the first
 * '?' of the decoded text as the start of the query, split the query on '&', ';' and '?',
 * and refuse a parameter whose name, without its '=' value and without whitespace or '+'
 * padding, is "attach" or "attachment". Values are never matched as names.
 */
static int mailto_attaches_file(const char* url)
{
	char text[LS_HOST_OPEN_MAX_URL_LENGTH + 1];
	const char* query;
	int round;

	strncpy(text, url + strlen("mailto:"), sizeof text - 1);
	text[sizeof text - 1] = '\0';

	for (round = 0; round <= MAILTO_DECODE_ROUNDS; round++)
	{
		int changed = 0;

		if (decode_round(text, round == 0, &changed) < 0)
			return 1;
		if (!changed)
			break;
		if (round == MAILTO_DECODE_ROUNDS)
			return 1;
	}

	query = strchr(text, '?');
	if (query == NULL)
		return 0;
	for (query++; ; )
	{
		size_t len = strcspn(query, "&;?");
		size_t name_len = strcspn(query, "=");

		if (name_is_attach(query, name_len < len ? name_len : len))
			return 1;
		if (query[len] == '\0')
			return 0;
		query += len + 1;
	}
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
