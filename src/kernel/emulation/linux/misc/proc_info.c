#include "proc_info.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include "../../../../../platform-include/sys/errno.h"
#include "../../../../../platform-include/sys/proc_info.h"
#include "../../../../../platform-include/mach/vm_prot.h"
#include "../ext/syslog.h"
#include "../fcntl/open.h"
#include "../unistd/close.h"
#include "../simple.h"
#include "../readline.h"
#include <stdbool.h>

#define LINUX_PR_SET_NAME 15

static long _proc_pidinfo(int32_t pid, uint32_t flavor, uint64_t arg, void* buffer, int32_t bufsize);

extern void *memset(void *s, int c, __SIZE_TYPE__ n);
extern void *memcpy(void *dest, const void *src, __SIZE_TYPE__ n);
extern char *strcpy(char *dest, const char *src);

long sys_proc_info(uint32_t callnum, int32_t pid, uint32_t flavor,
		uint64_t arg, void* buffer, int32_t bufsize)
{
	switch (callnum)
	{
		case 4: // proc_kmsgbuf
		{
			int ret;

			ret = __linux_syslog(SYSLOG_ACTION_READ_ALL, (char*) buffer,
					bufsize);
			if (ret < 0)
				return errno_linux_to_bsd(ret);

			return ret;
		}
		case 5: // proc_setthreadname
		{
			// On macOS, pthread_setname_np() takes only one argument,
			// thus we can ignore pid and assume we're talking about
			// the current thread.
			int ret;
			
			ret = LINUX_SYSCALL(__NR_prctl, LINUX_PR_SET_NAME, buffer, 0UL, 0UL, 0UL);
			if (ret < 0)
				return errno_linux_to_bsd(ret);

			return ret;
		}
		case 2: // proc_pidinfo
			return _proc_pidinfo(pid, flavor, arg, buffer, bufsize);
		case 3: // proc_pidfdinfo
		case 1: // proc_listpids
		default:
			__simple_printf("sys_proc_info(): Unsupported callnum: %d\n",
					callnum);
			return -ENOTSUP;
	}
}

static bool parse_smaps_firstline(const char* line, struct proc_regioninfo* ri, struct vnode_info_path* vip);
long _proc_pidinfo(int32_t pid, uint32_t flavor, uint64_t arg, void* buffer, int32_t bufsize)
{
	switch (flavor)
	{
		// arg is a memory location we want to know about
		// We return information on the mapping where the address belongs or any following mapping
		// Then we return -ESRCH.
		// Usage example: https://github.com/fhunleth/android_external_lsof/blob/master/dialects/darwin/libproc/dproc.c
		case PROC_PIDREGIONPATHINFO:
		{
			union {
				char proc_path[50];
				struct rdline_buffer buf;
			} vars; // to reduce stack usage
			int fd;
			const char* line;
			struct proc_regionwithpathinfo my_rpi;
			bool foundRegion = false;

			if (!buffer)
				return -EFAULT;
			if (bufsize < sizeof(my_rpi))
				return -ENOSPC;

			__simple_sprintf(vars.proc_path, "/proc/%d/smaps", pid);
			fd = sys_open_nocancel(vars.proc_path, BSD_O_RDONLY, 0);
			if (fd < 0)
				return fd;

			_readline_init(&vars.buf);
			memset(&my_rpi, 0, sizeof(my_rpi));

			while ((line = _readline(fd, &vars.buf)) != NULL)
			{
				if (!foundRegion)
				{
					// Searching for matching memory region.
					// This call returns true if we see the start of another mapping.
					if (parse_smaps_firstline(line, &my_rpi.prp_prinfo, &my_rpi.prp_vip))
					{
						if (my_rpi.prp_prinfo.pri_address + my_rpi.prp_prinfo.pri_size >= arg)
						{
							foundRegion = true;
						}
					}
				}
				else
				{
					// TODO: Parse additional information (e.g. RSS)
				}
			}
			sys_close(fd);

			if (my_rpi.prp_vip.vip_path[0])
			{
				// TODO: Provide information in struct vinfo_stat
			}

			memcpy(buffer, &my_rpi, sizeof(my_rpi));
			return foundRegion ? 0 : -ESRCH;
		}
		default:
		{
			__simple_printf("sys_proc_info(): Unsupported pidinfo flavor: %d\n",
					flavor);
			return -ENOTSUP;
		}
	}
}

// Parses line such as:
// 5568e1914000-5568e1915000 r--p 00024000 08:01 4861625                    /usr/bin/less
bool parse_smaps_firstline(const char* line, struct proc_regioninfo* ri, struct vnode_info_path* vip)
{
	const char* p;
	const char* minus = NULL;

	// Read address range
	p = line;
	while (*p && *p != ' ')
	{
		if (*p == '-')
		{
			ri->pri_address = __simple_atoi16(line, NULL);
			minus = p;
		}
		p++;
	}
	if (!minus)
		return false; // this is a different line

	ri->pri_size = __simple_atoi16(minus+1, NULL) - ri->pri_address;
	p++;

	// Now read protection
	ri->pri_protection = 0;
	if (*p++ == 'r')
		ri->pri_protection |= VM_PROT_READ;
	if (*p++ == 'w')
		ri->pri_protection |= VM_PROT_WRITE;
	if (*p++ == 'x')
		ri->pri_protection |= VM_PROT_EXECUTE;

	if (*p++ == 'p') // COW/private
		ri->pri_share_mode = SM_PRIVATE;
	else
		ri->pri_share_mode = SM_SHARED;
	p++; // space

	// Read file offset
	ri->pri_offset = __simple_atoi16(p, &p);
	p++;

	// Read device major:minor
	int maj, min;
	maj = (int) __simple_atoi16(p, &p);
	p++;
	min = (int) __simple_atoi16(p, &p);
	p++;
	vip->vip_vi.vi_stat.vst_dev = (maj << 8) | min;

	// Read inode
	vip->vip_vi.vi_stat.vst_ino = __simple_atoi(p, &p);

	// Read file name (may be empty)
	while (*p == ' ')
		p++;
	if (*p) // Is file name present?
		strcpy(vip->vip_path, p);

	return true;
}

