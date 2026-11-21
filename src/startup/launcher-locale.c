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

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "launcher-locale.h"

#define DEFAULT_LOCALE "en_US"
#define DEFAULT_LANGUAGE "en-US"

// The most of an existing file that is read; a property list this large is not
// the small preferences file this code is meant for.
#define MAX_PREFS_SIZE (1024 * 1024)

static const char* firstNonEmpty(const char* a, const char* b, const char* c)
{
	if (a != NULL && a[0] != '\0')
		return a;
	if (b != NULL && b[0] != '\0')
		return b;
	if (c != NULL && c[0] != '\0')
		return c;
	return NULL;
}

static bool isCOrPosix(const char* name)
{
	size_t len = strcspn(name, ".@");

	return (len == 1 && name[0] == 'C') || (len == 5 && strncmp(name, "POSIX", 5) == 0);
}

// Splits a name of the form language[_REGION][.encoding][@modifier] into the AppleLocale form and the
// AppleLanguages form. Returns false for a name that is not of that form (C and POSIX included).
static bool mapName(const char* name, char* locale, char* language)
{
	const char* modifier;
	size_t baseLen;
	char langCode[4];
	char region[4];
	const char* script = NULL;
	size_t langLen, regionLen = 0;

	// language[_REGION] ends at the encoding or at the modifier
	baseLen = strcspn(name, ".@");
	modifier = strchr(name, '@');

	for (langLen = 0; langLen < baseLen && islower((unsigned char) name[langLen]); langLen++)
		;
	if (langLen < 2 || langLen > 3)
		return false;
	memcpy(langCode, name, langLen);
	langCode[langLen] = '\0';

	if (langLen < baseLen)
	{
		const char* r = name + langLen + 1;
		size_t rlen = baseLen - langLen - 1;

		if (name[langLen] != '_')
			return false;
		if (rlen == 2 && isupper((unsigned char) r[0]) && isupper((unsigned char) r[1]))
			regionLen = 2;
		else if (rlen == 3 && isdigit((unsigned char) r[0]) && isdigit((unsigned char) r[1]) && isdigit((unsigned char) r[2]))
			regionLen = 3;
		else
			return false;
		memcpy(region, r, regionLen);
		region[regionLen] = '\0';
	}

	if (modifier != NULL)
	{
		if (strncmp(modifier, "@latin", 6) == 0 && (modifier[6] == '\0' || modifier[6] == ';' || modifier[6] == ','))
			script = "Latn";
		else if (strncmp(modifier, "@cyrillic", 9) == 0 && (modifier[9] == '\0' || modifier[9] == ';' || modifier[9] == ','))
			script = "Cyrl";
	}

	// AppleLocale carries the script only when the host named one
	snprintf(locale, LAUNCHER_LOCALE_SIZE, "%s%s%s%s%s", langCode, script ? "_" : "", script ? script : "",
		regionLen ? "_" : "", regionLen ? region : "");

	// Chinese is told apart by script in language lists
	if (script == NULL && strcmp(langCode, "zh") == 0)
	{
		if (regionLen == 0 || strcmp(region, "CN") == 0 || strcmp(region, "SG") == 0)
			script = "Hans";
		else if (strcmp(region, "TW") == 0 || strcmp(region, "HK") == 0 || strcmp(region, "MO") == 0)
			script = "Hant";
	}
	snprintf(language, LAUNCHER_LOCALE_SIZE, "%s%s%s%s%s", langCode, script ? "-" : "", script ? script : "",
		regionLen ? "-" : "", regionLen ? region : "");
	return true;
}

static void addLanguage(struct launcher_locale* out, const char* language)
{
	if (out->languageCount >= LAUNCHER_MAX_LANGUAGES)
		return;
	for (size_t i = 0; i < out->languageCount; i++)
		if (strcmp(out->languages[i], language) == 0)
			return;
	strcpy(out->languages[out->languageCount++], language);
}

