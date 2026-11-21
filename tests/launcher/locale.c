// Copyright (C) 2026 Lubos Dolezel
// Host-side test of the locale a new prefix gets (src/startup/launcher-locale.c): the mapping from the
// host's locale environment to AppleLocale and AppleLanguages, and the seeding of the prefix's
// .GlobalPreferences.plist, which never changes values that are already there.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "launcher-locale.h"

static int g_failures = 0;

#define CHECK(cond) do { \
		if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } \
	} while (0)

// languages is the expected AppleLanguages, entries joined with ","
static void expectFull(const char* all, const char* messages, const char* time, const char* lang,
	const char* languageList, const char* locale, const char* languages)
{
	struct launcher_locale l;
	char joined[256] = "";

	launcherDeriveLocale(all, messages, time, lang, languageList, &l);
	for (size_t i = 0; i < l.languageCount; i++)
	{
		if (i > 0)
			strcat(joined, ",");
		strcat(joined, l.languages[i]);
	}
	if (strcmp(l.locale, locale) != 0 || strcmp(joined, languages) != 0)
	{
		printf("FAIL (%s, %s, %s, %s, %s): got %s / %s, expected %s / %s\n", all ? all : "NULL",
			messages ? messages : "NULL", time ? time : "NULL", lang ? lang : "NULL",
			languageList ? languageList : "NULL", l.locale, joined, locale, languages);
		g_failures++;
	}
}

// LANG only
static void expectMapping(const char* lang, const char* locale, const char* language)
{
	expectFull(NULL, NULL, NULL, lang, NULL, locale, language);
}

static void test_mapping(void)
{
	// no usable host locale: the fallback
	expectFull(NULL, NULL, NULL, NULL, NULL, "en_US", "en-US");
	expectFull("", "", "", "", "", "en_US", "en-US");
	expectMapping("C", "en_US", "en-US");
	expectMapping("POSIX", "en_US", "en-US");
	expectMapping("C.UTF-8", "en_US", "en-US");
	// not of the form language[_REGION][.encoding][@modifier]
	expectMapping("de_de", "en_US", "en-US");
	expectMapping("garbage-1", "en_US", "en-US");
	expectMapping("_DE", "en_US", "en-US");
	expectMapping("d", "en_US", "en-US");

	expectMapping("de_DE.UTF-8", "de_DE", "de-DE");
	expectMapping("de_DE.UTF-8@euro", "de_DE", "de-DE");
	expectMapping("de_DE@euro", "de_DE", "de-DE");
	expectMapping("pt_BR", "pt_BR", "pt-BR");
	expectMapping("en_GB", "en_GB", "en-GB");
	expectMapping("en_GB.utf8", "en_GB", "en-GB");
	// a bare language is written without a region, as macOS writes it
	expectMapping("fr", "fr", "fr");
	expectMapping("fil_PH", "fil_PH", "fil-PH");
	expectMapping("es_419", "es_419", "es-419");
	// Chinese language lists name the script
	expectMapping("zh_CN.UTF-8", "zh_CN", "zh-Hans-CN");
	expectMapping("zh_TW", "zh_TW", "zh-Hant-TW");
	expectMapping("zh_HK.UTF-8", "zh_HK", "zh-Hant-HK");
	expectMapping("zh", "zh", "zh-Hans");
	// script modifiers become script subtags
	expectMapping("sr_RS@latin", "sr_Latn_RS", "sr-Latn-RS");
	expectMapping("sr_RS.UTF-8@cyrillic", "sr_Cyrl_RS", "sr-Cyrl-RS");

	// languages: LC_ALL, then LC_MESSAGES, then LANG; an empty one is skipped
	expectFull("fr_FR.UTF-8", "de_DE", NULL, "pt_BR", NULL, "fr_FR", "fr-FR");
	expectFull("", "de_DE", NULL, "pt_BR", NULL, "pt_BR", "de-DE");
	expectFull(NULL, "", NULL, "pt_BR", NULL, "pt_BR", "pt-BR");
	// formats: LC_ALL, then LC_TIME, then LANG (LC_MESSAGES does not count)
	expectFull("fr_FR.UTF-8", "de_DE", "it_IT", "pt_BR", NULL, "fr_FR", "fr-FR");
	expectFull(NULL, "de_DE", "it_IT", "pt_BR", NULL, "it_IT", "de-DE");
	expectFull(NULL, "de_DE", NULL, "pt_BR", NULL, "pt_BR", "de-DE");
	// English messages with German formats
	expectFull(NULL, NULL, "de_DE.UTF-8", "en_US.UTF-8", NULL, "de_DE", "en-US");
	// the first non-empty one decides, even when it is C
	expectFull("C", "de_DE", NULL, "pt_BR", NULL, "en_US", "en-US");

	// LANGUAGE: a list in order, each entry mapped like a locale name
	expectFull(NULL, NULL, NULL, "de_DE.UTF-8", "de:en_GB:fr", "de_DE", "de,en-GB,fr");
	expectFull(NULL, NULL, NULL, "de_DE.UTF-8", "pt_BR:zh_CN.UTF-8:sr_RS@latin", "de_DE", "pt-BR,zh-Hans-CN,sr-Latn-RS");
	// entries that are empty, C/POSIX or malformed are skipped, repeated ones kept once
	expectFull(NULL, NULL, NULL, "de_DE.UTF-8", ":C:de::POSIX:junk:de:fr_FR", "de_DE", "de,fr-FR");
	// the messages locale decides which LANGUAGE is looked at: not unset, C or POSIX
	expectFull(NULL, NULL, NULL, "C", "de:en_GB", "en_US", "en-US");
	expectFull(NULL, NULL, NULL, "POSIX", "de:en_GB", "en_US", "en-US");
	expectFull(NULL, NULL, NULL, NULL, "de:en_GB", "en_US", "en-US");
	expectFull(NULL, "C", NULL, "de_DE.UTF-8", "fr", "de_DE", "en-US");
	expectFull(NULL, "de_DE", NULL, "C", "fr", "en_US", "fr");
	expectFull("C", NULL, NULL, "de_DE.UTF-8", "fr", "en_US", "en-US");
	// a LANGUAGE with no usable entry falls back to the messages locale
	expectFull(NULL, NULL, NULL, "de_DE.UTF-8", "C:junk", "de_DE", "de-DE");
	expectFull(NULL, NULL, NULL, "de_DE.UTF-8", "", "de_DE", "de-DE");
	// at most LAUNCHER_MAX_LANGUAGES entries
	expectFull(NULL, NULL, NULL, "de_DE", "aa:bb:cc:dd:ee:ff:gg:hh:ii:jj", "de_DE", "aa,bb,cc,dd,ee,ff,gg,hh");
}

