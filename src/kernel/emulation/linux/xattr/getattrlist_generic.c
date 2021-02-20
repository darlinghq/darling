#include "../base.h"
#include "../errno.h"
#include <sys/errno.h>
#include "../common_at.h"
#include "../vchroot_expand.h"
#include "../dirent/getdirentries.h"
#include "../unistd/dup.h"
#include "../fcntl/open.h"
#include <lkm/api.h>
#include <linux-syscalls/linux.h>
#include <stddef.h>

#define ATTR_BIT_MAP_COUNT 5

#define COMMON_SUPPORTED (ATTR_CMN_FNDRINFO | ATTR_CMN_OBJTAG)
#define VOLUME_SUPPORTED 0
#define DIR_SUPPORTED (ATTR_DIR_ENTRYCOUNT)
#define FILE_SUPPORTED (ATTR_FILE_RSRCLENGTH)
#define FORK_SUPPORTED 0

#define ATTR_CMN_FNDRINFO 0x4000
#define ATTR_FILE_RSRCLENGTH 0x1000
#define ATTR_CMN_OBJTAG 0x00000010
#define ATTR_DIR_ENTRYCOUNT 0x00000002

#define XATTR_FINDER_INFO "com.apple.FinderInfo"
#define XATTR_RESOURCE_FORK "com.apple.ResourceFork"

#define FSOPT_NOFOLLOW 1
#define FSOPT_REPORT_FULLSIZE 4

#define VT_HFS 16

#define min(a,b) (((a) < (b)) ? (a) : (b))

extern void *memcpy(void *dest, const void *src, __SIZE_TYPE__ n);
extern void *memset(void *s, int c, __SIZE_TYPE__ n);
extern char *strcpy(char *dest, const char *src);

long
FUNC_NAME(int fd,

#if HAS_PATH
const char* path,
#endif

struct attrlist* alist, void *attributeBuffer, __SIZE_TYPE__ bufferSize, unsigned long options)
{
	int rv;
	char *ourBuffer, *next;
	__SIZE_TYPE__ spaceNeeded = 4; // 4 bytes for the length header

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

	if (alist->commonattr & ATTR_CMN_FNDRINFO)
		spaceNeeded += 32;
	if (alist->fileattr & ATTR_FILE_RSRCLENGTH)
		spaceNeeded += 8; // off_t
	if (alist->commonattr & ATTR_CMN_OBJTAG)
		spaceNeeded += sizeof(uint32_t); // fsobj_tag_t
	if (alist->dirattr & ATTR_DIR_ENTRYCOUNT)
		spaceNeeded += sizeof(uint32_t);

	ourBuffer = (char*) __builtin_alloca(spaceNeeded);
	next = ourBuffer + 4;

	if (alist->commonattr & ATTR_CMN_FNDRINFO)
	{
#if HAS_PATH
		rv = LINUX_SYSCALL(__NR_getxattr, vc.path, XATTR_FINDER_INFO, next, 32);
#else
		rv = LINUX_SYSCALL(__NR_fgetxattr, fd, XATTR_FINDER_INFO, next, 32);
#endif
		if (rv < 0)
			memset(next, 0, 32);
		next += 32;
	}
	if (alist->fileattr & ATTR_FILE_RSRCLENGTH)
	{
#if HAS_PATH
		rv = LINUX_SYSCALL(__NR_getxattr, vc.path, XATTR_FINDER_INFO, NULL, 0);
#else
		rv = LINUX_SYSCALL(__NR_fgetxattr, fd, XATTR_FINDER_INFO, NULL, 0);
#endif
		if (rv < 0)
			*((uint32_t*) next) = 0;
		else
			*((uint32_t*) next) = rv;
		next += 4;
	}

	if (alist->commonattr & ATTR_CMN_OBJTAG) {
		// pretend we're always on HFS
		*((uint32_t*)next) = VT_HFS;
		next += 4;
	}

	if (alist->dirattr & ATTR_DIR_ENTRYCOUNT) {
		char buf[1024]; // maybe this should be smaller?
		int tmp_fd;

		*((uint32_t*)next) = 0;

#if HAS_PATH
		tmp_fd = LINUX_SYSCALL(__NR_openat, vc.dfd, vc.path, LINUX_O_RDONLY | LINUX_O_DIRECTORY, 0);
		if (tmp_fd < 0) {
			rv = -EIO; // translate all errors into EIO (to avoid confusing our caller)
			goto attr_dir_entrycount_out_no_fd;
		}
#else
		// dupe it to avoid advancing the internal `getdents` position if our caller is using that
		tmp_fd = sys_dup(fd);
		if (tmp_fd < 0) {
			rv = -EIO; // see above
			goto attr_dir_entrycount_out_no_fd;
		}
#endif

		while (1) {
			// there's basically no other way to do this (at least none that either i or the internet know of)
			rv = LINUX_SYSCALL(__NR_getdents64, tmp_fd, buf, sizeof(buf));
			if (rv < 0) {
				rv = -EIO; // same as before: avoid confusing our caller with unexepected errnos
				goto attr_dir_entrycount_out;
			} else if (rv == 0) {
				break;
			}

			for (char* iter = buf; iter < buf + rv; iter += ((struct linux_dirent64*)iter)->d_reclen) {
				++(*((uint32_t*)next));
			}
		}

attr_dir_entrycount_out:
		close_internal(tmp_fd);
attr_dir_entrycount_out_no_fd:
		next += 4;
	}

	if (!(options & FSOPT_REPORT_FULLSIZE) && bufferSize < spaceNeeded)
		bufferSize = spaceNeeded;

	*((uint32_t*) ourBuffer) = bufferSize;

	memcpy(attributeBuffer, ourBuffer, min(bufferSize, spaceNeeded));
	
	return 0;
}

