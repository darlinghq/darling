#ifdef __cplusplus
extern "C" {
#endif

#include "removefile.h"

#include <sys/stat.h>
#include <sys/param.h>
#include <sys/mount.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

/* 
 * This structure contains variables necessary to keep track of verification,
 * the flags set by the user, as well as associated buffers and file
 * attributes.  These were defined previously by srm as global variables but a
 * structure is a much cleaner way of organizing the information and passing
 * it between functions.
 */

struct _removefile_state {
	removefile_callback_t confirm_callback;
	void * confirm_context;
	removefile_callback_t error_callback;
	void * error_context;
	int error_num;  // clear on proceed
	removefile_callback_t status_callback;
	void * status_context;

	// globals for srm
	int urand_file;
	off_t random_bytes_read;
	int file;
	off_t file_size;
	unsigned char * buffer;
	uint32_t buffsize;
	uint32_t allocated_buffsize;
	int unlink_flags;
	int cancelled;
};

int __removefile_rename_unlink(const char*path, removefile_state_t state);
int __removefile_tree_walker(char ** trees, removefile_state_t state);
int __removefile_sunlink(const char * path, removefile_state_t state);
void __removefile_init_random(const unsigned int seed, removefile_state_t state);
char __removefile_random_char(removefile_state_t state);
void __removefile_randomize_buffer(unsigned char *buffer, size_t length, removefile_state_t state);

#define __removefile_state_test_cancel(s) ((s)->cancelled != 0)

#ifdef __cplusplus
}
#endif

