#include "sysctl.h"
#include "../base.h"
#include "../errno.h"
#include "../simple.h"
#include <asm/unistd.h>
#include <mach/host_info.h>
#include <mach/machine.h>
#include <mach/mach_init.h>
#include "../../../../../platform-include/sys/errno.h"
#include "sysctl_inc.h"
#include <stddef.h>
#include <limits.h>
#include "../ext/sys/utsname.h"
#include "../ext/syslog.h"
#include "getrlimit.h"
#include "darling-config.h"
#include <util/IniConfig.h>

static long sysctl_name_to_oid(const char* name, int* oid_name,
		unsigned long* oid_len);

extern char *strchr(const char *s, int c);
extern int strncmp(const char *s1, const char *s2, __SIZE_TYPE__ n);
extern int strcmp(const char *s1, const char *s2);
extern kern_return_t mach_port_deallocate(ipc_space_t task, mach_port_name_t name);
extern kern_return_t host_info(mach_port_name_t host, int itype, void* hinfo, mach_msg_type_number_t* count);

/* Darling specific */
enum {
	_HW_PHYSICAL_CPU = 1000,
	_HW_PHYSICAL_CPU_MAX,
	_HW_LOGICAL_CPU,
	_HW_LOGICAL_CPU_MAX,
	_HW_CPUTYPE,
	_HW_CPUSUBTYPE,
	_HW_CPUTHREADTYPE
};
enum {
	_KERN_MSGBUF = 1000,
};

static struct linux_utsname lu;
static iniconfig_t version_conf = NULL;
static inivalmap_t version_conf_sect = NULL;
static void copyout_string(const char* str, char* out, unsigned long* out_len);
static void need_uname(void);

long sys_sysctl(int* name, unsigned int nlen, void* old,
		unsigned long* oldlen, void* _new, unsigned long newlen)
{
	int ret;

	if (nlen < 2)
		return -EINVAL;

	if (name[0] == 0 && name[1] == 3)
	{
		return sysctl_name_to_oid((const char*) _new, (int*) old, oldlen);
	}
	
	if (name[0] == CTL_HW)
	{
		int* ovalue = (int*) old;
		struct host_basic_info hinfo;
		mach_msg_type_number_t hcount = HOST_BASIC_INFO_COUNT;
		mach_port_t hself;
		unsigned long orig_old_len = *oldlen;

		hself = mach_host_self();
		host_info(hself, HOST_BASIC_INFO, &hinfo, &hcount);
		mach_port_deallocate(mach_task_self(), hself);

		if (nlen < 2)
			return -ENOTDIR;

		// String values
		switch (name[1])
		{
			case HW_MACHINE:
			{
				need_uname();
				copyout_string(lu.machine, (char*) old, oldlen);
				return 0;
			}
		}

		// Integer values
		if (*oldlen < sizeof(int))
			return -EINVAL;
		*oldlen = sizeof(int);
		
		switch (name[1])
		{
		case HW_AVAILCPU:
		case HW_NCPU:
			*ovalue = hinfo.avail_cpus;
			return 0;
		case _HW_PHYSICAL_CPU:
			*ovalue = hinfo.physical_cpu;
			return 0;
		case _HW_PHYSICAL_CPU_MAX:
			*ovalue = hinfo.physical_cpu_max;
			return 0;
		case _HW_LOGICAL_CPU:
			*ovalue = hinfo.logical_cpu;
			return 0;
		case _HW_LOGICAL_CPU_MAX:
			*ovalue = hinfo.logical_cpu_max;
			return 0;
		case HW_MEMSIZE:
		case HW_PHYSMEM:
		case HW_USERMEM:
			if (orig_old_len == sizeof(int))
			{
				if (hinfo.max_mem < INT_MAX)
					*ovalue = hinfo.max_mem;
				else
					*ovalue = INT_MAX;
			}
			else
			{
				*oldlen = 2 * sizeof(int);
				*((unsigned long long*) ovalue) = hinfo.max_mem;
			}
			return 0;
		case HW_PAGESIZE:
			*ovalue = 4096; // True on all Darling platforms
			return 0;
		case _HW_CPUTYPE:
			*ovalue = hinfo.cpu_type;
			return 0;
		case _HW_CPUSUBTYPE:
			*ovalue = hinfo.cpu_subtype;
			return 0;
		case _HW_CPUTHREADTYPE:
			*ovalue = hinfo.cpu_threadtype;
			return 0;
		}
	}
	else if (name[0] == CTL_KERN)
	{
		switch (name[1])
		{
			case _KERN_MSGBUF:
			{
				if (*oldlen <= 8)
				{
					// Caller is asking for buffer size
					if (*oldlen < sizeof(int))
						return -EINVAL;

					*((int*) old) = __linux_syslog(SYSLOG_ACTION_SIZE_BUFFER, NULL, 0);
					return 0;
				}
				else
				{
					// Caller is asking for buffer contents
					int ret;

					ret = __linux_syslog(SYSLOG_ACTION_READ_ALL, (char*) old, *oldlen);
					if (ret < 0)
						return errno_linux_to_bsd(ret);

					*oldlen = ret;
					return 0;
				}
				break;
			}
			case KERN_PROC:
			{
				// Returns array of struct kinfo_proc
				if (nlen < 4)
					return -ENOTDIR;
				switch (name[2])
				{
					case KERN_PROC_ALL:
					case KERN_PROC_PID:
					case KERN_PROC_TTY:
					case KERN_PROC_UID:
					case KERN_PROC_PGRP:
					case KERN_PROC_SESSION:
					case KERN_PROC_RUID:
					case KERN_PROC_LCID:
					default:
						return -ENOTDIR;
				}
			}
			case KERN_ARGMAX:
			{
				struct rlimit lim;
				int r;
				int* ovalue = (int*) old;

				r = sys_getrlimit(BSD_RLIMIT_STACK, &lim);
				if (r < 0)
					return r;

				*ovalue = lim.rlim_cur / 4;
				return 0;
			}
		}

		need_uname();

		// String values
		switch (name[1])
		{
			case KERN_OSTYPE:
			{
				const char* s = NULL;
				if (version_conf_sect != NULL)
					s = iniconfig_valmap_get(version_conf_sect, "sysname");
				if (s == NULL)
					s = lu.sysname;

				copyout_string(s, (char*) old, oldlen);
				return 0;
			}
			case KERN_HOSTNAME:
				copyout_string(lu.nodename, (char*) old, oldlen);
				return 0;
			case KERN_OSRELEASE:
			{
				const char* s = NULL;
				if (version_conf_sect != NULL)
					s = iniconfig_valmap_get(version_conf_sect, "release");
				if (s == NULL)
					s = lu.release;

				copyout_string(s, (char*) old, oldlen);
				return 0;
			}
			case KERN_VERSION:
			{
				const char* s = NULL;
				if (version_conf_sect != NULL)
					s = iniconfig_valmap_get(version_conf_sect, "version");
				if (s == NULL)
					s = lu.version;

				copyout_string(s, (char*) old, oldlen);
				return 0;
			}
			case KERN_DOMAINNAME:
				copyout_string(lu.domainname, (char*) old, oldlen);
				return 0;
		}
	}

	return -ENOTDIR;
}

