#include "file_handle.h"
#include "../errno.h"
#include "../base.h"
#include "../vchroot_expand.h"
#include "../bsdthread/per_thread_wd.h"
#include "../fcntl/open.h"
#include "../simple.h"
#include "../../../../../platform-include/sys/errno.h"
#include "../unistd/close.h"
#include "../mach/lkm.h"
#include "../duct_errno.h"
#include "../unistd/access.h"
#include <linux-syscalls/linux.h>
#include <os/lock.h>

struct SavedRef
{
	char* path;
	int gen;
};

#define MOUNT_ID_SAVED (-100)
static struct SavedRef g_savedRefs[200];
static int g_nextSavedRef = 0, g_nextGen = 0;
static os_unfair_lock g_savedRefLock = OS_UNFAIR_LOCK_INIT;

void __attribute__((weak)) os_unfair_lock_unlock(os_unfair_lock_t lock) {}
void __attribute__((weak)) os_unfair_lock_lock(os_unfair_lock_t lock) {}

VISIBLE
int sys_name_to_handle(const char* name, RefData* ref, int follow)
{
	int ret;

	struct vchroot_expand_args vc;
	vc.flags = follow ? VCHROOT_FOLLOW : 0;
	vc.dfd = get_perthread_wd();

	strcpy(vc.path, name);
	ret = vchroot_expand(&vc);
	if (ret < 0)
		return errno_linux_to_bsd(ret);

	ref->fh.handle_bytes = sizeof(RefData) - offsetof(RefData, fh.f_handle);

	ret = LINUX_SYSCALL(__NR_name_to_handle_at, LINUX_AT_FDCWD, vc.path, &ref->fh, &ref->mount_id, 0);

	// This is unfortunately the case for overlayfs, which doesn't support "nfs_export" along with "metacopy"
	if (ret == -LINUX_EOPNOTSUPP && sys_access(name, 0) == 0)
	{
		os_unfair_lock_lock(&g_savedRefLock);

		if (g_savedRefs[g_nextSavedRef].path)
			free(g_savedRefs[g_nextSavedRef].path);

		ref->gen = g_nextGen++;
		g_savedRefs[g_nextSavedRef].path = strdup(name);
		g_savedRefs[g_nextSavedRef].gen = ref->gen;
		ref->mount_id = MOUNT_ID_SAVED;
		ref->index = g_nextSavedRef;

		g_nextSavedRef = (g_nextSavedRef + 1) % (sizeof(g_savedRefs) / sizeof(g_savedRefs[0]));

		os_unfair_lock_unlock(&g_savedRefLock);

		ret = 0;
	}

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

VISIBLE
int sys_handle_to_name(RefData* ref, char name[4096])
{
	if (ref->mount_id == MOUNT_ID_SAVED)
	{
		int ret = -ENOENT;
		os_unfair_lock_lock(&g_savedRefLock);

		if (g_savedRefs[ref->index].gen == ref->gen)
		{
			strlcpy(name, g_savedRefs[ref->index].path, 4096);
			ret = sys_access(name, 0);
		}

		os_unfair_lock_unlock(&g_savedRefLock);
		return ret;
	}

	// Now we need to find out the path of ref->mount_id
	struct simple_readline_buf rbuf;
	char line[1024];

	int fd_m = sys_open("/proc/self/mountinfo", BSD_O_RDONLY, 0);
	if (fd_m < 0)
		return fd_m;
	const char* mount_path = NULL;
	
	__simple_readline_init(&rbuf);

	while (__simple_readline(fd_m, &rbuf, line, sizeof(line)))
	{
		char *p, *saveptr;

		p = strtok_r(line, " ", &saveptr);
		if (p == NULL)
			continue;

		if (__simple_atoi(p, NULL) != ref->mount_id)
			continue;

		for (int i = 0; i < 4; i++)
		{
			p = strtok_r(NULL, " ", &saveptr);
			if (p == NULL)
			{
				close_internal(fd_m);
				return -ENOENT;
			}
		}

		mount_path = p;

		break;
	}

	close_internal(fd_m);

	if (mount_path == NULL)
		return -ENOENT;
	
	// We have the path of the mount, lets get a file descriptor
	fd_m = LINUX_SYSCALL(__NR_open, mount_path, LINUX_O_RDONLY, 0);
	if (fd_m < 0)
		return fd_m;

	struct handle_to_path_args args;
	args.mfd = fd_m;
	memcpy(args.fh, &ref->fh, sizeof(RefData) - offsetof(RefData, fh));

	int ret = lkm_call(NR_handle_to_path, &args);

	close_internal(fd_m);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	struct vchroot_unexpand_args uargs;
	strcpy(uargs.path, args.path);

	ret = vchroot_unexpand(&uargs);

	if (ret < 0)
		return errno_linux_to_bsd(ret);

	strcpy(name, uargs.path);

	return 0;
}

// Requires CAP_DAC_READ_SEARCH
VISIBLE
int sys_open_by_handle(RefData* ref, int flags)
{
	int linux_flags = oflags_bsd_to_linux(flags);

	if (sizeof(void*) == 4)
		linux_flags |= LINUX_O_LARGEFILE;

	// Now we need to find out the path of ref->mount_id
	struct simple_readline_buf rbuf;
	char line[1024];

	int fd_m = sys_open("/proc/self/mountinfo", BSD_O_RDONLY, 0);
	if (fd_m < 0)
		return fd_m;
	const char* mount_path = NULL;
	
	__simple_readline_init(&rbuf);

	while (__simple_readline(fd_m, &rbuf, line, sizeof(line)))
	{
		char *p, *saveptr;

		p = strtok_r(line, " ", &saveptr);
		if (p == NULL)
			continue;

		if (__simple_atoi(p, NULL) != ref->mount_id)
			continue;

		for (int i = 0; i < 4; i++)
		{
			p = strtok_r(NULL, " ", &saveptr);
			if (p == NULL)
			{
				close_internal(fd_m);
				return -ENOENT;
			}
		}

		mount_path = p;

		break;
	}

	close_internal(fd_m);

	if (mount_path == NULL)
		return -ENOENT;
	
	// We have the path of the mount, lets get a file descriptor
	fd_m = sys_open(mount_path, BSD_O_RDONLY, 0);
	if (fd_m < 0)
		return fd_m;
	
	// And now, finally, open the file by handle relative to the mount
	int ret = LINUX_SYSCALL(__NR_open_by_handle_at, fd_m, &ref->fh, linux_flags);

	close_internal(fd_m);

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);

	return ret;
}