void launcherDeriveLocale(const char* lcAll, const char* lcMessages, const char* lcTime, const char* lang,
	const char* languageList, struct launcher_locale* out)
{
	const char* name;
	char scratch[LAUNCHER_LOCALE_SIZE];
	char language[LAUNCHER_LOCALE_SIZE];
	char loc[LAUNCHER_LOCALE_SIZE];

	out->languageCount = 0;

	// formats: LC_ALL, LC_TIME, LANG
	name = firstNonEmpty(lcAll, lcTime, lang);
	if (name == NULL || !mapName(name, out->locale, scratch))
		strcpy(out->locale, DEFAULT_LOCALE);

	// interface languages: the LANGUAGE list, which the C library ignores when the messages locale is
	// C or POSIX (or unset); then the messages locale itself
	name = firstNonEmpty(lcAll, lcMessages, lang);
	if (name != NULL && !isCOrPosix(name) && languageList != NULL)
	{
		const char* p = languageList;

		while (*p != '\0')
		{
			size_t len = strcspn(p, ":");

			if (len > 0 && len < sizeof(scratch))
			{
				memcpy(scratch, p, len);
				scratch[len] = '\0';
				if (mapName(scratch, loc, language))
					addLanguage(out, language);
			}
			p += len;
			if (*p == ':')
				p++;
		}
	}
	if (out->languageCount == 0)
	{
		if (name != NULL && mapName(name, scratch, language))
			addLanguage(out, language);
		else
			addLanguage(out, DEFAULT_LANGUAGE);
	}
}

static const char plistHead[] =
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
	"<plist version=\"1.0\">\n"
	"<dict>\n";
static const char plistTail[] = "</dict>\n</plist>\n";

static bool hasKey(const char* data, size_t size, const char* key)
{
	size_t len = strlen(key);

	if (size < len)
		return false;
	for (size_t i = 0; i + len <= size; i++)
	{
		if (memcmp(data + i, key, len) == 0)
			return true;
	}
	return false;
}

// Writes data to a new file next to path and renames it over path (or, with
// replace false, creates path and fails if it exists).
static bool writeFile(const char* path, const char* data, size_t size, bool replace)
{
	char tmp[4096];
	int fd;
	bool ok;

	if (!replace)
	{
		fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0644);
		if (fd < 0)
			return false;
		ok = write(fd, data, size) == (ssize_t) size;
		if (close(fd) != 0)
			ok = false;
		if (!ok)
			unlink(path);
		return ok;
	}

	if (snprintf(tmp, sizeof(tmp), "%s.tmp.%d", path, (int) getpid()) >= (int) sizeof(tmp))
		return false;
	fd = open(tmp, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0644);
	if (fd < 0)
		return false;
	ok = write(fd, data, size) == (ssize_t) size;
	if (close(fd) != 0)
		ok = false;
	if (ok && rename(tmp, path) != 0)
		ok = false;
	if (!ok)
		unlink(tmp);
	return ok;
}

// The AppleLanguages entry for the languages, ending with a newline
static size_t languagesEntry(char* out, size_t size, const struct launcher_locale* l)
{
	size_t n = (size_t) snprintf(out, size, "\t<key>AppleLanguages</key>\n\t<array>\n");

	for (size_t i = 0; i < l->languageCount && n < size; i++)
		n += (size_t) snprintf(out + n, size - n, "\t\t<string>%s</string>\n", l->languages[i]);
	if (n < size)
		n += (size_t) snprintf(out + n, size - n, "\t</array>\n");
	return n;
}

