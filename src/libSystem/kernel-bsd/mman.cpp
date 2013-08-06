#include "mman.h"
#include <sys/mman.h>
#include <util/debug.h>
#include "../common/auto.h"

const static Darling::MappedFlag mmapFlags[] = {
	{ DARWIN_MAP_ANON, MAP_ANONYMOUS },
	{ DARWIN_MAP_FIXED, MAP_FIXED },
	{ DARWIN_MAP_PRIVATE, MAP_PRIVATE },
	{ DARWIN_MAP_SHARED, MAP_SHARED },
};

void* __darwin_mmap(void *addr, size_t len, int prot, int flags, int fd, uint64_t offset)
{
	int nativeFlags = Darling::flagsDarwinToNative(mmapFlags, sizeof(mmapFlags)/sizeof(mmapFlags[0]), flags);
	void* rv = ::mmap(addr, len, prot, nativeFlags, fd, offset);
	
	if (rv == MAP_FAILED)
		errnoOut();
	return rv;
}

MAP_FUNCTION2(int, munmap, void*, size_t);
MAP_FUNCTION3(int, mprotect, void*, size_t, int);
MAP_FUNCTION3(int, mincore, void*, size_t, unsigned char*);

int __darwin_minherit(caddr_t addr, size_t len, int inherit)
{
	STUB();
	return 0;
}

int __darwin_madvise(void *addr, size_t len, int advice)
{
	if (advice == DARWIN_MADV_FREE)
		advice = MADV_DONTNEED;
	else if (advice == DARWIN_MADV_ZERO_WIRED_PAGES)
		return 0;
	
	return AutoErrno<int>(madvise, addr, len, advice);
}

MAP_FUNCTION3(int, posix_madvise, void*, size_t, int);

MAP_FUNCTION2(int, mlock, const void*, size_t);
MAP_FUNCTION2(int, munlock, const void*, size_t);
