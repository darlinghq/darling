#ifndef MMAN_H
#define MMAN_H
#include <stddef.h>
#include <stdint.h>

extern "C" {
	
#define DARWIN_MAP_ANON 0x1000
#define DARWIN_MAP_FIXED 16
#define DARWIN_MAP_SHARED 1
#define DARWIN_MAP_PRIVATE 2
	
#define DARWIN_MADV_FREE 5
#define DARWIN_MADV_ZERO_WIRED_PAGES 6

void* __darwin_mmap(void *addr, size_t len, int prot, int flags, int fd, uint64_t offset);
int __darwin_munmap(void *addr, size_t len);

int __darwin_mprotect(void *addr, size_t len, int prot);
int __darwin_mincore(void* addr, size_t len, unsigned char *vec);
int __darwin_minherit(void* addr, size_t len, int inherit);

int __darwin_madvise(void *addr, size_t len, int advice);
int __darwin_posix_madvise(void *addr, size_t len, int advice);

int __darwin_mlock(const void *addr, size_t len);
int __darwin_munlock(const void *addr, size_t len);

}

#endif