enum launcher_global_prefs_result launcherSeedGlobalPreferences(const char* path,
	const struct launcher_locale* l)
{
	char entries[1024];
	char* data;
	size_t size = 0;
	struct stat st;
	int fd;
	bool haveLocale, haveLanguages;
	size_t entriesLen = 0;
	const char* dictEnd;
	const char* end;
	const char* emptyDict;
	size_t insertAt, tailLen;
	char* out;
	size_t outSize;
	enum launcher_global_prefs_result result;

	fd = open(path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
	if (fd < 0)
	{
		if (errno != ENOENT)
			return LAUNCHER_GLOBAL_PREFS_ERROR;
		data = NULL;
	}
	else
	{
		if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode) || st.st_size > MAX_PREFS_SIZE)
		{
			close(fd);
			return LAUNCHER_GLOBAL_PREFS_ERROR;
		}
		data = malloc((size_t) st.st_size + 1);
		if (data == NULL)
		{
			close(fd);
			return LAUNCHER_GLOBAL_PREFS_ERROR;
		}
		while (size < (size_t) st.st_size)
		{
			ssize_t n = read(fd, data + size, (size_t) st.st_size - size);
			if (n <= 0)
				break;
			size += (size_t) n;
		}
		close(fd);
		if (size != (size_t) st.st_size)
		{
			free(data);
			return LAUNCHER_GLOBAL_PREFS_ERROR;
		}
		data[size] = '\0';
	}

	// An absent or empty file gets a fresh dictionary
	if (data == NULL || size == 0)
	{
		outSize = sizeof(plistHead) + sizeof(entries) + sizeof(plistTail) + 64;
		out = malloc(outSize);
		if (out == NULL)
		{
			free(data);
			return LAUNCHER_GLOBAL_PREFS_ERROR;
		}
		entriesLen = languagesEntry(entries, sizeof(entries), l);
		outSize = (size_t) snprintf(out, outSize, "%s%s\t<key>AppleLocale</key>\n\t<string>%s</string>\n%s",
			plistHead, entries, l->locale, plistTail);
		result = writeFile(path, out, outSize, data != NULL) ? LAUNCHER_GLOBAL_PREFS_WRITTEN : LAUNCHER_GLOBAL_PREFS_ERROR;
		free(out);
		free(data);
		return result;
	}

	haveLocale = hasKey(data, size, "AppleLocale");
	haveLanguages = hasKey(data, size, "AppleLanguages");
	if (haveLocale && haveLanguages)
	{
		free(data);
		return LAUNCHER_GLOBAL_PREFS_PRESENT;
	}

	// Only an XML dictionary is edited: the top-level dictionary ends with the
	// last </dict> (or is a lone <dict/>), which is followed by nothing but
	// </plist>.
	dictEnd = NULL;
	for (const char* p = data; (p = strstr(p, "</dict>")) != NULL; p++)
		dictEnd = p;
	emptyDict = strstr(data, "<dict/>");
	if (strncmp(data, "<?xml", 5) != 0 || (dictEnd == NULL && emptyDict == NULL))
	{
		free(data);
		return LAUNCHER_GLOBAL_PREFS_UNTOUCHED;
	}
	end = dictEnd != NULL ? dictEnd + strlen("</dict>") : emptyDict + strlen("<dict/>");
	while (*end != '\0' && isspace((unsigned char) *end))
		end++;
	if (strncmp(end, "</plist>", 8) != 0)
	{
		free(data);
		return LAUNCHER_GLOBAL_PREFS_UNTOUCHED;
	}

	if (!haveLanguages)
		entriesLen += languagesEntry(entries + entriesLen, sizeof(entries) - entriesLen, l);
	if (!haveLocale)
		entriesLen += (size_t) snprintf(entries + entriesLen, sizeof(entries) - entriesLen,
			"\t<key>AppleLocale</key>\n\t<string>%s</string>\n", l->locale);

	if (dictEnd != NULL)
	{
		insertAt = (size_t) (dictEnd - data);
		tailLen = size - insertAt;
		out = malloc(size + entriesLen + 1);
		if (out == NULL)
		{
			free(data);
			return LAUNCHER_GLOBAL_PREFS_ERROR;
		}
		memcpy(out, data, insertAt);
		memcpy(out + insertAt, entries, entriesLen);
		memcpy(out + insertAt + entriesLen, dictEnd, tailLen);
		outSize = size + entriesLen;
	}
	else
	{
		// <dict/> becomes <dict>entries</dict>
		insertAt = (size_t) (emptyDict - data);
		tailLen = size - insertAt - strlen("<dict/>");
		outSize = insertAt + strlen("<dict>\n") + entriesLen + strlen("</dict>") + tailLen;
		out = malloc(outSize + 1);
		if (out == NULL)
		{
			free(data);
			return LAUNCHER_GLOBAL_PREFS_ERROR;
		}
		memcpy(out, data, insertAt);
		memcpy(out + insertAt, "<dict>\n", strlen("<dict>\n"));
		memcpy(out + insertAt + strlen("<dict>\n"), entries, entriesLen);
		memcpy(out + insertAt + strlen("<dict>\n") + entriesLen, "</dict>", strlen("</dict>"));
		memcpy(out + insertAt + strlen("<dict>\n") + entriesLen + strlen("</dict>"), emptyDict + strlen("<dict/>"), tailLen);
	}
	result = writeFile(path, out, outSize, true) ? LAUNCHER_GLOBAL_PREFS_WRITTEN : LAUNCHER_GLOBAL_PREFS_ERROR;
	free(out);
	free(data);
	return result;
}
