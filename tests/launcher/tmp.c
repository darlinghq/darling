// Copyright (C) 2026 Lubos Dolezel
//
// Host-side test of the temporary directory choice and the link in the prefix
// (src/startup/launcher-tmp.c), run as an ordinary user in a directory it makes itself.
#define _XOPEN_SOURCE 700
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "launcher-tmp.h"

static int g_failures = 0;
static char base[256];

#define CHECK(cond) do { \
		if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } \
	} while (0)

static void at(char* out, size_t size, const char* rel) { snprintf(out, size, "%s/%s", base, rel); }

static void dir(const char* rel, mode_t mode)
{
	char p[512];

	at(p, sizeof(p), rel);
	if (mkdir(p, mode) != 0) { perror(p); exit(2); }
	chmod(p, mode);
}

static void link_(const char* target, const char* rel)
{
	char p[512];

	at(p, sizeof(p), rel);
	if (symlink(target, p) != 0) { perror(p); exit(2); }
}

static int removeEntry(const char* path, const struct stat* st, int flag, struct FTW* ftw)
{
	(void) st; (void) flag; (void) ftw;
	return remove(path);
}

static bool linkIs(const char* rel, const char* target)
{
	char p[512], got[512];
	ssize_t n;

	at(p, sizeof(p), rel);
	n = readlink(p, got, sizeof(got) - 1);
	if (n < 0)
		return false;
	got[n] = '\0';
	return strcmp(got, target) == 0;
}

int main(void)
{
	uid_t me = getuid();
	char cache[512], other[512], viaLink[512], writable[512], prefix[512], expected[600], file[512];
	struct stat st;

	snprintf(base, sizeof(base), "/tmp/darling-tmp-test.XXXXXX");
	if (!mkdtemp(base)) return 2;

	dir("cache", 0700);
	dir("shared", 0777);
	dir("grp", 0770);
	dir("prefix", 0700);
	dir("prefix/private", 0755);
	dir("prefix/private/tmp", 0755);
	link_("cache", "viacache");
	at(cache, sizeof(cache), "cache");
	at(writable, sizeof(writable), "shared");
	at(viaLink, sizeof(viaLink), "viacache");
	at(prefix, sizeof(prefix), "prefix");

	// acceptance
	CHECK(launcherTmpDirAcceptable(cache, me));
	CHECK(!launcherTmpDirAcceptable(cache, me + 1));          // not the user's
	CHECK(!launcherTmpDirAcceptable(writable, me));           // writable by others
	at(other, sizeof(other), "grp");
	CHECK(!launcherTmpDirAcceptable(other, me));              // writable by the group
	CHECK(!launcherTmpDirAcceptable(viaLink, me));            // a link as the last component is not followed
	CHECK(!launcherTmpDirAcceptable("relative", me));
	CHECK(!launcherTmpDirAcceptable("/tmp/../etc", me));
	CHECK(!launcherTmpDirAcceptable("/nonexistent-dir", me));
	CHECK(!launcherTmpDirAcceptable(NULL, me));
	CHECK(!launcherTmpDirAcceptable("", me));
	at(file, sizeof(file), "prefix/private");
	CHECK(launcherTmpDirAcceptable(file, me));
	CHECK(!launcherTmpDirAcceptable("/proc/self/exe", me));   // not a directory
	CHECK(!launcherTmpDirAcceptable("/", me) || me == 0);     // root's, writable by nobody else, only the user 0 may use it

	// choice: TMPDIR first, then the build default, else none
	CHECK(launcherChooseTmpDir(cache, NULL, me) == cache);
	CHECK(launcherChooseTmpDir(writable, cache, me) == cache);
	CHECK(launcherChooseTmpDir(NULL, cache, me) == cache);
	CHECK(launcherChooseTmpDir(NULL, "", me) == NULL);
	CHECK(launcherChooseTmpDir(writable, writable, me) == NULL);
	CHECK(launcherChooseTmpDir("/tmp/../etc", NULL, me) == NULL);

	// the link: an empty directory becomes the link
	snprintf(expected, sizeof(expected), "/Volumes/SystemRoot%s", cache);
	CHECK(launcherLinkPrefixTmp(prefix, cache));
	CHECK(linkIs("prefix/private/tmp", expected));
	// again, with the same target, and with another one
	CHECK(launcherLinkPrefixTmp(prefix, cache));
	CHECK(linkIs("prefix/private/tmp", expected));
	snprintf(expected, sizeof(expected), "/Volumes/SystemRoot%s", prefix);
	CHECK(launcherLinkPrefixTmp(prefix, prefix));
	CHECK(linkIs("prefix/private/tmp", expected));

	// a directory with contents, or a file, is left alone
	{
		char p[512];

		at(p, sizeof(p), "prefix/private/tmp");
		unlink(p);
		dir("prefix/private/tmp", 0755);
		at(p, sizeof(p), "prefix/private/tmp/keep");
		fclose(fopen(p, "w"));
		CHECK(!launcherLinkPrefixTmp(prefix, cache));
		CHECK(stat(p, &st) == 0);
		unlink(p);
	}

	// a link in place of "private" is not followed (the outside directory stays untouched)
	{
		char p[512];

		dir("prefix2", 0700);
		dir("outside", 0700);
		link_("../outside", "prefix2/private");
		at(p, sizeof(p), "prefix2");
		CHECK(!launcherLinkPrefixTmp(p, cache));
		at(p, sizeof(p), "outside/tmp");
		CHECK(lstat(p, &st) != 0);
	}

	// a prefix without private, and an unacceptable directory name
	dir("prefix3", 0700);
	at(file, sizeof(file), "prefix3");
	CHECK(!launcherLinkPrefixTmp(file, cache));
	CHECK(!launcherLinkPrefixTmp(prefix, "relative"));

	chdir("/");
	nftw(base, removeEntry, 16, FTW_DEPTH | FTW_PHYS);
	if (g_failures == 0)
		printf("all passed\n");
	return g_failures != 0;
}
