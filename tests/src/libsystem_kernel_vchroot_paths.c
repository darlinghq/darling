/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * How a guest path is resolved to a file (the vchroot lookup of libsystem_kernel), observed through open(), stat(),
 * readlink() and F_GETPATH on a tree the test builds in a new directory under /tmp:
 *  - symbolic links: relative, absolute (resolved inside the guest's tree), up to 31 in a row (macOS: MAXSYMLINKS is
 *    32; observed on 26.7.3: a chain of 31 links resolves, 32 is ELOOP), a loop, a dangling link, a link that climbs
 *    above the root, a link to a directory with a trailing slash;
 *  - ".." at the root stays at the root;
 *  - missing components (ENOENT) and a file used as a directory (ENOTDIR);
 *  - another spelling of a name in ASCII case, at every depth, resolves to the stored name;
 *  - the files of the system tree the guest sees through absolute links and ".." paths;
 *  - in Darling only: /proc/<pid>/<file>, /proc/self, /dev and /Volumes/SystemRoot/<host path>;
 *  - descriptor numbers: resolving paths does not take the lowest descriptor numbers.
 * Written against the macOS contract; the program also passes on macOS.
 */
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); failures++; } } while (0)

static char dir[PATH_MAX];

// removes what the test made (never follows a link)
static void rmtree(const char* path)
{
	struct stat st;
	if (lstat(path, &st) != 0)
		return;
	if (S_ISDIR(st.st_mode))
	{
		DIR* d = opendir(path);
		struct dirent* e;
		while (d && (e = readdir(d)))
		{
			char child[PATH_MAX];
			if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
				continue;
			snprintf(child, sizeof(child), "%s/%s", path, e->d_name);
			rmtree(child);
		}
		if (d)
			closedir(d);
		rmdir(path);
	}
	else
		unlink(path);
}

static void p(char* out, const char* rel)
{
	snprintf(out, PATH_MAX, "%s/%s", dir, rel);
}

static void mkdirs(const char* rel)
{
	char path[PATH_MAX];
	p(path, rel);
	CHECK(mkdir(path, 0755) == 0);
}

static void mkfile(const char* rel)
{
	char path[PATH_MAX];
	p(path, rel);
	int fd = open(path, O_CREAT | O_WRONLY | O_EXCL, 0644);
	CHECK(fd >= 0);
	if (fd >= 0)
		close(fd);
}

static void mklink(const char* rel, const char* target)
{
	char path[PATH_MAX];
	p(path, rel);
	CHECK(symlink(target, path) == 0);
}

// the path the descriptor of `path` is open at, as the guest names it (F_GETPATH), or "" if it cannot be opened
static int resolved(const char* path, char* out)
{
	int fd = open(path, O_RDONLY);
	if (fd < 0)
		return -1;
	out[0] = 0;
	int rv = fcntl(fd, F_GETPATH, out);
	close(fd);
	return rv;
}

// whether `rel` (below the test directory) opens the file `target_rel` (also below it)
static int opens_same(const char* rel, const char* target_rel)
{
	char a[PATH_MAX], b[PATH_MAX], pa[PATH_MAX], pb[PATH_MAX];
	p(a, rel);
	p(b, target_rel);
	if (resolved(a, pa) != 0 || resolved(b, pb) != 0)
		return 0;
	struct stat sa, sb;
	return stat(a, &sa) == 0 && stat(b, &sb) == 0 && sa.st_ino == sb.st_ino && sa.st_dev == sb.st_dev;
}

static int stat_errno(const char* path)
{
	struct stat st;
	errno = 0;
	return stat(path, &st) == 0 ? 0 : errno;
}

