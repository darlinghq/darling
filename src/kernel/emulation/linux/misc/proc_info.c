#define PRIVATE 1
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
#include "../unistd/read.h"
#include "../unistd/getuid.h"
#include "../unistd/getgid.h"
#include "../simple.h"
#include "../readline.h"
#include "../elfcalls_wrapper.h"
#include <stdbool.h>
#include <sys/proc.h>
#include "sysctl_proc.h"

#define LINUX_PR_SET_NAME 15

static long _proc_pidinfo(int32_t pid, uint32_t flavor, uint64_t arg, void* buffer, int32_t bufsize);

extern void *memset(void *s, int c, __SIZE_TYPE__ n);
extern void *memcpy(void *dest, const void *src, __SIZE_TYPE__ n);
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, __SIZE_TYPE__ n);

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
			if (flavor == 2 /*PROC_SELFSET_THREADNAME*/)
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
			else
				return 0; // PROC_SELFSET_PCONTROL (1)
		}
		case 8: // dirtycontrol
			return 0;
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
static long _proc_pidinfo_regionpathinfo(int32_t pid, uint64_t arg, void* buffer, int32_t bufsize);
static long _proc_pidinfo_shortbsdinfo(int32_t pid, void* buffer, int32_t bufsize);
static long _proc_pidonfo_uniqinfo(int32_t pid, void* buffer, int32_t bufsize);

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
			return _proc_pidinfo_regionpathinfo(pid, arg, buffer, bufsize);
		}
		case PROC_PIDT_SHORTBSDINFO:
		{
			return _proc_pidinfo_shortbsdinfo(pid, buffer, bufsize);
		}
		case PROC_PIDUNIQIDENTIFIERINFO:
		{
			return _proc_pidonfo_uniqinfo(pid, buffer, bufsize);
		}
		default:
		{
			__simple_printf("sys_proc_info(): Unsupported pidinfo flavor: %d\n",
					flavor);
			return -ENOTSUP;
		}
	}
}

static char fakeuuid[16];

static long _proc_pidonfo_uniqinfo(int32_t pid, void* buffer, int32_t bufsize)
{
#ifndef VARIANT_DYLD
	char path[64], stat[1024];
	char *statptr;
	const char* elem;
	unsigned long long starttime;
	int32_t ppid;

	struct proc_uniqidentifierinfo* info = (struct proc_uniqidentifierinfo*) buffer;

	if (bufsize < sizeof(*info))
		return -ENOSPC;

	memset(buffer, 0, bufsize);

#warning Get UUID from kernel
	memcpy(info->p_uuid, fakeuuid, sizeof(info->p_uuid));

	//////////////////////////
	// Read info for pid    //
	//////////////////////////

	__simple_sprintf(path, "/proc/%d/stat", pid);
	if (!read_string(path, stat, sizeof(stat)))
		return -ESRCH;

	statptr = stat;
	skip_stat_elems(&statptr, 3); // skip until ppid

	elem = next_stat_elem(&statptr);
	if (!elem)
		return -EINVAL;

	ppid = __simple_atoi(elem, NULL);

	skip_stat_elems(&statptr, 17); // skip until starttime
	elem = next_stat_elem(&statptr);

	if (!elem)
		return -EINVAL;

	starttime = __simple_atoi(elem, NULL);
	info->p_uniqueid = starttime << 16;
	info->p_uniqueid |= (pid & 0xffff);

	//////////////////////////
	// Read info for ppid   //
	//////////////////////////

	__simple_sprintf(path, "/proc/%d/stat", ppid);
	if (!read_string(path, stat, sizeof(stat)))
		return -ESRCH;

	statptr = stat;
	skip_stat_elems(&statptr, 21); // skip until starttime
	elem = next_stat_elem(&statptr);

	if (!elem)
		return -EINVAL;

	starttime = __simple_atoi(elem, NULL);
	info->p_puniqueid = starttime << 16;
	info->p_puniqueid |= (ppid & 0xffff);

#endif
	return 1;
}

static long _proc_pidinfo_shortbsdinfo(int32_t pid, void* buffer, int32_t bufsize)
{
	char path[64], stat[1024];
	char *statptr;
	const char* elem;
	static int uid = -1;
	static int gid = -1;

	struct proc_bsdshortinfo* info = (struct proc_bsdshortinfo*) buffer;
	if (bufsize < sizeof(*info))
		return -ENOSPC;

	memset(buffer, 0, bufsize);

	__simple_sprintf(path, "/proc/%d/stat", pid);
	if (!read_string(path, stat, sizeof(stat)))
		return -ESRCH;

	info->pbsi_pid = pid;

	statptr = stat;
	skip_stat_elems(&statptr, 1); // skip pid

#define READELEM() elem = next_stat_elem(&statptr); if (!elem) goto reterr

	// comm
	READELEM();
	strncpy(info->pbsi_comm, elem, sizeof(info->pbsi_comm));

	// process state
	READELEM();
	switch (*elem)
	{
		case 'R':
			info->pbsi_status = SRUN;
			break;
		case 'S':
		case 'D':
			info->pbsi_status = SSLEEP;
			break;
		case 'T':
			info->pbsi_status = SSTOP;
			break;
		case 'Z':
			info->pbsi_status = SZOMB;
			break;
		default:
			info->pbsi_status = SSLEEP;
	}

	// ppid
	READELEM();
	info->pbsi_ppid = __simple_atoi(elem, NULL);

	// pgid
	READELEM();
	info->pbsi_pgid = __simple_atoi(elem, NULL);

	if (uid == -1)
	{
		uid = sys_getuid();
		gid = sys_getgid();
	}
	info->pbsi_uid = info->pbsi_ruid = info->pbsi_svuid = uid;
	info->pbsi_gid = info->pbsi_rgid = info->pbsi_svgid = gid;

	// 32/64 bit detection
	{
		int fd;

		__simple_sprintf(path, "/proc/%d/exe", pid);
		fd = sys_open(path, BSD_O_RDONLY, 0);

		if (fd != -1)
		{
			char magic[5];
			if (sys_read(fd, magic, sizeof(magic)) == sizeof(magic))
			{
				if (magic[0] == 0x7f && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F')
				{
					if (magic[4] == 2)
					{
						info->pbsi_flags |= P_LP64;
					}
				}
			}
			sys_close(fd);
		}
	}

	return 1;
reterr:
	return -EINVAL;
}

static long _proc_pidinfo_regionpathinfo(int32_t pid, uint64_t arg, void* buffer, int32_t bufsize)
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
	return foundRegion ? 1 : -ESRCH;
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