static void need_uname(void)
{
	// Cache __linux_uname results
	if (!lu.sysname[0])
	{
		__linux_uname(&lu);
		version_conf = iniconfig_load(ETC_DARLING_PATH "/version.conf");
		if (version_conf != NULL)
			version_conf_sect = iniconfig_getsection(version_conf, "uname");
	}
}

extern unsigned long strlcpy(char* dst, const char* src, unsigned long size);
static void copyout_string(const char* str, char* out, unsigned long* out_len)
{
	unsigned long len;
	len = strlcpy(out, str, *out_len);

	if (len < *out_len)
		*out_len = len;
}

static long sysctl_name_to_oid(const char* name, int* oid_name,
		unsigned long* oid_len)
{
	const char* dot;
	unsigned long cat_len;
	
	if (*oid_len < 2)
		return -EINVAL;

	dot = strchr(name, '.');
	if (dot == NULL)
		return -ENOTDIR;

	cat_len = dot - name;

	if (strncmp(name, "hw", cat_len) == 0)
	{
		oid_name[0] = CTL_HW;
		*oid_len = 2 * sizeof(int);
		
		if (strcmp(dot+1, "activecpu") == 0)
			oid_name[1] = HW_AVAILCPU;
		else if (strcmp(dot+1, "ncpu") == 0)
			oid_name[1] = HW_NCPU;
		else if (strcmp(dot+1, "physicalcpu") == 0)
			oid_name[1] = _HW_PHYSICAL_CPU;
		else if (strcmp(dot+1, "physicalcpu_max") == 0)
			oid_name[1] = _HW_PHYSICAL_CPU_MAX;
		else if (strcmp(dot+1, "logicalcpu") == 0)
			oid_name[1] = _HW_LOGICAL_CPU;
		else if (strcmp(dot+1, "logicalcpu_max") == 0)
			oid_name[1] = _HW_LOGICAL_CPU_MAX;
		else if (strcmp(dot+1, "memsize") == 0)
			oid_name[1] = HW_MEMSIZE;
		else if (strcmp(dot+1, "pagesize") == 0)
			oid_name[1] = HW_PAGESIZE;
		else if (strcmp(dot+1, "cputype") == 0)
			oid_name[1] = _HW_CPUTYPE;
		else if (strcmp(dot+1, "cpusubtype") == 0)
			oid_name[1] = _HW_CPUSUBTYPE;
		else if (strcmp(dot+1, "cputhreadtype") == 0)
			oid_name[1] = _HW_CPUTHREADTYPE;
		else
			return -ENOTDIR;
		
		return 0;
	}
	else if (strncmp(name, "kern", cat_len) == 0)
	{
		oid_name[0] = CTL_KERN;
		*oid_len = 2 * sizeof(int);

		if (strcmp(dot+1, "msgbuf") == 0)
			oid_name[1] = _KERN_MSGBUF;
		else
			return -ENOTDIR;

		return 0;
	}

	__simple_printf("Unknown sysctl: %s\n", name);
	return -ENOTDIR;
}