static char g_dir[256];

static void readFile(const char* name, char* buf, size_t size)
{
	char path[512];
	int fd;
	ssize_t n = 0;

	snprintf(path, sizeof(path), "%s/%s", g_dir, name);
	fd = open(path, O_RDONLY);
	if (fd >= 0)
	{
		n = read(fd, buf, size - 1);
		close(fd);
	}
	buf[n > 0 ? n : 0] = '\0';
}

static void writeFileContent(const char* name, const char* content)
{
	char path[512];
	FILE* f;

	snprintf(path, sizeof(path), "%s/%s", g_dir, name);
	f = fopen(path, "w");
	fwrite(content, 1, strlen(content), f);
	fclose(f);
}

static char* path(const char* name)
{
	static char buf[512];

	snprintf(buf, sizeof(buf), "%s/%s", g_dir, name);
	return buf;
}

static struct launcher_locale make(const char* locale, const char* language)
{
	struct launcher_locale l;

	l.languageCount = 1;
	strcpy(l.locale, locale);
	strcpy(l.languages[0], language);
	return l;
}

static void test_seeding(void)
{
	struct launcher_locale de = make("de_DE", "de-DE"), fr = make("fr_FR", "fr-FR"), multi;

	char content[4096], again[4096];
	struct stat st;

	// several languages are written in order
	launcherDeriveLocale(NULL, NULL, NULL, "de_DE", "de:en_GB", &multi);
	CHECK(launcherSeedGlobalPreferences(path("m.plist"), &multi) == LAUNCHER_GLOBAL_PREFS_WRITTEN);
	{
		char m[4096];

		readFile("m.plist", m, sizeof(m));
		CHECK(strstr(m, "<array>\n\t\t<string>de</string>\n\t\t<string>en-GB</string>\n\t</array>") != NULL);
	}

	// no file: created with both entries
	CHECK(launcherSeedGlobalPreferences(path("a.plist"), &de) == LAUNCHER_GLOBAL_PREFS_WRITTEN);
	readFile("a.plist", content, sizeof(content));
	CHECK(strncmp(content, "<?xml", 5) == 0);
	CHECK(strstr(content, "<key>AppleLocale</key>\n\t<string>de_DE</string>") != NULL);
	CHECK(strstr(content, "<key>AppleLanguages</key>\n\t<array>\n\t\t<string>de-DE</string>") != NULL);
	CHECK(strstr(content, "</plist>") != NULL);
	CHECK(stat(path("a.plist"), &st) == 0 && (st.st_mode & 0777) == 0644);

	// a second start with another host locale changes nothing
	CHECK(launcherSeedGlobalPreferences(path("a.plist"), &fr) == LAUNCHER_GLOBAL_PREFS_PRESENT);
	readFile("a.plist", again, sizeof(again));
	CHECK(strcmp(content, again) == 0);

	// the user has AppleLocale only: it stays, AppleLanguages is added
	writeFileContent("b.plist",
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<plist version=\"1.0\">\n<dict>\n"
		"\t<key>AppleLocale</key>\n\t<string>cs_CZ</string>\n\t<key>Other</key>\n\t<true/>\n</dict>\n</plist>\n");
	CHECK(launcherSeedGlobalPreferences(path("b.plist"), &de) == LAUNCHER_GLOBAL_PREFS_WRITTEN);
	readFile("b.plist", content, sizeof(content));
	CHECK(strstr(content, "<string>cs_CZ</string>") != NULL);
	CHECK(strstr(content, "de_DE") == NULL);
	CHECK(strstr(content, "<key>Other</key>\n\t<true/>") != NULL);
	CHECK(strstr(content, "<string>de-DE</string>") != NULL);
	CHECK(strstr(content, "</dict>\n</plist>") != NULL);

	// the user has AppleLanguages only: it stays, AppleLocale is added
	writeFileContent("c.plist",
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<plist version=\"1.0\">\n<dict>\n"
		"\t<key>AppleLanguages</key>\n\t<array>\n\t\t<string>cs-CZ</string>\n\t</array>\n</dict>\n</plist>\n");
	CHECK(launcherSeedGlobalPreferences(path("c.plist"), &de) == LAUNCHER_GLOBAL_PREFS_WRITTEN);
	readFile("c.plist", content, sizeof(content));
	CHECK(strstr(content, "<string>cs-CZ</string>") != NULL);
	CHECK(strstr(content, "de-DE") == NULL);
	CHECK(strstr(content, "<key>AppleLocale</key>\n\t<string>de_DE</string>") != NULL);

	// both present, in whatever order and layout: byte-identical afterwards
	writeFileContent("d.plist",
		"<?xml version=\"1.0\"?><plist version=\"1.0\"><dict><key>AppleLocale</key><string>en_GB</string>"
		"<key>AppleLanguages</key><array><string>en-GB</string></array></dict></plist>");
	CHECK(launcherSeedGlobalPreferences(path("d.plist"), &de) == LAUNCHER_GLOBAL_PREFS_PRESENT);
	readFile("d.plist", content, sizeof(content));
	CHECK(strstr(content, "de") == NULL);

	// an empty dictionary
	writeFileContent("e.plist", "<?xml version=\"1.0\"?>\n<plist version=\"1.0\">\n<dict/>\n</plist>\n");
	CHECK(launcherSeedGlobalPreferences(path("e.plist"), &de) == LAUNCHER_GLOBAL_PREFS_WRITTEN);
	readFile("e.plist", content, sizeof(content));
	CHECK(strstr(content, "<dict>\n\t<key>AppleLanguages</key>") != NULL);
	CHECK(strstr(content, "<string>de_DE</string>") != NULL);
	CHECK(strstr(content, "<dict/>") == NULL);
	CHECK(strstr(content, "</dict>\n</plist>") != NULL);

	// an empty file counts as no file
	writeFileContent("f.plist", "");
	CHECK(launcherSeedGlobalPreferences(path("f.plist"), &de) == LAUNCHER_GLOBAL_PREFS_WRITTEN);
	readFile("f.plist", content, sizeof(content));
	CHECK(strstr(content, "<string>de_DE</string>") != NULL);

	// a binary property list that lacks an entry is not edited
	writeFileContent("g.plist", "bplist00\xd1\x01\x02_\x10\x0bSomeOtherKey\x08");
	CHECK(launcherSeedGlobalPreferences(path("g.plist"), &de) == LAUNCHER_GLOBAL_PREFS_UNTOUCHED);
	readFile("g.plist", content, sizeof(content));
	CHECK(strncmp(content, "bplist00", 8) == 0 && strstr(content, "de_DE") == NULL);

	// text that is not a property list is not edited
	writeFileContent("h.plist", "hello\n");
	CHECK(launcherSeedGlobalPreferences(path("h.plist"), &de) == LAUNCHER_GLOBAL_PREFS_UNTOUCHED);
	readFile("h.plist", content, sizeof(content));
	CHECK(strcmp(content, "hello\n") == 0);

	// a link is not followed
	{
		char target[512];

		snprintf(target, sizeof(target), "%s", path("a.plist"));
		CHECK(symlink(target, path("link.plist")) == 0);
	}
	CHECK(launcherSeedGlobalPreferences(path("link.plist"), &de) == LAUNCHER_GLOBAL_PREFS_ERROR);

	// a missing directory is an error, not a created tree
	CHECK(launcherSeedGlobalPreferences(path("nodir/x.plist"), &de) == LAUNCHER_GLOBAL_PREFS_ERROR);
}

int main(void)
{
	snprintf(g_dir, sizeof(g_dir), "/tmp/launcher-locale-test.XXXXXX");
	if (mkdtemp(g_dir) == NULL)
	{
		perror("mkdtemp");
		return 1;
	}

	test_mapping();
	test_seeding();

	{
		char cmd[400];
		snprintf(cmd, sizeof(cmd), "rm -rf '%s'", g_dir);
		if (system(cmd) != 0)
			printf("note: cannot remove %s\n", g_dir);
	}

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures != 0;
}
