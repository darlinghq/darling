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

// The host's libX11 runs on the host's C library, whose locale is separate from the locale
// of the Darwin C library a program (or AppKit) sees: a setlocale() call in a Darwin program
// never reaches it. A host process starts with the "C" locale, and Xlib bases its input
// methods on the locale's category LC_CTYPE: with it, text that is not ASCII (input method
// commits, compose and dead-key results outside Latin-1) cannot be converted for
// Xutf8LookupString(), XmbLookupString() and the like and is dropped.
//
// When the libX11 wrapper is loaded, this gives the host library the character-type locale
// that an Xlib program would have set up for itself with setlocale(LC_ALL, ""), restricted
// to LC_CTYPE so that number formatting and the other categories of the other host libraries
// are left alone, and to UTF-8, the encoding input methods deliver: the locale named by the
// host environment (LC_ALL, LC_CTYPE, LANG), the same language with the UTF-8 encoding if it
// names another encoding, and C.UTF-8 when that locale is not installed. The Darwin locale of
// the program is not touched.

#include <elfcalls.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

extern struct elf_calls* _elfcalls;

// The category number of LC_CTYPE in the host's C library (not the Darwin value).
#define HOST_LC_CTYPE 0

typedef char* (*host_setlocale_fn)(int category, const char* locale);

// Whether a locale name such as "en_US.UTF-8" or "de_DE.utf8@euro" names the UTF-8 encoding.
static int is_utf8_locale(const char* name)
{
	const char* dot = strchr(name, '.');
	if (dot == NULL)
		return 0;
	dot++;
	size_t length = strcspn(dot, "@");
	return (length == 5 && strncasecmp(dot, "UTF-8", 5) == 0) || (length == 4 && strncasecmp(dot, "UTF8", 4) == 0);
}

__attribute__((constructor))
static void prepare_host_locale(void)
{
	if (_elfcalls == NULL || _elfcalls->dlsym == NULL)
		return;

	host_setlocale_fn host_setlocale = (host_setlocale_fn) _elfcalls->dlsym(NULL, "setlocale");
	if (host_setlocale == NULL)
		return;

	// A host locale that is already a UTF-8 one stays.
	const char* current = host_setlocale(HOST_LC_CTYPE, NULL);
	if (current != NULL && is_utf8_locale(current))
		return;

	// The environment's locale; unchanged and NULL when it is not installed.
	char name[128] = "";
	const char* chosen = host_setlocale(HOST_LC_CTYPE, "");
	if (chosen != NULL)
	{
		if (is_utf8_locale(chosen))
			return;
		snprintf(name, sizeof(name), "%s", chosen);
	}

	// The language of that locale in UTF-8, e.g. "cs_CZ.ISO-8859-2" -> "cs_CZ.UTF-8".
	if (name[0] != '\0' && strcmp(name, "C") != 0 && strcmp(name, "POSIX") != 0)
	{
		char utf8[160];
		size_t base = strcspn(name, ".@");
		const char* modifier = strchr(name, '@');
		snprintf(utf8, sizeof(utf8), "%.*s.UTF-8%s", (int) base, name, modifier != NULL ? modifier : "");
		if (host_setlocale(HOST_LC_CTYPE, utf8) != NULL)
			return;
	}

	// A failed setlocale() leaves the locale as it was, so an installation without C.UTF-8 keeps "C".
	host_setlocale(HOST_LC_CTYPE, "C.UTF-8");
}
