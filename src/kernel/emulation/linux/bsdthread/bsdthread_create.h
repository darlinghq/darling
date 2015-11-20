#ifndef BSDTHREAD_CREATE_H
#define BSDTHREAD_CREATE_H
#include <stdint.h>

long sys_bsdthread_create(void* thread_start, void* arg, void** stack,
		void* pthread, uint32_t flags);

int darling_thread_create(void** stack, void* entry_point, uintptr_t arg3,
		uintptr_t arg4, uintptr_t arg5, uintptr_t arg6);

void* thread_stack_allocate(unsigned long stacksize);

#define LINUX_CLONE_VM      0x00000100
#define LINUX_CLONE_FS      0x00000200
#define LINUX_CLONE_FILES   0x00000400
#define LINUX_CLONE_SIGHAND 0x00000800
#define LINUX_CLONE_PTRACE  0x00002000
#define LINUX_CLONE_VFORK   0x00004000
#define LINUX_CLONE_PARENT  0x00008000
#define LINUX_CLONE_THREAD  0x00010000
#define LINUX_CLONE_NEWNS   0x00020000
#define LINUX_CLONE_SYSVSEM 0x00040000
#define LINUX_CLONE_SETTLS  0x00080000
#define LINUX_CLONE_PARENT_SETTID 0x00100000
#define LINUX_CLONE_CHILD_CLEARTID 0x00200000
#define LINUX_CLONE_DETACHED 0x00400000
#define LINUX_CLONE_UNTRACED 0x00800000
#define LINUX_CLONE_CHILD_SETTID 0x01000000

#endif

