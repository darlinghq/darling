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

#ifndef _LAUNCHER_LOCALE_H
#define _LAUNCHER_LOCALE_H

#include <stdbool.h>
#include <stddef.h>

// Longest identifier launcherDeriveLocale produces, with the terminating NUL, and the most
// AppleLanguages entries it produces.
#define LAUNCHER_LOCALE_SIZE 32
#define LAUNCHER_MAX_LANGUAGES 8

struct launcher_locale
{
	// the value for AppleLocale (formats): "de_DE"
	char locale[LAUNCHER_LOCALE_SIZE];
	// the entries of AppleLanguages (interface languages), most preferred first: "de-DE"
	char languages[LAUNCHER_MAX_LANGUAGES][LAUNCHER_LOCALE_SIZE];
	size_t languageCount;
};

// What a new prefix gets from the host's locale environment, as the launcher sees it (any argument may
// be NULL). Each value comes from its counterpart on Linux:
// - AppleLocale from the first non-empty of lcAll, lcTime and lang ("de_DE.UTF-8@euro" gives "de_DE";
//   a script modifier such as "@latin" becomes a script subtag: "sr_RS@latin" gives "sr_Latn_RS");
//   en_US when there is none or it is C, POSIX or not of the form language[_REGION][.encoding][@modifier].
// - AppleLanguages from languageList (the colon-separated LANGUAGE variable, each entry mapped like a
//   locale name: "de:en_GB" gives "de", "en-GB"; entries that are not names are skipped), but only when
//   the messages locale (the first non-empty of lcAll, lcMessages and lang) is not unset, C or POSIX,
//   as the C library ignores LANGUAGE then; otherwise from the messages locale alone ("de_DE.UTF-8"
//   gives "de-DE"); en-US when that is no name either. Chinese gets its script, as in "zh-Hans-CN".
void launcherDeriveLocale(const char* lcAll, const char* lcMessages, const char* lcTime, const char* lang,
	const char* languageList, struct launcher_locale* out);

enum launcher_global_prefs_result
{
	// the file was created, or the missing entries were added to it
	LAUNCHER_GLOBAL_PREFS_WRITTEN = 0,
	// it already had both entries
	LAUNCHER_GLOBAL_PREFS_PRESENT,
	// it is in a format this code does not edit (binary, or not a property
	// list dictionary) and lacks an entry; it was not touched
	LAUNCHER_GLOBAL_PREFS_UNTOUCHED,
	LAUNCHER_GLOBAL_PREFS_ERROR,
};

// Makes sure the property list at path (the prefix's
// /Library/Preferences/.GlobalPreferences.plist; its directory must exist) has
// AppleLocale and AppleLanguages (an array of the languages). A missing file is
// created; an XML property list that lacks an entry gets just that entry
// added; values that are present are never changed.
enum launcher_global_prefs_result launcherSeedGlobalPreferences(const char* path,
	const struct launcher_locale* locale);

#endif
