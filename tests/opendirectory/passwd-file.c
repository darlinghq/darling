// Copyright (C) 2026 Lubos Dolezel
//
// Host test of the passwd/master.passwd rewriting the CFOpenDirectory local
// node uses to store account changes (od_passwd_file.c).
//
// Pinned behavior: only the named user's shell and gecos fields change; every
// other line and field, comments, blank lines and a missing final newline
// stay as they were; the file keeps its mode; no temporary file is left; a
// missing user or file or an unusable value changes nothing.
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "od_passwd_file.h"

static int failures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); failures++; } } while (0)

static char dir[] = "/tmp/odpw-test.XXXXXX";
static char path[256];

static void put(const char* text, mode_t mode)
{
	FILE* f = fopen(path, "w");
	fputs(text, f);
	fclose(f);
	chmod(path, mode);
}

static char* get(void)
{
	static char buffer[4096];
	FILE* f = fopen(path, "r");
	size_t n = f ? fread(buffer, 1, sizeof(buffer) - 1, f) : 0;
	buffer[n] = '\0';
	if (f)
		fclose(f);
	return buffer;
}

static int entries(void)
{
	int n = 0;
	DIR* d = opendir(dir);
	struct dirent* e;
	while ((e = readdir(d)) != NULL)
		if (e->d_name[0] != '.')
			n++;
	closedir(d);
	return n;
}

int main(void)
{
	bool found;
	int err;

	if (mkdtemp(dir) == NULL)
		return 2;
	snprintf(path, sizeof(path), "%s/passwd", dir);

	// passwd format: shell and gecos change, the rest is kept
	put("# a comment\n\nroot:*:0:0:System Administrator:/var/root:/bin/sh\n"
		"joe:*:501:20:Joe:/Users/joe:/bin/zsh\n"
		"joey:*:502:20:Joey:/Users/joey:/bin/zsh\n", 0640);
	struct odpw_changes shell = { NULL, "/bin/bash" };
	err = odpw_update_user(path, ODPW_PASSWD, "joe", &shell, &found);
	CHECK(err == 0 && found);
	CHECK(strcmp(get(), "# a comment\n\nroot:*:0:0:System Administrator:/var/root:/bin/sh\n"
		"joe:*:501:20:Joe:/Users/joe:/bin/bash\n"
		"joey:*:502:20:Joey:/Users/joey:/bin/zsh\n") == 0);
	struct stat st;
	CHECK(stat(path, &st) == 0 && (st.st_mode & 07777) == 0640);
	CHECK(entries() == 1);

	struct odpw_changes both = { "Joe Doe,Room 1,555-1,555-2", "/bin/sh" };
	err = odpw_update_user(path, ODPW_PASSWD, "joe", &both, &found);
	CHECK(err == 0 && found);
	CHECK(strstr(get(), "joe:*:501:20:Joe Doe,Room 1,555-1,555-2:/Users/joe:/bin/sh\n") != NULL);
	struct odpw_changes empty = { "", NULL };
	err = odpw_update_user(path, ODPW_PASSWD, "joe", &empty, &found);
	CHECK(err == 0 && found);
	CHECK(strstr(get(), "joe:*:501:20::/Users/joe:/bin/sh\n") != NULL);

	// master.passwd format, last line without a newline
	put("root:*:0:0::0:0:System Administrator:/var/root:/bin/sh\n"
		"joe:$1$x:501:20::0:0:Joe:/Users/joe:/bin/zsh", 0600);
	err = odpw_update_user(path, ODPW_MASTER, "joe", &both, &found);
	CHECK(err == 0 && found);
	CHECK(strcmp(get(), "root:*:0:0::0:0:System Administrator:/var/root:/bin/sh\n"
		"joe:$1$x:501:20::0:0:Joe Doe,Room 1,555-1,555-2:/Users/joe:/bin/sh") == 0);
	CHECK(stat(path, &st) == 0 && (st.st_mode & 07777) == 0600);

	// a passwd-format line in a master-format file is not an entry
	char before[4096];
	strcpy(before, get());
	err = odpw_update_user(path, ODPW_PASSWD, "joe", &shell, &found);
	CHECK(err == 0 && !found && strcmp(get(), before) == 0);

	// unknown user: untouched, no error
	err = odpw_update_user(path, ODPW_MASTER, "nobody", &shell, &found);
	CHECK(err == 0 && !found && strcmp(get(), before) == 0 && entries() == 1);

	// values that would corrupt the file
	struct odpw_changes colon = { NULL, "/bin/sh:x" };
	err = odpw_update_user(path, ODPW_MASTER, "joe", &colon, &found);
	CHECK(err == EINVAL && !found && strcmp(get(), before) == 0);
	struct odpw_changes newline = { "a\nb", NULL };
	err = odpw_update_user(path, ODPW_MASTER, "joe", &newline, &found);
	CHECK(err == EINVAL && !found && strcmp(get(), before) == 0);

	// missing file
	unlink(path);
	err = odpw_update_user(path, ODPW_MASTER, "joe", &shell, &found);
	CHECK(err == ENOENT && !found && entries() == 0);

	rmdir(dir);
	if (failures == 0)
		printf("PASS\n");
	return failures != 0;
}
