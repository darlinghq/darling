// Copyright (C) 2026 Lubos Dolezel
//
// statfs_ext() and fstatfs_ext(), exported by libsystem_kernel since
// macOS 14 (declared only in the private part of sys/mount.h). With no
// flags they are statfs()/fstatfs(); STATFS_EXT_NOBLOCK fills the
// identifying fields from a volume attribute query (falling back to
// statfs when that query is not supported); any other flag is EINVAL.

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <unistd.h>

#ifndef STATFS_EXT_NOBLOCK
#define STATFS_EXT_NOBLOCK 0x0001
#endif

// Looked up at run time: the symbols are not in every SDK's stubs.
static int (*statfs_ext)(const char *, struct statfs *, int);
static int (*fstatfs_ext)(int, struct statfs *, int);

static int failed;

#define EXPECT(cond, ...) do { if (!(cond)) { printf("FAIL: " __VA_ARGS__); printf("\n"); failed = 1; } } while (0)

static void same_volume(const char *label, const struct statfs *a, const struct statfs *b)
{
	EXPECT(strcmp(a->f_fstypename, b->f_fstypename) == 0, "%s: f_fstypename \"%s\" vs \"%s\"", label,
		a->f_fstypename, b->f_fstypename);
	EXPECT(strcmp(a->f_mntonname, b->f_mntonname) == 0, "%s: f_mntonname \"%s\" vs \"%s\"", label,
		a->f_mntonname, b->f_mntonname);
	EXPECT(strcmp(a->f_mntfromname, b->f_mntfromname) == 0, "%s: f_mntfromname \"%s\" vs \"%s\"", label,
		a->f_mntfromname, b->f_mntfromname);
	EXPECT(a->f_type == b->f_type, "%s: f_type %u vs %u", label, a->f_type, b->f_type);
	EXPECT((a->f_flags & MNT_VISFLAGMASK) == (b->f_flags & MNT_VISFLAGMASK), "%s: f_flags 0x%x vs 0x%x",
		label, a->f_flags, b->f_flags);
}

int main(void)
{
	const char *path = "/";
	struct statfs ref, out;

	statfs_ext = (int (*)(const char *, struct statfs *, int))dlsym(RTLD_DEFAULT, "statfs_ext");
	fstatfs_ext = (int (*)(int, struct statfs *, int))dlsym(RTLD_DEFAULT, "fstatfs_ext");
	if (!statfs_ext || !fstatfs_ext) {
		printf("FAIL: statfs_ext or fstatfs_ext is not exported\n");
		return 1;
	}

	if (statfs(path, &ref) != 0) {
		perror("statfs");
		return 1;
	}

	memset(&out, 0xa5, sizeof(out));
	EXPECT(statfs_ext(path, &out, 0) == 0, "statfs_ext(0): errno %d", errno);
	same_volume("statfs_ext(0)", &out, &ref);
	EXPECT(out.f_blocks == ref.f_blocks && out.f_bsize == ref.f_bsize, "statfs_ext(0): sizes differ");

	memset(&out, 0xa5, sizeof(out));
	EXPECT(statfs_ext(path, &out, STATFS_EXT_NOBLOCK) == 0, "statfs_ext(NOBLOCK): errno %d", errno);
	same_volume("statfs_ext(NOBLOCK)", &out, &ref);

	// The descriptor variants are compared with fstatfs(): the volume
	// names fstatfs() reports for a path can differ from statfs()'s on
	// Darling (the mount table is searched with a different path form).
	int fd = open(path, O_RDONLY);
	struct statfs fref;
	if (fstatfs(fd, &fref) != 0) {
		perror("fstatfs");
		return 1;
	}
	memset(&out, 0xa5, sizeof(out));
	EXPECT(fstatfs_ext(fd, &out, 0) == 0, "fstatfs_ext(0): errno %d", errno);
	same_volume("fstatfs_ext(0)", &out, &fref);
	memset(&out, 0xa5, sizeof(out));
	EXPECT(fstatfs_ext(fd, &out, STATFS_EXT_NOBLOCK) == 0, "fstatfs_ext(NOBLOCK): errno %d", errno);
	same_volume("fstatfs_ext(NOBLOCK)", &out, &fref);
	close(fd);

	errno = 0;
	EXPECT(statfs_ext(path, &out, 2) == -1 && errno == EINVAL, "statfs_ext(2): errno %d, expected EINVAL", errno);
	errno = 0;
	EXPECT(fstatfs_ext(-1, &out, 0) == -1 && errno == EBADF, "fstatfs_ext(-1): errno %d, expected EBADF", errno);
	errno = 0;
	EXPECT(statfs_ext("/nonexistent-statfs-ext", &out, 0) == -1 && errno == ENOENT,
		"statfs_ext(missing): errno %d, expected ENOENT", errno);

	if (!failed)
		printf("PASS\n");
	return failed;
}
