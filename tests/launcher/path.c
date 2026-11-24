// Copyright (C) 2026 Lubos Dolezel
//
// Host-side test of how the launcher names a host file in the guest (src/startup/launcher-path.c): its real path
// with /Volumes/SystemRoot in front. Run as an ordinary user in a directory it makes itself under $TMPDIR.
#define _XOPEN_SOURCE 700
#include <errno.h>
#include <ftw.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "launcher-path.h"

static int g_failures = 0;
static char base[PATH_MAX];

static void expectPath(const char* hostPath, const char* realRel, const char* what)
{
	char want[PATH_MAX * 2];
	char* got = launcherGuestPath(hostPath, base);

	snprintf(want, sizeof(want), "/Volumes/SystemRoot%s%s%s", base, realRel[0] ? "/" : "", realRel);
	if (got == NULL || strcmp(got, want) != 0)
	{
		printf("FAIL: %s: got %s, want %s\n", what, got ? got : "NULL", want);
		g_failures++;
	}
	else
		printf("PASS: %s\n", what);
	free(got);
}

static void expectMissing(const char* hostPath, const char* what)
{
	errno = 0;
	char* got = launcherGuestPath(hostPath, base);
	if (got != NULL || errno != ENOENT)
	{
		printf("FAIL: %s: got %s, errno %d\n", what, got ? got : "NULL", errno);
		g_failures++;
	}
	else
		printf("PASS: %s\n", what);
	free(got);
}

static void touch(const char* rel)
{
	char p[PATH_MAX * 2];
	FILE* f;

	snprintf(p, sizeof(p), "%s/%s", base, rel);
	if ((f = fopen(p, "w")) == NULL) { perror(p); exit(2); }
	fclose(f);
}

static int removeEntry(const char* path, const struct stat* st, int flag, struct FTW* ftw)
{
	(void) st; (void) flag; (void) ftw;
	return remove(path);
}

int main(void)
{
	const char* tmp = getenv("TMPDIR");
	char made[PATH_MAX];
	char p[PATH_MAX * 2];

	snprintf(made, sizeof(made), "%s/launcher-path-XXXXXX", tmp && tmp[0] ? tmp : "/tmp");
	if (mkdtemp(made) == NULL || realpath(made, base) == NULL) { perror(made); return 2; }

	touch("a.dmg");
	snprintf(p, sizeof(p), "%s/sub", base);
	mkdir(p, 0700);
	snprintf(p, sizeof(p), "%s/link.dmg", base);
	symlink("a.dmg", p);
	snprintf(p, sizeof(p), "%s/dangling.dmg", base);
	symlink("nowhere.dmg", p);
	const char* odd[] = { "Sp\xc3\xa4" "ce & 'q' $x.dmg", "new\nline.dmg", "byte \xff.dmg" };
	for (int i = 0; i < 3; i++)
		touch(odd[i]);

	snprintf(p, sizeof(p), "%s/a.dmg", base);
	expectPath(p, "a.dmg", "an absolute path");
	if (chdir(base) != 0) { perror(base); return 2; }
	expectPath("a.dmg", "a.dmg", "a relative path");
	expectPath("./sub/../a.dmg", "a.dmg", "a path with . and ..");
	expectPath("link.dmg", "a.dmg", "a symbolic link is followed");
	expectPath("sub", "sub", "a directory");
	for (int i = 0; i < 3; i++)
		expectPath(odd[i], odd[i], "a name kept byte for byte");

	char* root = launcherGuestPath("/", base);
	if (root == NULL || strcmp(root, "/Volumes/SystemRoot") != 0)
	{
		printf("FAIL: / is /Volumes/SystemRoot: got %s\n", root ? root : "NULL");
		g_failures++;
	}
	else
		printf("PASS: / is /Volumes/SystemRoot\n");
	free(root);

	// relative to the working directory given, not the process's (the root mode's launcher has joined the
	// container's mount namespace by then, which moved it to /)
	if (chdir("/") != 0) { perror("/"); return 2; }
	expectPath("a.dmg", "a.dmg", "a relative path is taken from the given working directory");
	expectPath("sub/../link.dmg", "a.dmg", "so is one with .. and a link");
	expectMissing("missing.dmg", "a missing file is ENOENT");
	expectMissing("dangling.dmg", "a dangling link is ENOENT");

	nftw(base, removeEntry, 16, FTW_DEPTH | FTW_PHYS);
	printf(g_failures ? "FAILED %d\n" : "ALL PASSED\n", g_failures);
	return g_failures != 0;
}
