#define PRIVATE 1
#include "proc_info.h"
#include "../base.h"
#include "../errno.h"
#include <linux-syscalls/linux.h>
#include <sys/errno.h>
#include <sys/proc_info.h>
#include <mach/vm_prot.h>
#include "../dirent/getdirentries.h"
#include "../ext/syslog.h"
#include "../fcntl/open.h"
#include "../unistd/close.h"
#include "../unistd/read.h"
#include "../unistd/readlink.h"
#include "../unistd/getuid.h"
#include "../unistd/getgid.h"
#include "../simple.h"
#include "../readline.h"
#include "../elfcalls_wrapper.h"
#include "../vchroot_expand.h"
#include <stdbool.h>
#include <sys/proc.h>
#include "sysctl_proc.h"
#include <stddef.h>
#include "../elfcalls_wrapper.h"
#include <darlingserver/rpc.h>
#include "../readline.h"

#define LINUX_PR_SET_NAME 15

static long _proc_pidinfo(int32_t pid, uint32_t flavor, uint64_t arg, void* buffer, int32_t bufsize);

extern __SIZE_TYPE__ strlen(const char *s);
extern void *memset(void *s, int c, __SIZE_TYPE__ n);
extern void *memcpy(void *dest, const void *src, __SIZE_TYPE__ n);
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, __SIZE_TYPE__ n);
extern int strncmp (const char * str1, const char * str2, __SIZE_TYPE__ num);
extern char *strchr(char *str, int character);

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

