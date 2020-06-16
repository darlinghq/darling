//
//  main.c
//  copyfile_test
//

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <removefile.h>

#include "sparse_test.h"
#include "stat_test.h"
#include "test_utils.h"

#define DISK_IMAGE_SIZE_MB	512

#if TARGET_OS_OSX
#define TEST_DIR			MOUNT_PATH
#define USING_DISK_IMAGE	1
#else
#define TEST_DIR			"/tmp/copyfile_test"
#define USING_DISK_IMAGE	0
#endif // TARGET_OS_OSX

#define MIN_BLOCKSIZE_B		512
#define DEFAULT_BLOCKSIZE_B	4096
#define MAX_BLOCKSIZE_B		16384

int main(__unused int argc, __unused const char * argv[]) {
	bool failed = false;
	struct statfs stb;

	// Create a disk image to run our tests in.
	if (USING_DISK_IMAGE) {
		disk_image_create(APFS_FSTYPE, DISK_IMAGE_SIZE_MB);
	} else {
		(void)removefile(TEST_DIR, NULL, REMOVEFILE_RECURSIVE);
		assert_no_err(mkdir(TEST_DIR, 0777));
	}

	// Make sure the test directory exists, is apfs formatted,
	// and that we have a sane block size.
	assert_no_err(statfs(TEST_DIR, &stb));
	assert_no_err(memcmp(stb.f_fstypename, APFS_FSTYPE, sizeof(APFS_FSTYPE)));
	if (stb.f_bsize < MIN_BLOCKSIZE_B || stb.f_bsize > MAX_BLOCKSIZE_B) {
		stb.f_bsize = DEFAULT_BLOCKSIZE_B;
	}

	// Run our tests.
	sranddev();
	failed |= do_sparse_test(TEST_DIR, stb.f_bsize);
	failed |= do_sparse_recursive_test(TEST_DIR, stb.f_bsize);
	failed |= do_fcopyfile_offset_test(TEST_DIR, stb.f_bsize);
	failed |= do_preserve_dst_flags_test(TEST_DIR, stb.f_bsize);
	failed |= do_preserve_dst_tracked_test(TEST_DIR, stb.f_bsize);

	// Cleanup the disk image we ran our tests on.
	if (USING_DISK_IMAGE) {
		disk_image_destroy(false);
	} else {
		(void)removefile(TEST_DIR, NULL, REMOVEFILE_RECURSIVE);
	}

	return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
