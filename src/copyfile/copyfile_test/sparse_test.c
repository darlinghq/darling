//
//  sparse_test.c
//  copyfile_test
//

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <removefile.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

#include "../copyfile.h"
#include "sparse_test.h"
#include "test_utils.h"
#include "systemx.h"

/*
 * Copy the file pointed to by src_fd (and orig_name) to copy_name,
 * using copyfile()/fcopyfile() and COPYFILE_DATA. If do_sparse, also pass COPYFILE_DATA_SPARSE.
 * Before copying, rewind src_fd to start_off.
 */
static bool test_copy(int src_fd, char* orig_name, char* copy_name, bool do_sparse, off_t start_off) {
	struct stat orig_sb, copy_sb;
	int copy_fd;
	bool result = true;
	copyfile_state_t cpf_state;

	// Get ready for the test.
	memset(&orig_sb, 0, sizeof(orig_sb));
	memset(&copy_sb, 0, sizeof(copy_sb));
	assert_with_errno((cpf_state = copyfile_state_alloc()) != NULL);
	assert_with_errno(lseek(src_fd, start_off, SEEK_SET) == start_off);

	// First, verify copyfile().
	copyfile_flags_t flags = COPYFILE_ALL;
	if (do_sparse) {
		flags |= COPYFILE_DATA_SPARSE;
	}
	assert_no_err(copyfile(orig_name, copy_name, cpf_state, flags));

	// The file was (hopefully) copied. Now, we must verify three things:
	// 1. If (do_sparse), verify that the copy is a sparse file.
	// For now, let's approximate this by testing that the sizes of the two files are equal.
	// 2. The copyfile_state_t for the copy returns that all bytes were copied.
	// 3. The copy and the source have identical contents.

	// 1. The copy is a sparse file.
	// 2. The copyfile_state_t for the copy returns that all bytes were copied.
	assert_no_err(stat(orig_name, &orig_sb));
	assert_no_err(stat(copy_name, &copy_sb));
	result &= verify_copy_sizes(&orig_sb, &copy_sb, cpf_state, do_sparse, 0);

	// 3. The copy and the source have identical contents.
	result &= verify_copy_contents(orig_name, copy_name);

	// Post-test cleanup.
	assert_no_err(copyfile_state_free(cpf_state));
	assert_no_err(removefile(copy_name, NULL, REMOVEFILE_RECURSIVE));
	memset(&orig_sb, 0, sizeof(struct stat));
	memset(&copy_sb, 0, sizeof(struct stat));

	// Next, verify fcopyfile().
	// Make an fd for the destination.
	assert_with_errno((copy_fd = open(copy_name, DEFAULT_OPEN_FLAGS, DEFAULT_OPEN_PERM)) > 0);

	// Call fcopyfile().
	assert_with_errno((cpf_state = copyfile_state_alloc()) != NULL);
	assert_no_err(fcopyfile(src_fd, copy_fd, cpf_state, flags));

	// 1. The copy is a sparse file (if start_off is 0).
	// 2. The copyfile_state_t for the copy returns that all bytes were copied.
	assert_no_err(fstat(src_fd, &orig_sb));
	assert_no_err(fstat(copy_fd, &copy_sb));
	result &= verify_copy_sizes(&orig_sb, &copy_sb, cpf_state,
		start_off > 0 ? false : do_sparse, start_off);

	// 3. The copy and the source have identical contents.
	if (start_off == 0) {
		result &= verify_copy_contents(orig_name, copy_name);
	}

	// Post-test cleanup.
	assert_no_err(copyfile_state_free(cpf_state));
	assert_no_err(removefile(copy_name, NULL, REMOVEFILE_RECURSIVE));
	assert_no_err(close(copy_fd));

	return result;
}


// Sparse file creation functions.
// Each take the source file descriptor pointing at the beginning of the file and the block size.
// Each return the offset we should return the fd to before any copying should be performed.
typedef off_t (*creator_func)(int, off_t);

static off_t write_start_and_end_holes(int fd, off_t block_size) {
	assert_with_errno(pwrite(fd, "j", 1, block_size) == 1);
	assert_no_err(ftruncate(fd, 3 * block_size));

	assert_no_err(create_hole_in_fd(fd, 0, block_size));
	assert_no_err(create_hole_in_fd(fd, 2 * block_size, block_size));
	return 0;
}

static off_t write_end_hole(int fd, off_t block_size) {
	assert_with_errno(pwrite(fd, "n", 1, 0) == 1);
	assert_no_err(ftruncate(fd, 16 * block_size));

	assert_no_err(create_hole_in_fd(fd, block_size, 15 * block_size));
	return 0;
}

static off_t write_start_hole(int fd, off_t block_size) {
	assert_with_errno(pwrite(fd, "p", 1, 16 * block_size) == 1);

	assert_no_err(create_hole_in_fd(fd, 0, 16 * block_size));
	return 0;
}

