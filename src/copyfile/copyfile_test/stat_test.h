//
//  stat_test.h
//  copyfile_test
//

#ifndef stat_test_h
#define stat_test_h

#include <stdbool.h>
#include <stdlib.h>

bool do_preserve_dst_flags_test(const char *test_directory, size_t block_size);
bool do_preserve_dst_tracked_test(const char *test_directory, size_t block_size);

#endif /* stat_test_h */
