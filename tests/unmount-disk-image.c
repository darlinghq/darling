// unmount(2) on a mounted disk image: a directory inside the volume is not a mount point (EINVAL), a volume with
// an open file is busy (EBUSY) unless MNT_FORCE is given, and once unmounted the path is no longer a mount point
// (or no longer exists: DiskArbitration removes mount points in /Volumes whose volume went away).
// With "ignore-sigchld" the program ignores SIGCHLD first, so that the kernel reaps its children itself: unmount()
// must not depend on reaping a helper process it started.
// Usage: unmount-disk-image <mount point of tests/fixtures/unmount-test.dmg> [ignore-sigchld]
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <unistd.h>

static int failures;

static void check(int ok, const char* what)
{
	printf("%s: %s", ok ? "PASS" : "FAIL", what);
	if (!ok)
		printf(" (errno %d %s)", errno, strerror(errno));
	printf("\n");
	if (!ok)
		failures++;
}

static int readable(const char* path)
{
	char buf[16];
	int fd = open(path, O_RDONLY);
	if (fd < 0)
		return 0;
	ssize_t n = read(fd, buf, sizeof(buf));
	close(fd);
	return n == 6 && memcmp(buf, "hello\n", 6) == 0;
}

int main(int argc, char** argv)
{
	if (argc == 3 && strcmp(argv[2], "ignore-sigchld") == 0)
		signal(SIGCHLD, SIG_IGN);
	else if (argc != 2) {
		fprintf(stderr, "usage: %s <mount point> [ignore-sigchld]\n", argv[0]);
		return 2;
	}

	const char* mp = argv[1];
	char sub[MAXPATHLEN], file[MAXPATHLEN];
	struct statfs before, after;
	snprintf(sub, sizeof(sub), "%s/sub", mp);
	snprintf(file, sizeof(file), "%s/hello.txt", mp);

	check(statfs(mp, &before) == 0 && readable(file), "volume mounted and readable");

	errno = 0;
	check(unmount(sub, 0) == -1 && errno == EINVAL, "directory inside the volume is not a mount point");
	check(readable(file), "volume still mounted after unmounting a directory inside it failed");

	int fd = open(file, O_RDONLY);
	errno = 0;
	check(unmount(mp, 0) == -1 && errno == EBUSY, "volume with an open file is busy");
	check(readable(file), "busy volume still mounted");

	errno = 0;
	check(unmount(mp, MNT_FORCE) == 0, "MNT_FORCE unmounts a busy volume");
	if (fd >= 0)
		close(fd);

	// DiskArbitration removes a mount point in /Volumes once its volume is gone (as on macOS), so the path may be
	// gone too
	errno = 0;
	int stat_result = statfs(mp, &after);
	check((stat_result == 0 && strcmp(after.f_mntonname, before.f_mntonname) != 0)
		|| (stat_result == -1 && errno == ENOENT), "path is no longer the volume's mount point");

	errno = 0;
	check(unmount(mp, 0) == -1 && (errno == EINVAL || errno == ENOENT),
		"unmounting again fails: not a mount point (or no longer there)");

	return failures ? 1 : 0;
}
