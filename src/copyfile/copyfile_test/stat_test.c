//
//  stat_test.c
//  copyfile_test
//

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <removefile.h>
#include <sandbox/rootless.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../copyfile.h"
#include "stat_test.h"
#include "test_utils.h"

#define STORAGE_CLASS   	"copyfile_test"
#define SPECIAL_DIR_NAME	"special_dir/"
#define REGULAR_DIR_NAME	"regular_dir"
#define TEST_FILE_NAME  	"almighty_tallest"

typedef int (*special_mkdir_func)(const char *, mode_t, const char *);

static bool test_special_dir_with_flag(const char *source_directory, const char *test_directory,
	special_mkdir_func create_func, uint32_t flag_to_test) {
	char special_dir[BSIZE_B] = {0}, regular_dir[BSIZE_B] = {0}, test_file[BSIZE_B] = {0};
	struct stat sb = {0};
	bool success = true;

	// The plan here is as follows:
	//
	// (1) Create a special directory using 'create_func', and verify that its bsdflags
	// have `flag_to_test` set.
	//
	// (2) Copy the contents of `source_directory` into that special directory,
	// and verify that the directory and, optionally, a well-known file inside it
	// have `flag_to_test` set.
	//
	// (3) Copy the contents of the special directory into a directory that is not a child
	// of our special directory (a 'regular' directory), and verify that the directory and
	// a well-known file inside it do *NOT* have `flag_to_test` set.

	// Create path names.
	assert_with_errno(snprintf(special_dir, BSIZE_B, "%s/" SPECIAL_DIR_NAME, test_directory) > 0);
	assert_with_errno(snprintf(regular_dir, BSIZE_B, "%s/" REGULAR_DIR_NAME, test_directory) > 0);
	assert_with_errno(snprintf(test_file, BSIZE_B, "%s/" TEST_FILE_NAME, special_dir) > 0);

	// Create our regular directory.
	assert_no_err(mkdir(regular_dir, DEFAULT_MKDIR_PERM));

	// Create our special directory.
	assert_no_err(create_func(special_dir, DEFAULT_MKDIR_PERM, STORAGE_CLASS));

	// (1) Make sure the special directory has the specified bit set.
	assert_no_err(stat(special_dir, &sb));
	assert(sb.st_flags & flag_to_test);

	// Now, copy the source directory's into the special directory.
	assert_no_err(copyfile(source_directory, special_dir, NULL, COPYFILE_ALL|COPYFILE_RECURSIVE));

	// (2) Make sure that the resulting folder (and optionally, its well-known subfile)
	// have the specified bit set.
	assert_no_err(stat(special_dir, &sb));
	success &= verify_st_flags(&sb, flag_to_test);

	if (flag_to_test != SF_NOUNLINK) {
		assert_no_err(stat(test_file, &sb));
		success &= verify_st_flags(&sb, flag_to_test);
	}

	// Finally, copy the contents of the special directory into our regular directory.
	// Since at least one of the files in this directory will have a rootless xattr,
	// which cannot be copied here, we do not attempt to copy extended attributes here.
	assert_no_err(copyfile(special_dir, regular_dir, NULL,
		COPYFILE_DATA|COPYFILE_SECURITY|COPYFILE_RECURSIVE));

	// (3) Make sure that the regular directory (and optionally, its well-known subfile)
	// do *NOT* have the specified bit set.
	assert_no_err(stat(regular_dir, &sb));
	success &= ((sb.st_flags & flag_to_test) == 0);

	if (flag_to_test != SF_NOUNLINK) {
		// Rebuild the path to the subfile, as our original path is relative
		// to the special directory.
		memset(test_file, 0, BSIZE_B);
		assert_with_errno(snprintf(test_file, BSIZE_B, "%s/" TEST_FILE_NAME, regular_dir) > 0);

		assert_no_err(stat(test_file, &sb));
		success &= verify_st_flags(&sb, 0);
	}

	// Clean up after the test.
	assert_no_err(removefile(special_dir, NULL, REMOVEFILE_RECURSIVE));
	assert_no_err(removefile(regular_dir, NULL, REMOVEFILE_RECURSIVE));

	return success;
}