static off_t write_middle_hole(int fd, off_t block_size) {
	assert_with_errno(pwrite(fd, "k", 1, 0) == 1);
	assert_with_errno(pwrite(fd, "k", 1, 4 * block_size) == 1);
	assert_no_err(ftruncate(fd, 5 * block_size));

	assert_no_err(create_hole_in_fd(fd, block_size, 3 * block_size));
	return 0;
}

static off_t write_start_and_middle_holes(int fd, off_t block_size) {
	assert_with_errno(pwrite(fd, "l", 1, 16 * block_size) == 1);
	assert_with_errno(pwrite(fd, "l", 1, 32 * block_size) == 1);

	assert_no_err(create_hole_in_fd(fd, 0, 16 * block_size));
	assert_no_err(create_hole_in_fd(fd, 17 * block_size, 15 * block_size));
	return 0;
}

static off_t write_middle_and_end_holes(int fd, off_t block_size) {
	assert_with_errno(pwrite(fd, "m", 1, 0) == 1);
	assert_with_errno(pwrite(fd, "m", 1, 16 * block_size) == 1);
	assert_no_err(ftruncate(fd, 32 * block_size));

	assert_no_err(create_hole_in_fd(fd, block_size, 15 * block_size));
	assert_no_err(create_hole_in_fd(fd, 17 * block_size, 15 * block_size));
	return 0;
}

static off_t write_no_sparse(int fd, __unused off_t block_size) {
	assert_with_errno(pwrite(fd, "z", 1, 0) == 1);
	return 0;
}

static off_t write_sparse_odd_offset(int fd, off_t block_size) {
	assert_with_errno(pwrite(fd, "q", 1, block_size) == 1);

	assert_no_err(create_hole_in_fd(fd, 0, block_size));
	// Return with the fd pointing at offset 1.
	assert_with_errno(lseek(fd, 1, SEEK_SET) == 1);
	return 1;
}

static off_t write_sparse_bs_offset(int fd, off_t block_size) {
	assert_with_errno(pwrite(fd, "a", 1, block_size) == 1);
	assert_with_errno(pwrite(fd, "b", 1, 2 * block_size) == 1);

	assert_no_err(create_hole_in_fd(fd, 0, block_size));
	// Return with the fd pointing at block_size.
	assert_with_errno(lseek(fd, block_size, SEEK_SET) == block_size);
	return block_size;
}

static off_t write_diff_adj_holes(int fd, off_t block_size) {
	assert_with_errno(pwrite(fd, "w", 1, 0));
	assert_with_errno(pwrite(fd, "w", 1, 3 * block_size));
	assert_no_err(ftruncate(fd, 4 * block_size));

	assert_no_err(create_hole_in_fd(fd, block_size, block_size));
	assert_no_err(create_hole_in_fd(fd, 2 * block_size, block_size));
	return 0;
}

typedef struct {
	creator_func func; // pointer to function to create a sparse file
	const char * name; // null terminated string
} sparse_test_func;

#define NUM_TEST_FUNCTIONS 10
sparse_test_func test_functions[NUM_TEST_FUNCTIONS] = {
	{write_start_and_end_holes,		"start_and_end_holes"},
	{write_middle_hole,				"middle_hole"},
	{write_start_and_middle_holes,	"start_and_middle_holes"},
	{write_middle_and_end_holes,	"middle_and_end_holes"},
	{write_end_hole,				"end_hole"},
	{write_start_hole,				"start_hole"},
	{write_no_sparse,				"no_sparse"},
	{write_sparse_odd_offset,		"write_sparse_odd_offset"},
	{write_sparse_bs_offset,		"write_sparse_bs_offset"},
	{write_diff_adj_holes,			"write_diff_adj_holes"}
};

