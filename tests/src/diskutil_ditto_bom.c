/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Contract checks for the diskutil/ditto helper scripts used when a disk image
 * was extracted without FUSE:
 *  - `diskutil info -plist <dir>` succeeds and reports an empty
 *    APFSPhysicalStores array only for a directory that has a sibling
 *    "<dir>.darling-dmg-extracted" marker file, and fails otherwise;
 *  - `ditto --bom <file.bom> -- <src> <dst>` copies exactly the paths listed in
 *    the bill of materials, and rejects a symlinked parent directory.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static int run(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
#include <stdarg.h>
static int run(const char* fmt, ...)
{
	char cmd[2048];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(cmd, sizeof(cmd), fmt, ap);
	va_end(ap);
	int rc = system(cmd);
	return rc;
}

int main(void)
{
	char root[] = "/tmp/diskutil_ditto_bom.XXXXXX";
	if (!mkdtemp(root)) {
		perror("mkdtemp");
		return 1;
	}

	/* diskutil info */
	run("mkdir '%s/vol' '%s/plain'", root, root);
	CHECK(run("diskutil info -plist '%s/vol' >/dev/null 2>&1", root) != 0); /* no marker */
	run("touch '%s/vol.darling-dmg-extracted'", root);
	CHECK(run("diskutil info -plist '%s/vol' >'%s/info.plist' 2>/dev/null", root, root) == 0);
	CHECK(run("grep -q APFSPhysicalStores '%s/info.plist'", root) == 0);
	CHECK(run("diskutil info -plist '%s/vol/' >/dev/null 2>&1", root) == 0); /* trailing slash */
	CHECK(run("diskutil info -plist '%s/plain' >/dev/null 2>&1", root) != 0); /* marker absent */
	CHECK(run("diskutil info '%s/vol' >/dev/null 2>&1", root) != 0); /* -plist required */

	/* ditto --bom */
	run("mkdir -p '%s/src/App.app/Contents' '%s/src/Other' '%s/dst'", root, root, root);
	run("echo a > '%s/src/App.app/Contents/a'; echo b > '%s/src/Other/b'", root, root);
	run("ln -s /Applications '%s/src/Applications'", root);
	run("mkdir '%s/sel' && cp -a '%s/src/App.app' '%s/src/Applications' '%s/sel/'", root, root, root, root);
	if (run("mkbom '%s/sel' '%s/sel.bom' >/dev/null 2>&1", root, root) != 0) {
		printf("SKIP: mkbom not available\n");
		goto done;
	}
	CHECK(run("ditto --bom '%s/sel.bom' -- '%s/src' '%s/dst/' >/dev/null 2>&1", root, root, root) == 0);
	{
		char p[1024];
		snprintf(p, sizeof(p), "%s/dst/App.app/Contents/a", root);
		CHECK(access(p, F_OK) == 0);
		snprintf(p, sizeof(p), "%s/dst/Other", root);
		CHECK(access(p, F_OK) != 0); /* not in the BOM */
		struct stat st;
		snprintf(p, sizeof(p), "%s/dst/Applications", root);
		CHECK(lstat(p, &st) == 0 && S_ISLNK(st.st_mode)); /* symlink copied as a leaf */
	}

	/* A symlinked parent in the destination must be refused. */
	run("rm -rf '%s/dst2' && mkdir '%s/dst2' '%s/outside' && ln -s '%s/outside' '%s/dst2/App.app'", root, root, root, root, root);
	CHECK(run("ditto --bom '%s/sel.bom' -- '%s/src' '%s/dst2' >/dev/null 2>&1", root, root, root) != 0);
	{
		char p[1024];
		snprintf(p, sizeof(p), "%s/outside/Contents", root);
		CHECK(access(p, F_OK) != 0);
	}

done:
	run("rm -rf '%s'", root);
	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("OK\n");
	return 0;
}
