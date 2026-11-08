// Copyright (C) 2026 Lubos Dolezel
//
// F_ADDFILESIGS_RETURN, F_CHECK_LV and the Darling-private DC_F_SNAPSHOT_IMAGE.
// Usage: cs_fcntl <directory with darlingserver's tests/fixtures/codesign>
// (only `adhoc`, `linker-signed` and `unsigned` are used). Exits by itself.
//
// Under the default code signing policy (off):
// - F_ADDFILESIGS_RETURN on a signed image succeeds and returns the end of the
//   signed range: the CodeDirectory's code limit rounded up to its hash page
//   size (16 KiB for `adhoc`, 4 KiB for `linker-signed`), as on macOS 27;
// - F_ADDFILESIGS succeeds and leaves the structure alone;
// - on an unsigned image or a file that is not a Mach-O file, nothing is
//   refused and the signed range ends at 0;
// - F_CHECK_LV accepts anything; both fail with EFAULT for a NULL argument.
// DC_F_SNAPSHOT_IMAGE returns a descriptor for a sealed copy: same contents,
// not writable, not resizable, unaffected by later changes to the file, and
// the source's offset stays as it was; an empty file gives EINVAL.
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <kern/cs_blobs.h>
#include <mach-o/loader.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <darling/emulation/common/dc_image_snapshot.h>

static int failures;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

static char tmp_path[64];

static void finish(void)
{
	if (tmp_path[0])
		unlink(tmp_path);
	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	fflush(stdout);
	_exit(failures ? 1 : 0);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	failures++;
	finish();
}

// the code limit and hash page size of a thin 64-bit image's primary CodeDirectory
static int code_directory(int fd, uint32_t* code_limit, uint32_t* page_size)
{
	struct mach_header_64 mh;
	if (pread(fd, &mh, sizeof(mh), 0) != sizeof(mh) || mh.magic != MH_MAGIC_64)
		return -1;

	uint8_t* commands = malloc(mh.sizeofcmds);
	if (commands == NULL || pread(fd, commands, mh.sizeofcmds, sizeof(mh)) != (ssize_t) mh.sizeofcmds)
		return -1;

	uint32_t dataoff = 0, at = 0;
	for (uint32_t i = 0; i < mh.ncmds && at + sizeof(struct load_command) <= mh.sizeofcmds; i++) {
		struct load_command* lc = (struct load_command*) (commands + at);
		if (lc->cmd == LC_CODE_SIGNATURE)
			dataoff = ((struct linkedit_data_command*) lc)->dataoff;
		at += lc->cmdsize;
	}
	free(commands);
	if (dataoff == 0)
		return -1;

	CS_SuperBlob super;
	if (pread(fd, &super, sizeof(super), dataoff) != sizeof(super) || ntohl(super.magic) != CSMAGIC_EMBEDDED_SIGNATURE)
		return -1;
	for (uint32_t i = 0; i < ntohl(super.count); i++) {
		CS_BlobIndex index;
		if (pread(fd, &index, sizeof(index), dataoff + sizeof(super) + i * sizeof(index)) != sizeof(index))
			return -1;
		if (ntohl(index.type) != CSSLOT_CODEDIRECTORY)
			continue;
		CS_CodeDirectory cd;
		if (pread(fd, &cd, sizeof(cd), dataoff + ntohl(index.offset)) != sizeof(cd) || ntohl(cd.magic) != CSMAGIC_CODEDIRECTORY)
			return -1;
		*code_limit = ntohl(cd.codeLimit);
		*page_size = 1u << cd.pageSize;
		return 0;
	}
	return -1;
}

static void test_signed(const char* dir, const char* name, uint32_t expected_page_size)
{
	char path[1024], what[256];
	snprintf(path, sizeof(path), "%s/%s", dir, name);
	int fd = open(path, O_RDONLY);
	snprintf(what, sizeof(what), "%s: opened", name);
	check(fd >= 0, what);
	if (fd < 0)
		return;

	uint32_t code_limit = 0, page_size = 0;
	snprintf(what, sizeof(what), "%s: CodeDirectory with %u-byte pages", name, expected_page_size);
	check(code_directory(fd, &code_limit, &page_size) == 0 && page_size == expected_page_size, what);

	fsignatures_t fs;
	memset(&fs, 0, sizeof(fs));
	fs.fs_file_start = 0;
	errno = 0;
	int rc = fcntl(fd, F_ADDFILESIGS_RETURN, &fs);
	off_t expected = ((off_t) code_limit + page_size - 1) & ~((off_t) page_size - 1);
	printf("  %s: rc %d errno %d, code limit 0x%x, end 0x%llx (expected 0x%llx)\n", name, rc, rc ? errno : 0,
		code_limit, (unsigned long long) fs.fs_file_start, (unsigned long long) expected);
	snprintf(what, sizeof(what), "%s: F_ADDFILESIGS_RETURN returns the code limit rounded up to a hash page", name);
	check(rc == 0 && fs.fs_file_start == expected, what);

	memset(&fs, 0, sizeof(fs));
	fs.fs_file_start = 0;
	rc = fcntl(fd, F_ADDFILESIGS, &fs);
	snprintf(what, sizeof(what), "%s: F_ADDFILESIGS succeeds and returns nothing", name);
	check(rc == 0 && fs.fs_file_start == 0, what);

	char message[256] = "";
	fchecklv_t lv = { 0, sizeof(message), message };
	snprintf(what, sizeof(what), "%s: F_CHECK_LV succeeds", name);
	check(fcntl(fd, F_CHECK_LV, &lv) == 0, what);
	close(fd);
}