bool do_sparse_test(const char* apfs_test_directory, size_t block_size) {
	int fd, test_file_id;
	char out_name[BSIZE_B], sparse_copy_name[BSIZE_B], full_copy_name[BSIZE_B];
	bool success = true, sub_test_success;
	off_t start_off;

	for (size_t sub_test = 0; sub_test < NUM_TEST_FUNCTIONS; sub_test++) {
		printf("START [%s]\n", test_functions[sub_test].name);
		sub_test_success = false;

		// Make new names for this file and its copies.
		test_file_id = rand() % DEFAULT_NAME_MOD;
		create_test_file_name(apfs_test_directory, "sparse", test_file_id, out_name);
		create_test_file_name(apfs_test_directory, "copy_sparse", test_file_id, sparse_copy_name);
		create_test_file_name(apfs_test_directory, "copy_full", test_file_id, full_copy_name);

		// Create the test file.
		fd = open(out_name, DEFAULT_OPEN_FLAGS, DEFAULT_OPEN_PERM);
		assert_with_errno(fd >= 0);

		// Write to the test file, making it sparse.
		start_off = test_functions[sub_test].func(fd, (off_t) block_size);
		assert_no_err(fsync(fd));

		// Make sure that a sparse copy is successful.
		sub_test_success = test_copy(fd, out_name, sparse_copy_name, true, start_off);
		if (sub_test_success) {
			// Make sure that a full copy is successful.
			sub_test_success = test_copy(fd, out_name, full_copy_name, false, start_off);
		}

		// Report the result on stdout.
		if (!sub_test_success) {
			printf("FAIL  [%s]\n", test_functions[sub_test].name);
			success = false;
		} else {
			printf("PASS  [%s]\n", test_functions[sub_test].name);
		}

		// Cleanup for the next test.
		assert_no_err(close(fd));
		(void)removefile(out_name, NULL, 0);
		(void)removefile(sparse_copy_name, NULL, 0);
		(void)removefile(full_copy_name, NULL, 0);
		memset(out_name, 0, BSIZE_B);
		memset(sparse_copy_name, 0, BSIZE_B);
		memset(full_copy_name, 0, BSIZE_B);
	}

	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

bool do_sparse_recursive_test(const char *apfs_test_directory, size_t block_size) {
	int exterior_file_src_fd, interior_file_src_fd, test_file_id;
	char exterior_dir_src[BSIZE_B] = {0}, interior_dir_src[BSIZE_B] = {0}, exterior_dir_dst[BSIZE_B] = {0}, interior_dir_dst[BSIZE_B] = {0};
	char exterior_file_src[BSIZE_B] = {0}, interior_file_src[BSIZE_B] = {0}, exterior_file_dst[BSIZE_B] = {0}, interior_file_dst[BSIZE_B] = {0};
	struct stat exterior_file_src_sb, interior_file_src_sb, exterior_file_dst_sb, interior_file_dst_sb;
	bool success = true;

	printf("START [sparse_recursive]\n");

	// Get ready for the test.
	memset(&exterior_file_src_sb, 0, sizeof(exterior_file_src_sb));
	memset(&interior_file_src_sb, 0, sizeof(interior_file_src_sb));
	memset(&exterior_file_dst_sb, 0, sizeof(exterior_file_dst_sb));
	memset(&interior_file_dst_sb, 0, sizeof(interior_file_dst_sb));

	// Construct our source layout.
	assert_with_errno(snprintf(exterior_dir_src, BSIZE_B, "%s/recursive_src", apfs_test_directory) > 0);
	assert_with_errno(snprintf(interior_dir_src, BSIZE_B, "%s/interior", exterior_dir_src) > 0);

	assert_no_err(mkdir(exterior_dir_src, DEFAULT_MKDIR_PERM));
	assert_no_err(mkdir(interior_dir_src, DEFAULT_MKDIR_PERM));

	test_file_id = rand() % DEFAULT_NAME_MOD;
	create_test_file_name(exterior_dir_src, "exterior_sparse_file", test_file_id, exterior_file_src);
	create_test_file_name(interior_dir_src, "interior_sparse_file", test_file_id, interior_file_src);

	// Create the actual test files.
	exterior_file_src_fd = open(exterior_file_src, DEFAULT_OPEN_FLAGS, DEFAULT_OPEN_PERM);
	assert_with_errno(exterior_file_src_fd >= 0);
	write_start_and_end_holes(exterior_file_src_fd, block_size);

	interior_file_src_fd = open(interior_file_src, DEFAULT_OPEN_FLAGS, DEFAULT_OPEN_PERM);
	assert_with_errno(interior_file_src_fd >= 0);
	write_middle_hole(interior_file_src_fd, block_size);

	// Now, recursively copy our folder using sparse data copying.
	assert_with_errno(snprintf(exterior_dir_dst, BSIZE_B, "%s/recursive_dst", apfs_test_directory) > 0);
	assert_no_err(copyfile(exterior_dir_src, exterior_dir_dst, NULL, COPYFILE_ALL|COPYFILE_RECURSIVE|COPYFILE_DATA_SPARSE));

	// The files were (hopefully) copied. Now, we must verify three things:
	// 1. Verify that the copy is a sparse file.
	// For now, let's approximate this by testing that the sizes of the two files are equal.
	// 2. The copy and the source have identical contents.

	// First, construct our destination layout.
	assert_with_errno(snprintf(exterior_dir_dst, BSIZE_B, "%s/recursive_dst", apfs_test_directory) > 0);
	create_test_file_name(exterior_dir_dst, "exterior_sparse_file", test_file_id, exterior_file_dst);
	assert_with_errno(snprintf(interior_dir_dst, BSIZE_B, "%s/interior", exterior_dir_dst) > 0);
	create_test_file_name(interior_dir_dst, "interior_sparse_file", test_file_id, interior_file_dst);

	// 1. The copy is a sparse file.
	assert_no_err(fstat(exterior_file_src_fd, &exterior_file_src_sb));
	assert_no_err(stat(exterior_file_dst, &exterior_file_dst_sb));

	assert_no_err(fstat(interior_file_src_fd, &interior_file_src_sb));
	assert_no_err(stat(interior_file_dst, &interior_file_dst_sb));

	success &= verify_copy_sizes(&exterior_file_src_sb, &exterior_file_dst_sb, NULL, true, 0);
	success &= verify_copy_sizes(&interior_file_src_sb, &interior_file_dst_sb, NULL, true, 0);

	// 2. The copy and the source have identical contents.
	success &= verify_copy_contents(exterior_file_src, exterior_file_dst);
	success &= verify_copy_contents(interior_file_src, interior_file_dst);

	if (success) {
		printf("PASS  [sparse_recursive]\n");
	} else {
		printf("FAIL  [sparse_recursive]\n");
	}

	assert_no_err(close(interior_file_src_fd));
	assert_no_err(close(exterior_file_src_fd));
	(void)removefile(exterior_dir_src, NULL, REMOVEFILE_RECURSIVE);
	(void)removefile(exterior_dir_dst, NULL, REMOVEFILE_RECURSIVE);

	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

bool do_fcopyfile_offset_test(const char *apfs_test_directory, size_t block_size) {
	int src_fd, sparse_copy_fd, full_copy_fd, test_file_id;
	char out_name[BSIZE_B], sparse_copy_name[BSIZE_B], full_copy_name[BSIZE_B];
	bool success = true;

	printf("START [fcopyfile_offset]\n");

	// Make new names for this file and its copies.
	test_file_id = rand() % DEFAULT_NAME_MOD;

	create_test_file_name(apfs_test_directory, "foff_sparse", test_file_id, out_name);
	create_test_file_name(apfs_test_directory, "foff_copy_sparse", test_file_id, sparse_copy_name);
	create_test_file_name(apfs_test_directory, "foff_copy_full", test_file_id, full_copy_name);

	// Create the test file.
	src_fd = open(out_name, DEFAULT_OPEN_FLAGS, DEFAULT_OPEN_PERM);
	assert_with_errno(src_fd >= 0);
	// This writes 5 * block_size bytes.
	assert_with_errno(lseek(src_fd, write_middle_hole(src_fd, block_size), SEEK_SET) == 0);

	// Create a sparse copy using fcopyfile().
	sparse_copy_fd = open(sparse_copy_name, DEFAULT_OPEN_FLAGS, DEFAULT_OPEN_PERM);
	assert_with_errno(sparse_copy_fd >= 0);

	// Seek the sparse copy to a non-zero offset.
	assert_with_errno(lseek(sparse_copy_fd, block_size, SEEK_SET) == (off_t) block_size);
	// Write into the sparse copy a different byte.
	assert_with_errno(pwrite(sparse_copy_fd, "z", 1, block_size) == 1);

	// Run fcopyfile().
	assert_no_err(fcopyfile(src_fd, sparse_copy_fd, NULL, COPYFILE_ALL|COPYFILE_DATA_SPARSE));

	// Check that the source matches the copy at the appropriate region.
	success &= verify_fd_contents(src_fd, 0, sparse_copy_fd, block_size, 4 * block_size);

	// Now, repeat the same procedure with a full copy.
	assert_with_errno(lseek(src_fd, 0, SEEK_SET) == 0);
	full_copy_fd = open(full_copy_name, DEFAULT_OPEN_FLAGS, DEFAULT_OPEN_PERM);
	assert_with_errno(full_copy_name >= 0);

	assert_with_errno(lseek(full_copy_fd, block_size, SEEK_SET) == (off_t) block_size);
	assert_with_errno(pwrite(full_copy_fd, "r", 1, block_size) == 1);

	// Run fcopyfile().
	assert_no_err(fcopyfile(src_fd, full_copy_fd, NULL, COPYFILE_ALL));

	// Check that the source matches the copy at the appropriate region.
	success &= verify_fd_contents(src_fd, 0, full_copy_fd, block_size, 4 * block_size);

	if (success) {
		printf("PASS  [fcopyfile_offset]\n");
	} else {
		printf("FAIL  [fcopyfile_offset]\n");
	}

	assert_no_err(close(full_copy_fd));
	assert_no_err(close(sparse_copy_fd));
	assert_no_err(close(src_fd));
	(void)removefile(full_copy_name, NULL, 0);
	(void)removefile(sparse_copy_name, NULL, 0);
	(void)removefile(out_name, NULL, 0);

	return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
