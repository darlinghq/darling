// Copyright (C) 2026 Lubos Dolezel
//
// mkfifoat() and mknodat() create a FIFO relative to a directory
// descriptor. Both the libsystem_kernel wrappers (looked up with dlsym, so
// the test also builds against an SDK that does not declare them) and the
// raw syscalls with the macOS numbers (SYS_mkfifoat 553, SYS_mknodat 554)
// are checked.

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MACOS_SYS_mkfifoat 553
#define MACOS_SYS_mknodat 554

// syscall() is deprecated but is the only way to issue the raw calls.
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int failed;

static void check_fifo(int dirfd, const char *name, const char *how)
{
	struct stat st;
	if (fstatat(dirfd, name, &st, AT_SYMLINK_NOFOLLOW) != 0) {
		printf("FAIL: %s: %s not created in the directory: %s\n", how, name, strerror(errno));
		failed = 1;
	} else if (!S_ISFIFO(st.st_mode)) {
		printf("FAIL: %s: %s has mode %o, not a FIFO\n", how, name, st.st_mode);
		failed = 1;
	}
	// It must not have been created relative to the current directory.
	if (access(name, F_OK) == 0) {
		printf("FAIL: %s: %s was created in the current directory\n", how, name);
		failed = 1;
	}
}

int main(void)
{
	char dir[] = "/tmp/mkfifoat-test.XXXXXX";
	if (!mkdtemp(dir)) {
		perror("mkdtemp");
		return 1;
	}
	int dirfd = open(dir, O_RDONLY | O_DIRECTORY);
	if (dirfd < 0) {
		perror("open");
		return 1;
	}
	// Make the current directory a different one.
	chdir("/");

	int (*p_mkfifoat)(int, const char *, mode_t) = dlsym(RTLD_DEFAULT, "mkfifoat");
	int (*p_mknodat)(int, const char *, mode_t, dev_t) = dlsym(RTLD_DEFAULT, "mknodat");

	if (!p_mkfifoat || !p_mknodat) {
		printf("FAIL: mkfifoat %p, mknodat %p not exported\n", (void *)p_mkfifoat, (void *)p_mknodat);
		failed = 1;
	} else {
		if (p_mkfifoat(dirfd, "fifo1", 0600) != 0) {
			printf("FAIL: mkfifoat: %s\n", strerror(errno));
			failed = 1;
		} else
			check_fifo(dirfd, "fifo1", "mkfifoat");

		if (p_mknodat(dirfd, "fifo2", S_IFIFO | 0600, 0) != 0) {
			printf("FAIL: mknodat: %s\n", strerror(errno));
			failed = 1;
		} else
			check_fifo(dirfd, "fifo2", "mknodat");

		errno = 0;
		if (p_mkfifoat(dirfd, "fifo1", 0600) != -1 || errno != EEXIST) {
			printf("FAIL: mkfifoat on an existing name: errno %d, expected EEXIST\n", errno);
			failed = 1;
		}
	}

	if (syscall(MACOS_SYS_mkfifoat, dirfd, "fifo3", 0600) != 0) {
		printf("FAIL: syscall(553) mkfifoat: %s\n", strerror(errno));
		failed = 1;
	} else
		check_fifo(dirfd, "fifo3", "syscall 553");

	if (syscall(MACOS_SYS_mknodat, dirfd, "fifo4", S_IFIFO | 0600, 0) != 0) {
		printf("FAIL: syscall(554) mknodat: %s\n", strerror(errno));
		failed = 1;
	} else
		check_fifo(dirfd, "fifo4", "syscall 554");

	unlinkat(dirfd, "fifo1", 0);
	unlinkat(dirfd, "fifo2", 0);
	unlinkat(dirfd, "fifo3", 0);
	unlinkat(dirfd, "fifo4", 0);
	close(dirfd);
	rmdir(dir);

	if (!failed)
		printf("PASS\n");
	return failed;
}