static void test_not_signed(const char* path, const char* name)
{
	char what[256];
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		snprintf(what, sizeof(what), "%s: opened", name);
		check(0, what);
		return;
	}

	fsignatures_t fs;
	memset(&fs, 0, sizeof(fs));
	fs.fs_file_start = 0x1234;
	int rc = fcntl(fd, F_ADDFILESIGS_RETURN, &fs);
	snprintf(what, sizeof(what), "%s: F_ADDFILESIGS_RETURN is not refused, nothing is covered", name);
	check(rc == 0 && fs.fs_file_start == 0, what);

	char message[64] = "";
	fchecklv_t lv = { 0, sizeof(message), message };
	snprintf(what, sizeof(what), "%s: F_CHECK_LV is not refused", name);
	check(fcntl(fd, F_CHECK_LV, &lv) == 0, what);
	close(fd);
}

static void test_snapshot(void)
{
	static const char contents[] = "the bytes that were checked";
	strcpy(tmp_path, "/tmp/cs_fcntl.XXXXXX");
	int fd = mkstemp(tmp_path);
	check(fd >= 0, "snapshot: temporary file");
	if (fd < 0)
		return;

	errno = 0;
	check(fcntl(fd, DC_F_SNAPSHOT_IMAGE) == -1 && errno == EINVAL, "snapshot of an empty file: EINVAL");

	write(fd, contents, sizeof(contents));
	lseek(fd, 3, SEEK_SET);
	errno = 0;
	int snap = fcntl(fd, DC_F_SNAPSHOT_IMAGE);
	check(snap >= 0, "DC_F_SNAPSHOT_IMAGE returns a descriptor");
	if (snap < 0) {
		printf("  errno %d (%s)\n", errno, strerror(errno));
		close(fd);
		return;
	}
	check(lseek(fd, 0, SEEK_CUR) == 3, "snapshot: the source offset is unchanged");
	check(fcntl(snap, F_GETFD) == FD_CLOEXEC, "snapshot: close-on-exec");

	pwrite(fd, "CHANGED", 7, 0);

	char copy[sizeof(contents) + 8];
	memset(copy, 0, sizeof(copy));
	ssize_t got = pread(snap, copy, sizeof(copy), 0);
	check(got == (ssize_t) sizeof(contents) && memcmp(copy, contents, sizeof(contents)) == 0, "snapshot: the original contents, not the later change");

	errno = 0;
	check(pwrite(snap, "x", 1, 0) == -1, "snapshot: cannot be written");
	errno = 0;
	check(ftruncate(snap, 1) == -1, "snapshot: cannot be shrunk");
	errno = 0;
	check(ftruncate(snap, 4096) == -1, "snapshot: cannot be grown");

	close(snap);
	close(fd);
}

int main(int argc, char** argv)
{
	signal(SIGALRM, on_alarm);
	alarm(20);

	if (argc != 2) {
		fprintf(stderr, "usage: %s <codesign fixture directory>\n", argv[0]);
		return 2;
	}

	test_signed(argv[1], "adhoc", 16384);
	test_signed(argv[1], "linker-signed", 4096);

	char path[1024];
	snprintf(path, sizeof(path), "%s/unsigned", argv[1]);
	test_not_signed(path, "unsigned");
	snprintf(path, sizeof(path), "%s/hello.c", argv[1]);
	test_not_signed(path, "not a Mach-O file");

	errno = 0;
	check(fcntl(0, F_ADDFILESIGS_RETURN, NULL) == -1 && errno == EFAULT, "F_ADDFILESIGS_RETURN with NULL: EFAULT");
	errno = 0;
	check(fcntl(0, F_CHECK_LV, NULL) == -1 && errno == EFAULT, "F_CHECK_LV with NULL: EFAULT");

	test_snapshot();

	finish();
	return 0;
}
