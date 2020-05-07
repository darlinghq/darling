#ifndef LINUX_CLONEFILE_H
#define LINUX_CLONEFILE_H

#include <stdint.h>

long sys_clonefileat(int src_fd, const char* src_path, int dest_fd, const char* dest_path, uint32_t flags);
long sys_fclonefileat(int src_fd, int dest_fd, const char* dest_path, uint32_t flags);

#endif // LINUX_CLONEFILE_H
