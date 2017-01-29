#include "mman.h"
#include "duct_mman.h"
#include "../errno.h"
#include "../base.h"
#include <linux-syscalls/linux.h>

static int prot_bsd_to_linux(int prot)
{
	int linux_prot = 0;

	if (prot & PROT_READ)
		linux_prot |= LINUX_PROT_READ;
	if (prot & PROT_WRITE)
		linux_prot |= LINUX_PROT_WRITE;
	if (prot & PROT_EXEC)
		linux_prot |= LINUX_PROT_EXEC;

	return linux_prot;
}

void* sys_mmap(void* start, unsigned long len, int prot, int flags, int fd, long pos)
{
	int linux_prot;
	int linux_flags = 0;
	void* ret;

	linux_prot = prot_bsd_to_linux(prot);

	// if (flags & MAP_32BIT)
	// 	linux_flags |= LINUX_MAP_32BIT;
	if (flags & MAP_ANON)
		linux_flags |= LINUX_MAP_ANONYMOUS;
	if (flags & MAP_FIXED)
		linux_flags |= LINUX_MAP_FIXED;
	if (flags & MAP_SHARED)
		linux_flags |= LINUX_MAP_SHARED;
	if (flags & MAP_PRIVATE)
		linux_flags |= LINUX_MAP_PRIVATE;
	// if (flags & MAP_STACK)
	// 	linux_flags |= LINUX_MAP_STACK;

#ifdef __NR_mmap2
#ifdef __i386__
	linux_flags |= LINUX_MAP_32BIT;
#endif
	ret = (void*) LINUX_SYSCALL6(__NR_mmap2, start, len, linux_prot, linux_flags, fd, pos / 4096);
#else
	ret = (void*) LINUX_SYSCALL6(__NR_mmap, start, len, linux_prot, linux_flags, fd, pos);
#endif

	if ((unsigned long)ret > (unsigned long) -4096)
		ret = (void*)(long) errno_linux_to_bsd((int) ret);

	return ret;
}

long sys_munmap(void* addr, unsigned long len)
{
	int ret;

	ret = LINUX_SYSCALL2(__NR_munmap, addr, len);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

long sys_mprotect(void* addr, unsigned long len, int prot)
{
	int linux_prot, ret;

	linux_prot = prot_bsd_to_linux(prot);

	ret = LINUX_SYSCALL3(__NR_mprotect, addr, len, linux_prot);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

long sys_mincore(void* addr, unsigned long len, unsigned char* vec)
{
	int ret;

	ret = LINUX_SYSCALL3(__NR_mincore, addr, len, vec);
	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