static bool parse_smaps_firstline(
	const char* line,
	uint64_t* pri_address,
	uint64_t* pri_size,
	uint32_t* pri_protection,
	uint32_t* pri_share_mode,
	uint64_t* pri_offset,
	uint32_t* vst_dev,
	uint64_t* vst_ino,
	char* vip_path
);
static long _proc_pidinfo_regionpathinfo(int32_t pid, uint64_t arg, void* buffer, int32_t bufsize);
static long _proc_pidinfo_shortbsdinfo(int32_t pid, void* buffer, int32_t bufsize);
static long _proc_pidonfo_uniqinfo(int32_t pid, void* buffer, int32_t bufsize);
static long _proc_pidinfo_tbsdinfo(int32_t pid, void* buffer, int32_t bufsize);
static long _proc_pidinfo_pidthreadinfo(int32_t pid, uint64_t thread_handle, void* buffer, int32_t bufsize);
static long _proc_pidinfo_pathinfo(int32_t pid, void* buffer, int32_t bufsize);
static long _proc_pidinfo_regionpath(int32_t pid, uint64_t arg, void* buffer, int32_t buffer_size);
static long _proc_pidinfo_taskinfo(int32_t pid, void* buffer, int32_t bufsize);
static long _proc_pidinfo_taskallinfo(int32_t pid, void* buffer, int32_t bufsize);
static long _proc_pidinfo_listthreads(int32_t pid, void* buffer, int32_t bufsize);

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
		case PROC_PIDTBSDINFO:
		{
			return _proc_pidinfo_tbsdinfo(pid, buffer, bufsize);
		}
		case PROC_PIDTHREADINFO:
		{
			return _proc_pidinfo_pidthreadinfo(pid, arg, buffer, bufsize);
		}
		case PROC_PIDPATHINFO:
		{
			return _proc_pidinfo_pathinfo(pid, buffer, bufsize);
		}
		case PROC_PIDREGIONPATH:
		{
			return _proc_pidinfo_regionpath(pid, arg, buffer, bufsize);
		}
		case PROC_PIDTASKINFO:
		{
			return _proc_pidinfo_taskinfo(pid, buffer, bufsize);
		}
		case PROC_PIDTASKALLINFO:
		{
			return _proc_pidinfo_taskallinfo(pid, buffer, bufsize);
		}
		case PROC_PIDLISTTHREADS:
		{
			return _proc_pidinfo_listthreads(pid, buffer, bufsize);
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

// glibc bits/confname.h
#define _SC_CLK_TCK 2

static long _proc_pidinfo_tbsdinfo(int32_t pid, void* buffer, int32_t bufsize)
{
	struct proc_bsdinfo* info = (struct proc_bsdinfo*) buffer;

	if (bufsize < sizeof(*info))
		return -ENOSPC;

	struct proc_bsdshortinfo shortinfo;
	int err = _proc_pidinfo_shortbsdinfo(pid, &shortinfo, sizeof(shortinfo));

	if (err < 0)
		return err;

	memset(info, 0, sizeof(*info));
	info->pbi_flags = shortinfo.pbsi_flags;
	info->pbi_status = shortinfo.pbsi_status;
	// info->pbi_xstatus
	info->pbi_pid = shortinfo.pbsi_pid;
	info->pbi_ppid = shortinfo.pbsi_ppid;
	info->pbi_uid = shortinfo.pbsi_uid;
	info->pbi_gid = shortinfo.pbsi_gid;
	info->pbi_ruid = shortinfo.pbsi_ruid;
	info->pbi_rgid = shortinfo.pbsi_rgid;
	info->pbi_svuid = shortinfo.pbsi_svuid;
	info->pbi_svgid = shortinfo.pbsi_svgid;
	info->pbi_pgid = shortinfo.pbsi_pgid;

	char path[64], stat[4096];
	char *statptr;
	const char* elem;

	__simple_sprintf(path, "/proc/%d/stat", pid);
	if (!read_string(path, stat, sizeof(stat)))
		return -ESRCH;

#define READELEM() elem = next_stat_elem(&statptr); if (!elem) goto reterr

	statptr = stat;
	skip_stat_elems(&statptr, 18); // skip until ppid

	READELEM();
	info->pbi_nice = elem[0] == '-' ? -__simple_atoi(elem + 1, NULL) : __simple_atoi(elem, NULL);

	skip_stat_elems(&statptr, 2); // skip until starttime
	READELEM();

	uint64_t starttime = __simple_atoi(elem, NULL);

	struct rdline_buffer rbuf;
	int statfd = sys_open("/proc/stat", BSD_O_RDONLY, 0);

	if (statfd < 0)
		return -ESRCH;

	_readline_init(&rbuf);

	uint64_t btime;
	const char* line = NULL;

	while ((line = _readline(statfd, &rbuf)) != NULL)
	{
		if (strncmp(line, "btime ", 6) == 0)
		{
			line += 6;
			btime = __simple_atoi(line, NULL);
			break;
		}
	}

	close_internal(statfd);

	// if we see `line == NULL`, we didn't break out early, so we didn't see "btime"
	if (line == NULL) {
		return -ESRCH;
	}

	long ticks_per_sec = native_sysconf(_SC_CLK_TCK);
	uint64_t starttime_secs = starttime / ticks_per_sec;
	uint64_t starttime_usecs = (starttime % ticks_per_sec) * 1000000 / ticks_per_sec;

	info->pbi_start_tvsec = btime + starttime_secs;
	info->pbi_start_tvusec = starttime_usecs;

	memcpy(info->pbi_comm, shortinfo.pbsi_comm, sizeof(info->pbi_comm));

	return sizeof(struct proc_bsdinfo);
reterr:
	return -EINVAL;
}

static long _proc_pidinfo_pidthreadinfo(int32_t pid, uint64_t thread_handle, void* buffer, int32_t bufsize)
{
	char path[64], stat[1024];
	int32_t tid = (int32_t)(thread_handle & 0xffffffff);

	// __simple_printf("Asking for pid/tid %d/%d\n", pid, tid);

	struct proc_threadinfo* info = (struct proc_threadinfo*) buffer;
	if (bufsize < sizeof(*info))
		return -ENOSPC;

	memset(buffer, 0, bufsize);

	__simple_sprintf(path, "/proc/%d/task/%d/comm", pid, tid);
	// __simple_printf("Reading thread name from %s\n", path);
	if (!read_string(path, info->pth_name, sizeof(info->pth_name)))
		return -ESRCH;
	else
	{
		// Kill LF at the end of pth_name
		int length = strlen(info->pth_name);
		if (length > 0 && info->pth_name[length-1] == '\n')
			info->pth_name[length-1] = 0;
	}

	// TODO: Other proc_threadinfo fields
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
	bool is_64_bit;
	if (dserver_rpc_task_is_64_bit(pid, &is_64_bit) < 0) {
		is_64_bit = false;
	}
	if (is_64_bit)
		info->pbsi_flags |= P_LP64;

	return 1;
reterr:
	return -EINVAL;
}

static long _proc_pidinfo_regionpath_setup(int32_t pid, struct rdline_buffer* buf) {
	char proc_path[50];
	int fd;

	__simple_sprintf(proc_path, "/proc/%d/smaps", pid);
	fd = sys_open_nocancel(proc_path, BSD_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	_readline_init(buf);
	return fd;
};

static long _proc_pidinfo_regionpathinfo(int32_t pid, uint64_t arg, void* buffer, int32_t bufsize)
{
	struct rdline_buffer buf;
	int fd;
	const char* line;
	struct proc_regionwithpathinfo my_rpi;
	bool foundRegion = false;

	if (!buffer)
		return -EFAULT;
	if (bufsize < sizeof(my_rpi))
		return -ENOSPC;

	fd = _proc_pidinfo_regionpath_setup(pid, &buf);
	if (fd < 0)
		return fd;

	while ((line = _readline(fd, &buf)) != NULL)
	{
		if (!foundRegion)
		{
			// Searching for matching memory region.
			// This call returns true if we see the start of another mapping.
			if (parse_smaps_firstline(
				line,
				&my_rpi.prp_prinfo.pri_address,
				&my_rpi.prp_prinfo.pri_size,
				&my_rpi.prp_prinfo.pri_protection,
				&my_rpi.prp_prinfo.pri_share_mode,
				&my_rpi.prp_prinfo.pri_offset,
				&my_rpi.prp_vip.vip_vi.vi_stat.vst_dev,
				&my_rpi.prp_vip.vip_vi.vi_stat.vst_ino,
				my_rpi.prp_vip.vip_path
			))
			{
				if (arg >= my_rpi.prp_prinfo.pri_address && arg < (my_rpi.prp_prinfo.pri_address + my_rpi.prp_prinfo.pri_size))
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
	close_internal(fd);

	if (my_rpi.prp_vip.vip_path[0])
	{
		// TODO: Provide information in struct vinfo_stat
	}

	memcpy(buffer, &my_rpi, sizeof(my_rpi));
	return foundRegion ? 1 : -ESRCH;
}

static long _proc_pidinfo_regionpath(int32_t pid, uint64_t arg, void* buffer, int32_t buffer_size) {
	// `_proc_pidinfo_regionpathinfo` is a glutton for stack space, due to the size of `struct proc_regioninfo`
	//
	// so rather than calling it and extracting the necessary information, it's better to have most of the common code
	// moved out and duplicate some minimal amounts of code for this function

	struct proc_regionpath my_rp;
	int fd;
	struct rdline_buffer readline_buffer;
	const char* line;
	bool found = false;

	if (!buffer)
		return -EFAULT;
	if (buffer_size < sizeof(my_rp))
		return -ENOSPC;

	fd = _proc_pidinfo_regionpath_setup(pid, &readline_buffer);
	if (fd < 0)
		return fd;

	while ((line = _readline(fd, &readline_buffer)) != NULL) {
		if (parse_smaps_firstline(
			line,
			&my_rp.prpo_addr,
			&my_rp.prpo_regionlength,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			my_rp.prpo_path
		)) {
			if (arg >= my_rp.prpo_addr && arg < (my_rp.prpo_addr + my_rp.prpo_regionlength)) {
				found = true;
				break;
			}
		}
	}

	close_internal(fd);

	memcpy(buffer, &my_rp, sizeof(my_rp));
	return found ? 0 : -EINVAL;
};

// Parses line such as:
// 5568e1914000-5568e1915000 r--p 00024000 08:01 4861625                    /usr/bin/less
static bool parse_smaps_firstline(
	const char* line,
	uint64_t* pri_address,
	uint64_t* pri_size,
	uint32_t* pri_protection,
	uint32_t* pri_share_mode,
	uint64_t* pri_offset,
	uint32_t* vst_dev,
	uint64_t* vst_ino,
	char* vip_path
)
{
	const char* p;
	const char* minus = NULL;
	uint64_t backup_variable;

	if (!pri_address)
		pri_address = &backup_variable;

	// Read address range
	p = line;
	while (*p && *p != ' ')
	{
		if (*p == '-')
		{
			*pri_address = __simple_atoi16(line, NULL);
			minus = p;
		}
		p++;
	}
	if (!minus)
		return false; // this is a different line

	if (pri_size)
		*pri_size = __simple_atoi16(minus+1, NULL) - *pri_address;
	p++;

	// Now read protection
	if (pri_protection) {
		*pri_protection = 0;
		if (*p++ == 'r')
			*pri_protection |= VM_PROT_READ;
		if (*p++ == 'w')
			*pri_protection |= VM_PROT_WRITE;
		if (*p++ == 'x')
			*pri_protection |= VM_PROT_EXECUTE;
	} else {
		p += 3;
	}

	if (pri_share_mode) {
		if (*p++ == 'p') // COW/private
			*pri_share_mode = SM_PRIVATE;
		else
			*pri_share_mode = SM_SHARED;
	} else {
		p++;
	}
	p++; // space

	// Read file offset
	if (!pri_offset)
		pri_offset = &backup_variable;
	*pri_offset = __simple_atoi16(p, &p);
	p++;

	// Read device major:minor
	int maj, min;
	maj = (int) __simple_atoi16(p, &p);
	p++;
	min = (int) __simple_atoi16(p, &p);
	p++;
	if (vst_dev)
		*vst_dev = (maj << 8) | min;

	// Read inode
	if (!vst_ino)
		vst_ino = &backup_variable;
	*vst_ino = __simple_atoi(p, &p);

	// Read file name (may be empty)
	while (*p == ' ')
		p++;
	if (*p && vip_path) // Is file name present? (and the caller wants it)
		strcpy(vip_path, p);

	return true;
}

static long _proc_pidinfo_pathinfo(int32_t pid, void* buffer, int32_t bufsize)
{
	struct vchroot_unexpand_args args;

	int rv = dserver_rpc_get_executable_path(pid, args.path, sizeof(args.path));

	if (rv < 0)
		return rv;

	rv = vchroot_unexpand(&args);
	if (rv != 0)
		return rv;

	strncpy(buffer, args.path, bufsize);
	return strlen(args.path);
}

static long _proc_pidinfo_taskinfo(int32_t pid, void* buffer, int32_t bufsize)
{
	if (!buffer)
		return -EFAULT;
	if (bufsize < sizeof(struct proc_taskinfo))
		return -ENOSPC;

	char path[64], stat[1024];
	char *statptr;
	const char* elem;

	memset(buffer, 0, bufsize);
	struct proc_taskinfo* ti = (struct proc_taskinfo*) buffer;

	__simple_sprintf(path, "/proc/%d/stat", pid);
	if (!read_string(path, stat, sizeof(stat)))
		return -ESRCH;

#define READELEM() elem = next_stat_elem(&statptr); if (!elem) goto reterr

	statptr = stat;
	skip_stat_elems(&statptr, 9); // skip until minflt
	READELEM();
	int32_t minflt = __simple_atoi(elem, NULL);

	skip_stat_elems(&statptr, 1); // skip until majflt
	READELEM();
	int32_t majflt = __simple_atoi(elem, NULL);

	skip_stat_elems(&statptr, 1); // skip until utime
	READELEM();
	uint64_t utime = __simple_atoi(elem, NULL);

	READELEM();
	uint64_t stime = __simple_atoi(elem, NULL);

	skip_stat_elems(&statptr, 2); // skip until priority
	READELEM();
	// Can be negative, according to docs.
	int32_t priority = (elem[0] == '-') ? -__simple_atoi(elem + 1, NULL) : __simple_atoi(elem, NULL);

	skip_stat_elems(&statptr, 1); // skip until num_threads
	READELEM();
	int32_t num_threads = __simple_atoi(elem, NULL);

	skip_stat_elems(&statptr, 2); // skip until vsize
	READELEM();
	uint64_t vsize = __simple_atoi(elem, NULL);

	READELEM();
	uint64_t rss = __simple_atoi(elem, NULL);

	ti->pti_virtual_size = vsize;
	ti->pti_resident_size = rss;
	ti->pti_total_user = utime;
	ti->pti_total_system = stime;
	// ti->pti_threads_user
	// ti->pti_threads_system
	// ti->pti_policy
	ti->pti_faults = minflt + majflt;
	// ti->pti_pageins
	// ti->pti_cow_faults
	// ti->pti_messages_sent
	// ti->pti_messages_received
	// ti->pti_syscalls_mach
	// ti->pti_syscalls_unix
	// ti->pti_csw
	ti->pti_threadnum = num_threads;
	// ti->pti_numrunning
	ti->pti_priority = priority;

	return sizeof(struct proc_taskinfo);
reterr:
	return -EINVAL;
}

static long _proc_pidinfo_taskallinfo(int32_t pid, void* buffer, int32_t bufsize)
{
	if (!buffer)
		return -EFAULT;
	if (bufsize < sizeof(struct proc_taskallinfo))
		return -ENOSPC;
	
	long err = _proc_pidinfo_tbsdinfo(pid, buffer + offsetof(struct proc_taskallinfo, pbsd), sizeof(struct proc_bsdinfo));
	
	if (err < 0)
		return err;

	err = _proc_pidinfo_taskinfo(pid, buffer + offsetof(struct proc_taskallinfo, ptinfo), sizeof(struct proc_taskinfo));

	if (err < 0)
		return err;
	
	return sizeof(struct proc_taskallinfo);
}

#ifndef isdigit
#	define isdigit(c) (c >= '0' && c <= '9')
#endif
#ifndef DT_DIR
#	define DT_DIR	4
#endif

static long _proc_pidinfo_listthreads(int32_t pid, void* buffer, int32_t bufsize)
{
	if (!buffer)
		return -EFAULT;
	
	uint64_t* threads = (uint64_t*)buffer;
	int32_t maxCount = bufsize / sizeof(uint64_t);
	int32_t count = 0;
	int fd, ret;
	long basep = 0;
	char dents[256];
	char path[64];

	__simple_sprintf(path, "/proc/%d/task", pid);

	fd = sys_open_nocancel(path, BSD_O_RDONLY | BSD_O_DIRECTORY, 0);
	if (fd < 0)
		return -ESRCH;

	while ((ret = sys_getdirentries(fd, (char*) dents, sizeof(dents), &basep)) > 0)
	{
		int pos = 0;
		
		while (pos < ret)
		{
			struct bsd_dirent* dent = (struct bsd_dirent*) &dents[pos];

			if (dent->d_type != DT_DIR || !isdigit(dent->d_name[0]))
			{
				pos += dent->d_reclen;
				continue;
			}

			threads[count] = __simple_atoi(dent->d_name, NULL);
			++count;

			if (count >= maxCount)
			{
				goto bail;
				return -ENOSPC;
			}

			pos += dent->d_reclen;
		}
	}

bail:
	close_internal(fd);
	return count * sizeof(uint64_t);
}
