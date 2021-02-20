//
//  sparse_test.h
//  copyfile_test
//

#ifndef sparse_test_h
#define sparse_test_h

#include <stdbool.h>
#include <stdlib.h>

bool do_sparse_test(const char *apfs_test_directory, size_t block_size);
bool do_sparse_recursive_test(const char *apfs_test_directory, size_t block_size);
bool do_fcopyfile_offset_test(const char *apfs_test_directory, size_t block_size);

#endif /* sparse_test_h */
