//
//  test_utils.h
//  copyfile_test
//	Based on the test routines from the apfs project.
//

#ifndef test_utils_h
#define test_utils_h

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "../copyfile.h"

#define BSIZE_B					128
#define MAX_DISK_IMAGE_SIZE_MB	1024

#define DEFAULT_NAME_MOD  		999
#define DEFAULT_OPEN_FLAGS		O_CREAT|O_TRUNC|O_RDWR
#define DEFAULT_OPEN_PERM		0666
#define DEFAULT_MKDIR_PERM		0777

#define DISK_IMAGE_PATH			"/tmp/copyfile_sparse.sparseimage"
#define VOLUME_NAME				"apfs_sparse"
#define DEFAULT_FSTYPE			"JHFS+"
#define APFS_FSTYPE				"apfs"

// We assume that we're mounted on /Volumes.
#define MOUNT_PATH				"/Volumes/" VOLUME_NAME

#define HDIUTIL_PATH			"/usr/bin/hdiutil"
#define DIFF_PATH				"/usr/bin/diff"

// Test routine helpers.
bool verify_st_flags(struct stat *sb, uint32_t flags_to_expect);
bool verify_fd_contents(int orig_fd, off_t orig_pos, int copy_fd, off_t copy_pos, size_t length);
bool verify_copy_contents(const char *orig_name, const char *copy_name);
bool verify_copy_sizes(struct stat *orig_sb, struct stat *copy_sb, copyfile_state_t cpf_state,
					   bool do_sparse, off_t src_start);
int create_hole_in_fd(int fd, off_t offset, off_t length);
void create_test_file_name(const char *dir, const char *postfix, int id, char *string_out);

// Our disk image test functions.
void disk_image_create(const char *fstype, size_t size_in_mb);
void disk_image_destroy(bool allow_failure);

// Assertion functions/macros for tests.
static inline void
__attribute__((format (printf, 3, 4)))
__attribute__((noreturn))
assert_fail_(const char *file, int line, const char *assertion, ...)
{
	va_list args;
	va_start(args, assertion);
	char *msg;
	vasprintf(&msg, assertion, args);
	va_end(args);
	printf("\n%s:%u: error: %s\n", file, line, msg);
	exit(1);
}

#define assert_fail(str, ...)						\
	assert_fail_(__FILE__, __LINE__, str, ## __VA_ARGS__)

#undef assert
#define assert(condition)											\
	do {															\
		if (!(condition))											\
			assert_fail_(__FILE__, __LINE__,						\
						"assertion failed: %s", #condition);		\
	} while (0)

#define assert_with_errno_(condition, condition_str)					\
	do {																\
		if (!(condition))												\
			assert_fail_(__FILE__, __LINE__, "%s failed: %s",			\
						condition_str, strerror(errno));				\
	} while (0)

#define assert_with_errno(condition)			\
	assert_with_errno_((condition), #condition)

#define assert_no_err(condition) \
	assert_with_errno_(!(condition), #condition)

#define assert_equal(lhs, rhs, fmt)										\
	do {																\
		typeof (lhs) lhs_ = (lhs);										\
		typeof (lhs) rhs_ = (rhs);										\
		if (lhs_ != rhs_)												\
			assert_fail(#lhs " (" fmt ") != " #rhs " (" fmt ")",		\
						lhs_, rhs_);									\
	} while (0)

#define assert_equal_(lhs, rhs, lhs_str, rhs_str, fmt)					\
	do {																\
		typeof (lhs) lhs_ = (lhs);										\
		typeof (lhs) rhs_ = (rhs);										\
		if (lhs_ != rhs_)												\
			assert_fail(lhs_str " (" fmt ") != " rhs_str " (" fmt ")",	\
						lhs_, rhs_);									\
	} while (0)

#define assert_equal_int(lhs, rhs)	assert_equal_(lhs, rhs, #lhs, #rhs, "%d")
#define assert_equal_ll(lhs, rhs)	assert_equal_(lhs, rhs, #lhs, #rhs, "%lld")
#define assert_equal_str(lhs, rhs)										\
	do {																\
		const char *lhs_ = (lhs), *rhs_ = (rhs);						\
		if (strcmp(lhs_, rhs_))											\
			assert_fail("\"%s\" != \"%s\"", lhs_, rhs_);				\
	} while (0)

#define ignore_eintr(x, error_val)								\
	({															\
		typeof(x) eintr_ret_;									\
		do {													\
			eintr_ret_ = (x);									\
		} while (eintr_ret_ == (error_val) && errno == EINTR);	\
		eintr_ret_;												\
	})

#endif /* test_utils_h */
