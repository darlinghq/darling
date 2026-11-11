// Copyright (C) 2026 Lubos Dolezel
// SPDX-License-Identifier: GPL-3.0-or-later
//
// statfs() reports Darwin MNT_* flags and follows a final symlink that
// points into the Darwin root.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/param.h>

int main(void)
{
	int failures = 0;
	struct statfs sf, sf_link;
	char dir[] = "/tmp/statfs-test.XXXXXX";
	char link[MAXPATHLEN];

	if (statfs("/", &sf) != 0) {
		perror("statfs /");
		return 1;
	}

	// Linux sets ST_VALID (0x20) in every statfs result; copied raw it
	// reads as MNT_UNION.
	if (sf.f_flags & MNT_UNION) {
		printf("FAIL statfs(/) f_flags %#x has MNT_UNION\n", sf.f_flags);
		failures++;
	} else {
		printf("PASS statfs(/) f_flags %#x\n", sf.f_flags);
	}

	if (!mkdtemp(dir)) {
		perror("mkdtemp");
		return 1;
	}
	snprintf(link, sizeof(link), "%s/lnk", dir);

	// /System exists in the Darwin root, not on a Linux host.
	if (symlink("/System", link) != 0) {
		perror("symlink");
		return 1;
	}

	if (statfs(link, &sf_link) != 0) {
		perror("FAIL statfs(symlink to /System)");
		failures++;
	} else {
		printf("PASS statfs(symlink to /System) -> %s\n", sf_link.f_mntonname);
	}

	unlink(link);
	rmdir(dir);

	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures != 0;
}
