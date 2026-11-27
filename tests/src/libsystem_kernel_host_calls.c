/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * BSD system calls: ntp_gettime, ATTR_VOL_CAPABILITIES, the fcntl storage commands, F_SINGLE_WRITER,
 * and the argument checks of getgroups, statfs, access and fstatat.
 * Expected results observed on macOS 10.14.4 (i386, x86_64) and macOS 26.7.3 (x86_64).
 */
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/attr.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/timex.h>
#include <time.h>
#include <unistd.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s (errno %d)\n", __LINE__, #cond, errno); failures++; } } while (0)

#ifndef F_SINGLE_WRITER
#define F_SINGLE_WRITER 76
#endif
#ifndef AT_SYMLINK_NOFOLLOW_ANY
#define AT_SYMLINK_NOFOLLOW_ANY 0x0800
#endif

static void test_ntp_gettime(void)
{
	struct ntptimeval ntv;
	time_t now = time(NULL);

	memset(&ntv, 0xaa, sizeof(ntv));
	errno = 0;
	int state = ntp_gettime(&ntv);
	CHECK(state >= 0 && state <= 5);
	CHECK(ntv.time.tv_sec >= now - 2 && ntv.time.tv_sec <= now + 2);
	CHECK(ntv.time.tv_nsec >= 0 && ntv.time.tv_nsec < 1000000000);
}

static void test_volume_capabilities(void)
{
	struct attrlist al;
	struct {
		u_int32_t length;
		vol_capabilities_attr_t caps;
	} __attribute__((packed)) reply;

	memset(&al, 0, sizeof(al));
	al.bitmapcount = ATTR_BIT_MAP_COUNT;
	al.volattr = ATTR_VOL_INFO | ATTR_VOL_CAPABILITIES;
	errno = 0;
	CHECK(getattrlist("/", &al, &reply, sizeof(reply), 0) == 0);

	/* every capability that is set has to be one that is valid, and the interface ones this emulation
	 * stands behind are set: it has clonefile(), renamex_np(), allocation and locking */
	u_int32_t want = VOL_CAP_INT_CLONE | VOL_CAP_INT_RENAME_SWAP | VOL_CAP_INT_RENAME_EXCL | VOL_CAP_INT_ALLOCATE |
		VOL_CAP_INT_ADVLOCK | VOL_CAP_INT_FLOCK | VOL_CAP_INT_EXTENDED_ATTR;
	CHECK((reply.caps.valid[VOL_CAPABILITIES_INTERFACES] & want) == want);
	CHECK((reply.caps.capabilities[VOL_CAPABILITIES_INTERFACES] & want) == want);
	CHECK((reply.caps.capabilities[VOL_CAPABILITIES_INTERFACES] & ~reply.caps.valid[VOL_CAPABILITIES_INTERFACES]) == 0);
	CHECK((reply.caps.capabilities[VOL_CAPABILITIES_FORMAT] & ~reply.caps.valid[VOL_CAPABILITIES_FORMAT]) == 0);
	CHECK(reply.caps.valid[VOL_CAPABILITIES_FORMAT] & VOL_CAP_FMT_CASE_PRESERVING);
}