int main(void)
{
	char path[PATH_MAX], resolved_path[PATH_MAX];
	int in_darling = access("/Volumes/SystemRoot", F_OK) == 0;

	// resolving paths must not use up the lowest descriptor numbers
	int first = open("/dev/null", O_RDONLY);
	struct stat st;
	CHECK(stat("/usr/bin", &st) == 0);
	int second = open("/dev/null", O_RDONLY);
	CHECK(first >= 0 && second == first + 1);
	CHECK(first < 16);
	close(first);
	close(second);

	snprintf(dir, sizeof(dir), "/tmp/vchroot-paths-XXXXXX");
	CHECK(mkdtemp(dir) != NULL);

	mkdirs("a");
	mkdirs("a/b");
	mkdirs("a/b/c");
	mkfile("a/b/c/file");
	mkfile("a/top");
	mkfile("Mixed");
	mkdirs("CaseDir");
	mkfile("CaseDir/Inner.TXT");

	// relative links, to a file and to directories, with and without a trailing slash
	mklink("l1", "a/b");
	mklink("l2", "a/b/");
	mklink("tofile", "a/b/c/file");
	mklink("a/b/relup", "../top");
	mklink("a/b/relupup", "../../a/./top");
	CHECK(opens_same("l1/c/file", "a/b/c/file"));
	CHECK(opens_same("l2/c/file", "a/b/c/file"));
	CHECK(opens_same("tofile", "a/b/c/file"));
	CHECK(opens_same("a/b/relup", "a/top"));
	CHECK(opens_same("a/b/relupup", "a/top"));
	CHECK(opens_same("a/b/c/../../top", "a/top"));
	CHECK(opens_same("a//b/./c//file", "a/b/c/file"));

	// absolute links: resolved in the guest's tree, whatever the host's layout
	p(path, "a");
	mklink("abs", path);
	mklink("abslib", "/bin");
	CHECK(opens_same("abs/b/c/file", "a/b/c/file"));
	{
		char viaLink[PATH_MAX];
		p(viaLink, "abslib/ls");
		CHECK(resolved("/bin/ls", resolved_path) == 0);
		char direct[PATH_MAX];
		strcpy(direct, resolved_path);
		CHECK(resolved(viaLink, resolved_path) == 0 && strcmp(direct, resolved_path) == 0);
	}

	// a link climbs above the root: ".." at the root is the root
	mklink("climb", "../../../../../../../../../../../../../..");
	{
		char via[PATH_MAX], root[PATH_MAX];
		CHECK(resolved("/", root) == 0);
		p(via, "climb");
		CHECK(resolved(via, resolved_path) == 0 && strcmp(resolved_path, root) == 0);
		p(via, "climb/bin/ls");
		CHECK(stat(via, &st) == 0);
		CHECK(stat("/..", &st) == 0);
		struct stat rootStat;
		CHECK(stat("/", &rootStat) == 0 && stat("/../..", &st) == 0 && st.st_ino == rootStat.st_ino);
		CHECK(stat("/../../bin/../bin", &st) == 0);
	}

	// chains of links: 31 in a row resolve (macOS), a loop and a chain of more than the limit are ELOOP
	mklink("c1", "a/b/c/file");
	for (int i = 2; i <= 45; i++)
	{
		char name[16], target[16];
		snprintf(name, sizeof(name), "c%d", i);
		snprintf(target, sizeof(target), "c%d", i - 1);
		mklink(name, target);
	}
	for (int n = 1; n <= 31; n += (n < 5 ? 1 : 13))
	{
		char name[16];
		snprintf(name, sizeof(name), "c%d", n);
		p(path, name);
		if (stat_errno(path) != 0)
			printf("FAIL a chain of %d links does not resolve (errno %d)\n", n, stat_errno(path)), failures++;
	}
	p(path, "c31");
	CHECK(stat_errno(path) == 0);
	p(path, "c45");
	CHECK(stat_errno(path) == ELOOP);
	mklink("loopa", "loopb");
	mklink("loopb", "loopa");
	p(path, "loopa");
	CHECK(stat_errno(path) == ELOOP);
	{
		struct stat ls;
		CHECK(lstat(path, &ls) == 0 && S_ISLNK(ls.st_mode));
	}

	// dangling links, missing components, a file used as a directory
	mklink("dangling", "nowhere");
	mklink("dangabs", "/nonexistent-vchroot-paths-target");
	p(path, "dangling");
	CHECK(stat_errno(path) == ENOENT);
	{
		struct stat ls;
		CHECK(lstat(path, &ls) == 0 && S_ISLNK(ls.st_mode));
		char target[64];
		ssize_t n = readlink(path, target, sizeof(target) - 1);
		CHECK(n == 7 && memcmp(target, "nowhere", 7) == 0);
	}
	p(path, "dangabs");
	CHECK(stat_errno(path) == ENOENT);
	p(path, "dangling/x");
	CHECK(stat_errno(path) == ENOENT);
	p(path, "a/nope");
	CHECK(stat_errno(path) == ENOENT);
	p(path, "a/nope/deeper/file");
	CHECK(stat_errno(path) == ENOENT);
	p(path, "nope/b/c/file");
	CHECK(stat_errno(path) == ENOENT);
	p(path, "a/b/c/file/x");
	CHECK(stat_errno(path) == ENOTDIR);
	// a file created through a link lands in its target; a missing last component is created
	p(path, "l1/c/created");
	{
		int fd = open(path, O_CREAT | O_WRONLY | O_EXCL, 0644);
		CHECK(fd >= 0);
		if (fd >= 0)
			close(fd);
		CHECK(opens_same("a/b/c/created", "l1/c/created"));
		CHECK(unlink(path) == 0);
		CHECK(stat_errno(path) == ENOENT);
	}

	// another spelling of a name: the stored name comes back, at every depth
	p(path, "A/B/C/FILE");
	CHECK(stat_errno(path) == 0);
	p(path, "A/B/C/FILE");
	CHECK(resolved(path, resolved_path) == 0);
	{
		char want[PATH_MAX], wanted_path[PATH_MAX];
		p(want, "a/b/c/file");
		CHECK(resolved(want, wanted_path) == 0 && strcmp(resolved_path, wanted_path) == 0);
		CHECK(strstr(resolved_path, "/a/b/c/file") != NULL);
	}
	p(path, "mixed");
	CHECK(stat_errno(path) == 0);
	p(path, "casedir/inner.txt");
	CHECK(stat_errno(path) == 0);
	p(path, "CASEDIR/INNER.TXT");
	CHECK(stat_errno(path) == 0);
	p(path, "casedir/inner.txt/x");
	CHECK(stat_errno(path) == ENOTDIR);
	p(path, "casedir/other.txt");
	CHECK(stat_errno(path) == ENOENT);
	CHECK(stat_errno("/BIN/LS") == 0);

	// a name created in another spelling of an existing directory goes into that directory
	p(path, "casedir/New.txt");
	{
		int fd = open(path, O_CREAT | O_WRONLY | O_EXCL, 0644);
		CHECK(fd >= 0);
		if (fd >= 0)
			close(fd);
		p(path, "CaseDir/New.txt");
		CHECK(stat_errno(path) == 0);
		CHECK(unlink(path) == 0);
	}

	// relative paths, from the working directory and from a directory descriptor
	{
		char start[PATH_MAX];
		CHECK(getcwd(start, sizeof(start)) != NULL);
		p(path, "a/b");
		CHECK(chdir(path) == 0);
		CHECK(stat_errno("c/file") == 0);
		CHECK(stat_errno("./c/../c/file") == 0);
		CHECK(stat_errno("../top") == 0);
		CHECK(stat_errno("../../l1/c/file") == 0);
		CHECK(stat_errno("C/FILE") == 0);
		CHECK(stat_errno("../Top") == 0);
		CHECK(stat_errno("relup") == 0);
		CHECK(stat_errno("c/nope") == ENOENT);
		CHECK(chdir(start) == 0);

		int dfd = open(path, O_RDONLY | O_DIRECTORY);
		CHECK(dfd >= 0);
		struct stat sa, sb;
		CHECK(fstatat(dfd, "c/file", &sa, 0) == 0);
		p(path, "a/b/c/file");
		CHECK(stat(path, &sb) == 0 && sa.st_ino == sb.st_ino);
		CHECK(fstatat(dfd, "../top", &sa, 0) == 0);
		CHECK(fstatat(dfd, "relup", &sa, 0) == 0);
		CHECK(fstatat(dfd, "relup", &sa, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(sa.st_mode));
		CHECK(fstatat(dfd, "nope", &sa, 0) == -1 && errno == ENOENT);
		{
			int opened = openat(dfd, "C/File", O_RDONLY);
			CHECK(opened >= 0);
			if (opened >= 0)
				close(opened);
		}
		close(dfd);
	}

	// the system tree through absolute paths with ".." and links
	CHECK(stat_errno("/bin/../bin/ls") == 0);
	CHECK(stat_errno("/bin/./ls") == 0);
	CHECK(stat_errno("//bin///ls") == 0);
	CHECK(stat_errno("/bin/no-such-tool") == ENOENT);
	CHECK(stat_errno("/no-such-top/usr") == ENOENT);

	if (in_darling)
	{
		// /proc/<pid>/<file>: the files of the process, however the guest's /proc is set up
		char buf[512], name[64];
		static const char* const files[] = { "stat", "cmdline", "status", "statm", "comm", "environ", "auxv" };
		for (unsigned i = 0; i < sizeof(files) / sizeof(files[0]); i++)
		{
			snprintf(name, sizeof(name), "/proc/%d/%s", getpid(), files[i]);
			int fd = open(name, O_RDONLY);
			if (fd < 0)
				printf("FAIL cannot open %s (errno %d)\n", name, errno), failures++;
			else
			{
				CHECK(read(fd, buf, sizeof(buf)) >= 0);
				close(fd);
			}
		}
		snprintf(name, sizeof(name), "/proc/%d/stat", getpid());
		int fd = open(name, O_RDONLY);
		CHECK(fd >= 0);
		ssize_t n = fd >= 0 ? read(fd, buf, sizeof(buf) - 1) : -1;
		CHECK(n > 0);
		if (n > 0)
		{
			buf[n] = 0;
			CHECK(atoi(buf) == getpid());
		}
		if (fd >= 0)
			close(fd);
		fd = open("/proc/self/stat", O_RDONLY);
		CHECK(fd >= 0);
		n = fd >= 0 ? read(fd, buf, sizeof(buf) - 1) : -1;
		CHECK(n > 0);
		if (n > 0)
		{
			buf[n] = 0;
			CHECK(atoi(buf) == getpid());
		}
		if (fd >= 0)
			close(fd);
		CHECK(stat_errno("/proc/2147483646/stat") == ENOENT);
		CHECK(stat_errno("/proc/self/fd/0") == 0);

		// /dev
		fd = open("/dev/null", O_WRONLY);
		CHECK(fd >= 0 && write(fd, "x", 1) == 1);
		if (fd >= 0)
			close(fd);
		CHECK(stat_errno("/dev/stdin") == 0);
		CHECK(stat_errno("/dev/nonexistent-device") == ENOENT);

		// the host's tree below /Volumes/SystemRoot, and a way back
		CHECK(stat_errno("/Volumes/SystemRoot") == 0);
		CHECK(stat_errno("/Volumes/SystemRoot/proc/self") == 0);
		CHECK(stat_errno("/Volumes/SystemRoot/nonexistent-top") == ENOENT);
		CHECK(stat_errno("/Volumes/SystemRoot/Volumes/SystemRoot/proc") == ENOENT || stat_errno("/Volumes/SystemRoot/Volumes/SystemRoot/proc") == 0);
		CHECK(stat_errno("/volumes/systemroot/proc/self") == 0);
	}

	// descriptors again: the lookups above left the lowest numbers free
	{
		int a = open("/dev/null", O_RDONLY), b = open("/dev/null", O_RDONLY);
		CHECK(a >= 0 && b == a + 1 && a < 16);
		close(a);
		close(b);
	}

	rmtree(dir);
	printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
	return failures != 0;
}
