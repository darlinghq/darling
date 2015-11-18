#ifndef LINUX_PROC_INFO_H
#define LINUX_PROC_INFO_H
#include <stdint.h>

long sys_proc_info(uint32_t callnum, int32_t pid, uint32_t flavor,
		uint64_t arg, void* buffer, int32_t bufsize);

#endif

