#include "clonefile.h"
#include "../duct_errno.h"
#include <signal.h>

long sys_clonefileat(int src_fd, const char* src_path, int dest_fd, const char* dest_path, uint32_t flags) {
	return -ENOSYS;
};

long sys_fclonefileat(int src_fd, int dest_fd, const char* dest_path, uint32_t flags) {
	return -ENOSYS;
};
