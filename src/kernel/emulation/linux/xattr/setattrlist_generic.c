#include "../base.h"
#include "../errno.h"
#include "../../../../../platform-include/sys/errno.h"
#include "../common_at.h"
#include "../vchroot_expand.h"
#include <lkm/api.h>
#include <linux-syscalls/linux.h>
#include <stddef.h>

extern char *strcpy(char *dest, const char *src);

#define ATTR_BIT_MAP_COUNT 5

#define ATTR_CMN_CRTIME 0x200
#define ATTR_CMN_MODTIME 0x400
#define ATTR_CMN_CHGTIME 0x800
#define ATTR_CMN_ACCTIME 0x1000
#define ATTR_CMN_ACCESSMASK 0x20000

#define COMMON_SUPPORTED (ATTR_CMN_MODTIME|ATTR_CMN_ACCTIME|ATTR_CMN_ACCESSMASK)
#define VOLUME_SUPPORTED 0
#define DIR_SUPPORTED 0
#define FILE_SUPPORTED 0
#define FORK_SUPPORTED 0

#define FSOPT_NOFOLLOW 1

#define LINUX_UTIME_OMIT	((1l << 30) - 2l)

struct timespec
{
	long tv_sec;
	long tv_nsec;
};

long
FUNC_NAME(int fd,

#if HAS_PATH
const char* path,
#endif

struct attrlist* alist, void *attributeBuffer, __SIZE_TYPE__ bufferSize, unsigned long options)
{
	int rv;
	char* next;

	if (!alist)
		return -EFAULT;

#if HAS_PATH
	if (!path)
		return -EFAULT;

	struct vchroot_expand_args vc;
	vc.flags = (options & FSOPT_NOFOLLOW) ? 0 : VCHROOT_FOLLOW;
	vc.dfd = atfd(fd);

	strcpy(vc.path, path);
	rv = vchroot_expand(&vc);
	
	if (rv < 0)
		return rv;
#endif

	if (alist->bitmapcount < ATTR_BIT_MAP_COUNT)
		return -EINVAL;

	if ((alist->commonattr & COMMON_SUPPORTED) != alist->commonattr)
		return -EINVAL;
	if ((alist->volattr & VOLUME_SUPPORTED) != alist->volattr)
		return -EINVAL;
	if ((alist->dirattr & DIR_SUPPORTED) != alist->dirattr)
		return -EINVAL;
	if ((alist->fileattr & FILE_SUPPORTED) != alist->fileattr)
		return -EINVAL;
	if ((alist->forkattr & FORK_SUPPORTED) != alist->forkattr)
		return -EINVAL;

	next = (char*) attributeBuffer;

	if (alist->commonattr & ATTR_CMN_CRTIME)
	{
		struct timespec* ts = (struct timespec*) next;
		next += sizeof(struct timespec);
	}
    if (alist->commonattr & ATTR_CMN_MODTIME)
	{
		struct timespec* ts = (struct timespec*) next;
		struct timespec times[2] = {
			{ LINUX_UTIME_OMIT, LINUX_UTIME_OMIT },
			*ts
		};
#if HAS_PATH
		rv = LINUX_SYSCALL(__NR_utimensat, LINUX_AT_FDCWD, vc.path, times, 0);
#else
		rv = LINUX_SYSCALL(__NR_utimensat, fd, 0, times, 0);
#endif
		if (rv < 0)
			return errno_linux_to_bsd(rv);
		next += sizeof(struct timespec);
	}
	if (alist->commonattr & ATTR_CMN_CHGTIME)
	{
		struct timespec* ts = (struct timespec*) next;
		next += sizeof(struct timespec);
	}
	if (alist->commonattr & ATTR_CMN_ACCTIME)
	{
		struct timespec* ts = (struct timespec*) next;
		struct timespec times[2] = {
			*ts,
			{ LINUX_UTIME_OMIT, LINUX_UTIME_OMIT }
		};
#if HAS_PATH
		rv = LINUX_SYSCALL(__NR_utimensat, LINUX_AT_FDCWD, vc.path, times, 0);
#else
		rv = LINUX_SYSCALL(__NR_utimensat, fd, 0, times, 0);
#endif
		if (rv < 0)
			return errno_linux_to_bsd(rv);
		next += sizeof(struct timespec);
	}
    if (alist->commonattr & ATTR_CMN_ACCESSMASK)
	{
		int perms = *((int*) next);
#if HAS_PATH
		rv = LINUX_SYSCALL(__NR_chmod, vc.path, perms);
#else
		rv = LINUX_SYSCALL(__NR_fchmod, fd, perms);
#endif
		if (rv < 0)
			return errno_linux_to_bsd(rv);
		next += 4;
	}

	return 0;
}