bool do_preserve_dst_flags_test(const char *test_directory, __unused size_t block_size) {
	int interior_file_src_fd, test_file_id;
	char exterior_dir_src[BSIZE_B] = {0}, interior_file_src[BSIZE_B] = {0};
	uid_t euid = geteuid();
	bool success = true;

	printf("START [preserve_dst_flags]\n");

	// Construct our source layout.
	assert_with_errno(snprintf(exterior_dir_src, BSIZE_B, "%s/" TEST_FILE_NAME, test_directory) > 0);

	assert_no_err(mkdir(exterior_dir_src, DEFAULT_MKDIR_PERM));

	test_file_id = rand() % DEFAULT_NAME_MOD;
	create_test_file_name(exterior_dir_src, TEST_FILE_NAME, test_file_id, interior_file_src);

	// Create our interior test file.
	interior_file_src_fd = open(interior_file_src, DEFAULT_OPEN_FLAGS, DEFAULT_OPEN_PERM);
	assert_with_errno(interior_file_src_fd >= 0);
	assert_no_err(close(interior_file_src_fd)); // file only needs to exist

	if (euid == 0) {
		// Try to copy our directory into a restricted environment.
		success &= test_special_dir_with_flag(exterior_dir_src, test_directory,
			rootless_mkdir_restricted, SF_RESTRICTED);

		// Make sure SF_NOUNLINK works as well.
		success &= test_special_dir_with_flag(exterior_dir_src, test_directory,
			rootless_mkdir_nounlink, SF_NOUNLINK);
	} else {
		printf("Skipping SF_RESTRICTED and SF_NOUNLINK tests, because we are not root.\n");
	}

	// Try to copy our directory into a datavault.
	success &= test_special_dir_with_flag(exterior_dir_src, test_directory,
		rootless_mkdir_datavault, UF_DATAVAULT);

	if (success) {
		printf("PASS  [preserve_dst_flags]\n");
	} else {
		printf("FAIL  [preserve_dst_flags]\n");
	}

	(void)removefile(exterior_dir_src, NULL, REMOVEFILE_RECURSIVE);

	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

bool do_preserve_dst_tracked_test(const char *test_directory, __unused size_t block_size) {
	char file_src[BSIZE_B] = {0}, file_dst[BSIZE_B] = {0};
	off_t src_fsize = 0x1000;
	int test_file_id;
	struct stat dst_stb;
	bool success = true;

	printf("START [preserve_dst_tracked]\n");

	// Create source file
	assert_with_errno(snprintf(file_src, BSIZE_B, "%s/" TEST_FILE_NAME, test_directory) > 0);
	assert_no_err(close(open(file_src, O_CREAT|O_EXCL, 0644)));
	assert_no_err(truncate(file_src, src_fsize));

	// Create destination file
	test_file_id = rand() % DEFAULT_NAME_MOD;
	assert_with_errno(snprintf(file_dst, BSIZE_B, "%s/%s.%d", test_directory, TEST_FILE_NAME, test_file_id) > 0);
	assert_no_err(close(open(file_dst, O_CREAT|O_EXCL, 0644)));

	// Track destination file
	assert_no_err(chflags(file_dst, UF_TRACKED));

	// Try to copy src onto destination
	assert_no_err(copyfile(file_src, file_dst, NULL, COPYFILE_DATA|COPYFILE_STAT|COPYFILE_PRESERVE_DST_TRACKED));

	assert_no_err(stat(file_dst, &dst_stb));
	success &= (dst_stb.st_size == src_fsize);
	success &= (dst_stb.st_flags & UF_TRACKED);
	if (success) {
		printf("PASS  [preserve_dst_tracked]\n");
	} else {
		printf("FAIL  [preserve_dst_tracked]\n");
	}

	(void)unlink(file_src);
	(void)unlink(file_dst);

	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