static void test_storage_commands(void)
{
	char path[] = "/tmp/p4-fcntl.XXXXXX";
	int fd = mkstemp(path);
	struct stat st;

	CHECK(fd >= 0);
	CHECK(write(fd, "0123456789", 10) == 10);

	/* F_PREALLOCATE reserves space without changing the file size */
	fstore_t fs;
	memset(&fs, 0, sizeof(fs));
	fs.fst_flags = F_ALLOCATEALL;
	fs.fst_posmode = F_PEOFPOSMODE;
	fs.fst_offset = 0;
	fs.fst_length = 1 << 20;
	errno = 0;
	int rc = fcntl(fd, F_PREALLOCATE, &fs);
	CHECK(rc == 0 || (rc == -1 && errno == ENOTSUP));
	if (rc == 0)
		CHECK(fs.fst_bytesalloc >= 1 << 20);
	CHECK(fstat(fd, &st) == 0 && st.st_size == 10);

	errno = 0;
	CHECK(fcntl(fd, F_FULLFSYNC) == 0);
	errno = 0;
	CHECK(fcntl(fd, F_BARRIERFSYNC) == 0);

	struct radvisory ra = { 0, 10 };
	errno = 0;
	CHECK(fcntl(fd, F_RDADVISE, &ra) == 0);
	errno = 0;
	CHECK(fcntl(fd, F_RDAHEAD, 1) == 0);
	CHECK(fcntl(fd, F_RDAHEAD, 0) == 0);

	/* a file system that cannot hole-punch says ENOTSUP, one that can does it in whole blocks */
	fpunchhole_t ph;
	memset(&ph, 0, sizeof(ph));
	ph.fp_offset = 0;
	ph.fp_length = 4096;
	errno = 0;
	rc = fcntl(fd, F_PUNCHHOLE, &ph);
	CHECK(rc == 0 || (rc == -1 && (errno == ENOTSUP || errno == EINVAL)));
	CHECK(fstat(fd, &st) == 0 && st.st_size == 10);

	struct log2phys l2p;
	memset(&l2p, 0, sizeof(l2p));
	errno = 0;
	rc = fcntl(fd, F_LOG2PHYS, &l2p);
	CHECK(rc == 0 || (rc == -1 && (errno == ENOTSUP || errno == ENXIO || errno == EINVAL)));

	/* F_SINGLE_WRITER is a hint */
	errno = 0;
	CHECK(fcntl(fd, F_SINGLE_WRITER, 1) == 0);
	CHECK(fcntl(fd, F_SINGLE_WRITER, 0) == 0);

	fs.fst_length = 4096;
	errno = 0;
	CHECK(fcntl(-1, F_PREALLOCATE, &fs) == -1 && errno == EBADF);

	close(fd);
	unlink(path);
}

static void test_argument_checks(void)
{
	struct statfs sf;
	struct stat st;
	char* longpath = malloc(PATH_MAX + 64);

	errno = 0;
	CHECK(getgroups(-1, NULL) == -1 && errno == EINVAL);
	errno = 0;
	CHECK(statfs(NULL, &sf) == -1 && errno == EFAULT);
	errno = 0;
	CHECK(access(NULL, F_OK) == -1 && errno == EFAULT);

	/* a path longer than PATH_MAX: ENAMETOOLONG, the call survives */
	memset(longpath, 'a', PATH_MAX + 32);
	longpath[PATH_MAX + 32] = 0;
	errno = 0;
	CHECK(fstatat(AT_FDCWD, longpath, &st, 0) == -1 && errno == ENAMETOOLONG);
	free(longpath);

	/* the newer fstatat flags are accepted; here the last component is not a symlink */
	errno = 0;
	CHECK(fstatat(AT_FDCWD, "/", &st, AT_SYMLINK_NOFOLLOW_ANY) == 0);
	errno = 0;
	CHECK(faccessat(AT_FDCWD, "/", F_OK, AT_SYMLINK_NOFOLLOW_ANY) == 0);

	/* MADV_ZERO (11): either it zeroes the range or the call fails and leaves the memory alone */
	char* mem = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	CHECK(mem != MAP_FAILED);
	memset(mem, 7, 4096);
	errno = 0;
	int rc = madvise(mem, 4096, 11);
	if (rc == 0)
		CHECK(mem[0] == 0 && mem[4095] == 0);
	else
		CHECK(rc == -1 && (errno == ENOTSUP || errno == EINVAL) && mem[0] == 7 && mem[4095] == 7);
	munmap(mem, 4096);
}

int main(void)
{
	test_ntp_gettime();
	test_volume_capabilities();
	test_storage_commands();
	test_argument_checks();
	if (failures == 0)
		puts("PASS");
	return failures != 0;
}
